#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
// GUIDELINE: Keira all strings used in messages, or somehow written to serial, should be defined here

// clang-format off

// Multi purpose strings  /////////////////////////////////////////////////////////////////////////////
#define K_S_CURRENT_LANGUAGE_SHORT      "bg"
#define K_S_CURRENT_LANGUAGE_FULL       "Български"

#define K_S_ERROR                       "Грешка"
#define K_S_ATTENTION                   "Внимание"
#define K_S_SUCCESS                     "Успех"
#define K_S_PLEASE_STANDBY              "Моля, изчакайте..."
#define K_S_LILKA_V2_OR_HIGHER_REQUIRED "Тази програма изисква Lilka V2 или по-нова"
#define K_S_FEATURE_IN_DEVELOPMENT      "Тази функция е в разработка"
#define K_S_CANT_OPEN_DIR_FMT           "Не може да се отвори директория %s"
#define K_S_CANT_OPEN_FILE_FMT          "Не може да се отвори файл %s"
#define K_S_CANT_CREATE_DIR_FMT         "Не може да се създаде директория %s"
#define K_S_CANT_REMOVE_FILE_FMT        "Не може да се изтрие файл %s"
#define K_S_DIR_EMPTY_FMT               "Директория %s е празна"

#define K_S_MENU_BACK                   "<< Назад"

#define K_S_PASSWORD                    "Парола"
#define K_S_USER                        "Потребител"

#define K_S_SETTINGS                    "Настройки"
#define K_S_ON                          "ON"
#define K_S_OFF                         "OFF"
#define K_S_STATUS                      "Статус"

