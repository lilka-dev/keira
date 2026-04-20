#include <ff.h>
#include <FS.h>
#include <qrcode.h>
#include "keira/keira.h"
#include "launcher.h"
#include "keira/appmanager.h"

#include "keira/servicemanager.h"
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
#include "apps/statusbar/statusbar.h"
#include "apps/wificonfig/wificonfig.h"
#include "apps/letris/letris.h"
#include "apps/gpiomanager/gpiomanager.h"
#include "apps/tamagotchi/tamagotchi.h"
#include "apps/lua/luarunner.h"
#include "apps/mjs/mjsrunner.h"
#include "apps/nes/nesapp.h"
#include "apps/weather/weather.h"
#include "apps/madplayer/madplayer.h"
#include "apps/lilcatalog/lilcatalog.h"
#include "apps/liltracker/liltracker.h"
#include "apps/fmanager/fmanager.h"
#include "apps/pastebin/pastebinApp.h"
#include "apps/usbdrive/usbdrive.h"
#include "apps/soundsettings/sound.h"

// Icons
#include "apps/icons/demos.h"
#include "apps/icons/sdcard.h"
#include "apps/icons/memory.h"
#include "apps/icons/dev.h"
#include "apps/icons/settings.h"
#include "apps/icons/info.h"
#include "apps/icons/app_group.h"

// Libs
#include <WiFi.h> // for setWiFiTxPower
#include <Preferences.h>
#include <lilka/spi.h>
#include <nvs_flash.h>

#include "keira/ksystem.h"

#define TOGGLE_SERVICE(SERVICE_NAME) \
    ksystem.services.status(SERVICE_NAME) ? ksystem.services.down(SERVICE_NAME) : ksystem.services.up(SERVICE_NAME)

LauncherApp::LauncherApp() : App("Launcher") {
    setktStackSize(8192);
}

void LauncherApp::onAnyServiceMenuItem() {
    if (servicesMenu.getButton() == K_BTN_BACK) return;
    // Find selected service by index
    int index = servicesMenu.getCursor();
    int i = 0;
    const char* selectedName = nullptr;

    ksystem.registry.lock();
    for (auto& unit : ksystem.registry) {
        if (unit.type != KREG_SERVICE) continue;
        if (i == index) {
            selectedName = unit.name;
            break;
        }
        i++;
    }
    ksystem.registry.unlock();

    if (!selectedName) return;

    auto cfg = ksystem.registry[selectedName]->config;

    // // Show config menu for selected service
    // pServiceMenu = cfg->getMenu();

    // while (!pServiceMenu->isFinished()) {
    //     pServiceMenu->update();
    //     pServiceMenu->draw(canvas);
    //     queueDraw();
    // }
}

