#include "ksystem.h"

// Boot:
#include <lilka/multiboot.h>
#include <esp_ota_ops.h>

// Services:
#ifdef KEIRA_WATCHDOG
#    include "services/watchdog/watchdog.h"
#endif
#include "services/clock/clock.h"
#include "services/network/network.h"
#include "services/screenshot/screenshot.h"
#include "services/telnet/telnet.h"
#include "services/ftp/ftp.h"
#include "services/web/web.h"

// Apps:
#include "apps/statusbar/statusbar.h"
#include "apps/launcher/launcher.h"

// Resources:
#include "keira/keira_splash.h"

// Versioning:
#include <lilka/sdk.h>
#include "keira_version_auto_gen.h"

// Libs:
#include <string.h>
#include <lilka/display.h>
#include <lilka/ui.h>

//////////////////////////////////////////////////////////////////////////////
// GUIDELINE: external libraries to use <> in includes
//////////////////////////////////////////////////////////////////////////////
// Even though PlatformIO allows us to do everything wrong way, it's still
// a thing for further code reusage and visually shows local/global scopes
// of code parts
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// GUIDELINE: all includes across Keira, except ones located in a same folder should specify a full path to included file starting from a projectRoot/src
//////////////////////////////////////////////////////////////////////////////
// This would make all dependency trackage more simple, and just a good way
// to do it
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Further ideas:
//////////////////////////////////////////////////////////////////////////////
//
// Since we can't pass an app constructor directly to KeiraSystem to make a
// comprehensive app registry to easily iterate over in runtime without
// stupid if type == some enum in case of and other non linear blah blah blah
//
// We still able to provide a macro which moves our app constructor into a
// simple sexy function, whose we can then pass to a system
// for example:
//
//       AppGroup       AppType       VirtualEXE  Mime-Type          Ext
// R_APP(GEntertainment, FManagerApp, "/bin/fm", "inode/directory", "*")
//
// R_APP macro can later be defined as
// R_APP(APP_GROUP, APP_TYPE, VIRT_EXE, MIME, EXT) namespace AppInvisibleRegistry{void AppType(){ auto x= new AppType(argc, argv) }} ksystem.regApp(AppInvisibleRegistry::AppType, APP_GROUP, VIRT_EXE, MIME_EXT)
//
// or sort of.
//
// Here c++ is our Gendalf standing on a bridge not allowing us to pass,
// so we've to find a hack arround(maybe is non-much meaningfull
// namespace-shelter is an actual answer, cause somehow c++ mangles
// same names :D)
//
// While it feels almost impossible, I believe there's a better way
// to organize all that stuff, and make our system more interactive, shortcut
// makeable and multi-purposable
//
// This may simplify some other things as well like Lua interpreter
// #!/bin/lua as well as telnet commands implementations in Lua(yeah, imagine)
//
// This seems to be possible cause we can overload stdin/stdout/stderr and
// send our new born thread stdios in a shape of regualar file to our RamVFS
//
// Something similar already done in STDIO VFS implemented in sdk and seems
// works purfectly (^__^) == \~
//
// Aftewards,
//
// Feels logical to move all appManager logics into KeiraSystem
//
// But first:
//
// TODO: Move App constructors to (int argc, char **argv) scheme and provide exit code return to a system
//////////////////////////////////////////////////////////////////////////////

KeiraSystem::KeiraSystem() {
    versionType = KEIRA_VERSION_TYPE;
    // Keira version
    this->version = StringFormat(
        "KeiraOS v%d.%d.%d %s %s",
        KEIRA_VERSION_MAJOR,
        KEIRA_VERSION_MINOR,
        KEIRA_VERSION_PATCH,
        KEIRA_VERSION_TYPE_ACSTR[KEIRA_VERSION_TYPE],
        K_S_CURRENT_LANGUAGE_SHORT
    );
    // Prepare RootVFS
    // / doesn't work here. 0_0==\~ Give me control filthy bitch -_-
    this->rootVFS = new RootVFS("");

    // Add well known dirs
    rootVFS->addDir(LILKA_SD_ROOT);
    rootVFS->addDir(LILKA_SPIFFS_ROOT);

    // Time to reg it (^_^)==\~
    rootVFS->reg();
}

