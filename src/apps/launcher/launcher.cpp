#include <ff.h>
#include <FS.h>
#include "keira/keira.h"
#include "launcher.h"
#include "keira/appmanager.h"

#include "keira/servicemanager.h"
// Services:
#include "services/network/network.h"
#include "services/ftp/ftp.h"
#include "services/telnet/telnet.h"
// Demos:
#include "apps/demos/lines/lines.h"
#include "apps/demos/disk/disk.h"
#include "apps/demos/ball/ball.h"
#include "apps/demos/transform/transform.h"
#include "apps/demos/cube/cube.h"
#include "apps/demos/epilepsy/epilepsy.h"
#include "apps/demos/petpet/petpet.h"
// Tests:
#include "apps/tests/keyboard/keyboard.h"
#include "apps/tests/user_spi/user_spi.h"
#include "apps/tests/scan_i2c/scan_i2c.h"
#include "apps/tests/callbacktest/callbacktest.h"
#include "apps/tests/combo/combo.h"
// Apps
#include "apps/wificonfig/wificonfig.h"
#include "apps/letris/letris.h"
#include "apps/gpiomanager/gpiomanager.h"
#include "apps/tamagotchi/tamagotchi.h"
#include "apps/lua/luarunner.h"
#include "apps/mjs/mjsrunner.h"
#include "apps/nes/nesapp.h"
#include "apps/weather/weather.h"
#include "apps/modplayer/modplayer.h"
#include "apps/lilcatalog/lilcatalog.h"
#include "apps/liltracker/liltracker.h"
#include "apps/fmanager/fmanager.h"
#include "apps/pastebin/pastebinApp.h"

#include "apps/soundsettings/sound.h"

#include "apps/icons/demos.h"
#include "apps/icons/sdcard.h"
#include "apps/icons/memory.h"
#include "apps/icons/dev.h"
#include "apps/icons/settings.h"
#include "apps/icons/info.h"
#include "apps/icons/app_group.h"

#include <WiFi.h> // for setWiFiTxPower
#include <Preferences.h>
#include <lilka/spi.h>

LauncherApp::LauncherApp() : App("Launcher") {
    networkService = static_cast<NetworkService*>(ServiceManager::getInstance()->getService<NetworkService>("network"));
    setStackSize(8192); // Yeah, this one is heavy as fuck
}

