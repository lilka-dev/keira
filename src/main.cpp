#include <lilka.h>
#include <esp_system.h>

#include "keira/keira_splash.h"

#include "keira/servicemanager.h"
#include "keira/appmanager.h"

#include "services/watchdog/watchdog.h"
#include "services/clock/clock.h"
#include "services/network/network.h"
#include "services/screenshot/screenshot.h"
#include "services/telnet/telnet.h"
#include "services/ftp/ftp.h"
#include "apps/statusbar/statusbar.h"
#include "apps/launcher/launcher.h"

AppManager* appManager = AppManager::getInstance();
ServiceManager* serviceManager = ServiceManager::getInstance();

void setup() {
    lilka::display.setSplash(keira_splash, keira_splash_length);
    lilka::begin();

    // Play startup sound only on cold boot
    if (lilka::multiboot.getResetReason() == ESP_RST_POWERON) {
        lilka::audio.playStartupSound();
        lilka::display.showStartupScreen();
    } else {
        lilka::serial.log("Reset reason is %d", lilka::multiboot.getResetReason());
    }

#ifdef KEIRA_WATCHDOG
    serviceManager->addService(new WatchdogService());
#endif
    serviceManager->addService(new NetworkService());
    serviceManager->addService(new ClockService());
    serviceManager->addService(new ScreenshotService());
    serviceManager->addService(new TelnetService());
    serviceManager->addService(new FTPService());
    appManager->setPanel(new StatusBarApp());
    appManager->runApp(new LauncherApp());
}

void loop() {
    appManager->loop();
}
