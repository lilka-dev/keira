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
}

// TODO: CMD Params Handling
void KeiraSystem::handleCMDParams() {
    this->argc = lilka::multiboot.getArgc();
    this->argv = lilka::multiboot.getArgv();
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