void LauncherApp::run() {
#ifdef KEIRA_DEBUG_APP
#    ifdef KEIRA_DEBUG_APP_PARAMS
    this->runApp<KEIRA_DEBUG_APP>(KEIRA_DEBUG_APP_PARAMS);
#    else
    this->runApp<KEIRA_DEBUG_APP>();
#    endif
#endif
    for (lilka::Button button : {lilka::Button::UP, lilka::Button::DOWN, lilka::Button::LEFT, lilka::Button::RIGHT}) {
        lilka::controller.setAutoRepeat(button, 10, 300);
    }

    item_t root_item = ITEM::SUBMENU(
        K_S_LAUNCHER_MAIN_MENU,
        {
            ITEM::SUBMENU(
                K_S_LAUNCHER_APPS,
                {
                    ITEM::SUBMENU(
                        K_S_LAUNCHER_DEMOS,
                        {
                            ITEM::APP(K_S_LAUNCHER_LINES, [this]() { this->runApp<DemoLines>(); }),
                            ITEM::APP(K_S_LAUNCHER_DISK, [this]() { this->runApp<DiskApp>(); }),
                            ITEM::APP(K_S_LAUNCHER_TRANSFORM, [this]() { this->runApp<TransformApp>(); }),
                            ITEM::APP(K_S_LAUNCHER_BALL, [this]() { this->runApp<BallApp>(); }),
                            ITEM::APP(K_S_LAUNCHER_CUBE, [this]() { this->runApp<CubeApp>(); }),
                            ITEM::APP(K_S_LAUNCHER_EPILEPSY, [this]() { this->runApp<EpilepsyApp>(); }),
                            ITEM::APP(K_S_LAUNCHER_PET_PET, [this]() { this->runApp<PetPetApp>(); }),
                        },
                        &app_group_img, lilka::colors::White
                    ),
                    ITEM::SUBMENU(
                        K_S_LAUNCHER_TESTS,
                        {
                            ITEM::APP(K_S_LAUNCHER_KEYBOARD, [this]() { this->runApp<KeyboardApp>(); }),
                            ITEM::APP(K_S_LAUNCHER_SPI_TEST, [this]() { this->runApp<UserSPIApp>(); }),
                            ITEM::APP(K_S_LAUNCHER_I2C_SCANNER, [this]() { this->runApp<ScanI2CApp>(); }),
                            ITEM::APP(K_S_LAUNCHER_COMBO, [this]() { this->runApp<ComboApp>(); }),
                            ITEM::APP(K_S_LAUNCHER_CALLBACK_TEST, [this]() { this->runApp<CallBackTestApp>(); }),
                        },
                        &app_group_img, lilka::colors::White
                    ),
                    ITEM::APP(K_S_LAUNCHER_LILCATALOG, [this]() { this->runApp<LilCatalogApp>(); }),
                    ITEM::APP(K_S_LAUNCHER_LILTRACKER, [this]() { this->runApp<LilTrackerApp>(); }),
                    ITEM::APP(K_S_LAUNCHER_LETRIS, [this]() { this->runApp<LetrisApp>(); }),
                    ITEM::APP(K_S_LAUNCHER_TAMAGOTCHI, [this]() { this->runApp<TamagotchiApp>(); }),
                    ITEM::APP(K_S_LAUNCHER_WEATHER, [this]() { this->runApp<WeatherApp>(); }),
                    ITEM::APP(K_S_LAUNCHER_PASTEBIN, [this]() { this->runApp<pastebinApp>(); }),
                    ITEM::APP(K_S_LAUNCHER_GPIO_MANAGER, [this]() { this->runApp<GPIOManagerApp>(); }),
                },
                &demos_img,
                lilka::colors::Pink
            ),
            ITEM::APP(
                K_S_LAUNCHER_SD_BROWSER,
                [this]() { this->runApp<FileManagerApp>(LILKA_SD_ROOT); },
                &sdcard_img,
                lilka::colors::Arylide_yellow
            ),
            ITEM::APP(
                K_S_LAUNCHER_SPIFFS_BROWSER,
                [this]() { this->runApp<FileManagerApp>(LILKA_SPIFFS_ROOT); },
                &memory_img,
                lilka::colors::Dark_sea_green
            ),
            ITEM::SUBMENU(
                K_S_LAUNCHER_DEV_MENU,
                {
                    ITEM::APP(K_S_LAUNCHER_LIVE_LUA, [this]() { this->runApp<LuaLiveRunnerApp>(); }),
                    ITEM::APP(K_S_LAUNCHER_LUA_REPL, [this]() { this->runApp<LuaReplApp>(); }),
                },
                &dev_img,
                lilka::colors::Jasmine
            ),
            ITEM::SUBMENU(
                K_S_SETTINGS,
                {
                    ITEM::MENU(
                        K_S_LAUNCHER_WIFI_ADAPTER,
                        [this]() { this->wifiToggle(); },
                        nullptr,
                        lilka::colors::White,
                        [this](void* item) {
                            lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                            menuItem->postfix = networkService->getEnabled() ? K_S_ON : K_S_OFF;
                        }
                    ),
                    ITEM::MENU(K_S_LAUNCHER_WIFI_NETWORKS, [this]() { this->wifiManager(); }),
                    ITEM::MENU(K_S_LAUNCHER_WIFI_TX_POWER, [this]() { this->setWiFiTxPower(); }),
                    ITEM::MENU(K_S_LAUNCHER_SPI_SD_SPEED, [this]() { this->setSpiSDSpeed(); }),
                    ITEM::MENU(K_S_LAUNCHER_SOUND, [this]() { this->runApp<SoundConfigApp>(); }),
                    ITEM::SUBMENU(K_S_LAUNCHER_SERVICES, {
                        ITEM::SUBMENU(K_S_LAUNCHER_TELNET, {
                            ITEM::MENU(
                                K_S_STATUS,
                                [this]() {
                                            TelnetService* telnetService = static_cast<TelnetService*>(
                                                ServiceManager::getInstance()->getService<TelnetService>("telnet")
                                            );
                                            telnetService->setEnabled(!telnetService->getEnabled());
                                },
                                nullptr,
                                lilka::colors::White,
                                [this](void* item) {
                                            lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                                            TelnetService* telnetService = static_cast<TelnetService*>(
                                                ServiceManager::getInstance()->getService<TelnetService>("telnet")
                                            );
                                            menuItem->postfix = telnetService->getEnabled() ? K_S_ON : K_S_OFF;
                                }
                            ),
                    }),
                        ITEM::SUBMENU(K_S_LAUNCHER_FTP, {
                            ITEM::MENU(
                                K_S_STATUS,
                                [this]() {
                                            FTPService* ftpService = static_cast<FTPService*>(
                                                ServiceManager::getInstance()->getService<FTPService>("ftp")
                                            );
                                            ftpService->setEnabled(!ftpService->getEnabled());
                                },
                                nullptr,
                                lilka::colors::White,
                                [this](void* item) {
                                            lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                                            FTPService* ftpService = static_cast<FTPService*>(
                                                ServiceManager::getInstance()->getService<FTPService>("ftp")
                                            );
                                            menuItem->postfix = ftpService->getEnabled() ? K_S_ON : K_S_OFF;
                                }
                            ),
                            ITEM::MENU(
                                K_S_LAUNCHER_FTP_USER,
                                nullptr,
                                nullptr,
                                lilka::colors::White,
                                [this](void* item) {
                                            lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                                            FTPService* ftpService = static_cast<FTPService*>(
                                                ServiceManager::getInstance()->getService<FTPService>("ftp")
                                            );
                                            menuItem->postfix = ftpService->getUser();
                                }
                            ),
                            ITEM::MENU(
                                K_S_LAUNCHER_FTP_PASSWORD,
                                [this]() {
                                            FTPService* ftpService = static_cast<FTPService*>(
                                                ServiceManager::getInstance()->getService<FTPService>("ftp")
                                            );
                                            ftpService->createPassword();
                                },
                                nullptr,
                                lilka::colors::White,
                                [this](void* item) {
                                            lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                                            FTPService* ftpService = static_cast<FTPService*>(
                                                ServiceManager::getInstance()->getService<FTPService>("ftp")
                                            );
                                            menuItem->postfix = ftpService->getPassword();
                                }
                            ),
                        }),
                    }),
                    ITEM::MENU(K_S_LAUNCHER_ABOUT_SYSTEM, [this]() { this->about(); }),
                    ITEM::MENU(K_S_LAUNCHER_DEVICE_INFO, [this]() { this->info(); }),
                    ITEM::MENU(K_S_PARTITION_TABLE, [this]() { this->partitions(); }),
                    ITEM::MENU(K_S_LAUNCHER_SD_FORMAT, [this]() { this->formatSD(); }),
                    ITEM::MENU(K_S_LAUNCHER_LIGHT_SLEEP, []() {
                            lilka::board.enablePowerSavingMode();
                            esp_light_sleep_start();
                    }),
                    ITEM::MENU(K_S_LAUNCHER_DEEP_SLEEP, []() {
                            lilka::board.enablePowerSavingMode();
                            esp_deep_sleep_start();
                     }),
                    ITEM::MENU(K_S_LAUNCHER_REBOOT, []() { esp_restart();
                     }),
                },
                &settings_img,
                lilka::colors::Orchid
            ),
}
    );
    showMenu(root_item.name, root_item.submenu, false);
}
void LauncherApp::showMenu(const char* title, ITEM_LIST& list, bool back) {
    int itemCount = list.size();
    lilka::Menu menu(title);
    for (int i = 0; i < list.size(); i++) {
        menu.addItem(list[i].name, list[i].icon, list[i].color);
    }
    if (back) {
        menu.addActivationButton(K_BTN_BACK);
        menu.addItem(K_S_MENU_BACK);
    }
    while (1) {
        while (!menu.isFinished()) {
            for (int i = 0; i < list.size(); i++) {
                if (list[i].update != nullptr) {
                    lilka::MenuItem menuItem;
                    menu.getItem(i, &menuItem);
                    list[i].update(&menuItem);
                    menu.setItem(i, menuItem.title, menuItem.icon, menuItem.color, menuItem.postfix);
                }
            }
            menu.update();
            menu.draw(canvas);
            queueDraw();
        }
        if (menu.getButton() == K_BTN_BACK) {
            break;
        }
        int16_t index = menu.getCursor();
        if (back && index == itemCount) {
            break;
        }

        item_t item = list[index];
        if (item.callback != nullptr) {
            item.callback();
        }
        if (!item.submenu.empty()) {
            showMenu(item.name, item.submenu);
        }
    }
}
void LauncherApp::alert(String title, String message) {
    lilka::Alert alert(title, message);
    alert.draw(canvas);
    queueDraw();
    while (!alert.isFinished()) {
        alert.update();
        taskYIELD();
    }
}
template <typename T, typename... Args>
void LauncherApp::runApp(Args&&... args) {
    AppManager::getInstance()->runApp(new T(std::forward<Args>(args)...));
}
void LauncherApp::setWiFiTxPower() {
    String names[] = {
        "19.5 dBm", "19 dBm", "18.5 dBm", "17 dBm", "15 dBm", "13 dBm", "11 dBm", "8.5 dBm", "7 dBm", "2 dBm", "-1 dBm"
    };
    wifi_power_t values[] = {
        WIFI_POWER_19_5dBm,
        WIFI_POWER_19dBm,
        WIFI_POWER_18_5dBm,
        WIFI_POWER_17dBm,
        WIFI_POWER_15dBm,
        WIFI_POWER_13dBm,
        WIFI_POWER_11dBm,
        WIFI_POWER_8_5dBm,
        WIFI_POWER_7dBm,
        WIFI_POWER_5dBm,
        WIFI_POWER_2dBm,
        WIFI_POWER_MINUS_1dBm
    };
    lilka::Menu wifiSetTxMenu;
    wifiSetTxMenu.setTitle(K_S_LAUNCHER_SELECT_TX_POWER);
    wifiSetTxMenu.addActivationButton(K_BTN_BACK); // Exit
    // Add names
    for (auto i = 0; i < sizeof(names) / sizeof(names[0]); i++)
        wifiSetTxMenu.addItem(names[i]);
    // Perform draw
    while (!wifiSetTxMenu.isFinished()) {
        wifiSetTxMenu.update();
        wifiSetTxMenu.draw(canvas);
        queueDraw();
    }
    auto button = wifiSetTxMenu.getButton();

    if (button == K_BTN_BACK) return;

    auto index = wifiSetTxMenu.getCursor();

    // Set power immediately
    WiFi.setTxPower(values[index]);

    // Save value to NVS
    Preferences prefs;
    prefs.begin(WIFI_KEIRA_NAMESPACE, false);
    prefs.putInt("txPower", static_cast<int>(values[index]));
    prefs.end();
}