#define K_S_SERVICES                    "Услуги"
#define K_S_FTP                         "FTP"
#define K_S_WEB                         "Web"
#define K_S_TELNET                      "Telnet"
#define K_S_MDNS                        "mDNS"
#define K_S_LAUNCHER_MDNS               K_S_MDNS
#define K_S_LAUNCHER_MDNS_HOSTNAME      "Име на хост"
#define K_S_LAUNCHER_MDNS_ENTER_HOSTNAME "Въведете име на хост:"
#define K_S_CHANGE_ON_NEXT_BOOT         "Промените ще се приложат при следващо стартиране"
#define K_S_OS_NAME                     "Keira OS"
#define K_S_OS_DESCRIPTION              "by Anderson & friends"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// service.cpp  ///////////////////////////////////////////////////////////////////////////////////////
#define K_S_SERVICE_STARTUP_FMT "Starting service %s"
#define K_S_SERVICE_DIE_FMT     "Service %s died"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/launcher.cpp //////////////////////////////////////////////////////////////////////////////////
#define K_S_LAUNCHER_MAIN_MENU         "Главно меню"
#define K_S_LAUNCHER_APPS              "Приложения"
#define K_S_LAUNCHER_DEMOS             "Демо"
#define K_S_LAUNCHER_LINES             "Линии"
#define K_S_LAUNCHER_DISK              "Диск"
#define K_S_LAUNCHER_TRANSFORM         "Трансформация"
#define K_S_LAUNCHER_BALL              "Топка"
#define K_S_LAUNCHER_CUBE              "Куб"
#define K_S_LAUNCHER_EPILEPSY          "Епилепсия"
#define K_S_LAUNCHER_PET_PET           "PetPet"
#define K_S_LAUNCHER_TESTS             "Тестове"
#define K_S_LAUNCHER_KEYBOARD          "Клавиатура"
#define K_S_LAUNCHER_SPI_TEST          "SPI тест"
#define K_S_LAUNCHER_I2C_SCANNER       "I2C-скенер"
#define K_S_LAUNCHER_GPIO_MANAGER      "GPIO-мениджър"
#define K_S_LAUNCHER_COMBO             "Combo" // wtf?
#define K_S_LAUNCHER_CALLBACK_TEST     "CallbackTest"
#define K_S_LAUNCHER_LILCATALOG        "LilКаталог"
#define K_S_LAUNCHER_LILTRACKER        "LilTracker"
#define K_S_LAUNCHER_LETRIS            "Letris"
#define K_S_LAUNCHER_TAMAGOTCHI        "Tamagotchi"
#define K_S_LAUNCHER_WEATHER           "Времето"
#define K_S_LAUNCHER_PASTEBIN          "Pastebin"
#define K_S_LAUNCHER_FMANAGER          "Файлов мениджър"
#define K_S_LAUNCHER_DEV_MENU          "Разработка"
#define K_S_LAUNCHER_LIVE_LUA          "Live Lua" // Lua live?
#define K_S_LAUNCHER_LUA_REPL          "Lua REPL"
#define K_S_LAUNCHER_WIFI              "WiFi"
#define K_S_LAUNCHER_WIFI_ADAPTER      "WiFi"
#define K_S_LAUNCHER_WIFI_NETWORKS     "WiFi мрежи"
#define K_S_LAUNCHER_WIFI_TX_POWER     "Мощност WiFi"
#define K_S_LAUNCHER_SD                "SD"
#define K_S_LAUNCHER_SD_FORMAT         "Форматиране на SD"
#define K_S_LAUNCHER_SD_SPEED          "Честота на SD карта"
#define K_S_LAUNCHER_SOUND             "Звук"
#define K_S_LAUNCHER_SERVICES          K_S_SERVICES
#define K_S_LAUNCHER_WEB               K_S_WEB
#define K_S_LAUNCHER_TELNET            K_S_TELNET
#define K_S_LAUNCHER_FTP               K_S_FTP
#define K_S_LAUNCHER_FTP_USER          K_S_USER
#define K_S_LAUNCHER_FTP_PASSWORD      K_S_PASSWORD
#define K_S_LAUNCHER_FTP_IP            "IP"
#define K_S_LAUNCHER_STATUSBAR         "Лента на състоянието"
#define K_S_LAUNCHER_CLOCK             "Часовник"
#define K_S_LAUNCHER_CLOCK_0           "Няма"
#define K_S_LAUNCHER_CLOCK_1           "HH:MM:SS"
#define K_S_LAUNCHER_CLOCK_2           "HH:MM"
#define K_S_LAUNCHER_CLOCK_3           "*HH:MM"
#define K_S_LAUNCHER_MEM               "Памет"
#define K_S_LAUNCHER_MEM_0             "Няма"
#define K_S_LAUNCHER_MEM_1             "Икона"
#define K_S_LAUNCHER_MEM_2             "Текст"
#define K_S_LAUNCHER_NETWORK           K_S_LAUNCHER_WIFI
#define K_S_LAUNCHER_NETWORK_0         "Няма"
#define K_S_LAUNCHER_NETWORK_1         "Икона"
#define K_S_LAUNCHER_BATTERY           "Батерия"
#define K_S_LAUNCHER_BATTERY_0         "Няма"
#define K_S_LAUNCHER_BATTERY_1         "Икона + текст"
#define K_S_LAUNCHER_BATTERY_2         "Икона"
#define K_S_LAUNCHER_BATTERY_3         "Текст"

#define K_S_LAUNCHER_ABOUT_SYSTEM      "За системата"
#define K_S_LAUNCHER_DEVICE_INFO       "За устройството"
#define K_S_PARTITION_TABLE            "Таблица на дяловете"
#define K_S_LAUNCHER_LIGHT_SLEEP       "Light sleep"
#define K_S_LAUNCHER_DEEP_SLEEP        "Deep sleep"
#define K_S_LAUNCHER_REBOOT            "Рестартиране"

#define K_S_LAUNCHER_SELECT_TX_POWER   "Задаване на мощност"

#define K_S_LAUNCHER_ENABLE_WIFI_FIRST "Първо включете WiFi"

#define K_S_LAUNCHER_DEVICE_INFO_FMT \
    "Модел: %s\n"                    \
    "Ревизия: %d\n"                  \
    "Версия ESP-IDF: %s\n"          \
    "Честота: %d MHz\n"              \
    "Брой ядра: %d\n"                \
    "IP: %s"

#define K_S_LAUNCHER_PARTITION_FMT "Адрес: 0x%s\nРазмер: 0x%s"

#define K_S_LAUNCHER_FORMAT        "Форматиране"

