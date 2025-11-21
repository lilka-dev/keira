#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
// GUIDELINE: Keira all strings used in messages, or somehow written to serial, should be defined here

// clang-format off

// Multi purpose strings  /////////////////////////////////////////////////////////////////////////////
#define K_S_ERROR                       "Помилка"
#define K_S_ATTENTION                   "Увага"
#define K_S_SUCCESS                     "Успіх"
#define K_S_PLEASE_STANDBY              "Будь ласка, зачекайте..."
#define K_S_LILKA_V2_OR_HIGHER_REQUIRED "Ця програма потребує Лілку версії 2 або вище."
#define K_S_FEATURE_IN_DEVELOPMENT      "Ця функція перебуває в розробці"
#define K_S_CANT_OPEN_DIR_FMT           "Не вдалося відкрити директорію %s"
#define K_S_CANT_OPEN_FILE_FMT          "Не вдалося відкрити файл %s"
#define K_S_CANT_CREATE_DIR_FMT         "Не вдалося створити директорію %s"
#define K_S_CANT_REMOVE_FILE_FMT        "Не вдалося видалити файл %s"
#define K_S_DIR_EMPTY_FMT               "Директорія %s порожня"

#define K_S_MENU_BACK                   "<< Назад"

#define K_S_PASSWORD                    "Пароль"
#define K_S_USER                        "Користувач"

#define K_S_SETTINGS                    "Налаштування"
#define K_S_ON                          "ON"
#define K_S_OFF                         "OFF"
#define K_S_STATUS                      "Статус"

#define K_S_SERVICES                    "Сервіси"
#define K_S_FTP                         "FTP"
#define K_S_TELNET                      "Telnet"

#define K_S_OS_NAME                     "Keira OS"
#define K_S_OS_DESCRIPTION              "by Андерсон & friends"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// service.cpp  ///////////////////////////////////////////////////////////////////////////////////////
#define K_S_SERVICE_STARTUP_FMT "Starting service %s"
#define K_S_SERVICE_DIE_FMT     "Service %s died"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/launcher.cpp //////////////////////////////////////////////////////////////////////////////////
#define K_S_LAUNCHER_MAIN_MENU         "Головне меню"
#define K_S_LAUNCHER_APPS              "Додатки"
#define K_S_LAUNCHER_DEMOS             "Демо"
#define K_S_LAUNCHER_LINES             "Лінії"
#define K_S_LAUNCHER_DISK              "Диск"
#define K_S_LAUNCHER_TRANSFORM         "Перетворення"
#define K_S_LAUNCHER_BALL              "М'ячик"
#define K_S_LAUNCHER_CUBE              "Куб"
#define K_S_LAUNCHER_EPILEPSY          "Епілепсія"
#define K_S_LAUNCHER_PET_PET           "PetPet"
#define K_S_LAUNCHER_TESTS             "Тести"
#define K_S_LAUNCHER_KEYBOARD          "Клавіатура"
#define K_S_LAUNCHER_SPI_TEST          "Тест SPI"
#define K_S_LAUNCHER_I2C_SCANNER       "I2C-сканер"
#define K_S_LAUNCHER_GPIO_MANAGER      "GPIO-менеджер"
#define K_S_LAUNCHER_COMBO             "Combo" // wtf?
#define K_S_LAUNCHER_CALLBACK_TEST     "CallbackTest"
#define K_S_LAUNCHER_LILCATALOG        "ЛілКаталог"
#define K_S_LAUNCHER_LILTRACKER        "ЛілТрекер"
#define K_S_LAUNCHER_LETRIS            "Летріс"
#define K_S_LAUNCHER_TAMAGOTCHI        "Тамагочі"
#define K_S_LAUNCHER_WEATHER           "Погода"
#define K_S_LAUNCHER_PASTEBIN          "Pastebin"
#define K_S_LAUNCHER_SD_BROWSER        "Браузер SD-карти"
#define K_S_LAUNCHER_SPIFFS_BROWSER    "Браузер SPIFFS"
#define K_S_LAUNCHER_DEV_MENU          "Розробка"
#define K_S_LAUNCHER_LIVE_LUA          "Live Lua" // Lua live?
#define K_S_LAUNCHER_LUA_REPL          "Lua REPL"
#define K_S_LAUNCHER_WIFI_ADAPTER      "WiFi-адаптер"
#define K_S_LAUNCHER_WIFI_NETWORKS     "Мережі WiFi"
#define K_S_LAUNCHER_WIFI_TX_POWER     "Потужність WiFi"
#define K_S_LAUNCHER_SOUND             "Звук"
#define K_S_LAUNCHER_SERVICES          K_S_SERVICES
#define K_S_LAUNCHER_FTP               K_S_FTP
#define K_S_LAUNCHER_TELNET            K_S_TELNET
#define K_S_LAUNCHER_FTP_USER          K_S_USER
#define K_S_LAUNCHER_FTP_PASSWORD      K_S_PASSWORD

