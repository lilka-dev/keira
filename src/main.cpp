#include <lilka.h>

#include "keira/keira_splash.h"

#include "keira/servicemanager.h"
#include "keira/appmanager.h"

#include "services/watchdog/watchdog.h"
#include "services/clock/clock.h"
#include "services/network/network.h"
#include "services/screenshot/screenshot.h"
#include "services/telnet/telnet.h"
#include "services/ftp/ftp.h"
#include "services/web.h"
#include "apps/statusbar/statusbar.h"
#include "apps/launcher/launcher.h"

AppManager* appManager = AppManager::getInstance();
ServiceManager* serviceManager = ServiceManager::getInstance();

void setup() {
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