#define K_S_LAUNCHER_FORMAT_DISCLAIMER_ALERT                  \
    "ВНИМАНИЕ: Всички данни на SD картата ще бъдат изтрити!\n" \
    "\nПродължаване?\n\nSTART - продължи\nA - изход"

#define K_S_LAUNCHER_PLEASE_STANDBY K_S_PLEASE_STANDBY

#define K_S_LAUNCHER_FORMAT_ERROR_ALERT           \
    "Не може да се форматира SD картата.\n\n"      \
    "Системата ще се рестартира."

#define K_S_LAUNCHER_FORMAT_SUCCESS_ALLERT         \
    "SD картата е форматирана успешно!\n\n"        \
    "Системата ще се рестартира."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/wifi_config.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_WIFI_CONFIG_SCANING_NETWORKS    "Сканиране на WiFi мрежи..."
#define K_S_WIFI_CONFIG_SCAN_ERROR_CODE_FMT "Не може да се сканират мрежи, код на грешка: %d"
#define K_S_WIFI_CONFIG_NETWORKS            "Мрежи"

#define K_S_WIFI_CONFIG_ABOUT_NETWORK_FMT \
    "Канал: %d\n"                         \
    "Сила на сигнала: %ddb\n"            \
    "MAC: %s\n"                           \
    "Защита: %s"

#define K_S_WIFI_CONFIG_CONNECTING_TO_OPEN_INSECURE_NETWORK_FMT \
    "Свързвате се с незащитена мрежа %s\n\n"                     \
    "A - продължи\n"                                              \
    "B - избери друга мрежа"

#define K_S_WIFI_CONFIG_ENTER_PASSWORD              "Въведете парола:"
#define K_S_WIFI_CONFIG_CONNECTING                  "Свързване..."
#define K_S_WIFI_CONFIG_CONNECTED_TO_NETWORK_FMT    "Свързан с %s"
#define K_S_WIFI_CONFIG_CANT_CONNECT_TO_NETWORK_FMT "Не може да се свърже с %s"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/scan_i2c.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_I2C_INIT_ABOUT            "I2C init: SDA=%d, SCL=%d"
#define K_S_I2C_SCANNER_SCAN_START    "Starting I2C scan..."
#define K_S_I2C_SCANNER_SCAN_DONE     "I2C scan done."
#define K_S_I2C_SCANNER_DEVICES_FOUND "Found %d devices."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/transform.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_TRANSFORM_CANT_LOAD_FACE     "Не може да се зареди face.bmp от SD картата." // FACEPALM.BMP
#define K_S_TRANFORM_DRAWING_FACE_AT_FMT "Drawing face at %d, %d"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/fmanager.cpp/.h ///////////////////////////////////////////////////////////////////////////////
#define K_S_FMANAGER_COPYING                              "Копиране..."
#define K_S_FMANAGER_MD5_CALC                             "Изчисляване на MD5"
#define K_S_FMANAGER_LOADING                              "Зареждане"
#define K_S_FMANAGER_ENTER_NEW_FOLDER_NAME                "Въведете име на нова папка"
#define K_S_FMANAGER_ENTER_NEW_NAME                       "Въведете ново име"
#define K_S_FMANAGER_OPTIONS                              "Опции"
#define K_S_FMANAGER_OPEN                                 "Отвори"
#define K_S_FMANAGER_OPEN_WITH                            "Отвори с"
#define K_S_FMANAGER_CREATE_FOLDER                        "Създай папка"
#define K_S_FMANAGER_RENAME                               "Преименувай"
#define K_S_FMANAGER_DELETE                                "Изтрий"
#define K_S_FMANAGER_INFO_ABOUT_FILE                      "Информация"
#define K_S_FMANAGER_SELECT_APP                           "Избери приложение"
#define K_S_FMANAGER_FILE_MANAGER                         "Файлов мениджър"
#define K_S_FMANAGER_NES_EMULATOR                         "NES емулатор"
#define K_S_FMANAGER_FIRMWARE_LOADER                      "Зареждане на фърмуер"
#define K_S_FMANAGER_LUA                                  "Lua"
#define K_S_FMANAGER_MJS                                  "mJS"
#define K_S_FMANAGER_LILTRACKER                           K_S_LAUNCHER_LILTRACKER
#define K_S_FMANAGER_MAD_PLAYER                           "MadPlayer"
#define K_S_FMANAGER_DYNAPP                               "DynApp (.so)"
#define K_S_FMANAGER_ACTIONS_ON_SELECTED                  "Действия с избраните"
#define K_S_FMANAGER_COPY_SELECTED                        "Копирай избраните"
#define K_S_FMANAGER_MOVE_SELECTED                        "Премести избраните"
#define K_S_FMANAGER_DELETE_SELECTED                      "Изтрий избраните"
#define K_S_FMANAGER_CLEAR_SELECTION                      "Изчисти избора"
#define K_S_FMANAGER_CALC_INTERRUPTED                     "Не е изчислено"
#define K_S_FMANAGER_ARE_YOU_SURE_ALERT                   "Сигурни ли сте?"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE_FMT        "Тази операция ще изтрие файл %s\nПродължи: START\nИзход: B"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_COUNT_FILES_FMT "Тази операция ще изтрие %d файл(а)\nПродължи: START\nИзход: B"
#define K_S_FMANAGER_SELECTED_ENTRIES_EXIT_FMT "Избрани %d файла\nПотвърди изход: START\nНазад: B"
#define K_S_FMANAGER_ABOUT_DIR_FMT \
    "Тип: директория\n"            \
    "Път: %s"