#define K_S_LAUNCHER_ABOUT_SYSTEM      "Про систему"
#define K_S_LAUNCHER_DEVICE_INFO       "Інфо про пристрій"
#define K_S_PARTITION_TABLE            "Таблиця розділів"
#define K_S_LAUNCHER_SD_FORMAT         "Форматування SD-карти"
#define K_S_LAUNCHER_LIGHT_SLEEP       "Light sleep"
#define K_S_LAUNCHER_DEEP_SLEEP        "Deep sleep"
#define K_S_LAUNCHER_REBOOT            "Перезавантаження"

#define K_S_LAUNCHER_SELECT_TX_POWER   "Оберіть потужність"

#define K_S_LAUNCHER_ENABLE_WIFI_FIRST "Спочатку увімкніть WiFi-адаптер"

#define K_S_LAUNCHER_DEVICE_INFO_FMT \
    "Модель: %s\n"                   \
    "Ревізія: %d\n"                  \
    "Версія ESP-IDF: %s\n"           \
    "Частота: %d МГц\n"              \
    "Кількість ядер: %d\n"           \
    "IP: %s"

#define K_S_LAUNCHER_PARTITION_FMT "Адреса: 0x%s\nРозмір: 0x%s"

#define K_S_LAUNCHER_FORMAT        "Форматування"

#define K_S_LAUNCHER_FORMAT_DISCLAIMER_ALERT    \
    "УВАГА: Це очистить ВСІ дані з SD-карти!\n" \
    "\nПродовжити?\n\nSTART - продовжити\nA - скасувати"

#define K_S_LAUNCHER_PLEASE_STANDBY K_S_PLEASE_STANDBY

#define K_S_LAUNCHER_FORMAT_ERROR_ALERT    \
    "Не вдалося форматувати SD-карту.\n\n" \
    "Систему буде перезавантажено."

#define K_S_LAUNCHER_FORMAT_SUCCESS_ALLERT \
    "Форматування SD-карти завершено!\n\n" \
    "Систему буде перезавантажено."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/wifi_config.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_WIFI_CONFIG_SCANING_NETWORKS    "Скануємо мережі WiFi..."
#define K_S_WIFI_CONFIG_SCAN_ERROR_CODE_FMT "Не вдалося сканувати мережі, код помилки: %d"
#define K_S_WIFI_CONFIG_NETWORKS            "Мережі"

#define K_S_WIFI_CONFIG_ABOUT_NETWORK_FMT \
    "Канал: %d\n"                         \
    "Сила сигналу: %ddb\n"                \
    "MAC: %s\n"                           \
    "Захист: %s"

#define K_S_WIFI_CONFIG_CONNECTING_TO_OPEN_INSECURE_NETWORK_FMT \
    "Ви під'єднуєтеся до незахищеної мережі %s\n\n"             \
    "A - продовжити\n"                                          \
    "B - обрати іншу мережу"