void LauncherApp::setSpiSDSpeed() {
    uint32_t sdFrequencies[] = {
        4000000, // 4  MHz
        16000000, // 16 MHz
        20000000, // 20 MHz
        40000000, // 40 MHz
        80000000 // 80 MHz
    };

    lilka::Menu setSpiSDSpeedMenu;
    setSpiSDSpeedMenu.setTitle(K_S_LAUNCHER_SPI_SD_SPEED);
    setSpiSDSpeedMenu.addActivationButton(K_BTN_BACK); // Exit

    // Add frequencies to menu
    for (auto i = 0; i < sizeof(sdFrequencies) / sizeof(sdFrequencies[0]); i++)
        setSpiSDSpeedMenu.addItem(StringFormat("%d MHz", sdFrequencies[i] / 1000000));

    // Perform draw
    while (!setSpiSDSpeedMenu.isFinished()) {
        setSpiSDSpeedMenu.update();
        setSpiSDSpeedMenu.draw(canvas);
        queueDraw();
    }
    auto button = setSpiSDSpeedMenu.getButton();

    if (button == K_BTN_BACK) return;

    auto index = setSpiSDSpeedMenu.getCursor();

    // store new frequency to NVS
    Preferences prefs;
    prefs.begin(LILKA_SPI_NVS_NAMESPACE, false);
    uint32_t sdFrequency = sdFrequencies[index];
    prefs.putUInt(LILKA_SPI_NVS_SD_FREQUENCY_KEY, sdFrequency);
    prefs.end();

    alert("", K_S_CHANGE_ON_NEXT_BOOT);
}