#define K_S_FMANAGER_ABOUT_FILE_FMT \
    "Тип: файл\n"                    \
    "Размер: %s\n"                   \
    "MD5: %s\n"
// TODO: MOVE MULTIBOOT TO SEPARATE APP
#define K_S_FMANAGER_MULTIBOOT_ABOUT_FMT            "%s\n\nРазмер: %s"

#define K_S_FMANAGER_MULTIBOOT_STARTING             "Стартиране..."
#define K_S_FMANAGER_MULTIBOOT_ERROR_FMT            "Етап: %d\nКод: %d"
#define K_S_FMANAGER_SORTING                        "Сортиране..."
#define K_S_FMANAGER_ALMOST_DONE                    "Почти готово..."
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE      "Тази операция ще изтрие файл\n"
#define K_S_FMANAGER_NOT_ENOUGH_MEMORY_TO_FINISH_OP "Недостатъчно памет за завършване на операцията"
#define K_S_FMANAGER_SELECTED_FILES_FMT             "Избрани %d файл(а)"
#define K_S_FMANAGER_CANT_DO_OP                     "Операцията не може да се изпълни"
#define K_S_FMANAGER_FILE_ADDED_TO_BUFFER_EXCHANGE  "Файлът е добавен в клипборда"
///////////////////////////////////////////////////////////////////////////////////////////////////////
// apps/liltracker/liltracker.cpp ////////////////////////////////////////////
#define K_S_LILTRACKER_SAVE_TRACK             "Запази трак"
#define K_S_LILTRACKER_OPEN_TRACK             "Отвори трак"
#define K_S_LILTRACKER_CREATE_NEW_TRACK       "++ Създай нов"
#define K_S_LILTRACKER_ENTER_FILENAME         "Въведете име на файл"
#define K_S_LILTRACKER_FILENAME_CANT_BE_EMPTY "Името на файла не може да е празно"
#define K_S_LILTRACKER_CLEAR_TRACK_DATA       "Изтрий всички данни\nна трака?\n[START]Да\n[B]Не"
#define K_S_LILTRACKER_OPEN                   "Отвори"
#define K_S_LILTRACKER_SAVE                   "Запази"
#define K_S_LILTRACKER_RESET                  "Нулирай"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/lilcatalog/lilcatalog.cpp /////////////////////////////////////////////////////////////////////
#define K_S_LILCATALOG_APP                           "LilКаталог"

#define K_S_LILCATALOG_APPS                          "Приложения"
#define K_S_LILCATALOG_MODS                          "Модове"
#define K_S_LILCATALOG_STOP                          "Изход"
#define K_S_LILCATALOG_BACK                          "Назад"
#define K_S_LILCATALOG_EMPTY                         ""

