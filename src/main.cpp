#include <lilka.h>
#include <esp_ota_ops.h>

#include "keira/keira_splash.h"

#include "keira/servicemanager.h"
#include "keira/appmanager.h"

#include "lilka/version.h"
#include "services/watchdog/watchdog.h"
#include "services/clock/clock.h"
#include "services/network/network.h"
#include "services/screenshot/screenshot.h"
#include "services/telnet/telnet.h"
#include "services/ftp/ftp.h"
#include "services/web/web.h"
#include "apps/statusbar/statusbar.h"
#include "apps/launcher/launcher.h"
#include "keira/keira_version.h"

AppManager* appManager = AppManager::getInstance();
ServiceManager* serviceManager = ServiceManager::getInstance();

void setup() {
    // Init Hardware
    lilka::begin();

    // TODO: to be moved to KeiraSystem

    // KeiraSystem as a superclass providing Keira OS launch

    // 1. Handle commandline params, for example
    // a. -usbdrive to immediately jump into usb sharing mode
    // it would allow us to run before all Keira Services launch, avoiding possible data loss
    // while trying to access something(FTPService, WebService)

    // b. -message to return some data from external firmware backwards(need improvement in argv parsing)
    // in a form of lilka->alert

    // c. -noserial, to block serial launch at all, I find it useful to be implemented somewhere in sdk

    // 2. Register various VFS to handle (ZIPS/RARFS/PSRAMFS), last one to be registered is a ROOT
    // filesystem, providing a list of registered paths

    // 3. Launch system services

    // 4. Provide system app handlers for various filetypes, via ksystem.open() and ksystem.openwith()
    // in the end I want something like macro K_REG_APP(<AppClass name>, <list of extensions>, <path to executable in APPFS>)
    // last part is about commandline apps mostly, and minimal shell functionality to be later reused in TelnetService
    // and maybe something more encrypted in future, pass params to apps, etc.
    // Needs to change a lot to end in argc/argv app launch scheme, maybe even with returning exit code back to a system as well
    // cause why not. More similarities to how actual OSes work would be logical to do (^_^) == \~

    // Approve current OTA firmware to prevent rollback
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            lilka::serial.log("OTA firmware pending verification, marking as valid");
            esp_ota_mark_app_valid_cancel_rollback();
        }
    }

    lilka::version_t current_keira_version = KEIRA_get_curr_version();
    int current_keira_version_type = KEIRA_get_curr_version_type();

    lilka::serial.log(
        "Booting KeiraOS v%d,%d,%d, language %s",
        current_keira_version.major,
        current_keira_version.minor,
        current_keira_version.patch,
        KEIRA_get_firmware_languge().c_str()
    );
    if (current_keira_version_type == KEIRA_VERSION_TYPE_DEV) lilka::serial.log("DEV VERSION");
    if (current_keira_version_type == KEIRA_VERSION_TYPE_PRE_RELEASE) lilka::serial.log("PRE RELEASED VERSION");
    if (current_keira_version_type == KEIRA_VERSION_TYPE_RELEASE) lilka::serial.log("RELEASE VERSION");

    lilka::version_t current_sdk_version = lilka::SDK_get_curr_version();
    int current_sdk_version_type = lilka::SDK_get_curr_version_type();

    lilka::serial.log(
        "Using Lilka SDK version v%d,%d,%d",
        current_sdk_version.major,
        current_sdk_version.minor,
        current_sdk_version.patch
    );
    if (current_sdk_version_type == lilka::SDK_VERSION_TYPE) lilka::serial.log("DEV VERSION");
    if (current_sdk_version_type == lilka::SDK_VERSION_TYPE_PRE_RELEASE) lilka::serial.log("PRE RELEASED VERSION");
    if (current_sdk_version_type == lilka::SDK_VERSION_TYPE_RELEASE) lilka::serial.log("RELEASE VERSION");

    // Splash Screen
    lilka::display.setSplash(keira_splash, keira_splash_length);
    auto resetReason = lilka::multiboot.getResetReason();
    // chip always run in ESP_RST_PANIC instead of ESP_RST_POWERON
    // to speedify dev, we just do not display splash in a dev mode at all
    bool displaySplash =
        (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_PANIC) &&
        (current_sdk_version_type != lilka::SDK_VERSION_TYPE || current_keira_version_type == KEIRA_VERSION_TYPE_DEV);

    if (displaySplash) lilka::display.showStartupScreen();

#ifdef KEIRA_WATCHDOG
    serviceManager->addService(new WatchdogService());
#endif
    serviceManager->addService(new NetworkService());
    serviceManager->addService(new ClockService());
    serviceManager->addService(new ScreenshotService());
    serviceManager->addService(new TelnetService());
    serviceManager->addService(new FTPService());
    serviceManager->addService(new WebService());
    appManager->setPanel(new StatusBarApp());
    appManager->runApp(new LauncherApp());
}

void loop() {
    appManager->loop();
}