#define K_S_WIFI_CONFIG_ENTER_PASSWORD              "Введіть пароль:"
#define K_S_WIFI_CONFIG_CONNECTING                  "Під'єднуємось..."
#define K_S_WIFI_CONFIG_CONNECTED_TO_NETWORK_FMT    "Під'єднано до мережі %s"
#define K_S_WIFI_CONFIG_CANT_CONNECT_TO_NETWORK_FMT "Не вдалося під'єднатись до мережі %s"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/scan_i2c.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_I2C_INIT_ABOUT            "I2C init: SDA=%d, SCL=%d"
#define K_S_I2C_SCANNER_SCAN_START    "Starting I2C scan..."
#define K_S_I2C_SCANNER_SCAN_DONE     "I2C scan done."
#define K_S_I2C_SCANNER_DEVICES_FOUND "Found %d devices."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/transform.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_TRANSFORM_CANT_LOAD_FACE     "Не вдалось завантажити face.bmp з SD-карти." // FACEPALM.BMP
#define K_S_TRANFORM_DRAWING_FACE_AT_FMT "Drawing face at %d, %d"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/fmanager.cpp/.h ///////////////////////////////////////////////////////////////////////////////
#define K_S_FMANAGER_COPYING                              "Копіювання..."
#define K_S_FMANAGER_MD5_CALC                             "Обчислення МD5"
#define K_S_FMANAGER_LOADING                              "Завантаження"
#define K_S_FMANAGER_ENTER_NEW_FOLDER_NAME                "Введіть назву нової папки"
#define K_S_FMANAGER_ENTER_NEW_NAME                       "Введіть нову назву"
#define K_S_FMANAGER_OPTIONS                              "Опції"
#define K_S_FMANAGER_OPEN                                 "Відкрити"
#define K_S_FMANAGER_OPEN_WITH                            "Відкрити з"
#define K_S_FMANAGER_CREATE_FOLDER                        "Створити папку"
#define K_S_FMANAGER_RENAME                               "Перейменувати"
#define K_S_FMANAGER_DELETE                               "Видалити"
#define K_S_FMANAGER_INFO_ABOUT_FILE                      "Інформація"
#define K_S_FMANAGER_SELECT_APP                           "Оберіть додаток"
#define K_S_FMANAGER_FILE_MANAGER                         "Файловий менеджер"
#define K_S_FMANAGER_NES_EMULATOR                         "Емулятор NES"
#define K_S_FMANAGER_FIRMWARE_LOADER                      "Завантажувач прошивок"
#define K_S_FMANAGER_LUA                                  "Lua"
#define K_S_FMANAGER_MJS                                  "mJS"
#define K_S_FMANAGER_LILTRACKER                           K_S_LAUNCHER_LILTRACKER
#define K_S_FMANAGER_MOD_PLAYER                           "Програвач MOD"
#define K_S_FMANAGER_TXT_VIEWER                           "Текстовий переглядач"
#define K_S_FMANAGER_ACTIONS_ON_SELECTED                  "Дії над вибраним"
#define K_S_FMANAGER_COPY_SELECTED                        "Копіювати вибране"
#define K_S_FMANAGER_MOVE_SELECTED                        "Перемістити вибране"
#define K_S_FMANAGER_DELETE_SELECTED                      "Видалити вибране"
#define K_S_FMANAGER_CLEAR_SELECTION                      "Очистити вибране"
#define K_S_FMANAGER_CALC_INTERRUPTED                     "Не обчислено"
#define K_S_FMANAGER_ARE_YOU_SURE_ALERT                   "Ви впевнені?"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE_FMT        "Ця операція видалить файл %s\nПродовжити: START\nВихід: B"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_COUNT_FILES_FMT "Ця операція видалить %d файлів\nПродовжити: START\nВихід: B"
#define K_S_FMANAGER_ABOUT_DIR_FMT \
    "Тип: директорія\n"            \
    "Шлях: %s"

#define K_S_FMANAGER_ABOUT_FILE_FMT \
    "Тип: файл\n"                   \
    "Розмір: %s\n" \ 
    "MD5: %s\n"
// TODO: MOVE MULTIBOOT TO SEPARATE APP
#define K_S_FMANAGER_MULTIBOOT_ABOUT_FMT            "%s\n\nРозмір: %s"

#define K_S_FMANAGER_MULTIBOOT_STARTING             "Починаємо..."
#define K_S_FMANAGER_MULTIBOOT_ERROR_FMT            "Етап: %d\nКод: %d"
#define K_S_FMANAGER_SORTING                        "Сортування..."
#define K_S_FMANAGER_ALMOST_DONE                    "Майже готово..."
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE      "Ця операція видалить файл\n"
#define K_S_FMANAGER_NOT_ENOUGH_MEMORY_TO_FINISH_OP "Недостатньо пам'яті аби завершити операіцю"
#define K_S_FMANAGER_SELECTED_FILES_FMT             "Вибрано %d файл(ів)"
#define K_S_FMANAGER_CANT_DO_OP                     "Не можу виконати операцію"
#define K_S_FMANAGER_FILE_ADDED_TO_BUFFER_EXCHANGE  "Файл додано в буфер обміну"
///////////////////////////////////////////////////////////////////////////////////////////////////////
// apps/liltracker/liltracker.cpp ////////////////////////////////////////////
#define K_S_LILTRACKER_SAVE_TRACK             "Зберегти трек"
#define K_S_LILTRACKER_OPEN_TRACK             "Відкрити трек"
#define K_S_LILTRACKER_CREATE_NEW_TRACK       "++ Створити новий"
#define K_S_LILTRACKER_ENTER_FILENAME         "Введіть назву файлу"
#define K_S_LILTRACKER_FILENAME_CANT_BE_EMPTY "Назва файлу не може бути порожньою"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/lilcatalog/lilcatalog.cpp /////////////////////////////////////////////////////////////////////
#define K_S_LILCATALOG_LANGUAGE                      "uk"
#define K_S_LILCATALOG_APP                           "ЛілКаталог"