#define K_S_LILCATALOG_START                         "Стартирай"
#define K_S_LILCATALOG_INSTALL                       "Инсталирай"
#define K_S_LILCATALOG_REMOVE                        "Изтрий"
#define K_S_LILCATALOG_UPDATE                        "Обнови"
#define K_S_LILCATALOG_SOURCE                        "Източник"
#define K_S_LILCATALOG_ENTRY_DESCRIPTION             "Описание"
#define K_S_LILCATALOG_LOADING                       "Изтегляне"
#define K_S_LILCATALOG_LOADING_CATALOG               "Зареждане на каталога..."
#define K_S_LILCATALOG_STARTING                      "Стартиране..."
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_NAME        "Име: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_AUTHOR      "Автор: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_DESCRIPTION "Описание: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_FILE        "Файл: "

#define K_S_LILCATALOG_NEXT_PAGE                     ">> Следваща страница"
#define K_S_LILCATALOG_PREV_PAGE                     "<< Предишна страница"
#define K_S_LILCATALOG_SIZE                          "Размер:"
#define K_S_LILCATALOG_FILE_LOADING                  "Изтегляне на файл..."
#define K_S_LILCATALOG_FILE_LOADING_COMPLETE         "Файлът е изтеглен и запазен"

#define K_S_LILCATALOG_ARCHIVE_NOTICE                "Архивите трябва да бъдат разопаковани ръчно"
#define K_S_LILCATALOG_UNSUPPORTED_TYPE              "Неподдържан тип файл"
#define K_S_LILCATALOG_CLEAR_CACHE                   "Изчисти кеша"
#define K_S_LILCATALOG_CACHE_CLEARED                 "Кешът е изчистен"
#define K_S_LILCATALOG_INSTALLED                     "Инсталирани"
#define K_S_LILCATALOG_NO_INSTALLED                  "Няма инсталирани приложения"
#define K_S_LILCATALOG_NO_BINARY                     "Това приложение няма файлове за изтегляне"
#define K_S_LILCATALOG_VIEW_SOURCE                   "Виж източника"

#define K_S_LILCATALOG_ERROR_CREATE_FOLDER           "Грешка при създаване на папка"
#define K_S_LILCATALOG_ERROR_LOAD_CATALOG            "Грешка при зареждане на каталога"
#define K_S_LILCATALOG_ERROR_FILE_OPEN               "Грешка при отваряне на файл"
#define K_S_LILCATALOG_ERROR_CONNECTION              "Грешка при свързване: "
#define K_S_LILCATALOG_ERROR_DIRETORY_CREATE         "Грешка при създаване на директория"
#define K_S_LILCATALOG_ERROR_STAGE1                  "Етап: 1\nКод: "
#define K_S_LILCATALOG_ERROR_STAGE2                  "Етап: 2\nКод: "
#define K_S_LILCATALOG_ERROR_STAGE3                  "Етап: 3\nКод: "
#define K_S_LILCATALOG_SD_NOTFOUND                   "SD картата не е намерена. Не може да се продължи"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/letris.cpp //////////////////////////////////////////////////////////////////////////////
#define K_S_LETRIS_GAME_OVER      "Game over"
#define K_S_LETRIS_GAME_OVER_LONG "Край на играта!\nОпита се. :)"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/settings/sound.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SETTINGS_SOUND                "Звук"
#define K_S_SETTINGS_SOUND_VOLUME         "Сила на звука:"
#define K_S_SETTINGS_SOUND_STARTRUP       "Звук при стартиране:"
#define K_S_SETTINGS_SOUND_BUZZER_STARTUP "Зумер при стартиране:"
#define K_S_SETTINGS_SOUND_SAVE           "<< Запази"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/screenshot.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SCREENSHOT_SAVED      "Екранната снимка е запазена"
#define K_S_SCREENSHOT_SAVE_ERROR "Грешка при запазване на екранна снимка"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/network.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_NET_OFFLINE "WiFi връзката е загубена"
#define K_S_NET_ONLINE  "WiFi е свързан"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/combo.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_COMBO_FOR_EXIT_HOLD            "За изход задръжте"
#define K_S_COMBO_HOLD_SELECT_FOR_TIME_FMT "[SELECT] за %d с"
#define K_S_COMBO_PRESS_START              "Натиснете [START]"
#define K_S_COMBO_TO_CONTINUE              "за продължаване"
#define K_S_COMBO_PRESS                    "Натиснете"
#define K_S_COMBO_HOLD_ANY                 "Задръжте произволен"
#define K_S_COMBO_BUTTON_FOR_ONE_SEC       "бутон за 1 с"
#define K_S_COMBO_PRESS_TOGETHER           "Натиснете заедно"
#define K_S_COMBO_TIMEOUT_FMT              "Таймаут %d мс"
#define K_S_COMBO_END                      "Край!"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/weather/weather.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_WEATHER_CLEAR_SKY                     "Ясно небе" // 0