void LauncherApp::wifiToggle() {
    networkService->setEnabled(!networkService->getEnabled());
}
void LauncherApp::wifiManager() {
    if (!networkService->getEnabled()) {
        alert(K_S_ERROR, K_S_LAUNCHER_ENABLE_WIFI_FIRST);
        return;
    }
    AppManager::getInstance()->runApp(new WiFiConfigApp());
}
void LauncherApp::about() {
    alert(K_S_OS_NAME, K_S_OS_DESCRIPTION);
}
void LauncherApp::info() {
    alert(
        K_S_LAUNCHER_DEVICE_INFO,
        StringFormat(
            K_S_LAUNCHER_DEVICE_INFO_FMT,
            ESP.getChipModel(),
            ESP.getChipRevision(),
            esp_get_idf_version(),
            ESP.getCpuFreqMHz(),
            ESP.getChipCores(),
            networkService->getIpAddr().c_str()
        )
    );
}
void LauncherApp::partitions() {
    // TODO : support more than 16 partitions
    String names[16];
    int partitionCount = lilka::sys.get_partition_labels(names);

    ITEM_LIST partitionsMenu;
    for (int i = 0; i < partitionCount; i++) {
        String partition = names[i];
        partitionsMenu.push_back(ITEM::MENU(names[i].c_str(), [this, partition]() {
            alert(
                partition,
                StringFormat(
                    K_S_LAUNCHER_PARTITION_FMT,
                    String(lilka::sys.get_partition_address(partition.c_str()), HEX).c_str(),
                    String(lilka::sys.get_partition_size(partition.c_str()), HEX).c_str()
                )
            );
        }));
    }
    showMenu(K_S_PARTITION_TABLE, partitionsMenu);
}
void LauncherApp::formatSD() {
    lilka::Alert confirm(K_S_LAUNCHER_FORMAT, K_S_LAUNCHER_FORMAT_DISCLAIMER_ALERT);
    confirm.addActivationButton(K_BTN_CONFIRM);
    confirm.draw(canvas);
    queueDraw();
    while (!confirm.isFinished()) {
        confirm.update();
        taskYIELD();
    }
    if (confirm.getButton() != K_BTN_CONFIRM) {
        return;
    }

    lilka::ProgressDialog dialog(K_S_LAUNCHER_FORMAT, K_S_LAUNCHER_PLEASE_STANDBY);
    dialog.draw(canvas);
    queueDraw();
    if (!lilka::fileutils.createSDPartTable()) {
        alert(K_S_ERROR, K_S_LAUNCHER_FORMAT_ERROR_ALERT);
        esp_restart();
    }
    if (!lilka::fileutils.formatSD()) {
        this->alert(K_S_ERROR, K_S_LAUNCHER_FORMAT_ERROR_ALERT);
        esp_restart();
    }
    this->alert(K_S_LAUNCHER_FORMAT, K_S_LAUNCHER_FORMAT_SUCCESS_ALLERT);
    esp_restart();
}