#define K_S_LILCATALOG_FETCH_CATALOG                 "Оновити каталог"
#define K_S_LILCATALOG_STOP                          "Вихід"
#define K_S_LILCATALOG_BACK                          "Назад"
#define K_S_LILCATALOG_EMPTY                         ""

#define K_S_LILCATALOG_START                         "Запустити"
#define K_S_LILCATALOG_INSTALL                       "Встановити"
#define K_S_LILCATALOG_REMOVE                        "Видалити"
#define K_S_LILCATALOG_UPDATE                        "Оновити"
#define K_S_LILCATALOG_ENTRY_DESCRIPTION             "Опис"
#define K_S_LILCATALOG_LOADING                       "Завантаження"
#define K_S_LILCATALOG_STARTING                      "Починаємо..."
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_NAME        "Назва: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_AUTHOR      "Автор: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_DESCRIPTION "Опис: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_FILES       "Файли: "

#define K_S_LILCATALOG_CATEGORY_POSTFIX              " елементів"
#define K_S_LILCATALOG_SIZE                          "Розмір:"
#define K_S_LILCATALOG_FILE_LOADING                  "Завантаження файлу..."
#define K_S_LILCATALOG_FILE_LOADING_COMPLETE         "Файл завантажено, та збережено"

#define K_S_LILCATALOG_ERROR_CREATE_FOLDER           "Помилка створення каталогу"
#define K_S_LILCATALOG_ERROR_NO_CATALOG              "Каталог не знайдено. Завантажте його з інтернету"
#define K_S_LILCATALOG_ERROR_LOAD_CATALOG            "Помилка завантаження каталогу"
#define K_S_LILCATALOG_ERROR_FILE_OPEN               "Помилка відкриття файлу"
#define K_S_LILCATALOG_ERROR_CONNECTION              "Помилка підключення:"
#define K_S_LILCATALOG_ERROR_DIRETORY_CREATE         "Помилка створення директорії"
#define K_S_LILCATALOG_ERROR_STAGE1                  "Етап: 1\nКод: "
#define K_S_LILCATALOG_ERROR_STAGE2                  "Етап: 2\nКод: "
#define K_S_LILCATALOG_ERROR_STAGE3                  "Етап: 3\nКод: "
#define K_S_LILCATALOG_SD_NOTFOUND                   "SD карта не знайдена. Неможливо продовжити"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/letris.cpp //////////////////////////////////////////////////////////////////////////////
#define K_S_LETRIS_GAME_OVER      "Game over"
#define K_S_LETRIS_GAME_OVER_LONG "Гру завершено!\nТи намагався. :)"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/settings/sound.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SETTINGS_SOUND                "Звук"
#define K_S_SETTINGS_SOUND_VOLUME         "Гучність:"
#define K_S_SETTINGS_SOUND_STARTRUP       "Звук вітання:"
#define K_S_SETTINGS_SOUND_BUZZER_STARTUP "Вітання бузером:"
#define K_S_SETTINGS_SOUND_SAVE           "<< Зберегти"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/screenshot.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SCREENSHOT_SAVED      "Скріншот збережено"
#define K_S_SCREENSHOT_SAVE_ERROR "Помилка збереження скріншоту"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/network.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_NET_OFFLINE "WiFi втрачено"
#define K_S_NET_ONLINE  "Приєднано до WiFi"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/combo.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_COMBO_FOR_EXIT_HOLD            "Для виходу затисніть"
#define K_S_COMBO_HOLD_SELECT_FOR_TIME_FMT "[SELECT] на %d сек"
#define K_S_COMBO_PRESS_START              "Натисніть [START]"
#define K_S_COMBO_TO_CONTINUE              "для продовження"
#define K_S_COMBO_PRESS                    "Натисніть"
#define K_S_COMBO_HOLD_ANY                 "Затисніть будь-яку"
#define K_S_COMBO_BUTTON_FOR_ONE_SEC       "кнопку на 1 сек"
#define K_S_COMBO_PRESS_TOGETHER           "Натисніть разом"
#define K_S_COMBO_TIMEOUT_FMT              "Таймаут %d мс"
#define K_S_COMBO_END                      "Кінець!"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/weather/weather.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_WEATHER_CLEAR_SKY                     "Чисте небо" // 0