#define K_S_WEATHER_MOSTLY_CLEAR                  "Предимно ясно" // 1
#define K_S_WEATHER_PARTLY_CLOUDY                 "Частично облачно" // 2
#define K_S_WEATHER_OVERCAST                      "Облачно" //3

#define K_S_WEATHER_FOG                           "Мъгла" //45
#define K_S_WEATHER_FREEZING_FOG                  "Замръзваща мъгла" //48

#define K_S_WEATHER_LIGHT_DRIZZLE                 "Лек ръмеж" //51
#define K_S_WEATHER_DRIZZLE                       "Ръмеж" //53
#define K_S_WEATHER_HEAVY_DRIZZLE                 "Силен ръмеж" //55

#define K_S_WEATHER_LIGHT_FREEZING_DRIZZLE        "Лек замръзващ ръмеж" //56
#define K_S_WEATHER_HEAVY_FREEZING_DRIZZLE        "Силен замръзващ ръмеж" //57

#define K_S_WEATHER_LIGHT_RAIN                    "Лек дъжд" //61
#define K_S_WEATHER_RAIN                          "Дъжд" //63
#define K_S_WEATHER_HEAVY_RAIN                    "Силен дъжд" // 65

#define K_S_WEATHER_LIGHT_FREEZING_RAIN           "Лек замръзващ дъжд" // 66
#define K_S_WEATHER_HEAVY_FREEZING_RAIN           "Силен замръзващ дъжд" //67

#define K_S_WEATHER_LIGHT_SNOW                    "Лек сняг" // 71
#define K_S_WEATHER_SNOW                          "Сняг" // 73
#define K_S_WEATHER_HEAVY_SNOW                    "Силен сняг" // 75

#define K_S_WEATHER_SNOW_GRAINS                   "Снежни зърна" // 77

#define K_S_WEATHER_LIGHT_SHOWERS                 "Леки валежи" // 80
#define K_S_WEATHER_SHOWERS                       "Валежи" // 81
#define K_S_WEATHER_HEAVY_SHOWERS                 "Силни валежи" // 82

#define K_S_WEATHER_RAIN_AND_SNOW                 "Дъжд със сняг" // 85
#define K_S_WEATHER_HEAVY_RAIN_AND_SNOW           "Силен дъжд със сняг" // 86

#define K_S_WEATHER_THUNDERSTORM                  "Гръмотевица" // 95
#define K_S_WEATHER_THUNDERSTORM_WITH_HAIL        "Гръмотевица с градушка" // 96
#define K_S_WEATHER_SEVERE_THUNDERSTORM_WITH_HAIL "Силна гръмотевица с градушка" // 99

#define K_S_WEATHER_LOADING_DATA                  "Зареждане на данни..."
#define K_S_WEATHER_DATA_PATTERN_ERROR            "Грешка във формата на данните"
#define K_S_WEATHER_DATA_LOAD_ERROR_FMT           "Грешка при зареждане:\nКод на статус: %d"

#define K_S_WEATHER_LOCATION_NOT_SET              "Местоположението не е зададено"
#define K_S_WEATHER_SELECT_TO_SETUP               "[SELECT] - настройки"
#define K_S_WEATHER_A_TO_EXIT                     "[A] - изход" // LOL WHY
#define K_S_WEATHER_TEMP_FMT                      "%.1f °C"
#define K_S_WEATHER_WIND_SPEED_FMT                "%.1f км/ч"

#define K_S_WEATHER_LATITUDE                      "Географска ширина"
#define K_S_WEATHER_LATITUDE_S                    "ширина"
#define K_S_WEATHER_LONGITUDE                     "Географска дължина"
#define K_S_WEATHER_LONGITUDE_S                   "дължина"