// TODO: CMD Params Handling
void KeiraSystem::handleCMDParams() {
#define CHECK_ARG(ARG, SHORT_ARG, NEEDLE) \
    ((ARG && strcmp(ARG, NEEDLE) == 0) || (SHORT_ARG && strcmp(SHORT_ARG, NEEDLE) == 0))
    // Retrieve cmd params
    this->argc = lilka::multiboot.getArgc();
    this->argv = lilka::multiboot.getArgv();

/*
    // Do actions based on params
    for (auto i = 0; i < argc; i++) {
        auto cLeftArgs = argc - i - 1;
        // Display message set via external/current firmware
        if (cLeftArgs && CHECK_ARG("-m", "--message", argv[i])) {
            lilka::Alert message("", argv[i + 1]);
            lilka::Canvas tmpCanvas(lilka::display.width(), lilka::display.height());
            while (!message.isFinished()) {
                message.draw(tmpCanvas);
                lilka::display.drawCanvas(tmpCanvas);
            }
            continue;
        }

    }
*/
#undef CHECK_CMD
}

// Sends greeting message to serial
void KeiraSystem::showWelcomeMessage() {
    lilka::serial.log("\nBooting %s\nBuilt on %s\n", version.c_str(), lilka::sdk.getVersionStr().c_str());
}

// Launch Services
void KeiraSystem::launchServices() {
    // TODO: implement on/off service via complete destroyment of its resources/threads
    // Prepare Service Manager
    this->serviceManager = ServiceManager::getInstance();

#ifdef KEIRA_WATCHDOG
    serviceManager->addService(new WatchdogService());
#endif
    serviceManager->addService(new NetworkService());
    serviceManager->addService(new ClockService());
    serviceManager->addService(new ScreenshotService());
    serviceManager->addService(new TelnetService());
    serviceManager->addService(new FTPService());
    serviceManager->addService(new WebService());

    // GUIDELINE: To add a new service register it here
}

// Verify OTA Update
void KeiraSystem::verifyOTA() {
    // Approve current OTA firmware to prevent rollback
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            lilka::serial.log("OTA firmware pending verification, marking as valid");
            esp_ota_mark_app_valid_cancel_rollback();
        }
    }
}

// Displays startup screen, a.k.a bootlogo/splash
void KeiraSystem::showStartupScreen() {
    lilka::display.setSplash(keira_splash, keira_splash_length);
    auto resetReason = lilka::multiboot.getResetReason();
    // chip always run in ESP_RST_PANIC instead of ESP_RST_POWERON
    // to speedify dev, we just do not display splash in a dev mode at all
    bool displaySplash =
        (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_PANIC) &&
        (!(lilka::sdk.getVersionType() == lilka::SDK_VERSION_TYPE_DEV || versionType == KEIRA_VERSION_TYPE_DEV));

    if (displaySplash) lilka::display.showStartupScreen();
}

// Prepare system to launch
void KeiraSystem::setup() {
    // Init Hardware
    lilka::begin();

    // Display splash [Should be done as fast as possible]
    // Maybe pass a black screen on display init in SDK to avoid noise display
    showStartupScreen();

    // Handle CMD Params
    handleCMDParams();

    // Verify OTA Update
    verifyOTA();

    // Launch appManager
    this->appManager = AppManager::getInstance();

    // Send greetings to serial
    showWelcomeMessage();

    // Time to launch services
    launchServices();

    // Run first apps
    appManager->setPanel(new StatusBarApp());
    appManager->runApp(new LauncherApp());
}

// TODO: To be run in separate KeiraThread[TO_IMPLEMENT]
// System tick
void KeiraSystem::loop() {
    appManager->loop();
}

KeiraSystem ksystem;
