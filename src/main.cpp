#include <lilka.h>
#include <esp_ota_ops.h>

#include "keira/keira_splash.h"

#include "keira/servicemanager.h"
#include "keira/appmanager.h"

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
    lilka::serial_log("WELCOME TO LILKA");

    // Approve current OTA firmware to prevent rollback
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            lilka::serial_log("OTA firmware pending verification, marking as valid");
            esp_ota_mark_app_valid_cancel_rollback();
        }
    }

    lilka::version_t current_keira_version = KEIRA_get_curr_version();
    int current_keira_version_type = KEIRA_get_curr_version_type();

    lilka::serial_log(
        "Current Keira version is v%d,%d,%d, language %s",
        current_keira_version.major,
        current_keira_version.minor,
        current_keira_version.patch,
        KEIRA_get_firmware_languge().c_str()
    );
    if (current_keira_version_type == KEIRA_VERSION_TYPE_DEV) lilka::serial_log("DEV VERSION");
    if (current_keira_version_type == KEIRA_VERSION_TYPE_PRE_RELEASE) lilka::serial_log("PRE REALESED VERSION");
    if (current_keira_version_type == KEIRA_VERSION_TYPE_RELEASE) lilka::serial_log("REALESE VERSION");

    lilka::version_t current_sdk_version = lilka::SDK_get_curr_version();
    int current_sdk_version_type = lilka::SDK_get_curr_version_type();

    lilka::serial_log(
        "Using Lilka SDK version v%d,%d,%d",
        current_sdk_version.major,
        current_sdk_version.minor,
        current_sdk_version.patch
    );
    if (current_sdk_version_type == lilka::SDK_VERSION_TYPE) lilka::serial_log("DEV VERSION");
    if (current_sdk_version_type == lilka::SDK_VERSION_TYPE_PRE_RELEASE) lilka::serial_log("PRE REALESED VERSION");
    if (current_sdk_version_type == lilka::SDK_VERSION_TYPE_RELEASE) lilka::serial_log("REALESE VERSION");

    lilka::display.setSplash(keira_splash, keira_splash_length);
    lilka::begin();
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