#define K_S_WEATHER_MOSTLY_CLEAR                  "Переважно ясно" // 1
#define K_S_WEATHER_PARTLY_CLOUDY                 "Частково хмарно" // 2
#define K_S_WEATHER_OVERCAST                      "Хмарно" //3

#define K_S_WEATHER_FOG                           "Туман" //45
#define K_S_WEATHER_FREEZING_FOG                  "Паморозь" //48

#define K_S_WEATHER_LIGHT_DRIZZLE                 "Легка мжичка" //51
#define K_S_WEATHER_DRIZZLE                       "Мжичка" //53
#define K_S_WEATHER_HEAVY_DRIZZLE                 "Сильна мжичка" //55

#define K_S_WEATHER_LIGHT_FREEZING_DRIZZLE        "Легка мжичка" //56
#define K_S_WEATHER_HEAVY_FREEZING_DRIZZLE        "Сильна мжичка" //57

#define K_S_WEATHER_LIGHT_RAIN                    "Легкий дощ" //61
#define K_S_WEATHER_RAIN                          "Дощ" //63
#define K_S_WEATHER_HEAVY_RAIN                    "Сильний дощ" // 65

#define K_S_WEATHER_LIGHT_FREEZING_RAIN           "Дощ" // 66
#define K_S_WEATHER_HEAVY_FREEZING_RAIN           "Сильний дощ" //67

#define K_S_WEATHER_LIGHT_SNOW                    "Легкий сніг" // 71
#define K_S_WEATHER_SNOW                          "Сніг" // 73
#define K_S_WEATHER_HEAVY_SNOW                    "Сильний сніг" // 75

#define K_S_WEATHER_SNOW_GRAINS                   "Сніжинки" // 77

#define K_S_WEATHER_LIGHT_SHOWERS                 "Легка злива" // 80
#define K_S_WEATHER_SHOWERS                       "Злива" // 81
#define K_S_WEATHER_HEAVY_SHOWERS                 "Сильна злива" // 82

#define K_S_WEATHER_RAIN_AND_SNOW                 "Дощ зі снігом" // 85
#define K_S_WEATHER_HEAVY_RAIN_AND_SNOW           "Сильний дощ зі снігом" // 86

#define K_S_WEATHER_THUNDERSTORM                  "Легка гроза" // 95
#define K_S_WEATHER_THUNDERSTORM_WITH_HAIL        "Гроза" // 96
#define K_S_WEATHER_SEVERE_THUNDERSTORM_WITH_HAIL "Сильна гроза" // 99

#define K_S_WEATHER_LOADING_DATA                  "Отримання даних..."
#define K_S_WEATHER_DATA_PATTERN_ERROR            "Помилка десеріалізації"
#define K_S_WEATHER_DATA_LOAD_ERROR_FMT           "Помилка отримання даних:\nКод відповіді: %d"

#define K_S_WEATHER_LOCATION_NOT_SET              "Локацію не налаштовано"
#define K_S_WEATHER_SELECT_TO_SETUP               "[SELECT] - налаштування"
#define K_S_WEATHER_A_TO_EXIT                     "[A] - вихід" // LOL WHY
#define K_S_WEATHER_TEMP_FMT                      "%.1f °C"
#define K_S_WEATHER_WIND_SPEED_FMT                "%.1f км/год"

#define K_S_WEATHER_LATITUDE                      "Широта"
#define K_S_WEATHER_LATITUDE_S                    "широту"
#define K_S_WEATHER_LONGITUDE                     "Довгота"
#define K_S_WEATHER_LONGITUDE_S                   "довготу"

#define K_S_WEATHER_INPUT                         "Введіть"

#define K_S_WEATHER_SAVE                          "Зберегти"
#define K_S_WEATHER_CANCEL                        "Скасувати"

///////////////////////////////////////////////////////////////////////////////////////////////////////
// clang-format on