void LauncherApp::showServicesMenu() {
    servicesMenu.clearItems();
    servicesMenu.setTitle(K_S_LAUNCHER_SERVICES);
    servicesMenu.addActivationButton(K_BTN_BACK);

    ksystem.registry.lock();
    for (auto& unit : ksystem.registry) {
        if (unit.type != KREG_SERVICE) continue;
        servicesMenu.addItem(
            unit.name,
            nullptr,
            ksystem.services.status(unit.name) ? lilka::colors::Green : lilka::colors::Red,
            "",
            LILKA_MENU_CLBK_CAST(&LauncherApp::onAnyServiceMenuItem),
            LILKA_MENU_CLBK_DATA_CAST(this)
        );
    }
    ksystem.registry.unlock();

    while (!servicesMenu.isFinished()) {
        servicesMenu.update();
        servicesMenu.draw(canvas);
        queueDraw();
    }
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
                        &app_group_img,
                        lilka::colors::White
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
                        &app_group_img,
                        lilka::colors::White
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
                K_S_LAUNCHER_FMANAGER,
                [this]() { this->runApp<FileManagerApp>("/"); },
                &sdcard_img,
                lilka::colors::Arylide_yellow
            ),
            ITEM::APP(
                K_S_LAUNCHER_USB_DRIVE, [this]() { this->runApp<USBDriveApp>(); }, &memory_img, lilka::colors::Mint
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
                    ITEM::SUBMENU(K_S_LAUNCHER_WIFI, {
                        ITEM::MENU(
                            K_S_LAUNCHER_WIFI_ADAPTER,
                            [this]() { TOGGLE_SERVICE("network"); },
                            nullptr,
                            lilka::colors::White,
                            [this](void* item) {
                                    lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                                    menuItem->postfix = ksystem.services.status("network") ? K_S_ON : K_S_OFF;
                            }
                        ),
                        ITEM::MENU(K_S_LAUNCHER_WIFI_NETWORKS, [this]() { this->wifiManager(); }),
                        ITEM::MENU(K_S_LAUNCHER_WIFI_TX_POWER, [this]() { this->setWiFiTxPower(); }),
                    }),
                    ITEM::SUBMENU(K_S_LAUNCHER_SD, {
                        ITEM::MENU(K_S_PARTITION_TABLE, [this]() { this->partitions(); }),
                        ITEM::MENU(K_S_LAUNCHER_SD_FORMAT, [this]() { this->formatSD(); }),
                        ITEM::MENU(K_S_LAUNCHER_SD_SPEED, [this]() { this->setSpiSDSpeed(); }),
                    }),
                    ITEM::MENU(K_S_LAUNCHER_SOUND, [this]() { this->runApp<SoundConfigApp>(); }),


                    ITEM::MENU(K_S_LAUNCHER_SERVICES, [this]() { this->showServicesMenu(); }),
                    ITEM::SUBMENU(K_S_LAUNCHER_STATUSBAR, {
                        ITEM::MENU(
                            K_S_LAUNCHER_CLOCK,
                            [this]() {
                                    auto statusBar = static_cast<StatusBarApp*>(ksystem.apps.getpanel());
                                    statusBar->setClockMode((statusBar->getClockMode() + 1));
                            },
                            nullptr,
                            lilka::colors::White,
                            [this](void* item) {
                                    lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                                    auto statusBar = static_cast<StatusBarApp*>(ksystem.apps.getpanel());
                                    switch (statusBar->getClockMode()) {
                                        case 0:
                                            menuItem->postfix = K_S_LAUNCHER_CLOCK_0;
                                            break;
                                        case 1:
                                            menuItem->postfix = K_S_LAUNCHER_CLOCK_1;
                                            break;
                                        case 2:
                                            menuItem->postfix = K_S_LAUNCHER_CLOCK_2;
                                            break;
                                        default:
                                            menuItem->postfix = K_S_LAUNCHER_CLOCK_3;
                                            break;
                                    }
                                }
                            ),
                            ITEM::MENU(
                                K_S_LAUNCHER_MEM,
                                [this]() {
                                    auto statusBar = static_cast<StatusBarApp*>(ksystem.apps.getpanel());
                                    statusBar->setMemMode((statusBar->getMemMode() + 1));
                                },
                                nullptr,
                                lilka::colors::White,
                                [this](void* item) {
                                    lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                                    auto statusBar = static_cast<StatusBarApp*>(ksystem.apps.getpanel());
                                    switch (statusBar->getMemMode()) {
                                        case 0:
                                            menuItem->postfix = K_S_LAUNCHER_MEM_0;
                                            break;
                                        case 1:
                                            menuItem->postfix = K_S_LAUNCHER_MEM_1;
                                            break;
                                        default:
                                            menuItem->postfix = K_S_LAUNCHER_MEM_2;
                                            break;
                                    }
                                }
                            ),
                            ITEM::MENU(
                                K_S_LAUNCHER_NETWORK,
                                [this]() {
                                    auto statusBar = static_cast<StatusBarApp*>(ksystem.apps.getpanel());
                                    statusBar->setNetworkMode((statusBar->getNetworkMode() + 1));
                                },
                                nullptr,
                                lilka::colors::White,
                                [this](void* item) {
                                    lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                                    auto statusBar = static_cast<StatusBarApp*>(ksystem.apps.getpanel());
                                    switch (statusBar->getNetworkMode()) {
                                        case 0:
                                            menuItem->postfix = K_S_LAUNCHER_NETWORK_0;
                                            break;
                                        default:
                                            menuItem->postfix = K_S_LAUNCHER_NETWORK_1;
                                            break;
                                    }
                                }
                            ),
                            ITEM::MENU(
                                K_S_LAUNCHER_BATTERY,
                                [this]() {
                                    auto statusBar = static_cast<StatusBarApp*>(ksystem.apps.getpanel());
                                    statusBar->setBatteryMode((statusBar->getBatteryMode() + 1));
                                },
                                nullptr,
                                lilka::colors::White,
                                [this](void* item) {
                                    lilka::MenuItem* menuItem = static_cast<lilka::MenuItem*>(item);
                                    auto statusBar = static_cast<StatusBarApp*>(ksystem.apps.getpanel());
                                    switch (statusBar->getBatteryMode()) {
                                        case 0:
                                            menuItem->postfix = K_S_LAUNCHER_BATTERY_0;
                                            break;
                                        case 1:
                                            menuItem->postfix = K_S_LAUNCHER_BATTERY_1;
                                            break;
                                        case 2:
                                            menuItem->postfix = K_S_LAUNCHER_BATTERY_2;
                                            break;
                                        default:
                                            menuItem->postfix = K_S_LAUNCHER_BATTERY_3;
                                            break;
                                    }
                                }
                            ),
                        }
                    ),
                    ITEM::SUBMENU(
                        K_S_LAUNCHER_ABOUT,
                        {
                            ITEM::MENU(K_S_OS_NAME, [this]() { this->about(); }),
                            ITEM::MENU(K_S_LAUNCHER_DEVICE_INFO, [this]() { this->info(); }),
                        }
                    ),
                    ITEM::MENU(K_S_LAUNCHER_FACTORY_RESET, [this]() { this->factoryReset(); }),
                    ITEM::MENU(K_S_LAUNCHER_REBOOT, []() { esp_restart(); }),
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
template <typename T, typename... Args>
void LauncherApp::runApp(Args&&... args) {
    ksystem.apps.spawn(new T(std::forward<Args>(args)...));
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
    NVS_LOCK;
    Preferences prefs;
    prefs.begin("network", false);
    prefs.putInt("txPower", static_cast<int>(values[index]));
    prefs.end();
    NVS_UNLOCK;
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
    setSpiSDSpeedMenu.setTitle(K_S_LAUNCHER_SD_SPEED);
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
    NVS_LOCK;
    Preferences prefs;
    prefs.begin(LILKA_SPI_NVS_NAMESPACE, false);
    uint32_t sdFrequency = sdFrequencies[index];
    prefs.putUInt(LILKA_SPI_NVS_SD_FREQUENCY_KEY, sdFrequency);
    prefs.end();
    NVS_UNLOCK;

    alert("", K_S_CHANGE_ON_NEXT_BOOT);
}

void LauncherApp::wifiManager() {
    if (!ksystem.services.status("network")) {
        alert(K_S_ERROR, K_S_LAUNCHER_ENABLE_WIFI_FIRST);
        return;
    }
    ksystem.apps.spawn(new WiFiConfigApp());
}

void LauncherApp::about() {
    static int clickCount = 0;
    clickCount++;

    if (clickCount >= 5) {
        clickCount = 0;
        showEasterEgg();
        return;
    }

    alert(
        K_S_OS_NAME,
        StringFormat(
            K_S_LAUNCHER_ABOUT_FMT,
            ksystem.getVersionStr().c_str(),
            lilka::sdk.getVersionStr().c_str(),
            esp_get_idf_version()
        )
    );
}
void LauncherApp::info() {
    NetworkService* networkService = static_cast<NetworkService*>(ksystem.services["network"]);
    alert(
        K_S_LAUNCHER_DEVICE_INFO,
        StringFormat(
            K_S_LAUNCHER_DEVICE_INFO_FMT,
            ARDUINO_BOARD,
            ESP.getChipModel(),
            ESP.getChipRevision(),
            ESP.getCpuFreqMHz(),
            ESP.getChipCores(),
            networkService ? networkService->getipAddr().c_str() : "0.0.0.0"
        )
    );
}
void LauncherApp::showEasterEgg() {
    const char* url = "https://youtu.be/dQw4w9WgXcQ";

    // Use a static variable to store QR code for display callback
    static lilka::Canvas* qrCanvas = nullptr;
    static int qrScale = 0;
    static int qrOffsetX = 0;
    static int qrOffsetY = 0;

    qrCanvas = canvas;

    auto displayFunc = [](esp_qrcode_handle_t qrcode) {
        int size = esp_qrcode_get_size(qrcode);
        qrScale = min((int)qrCanvas->width(), (int)(qrCanvas->height() - 40)) / size;
        qrOffsetX = (qrCanvas->width() - size * qrScale) / 2;
        qrOffsetY = 30 + (qrCanvas->height() - 40 - size * qrScale) / 2;

        qrCanvas->fillScreen(lilka::colors::White);
        qrCanvas->setTextColor(lilka::colors::Black);
        qrCanvas->setFont(FONT_9x15);
        qrCanvas->setTextBound(0, 0, qrCanvas->width(), qrCanvas->height());
        qrCanvas->setCursor(qrCanvas->width() / 2 - 40, 20);
        qrCanvas->print(K_S_LAUNCHER_EASTER_EGG);

        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (esp_qrcode_get_module(qrcode, x, y)) {
                    qrCanvas->fillRect(
                        qrOffsetX + x * qrScale, qrOffsetY + y * qrScale, qrScale, qrScale, lilka::colors::Black
                    );
                }
            }
        }
    };

    esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
    cfg.display_func = displayFunc;
    cfg.max_qrcode_version = 10;
    cfg.qrcode_ecc_level = ESP_QRCODE_ECC_LOW;

    if (esp_qrcode_generate(&cfg, url) != ESP_OK) {
        alert(K_S_ERROR, "Failed to generate QR code");
        return;
    }

    queueDraw();

    while (true) {
        lilka::State state = lilka::controller.getState();
        if (state.a.justPressed || state.b.justPressed || state.start.justPressed) {
            break;
        }
        taskYIELD();
    }
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
    if (!confirm(K_S_LAUNCHER_FORMAT, K_S_LAUNCHER_FORMAT_DISCLAIMER_ALERT)) return;

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

void LauncherApp::factoryReset() {
    if (!confirm(K_S_LAUNCHER_FACTORY_RESET, K_S_LAUNCHER_FACTORY_RESET_DISCLAIMER_ALERT)) return;

    lilka::ProgressDialog dialog(K_S_LAUNCHER_FACTORY_RESET, K_S_LAUNCHER_PLEASE_STANDBY);
    dialog.draw(canvas);
    queueDraw();

    nvs_flash_erase();

    this->alert(K_S_LAUNCHER_FACTORY_RESET, K_S_LAUNCHER_FACTORY_RESET_SUCCESS_ALERT);
    esp_restart();
}