#define K_S_WEATHER_INPUT                         "Въведете"

#define K_S_WEATHER_SAVE                          "Запази"
#define K_S_WEATHER_CANCEL                        "Отказ"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/usbdrive/usbdrive.cpp /////////////////////////////////////////////////////////////////////////
#define K_S_USB_DRIVE_TITLE                       "USB устройство"
#define K_S_USB_DRIVE_EXPERIMENTAL_TITLE          "Предупреждение"
#define K_S_USB_DRIVE_EXPERIMENTAL_WARNING        "Експериментална функция.\n" \
                                                  "Serial ще бъде деактивиран.\n\n" \
                                                  "[START] - OK  [A] - Отказ"
#define K_S_USB_DRIVE_INITIALIZING                "Инициализация на USB..."
#define K_S_USB_DRIVE_INIT_ERROR                  "Грешка при инициализация на USB"
#define K_S_USB_DRIVE_NO_SD                       "SD картата не е достъпна"
#define K_S_USB_DRIVE_CONNECT_USB                 "Свържете USB кабел с компютъра"
#define K_S_USB_DRIVE_CONNECTED                   "Свързан с компютъра"
#define K_S_USB_DRIVE_EJECTED                     "Безопасно извадено"
#define K_S_USB_DRIVE_PC_INSTRUCTION              "SD картата е достъпна на компютъра"
#define K_S_USB_DRIVE_SAFE_EJECT                  "Извадете устройството преди изход"
#define K_S_USB_DRIVE_PRESS_A_TO_EXIT             "[A] - изход"
#define K_S_USB_DRIVE_DISCONNECTING               "Прекъсване на връзката..."
#define K_S_USB_DRIVE_REBOOT_REQUIRED             "Устройството ще се рестартира..."
#define K_S_USB_DRIVE_NOT_EJECTED                 "Не е извадено безопасно!"
#define K_S_USB_DRIVE_EJECT_WARNING               "Данните може да са повредени"
#define K_S_USB_DRIVE_PRESS_START_CONTINUE        "[START] - продължи"
#define K_S_USB_DRIVE_PRESS_B_CANCEL              "[B] - отказ"
#define K_S_LAUNCHER_USB_DRIVE                    "USB устройство"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/pastebin/pastebinApp.cpp //////////////////////////////////////////////////////////////////////
#define K_S_PASTEBIN_CODE                         "Код"
#define K_S_PASTEBIN_NAME                         "Име"
#define K_S_PASTEBIN_DOWNLOAD                     "Изтегли"
#define K_S_PASTEBIN_ENTER_CODE                   "Въведете код"
#define K_S_PASTEBIN_ENTER_NAME                   "Въведете име"
#define K_S_PASTEBIN_ERROR_CREATE_DIR             "Грешка при създаване на директория"
#define K_S_PASTEBIN_ERROR_OPEN_FILE              "Грешка при отваряне на файл"
#define K_S_PASTEBIN_FILE_DOWNLOADED_FMT          "Файлът е изтеглен в\n%s"
#define K_S_PASTEBIN_REQUEST_FAIL_FMT             "HTTP грешка\n%s"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/mjs/mjsrunner.cpp /////////////////////////////////////////////////////////////////////////////
#define K_S_MJS_ERROR                             "Грешка: "
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/mjs/luarunner.cpp /////////////////////////////////////////////////////////////////////////////
#define K_S_LUA_ERROR                             "Грешка: "
#define K_S_LUA_AWAIT_CODE_FROM_UART              "Очакване на код\nот UART...\n\nНатиснете [A]\nза изход."
#define K_S_LUA_DOWNLOAD                          "Изтегляне..."
#define K_S_LUA_REPL_AWAIT_CODE_FROM_UART         "REPL:\nОчакване на код\nот UART...\n\nНатиснете [A]\nза изход."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/tests/keyboard/keyboard.cpp //////////////////////////////////////////////////////////////////
#define K_S_KEYBOARD_ENTER_TEXT                    "Въведете текст: "
#define K_S_KEYBOARD_YOU_ENTERED                   "Вие въведохте:"
//////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format on
