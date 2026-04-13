#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
// GUIDELINE: Keira all strings used in messages, or somehow written to serial, should be defined here

// clang-format off

// Multi purpose strings  /////////////////////////////////////////////////////////////////////////////
#define K_S_CURRENT_LANGUAGE_SHORT      "pl"
#define K_S_CURRENT_LANGUAGE_FULL       "Polski"

#define K_S_ERROR                       "Błąd"
#define K_S_ATTENTION                   "Uwaga"
#define K_S_SUCCESS                     "Sukces"
#define K_S_PLEASE_STANDBY              "Proszę czekać..."
#define K_S_LILKA_V2_OR_HIGHER_REQUIRED "Ten program wymaga Lilka V2 lub nowszej"
#define K_S_FEATURE_IN_DEVELOPMENT      "Ta funkcja jest w trakcie rozwoju"
#define K_S_CANT_OPEN_DIR_FMT           "Nie można otworzyć katalogu %s"
#define K_S_CANT_OPEN_FILE_FMT          "Nie można otworzyć pliku %s"
#define K_S_CANT_CREATE_DIR_FMT         "Nie można utworzyć katalogu %s"
#define K_S_CANT_REMOVE_FILE_FMT        "Nie można usunąć pliku %s"
#define K_S_DIR_EMPTY_FMT               "Katalog %s jest pusty"

#define K_S_MENU_BACK                   "<< Wstecz"

#define K_S_PASSWORD                    "Hasło"
#define K_S_USER                        "Użytkownik"

#define K_S_SETTINGS                    "Ustawienia"
#define K_S_ON                          "ON"
#define K_S_OFF                         "OFF"
#define K_S_STATUS                      "Status"

#define K_S_SERVICES                    "Usługi"
#define K_S_FTP                         "FTP"
#define K_S_WEB                         "Web"
#define K_S_TELNET                      "Telnet"
#define K_S_MDNS                        "mDNS"
#define K_S_LAUNCHER_MDNS               K_S_MDNS
#define K_S_LAUNCHER_MDNS_HOSTNAME      "Nazwa hosta"
#define K_S_LAUNCHER_MDNS_ENTER_HOSTNAME "Wprowadź nazwę hosta:"
#define K_S_CHANGE_ON_NEXT_BOOT         "Zmiany zostaną zastosowane po ponownym uruchomieniu"
#define K_S_OS_NAME                     "Keira OS"
#define K_S_OS_DESCRIPTION              "by Anderson & friends"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// service.cpp  ///////////////////////////////////////////////////////////////////////////////////////
#define K_S_SERVICE_STARTUP_FMT "Starting service %s"
#define K_S_SERVICE_DIE_FMT     "Service %s died"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/launcher.cpp //////////////////////////////////////////////////////////////////////////////////
#define K_S_LAUNCHER_MAIN_MENU         "Menu główne"
#define K_S_LAUNCHER_APPS              "Aplikacje"
#define K_S_LAUNCHER_DEMOS             "Demo"
#define K_S_LAUNCHER_LINES             "Linie"
#define K_S_LAUNCHER_DISK              "Dysk"
#define K_S_LAUNCHER_TRANSFORM         "Transformacja"
#define K_S_LAUNCHER_BALL              "Piłka"
#define K_S_LAUNCHER_CUBE              "Sześcian"
#define K_S_LAUNCHER_EPILEPSY          "Epilepsja"
#define K_S_LAUNCHER_PET_PET           "PetPet"
#define K_S_LAUNCHER_TESTS             "Testy"
#define K_S_LAUNCHER_KEYBOARD          "Klawiatura"
#define K_S_LAUNCHER_SPI_TEST          "Test SPI"
#define K_S_LAUNCHER_I2C_SCANNER       "I2C-skaner"
#define K_S_LAUNCHER_GPIO_MANAGER      "GPIO-menedżer"
#define K_S_LAUNCHER_COMBO             "Combo" // wtf?
#define K_S_LAUNCHER_CALLBACK_TEST     "CallbackTest"
#define K_S_LAUNCHER_LILCATALOG        "LilKatalog"
#define K_S_LAUNCHER_LILTRACKER        "LilTracker"
#define K_S_LAUNCHER_LETRIS            "Letris"
#define K_S_LAUNCHER_TAMAGOTCHI        "Tamagotchi"
#define K_S_LAUNCHER_WEATHER           "Pogoda"
#define K_S_LAUNCHER_PASTEBIN          "Pastebin"
#define K_S_LAUNCHER_FMANAGER          "Menedżer plików"
#define K_S_LAUNCHER_DEV_MENU          "Rozwój"
#define K_S_LAUNCHER_LIVE_LUA          "Live Lua" // Lua live?
#define K_S_LAUNCHER_LUA_REPL          "Lua REPL"
#define K_S_LAUNCHER_WIFI              "WiFi"
#define K_S_LAUNCHER_WIFI_ADAPTER      "WiFi"
#define K_S_LAUNCHER_WIFI_NETWORKS     "Sieci WiFi"
#define K_S_LAUNCHER_WIFI_TX_POWER     "Moc WiFi"
#define K_S_LAUNCHER_SD                "SD"
#define K_S_LAUNCHER_SD_FORMAT         "Formatuj SD"
#define K_S_LAUNCHER_SD_SPEED          "Częstotliwość karty SD"
#define K_S_LAUNCHER_SOUND             "Dźwięk"
#define K_S_LAUNCHER_SERVICES          K_S_SERVICES
#define K_S_LAUNCHER_WEB               K_S_WEB
#define K_S_LAUNCHER_TELNET            K_S_TELNET
#define K_S_LAUNCHER_FTP               K_S_FTP
#define K_S_LAUNCHER_FTP_USER          K_S_USER
#define K_S_LAUNCHER_FTP_PASSWORD      K_S_PASSWORD
#define K_S_LAUNCHER_FTP_IP            "IP"
#define K_S_LAUNCHER_STATUSBAR         "Pasek stanu"
#define K_S_LAUNCHER_CLOCK             "Zegar"
#define K_S_LAUNCHER_CLOCK_0           "Brak"
#define K_S_LAUNCHER_CLOCK_1           "HH:MM:SS"
#define K_S_LAUNCHER_CLOCK_2           "HH:MM"
#define K_S_LAUNCHER_CLOCK_3           "*HH:MM"
#define K_S_LAUNCHER_MEM               "Pamięć"
#define K_S_LAUNCHER_MEM_0             "Brak"
#define K_S_LAUNCHER_MEM_1             "Ikona"
#define K_S_LAUNCHER_MEM_2             "Tekst"
#define K_S_LAUNCHER_NETWORK           K_S_LAUNCHER_WIFI
#define K_S_LAUNCHER_NETWORK_0         "Brak"
#define K_S_LAUNCHER_NETWORK_1         "Ikona"
#define K_S_LAUNCHER_BATTERY           "Bateria"
#define K_S_LAUNCHER_BATTERY_0         "Brak"
#define K_S_LAUNCHER_BATTERY_1         "Ikona + tekst"
#define K_S_LAUNCHER_BATTERY_2         "Ikona"
#define K_S_LAUNCHER_BATTERY_3         "Tekst"

#define K_S_LAUNCHER_ABOUT_SYSTEM      "O systemie"
#define K_S_LAUNCHER_DEVICE_INFO       "O urządzeniu"
#define K_S_PARTITION_TABLE            "Tablica partycji"
#define K_S_LAUNCHER_LIGHT_SLEEP       "Light sleep"
#define K_S_LAUNCHER_DEEP_SLEEP        "Deep sleep"
#define K_S_LAUNCHER_REBOOT            "Restart"

#define K_S_LAUNCHER_SELECT_TX_POWER   "Ustaw moc"

#define K_S_LAUNCHER_ENABLE_WIFI_FIRST "Najpierw włącz WiFi"

#define K_S_LAUNCHER_DEVICE_INFO_FMT \
    "Model: %s\n"                    \
    "Rewizja: %d\n"                  \
    "Wersja ESP-IDF: %s\n"          \
    "Częstotliwość: %d MHz\n"        \
    "Liczba rdzeni: %d\n"            \
    "IP: %s"

#define K_S_LAUNCHER_PARTITION_FMT "Adres: 0x%s\nRozmiar: 0x%s"

#define K_S_LAUNCHER_FORMAT        "Formatowanie"

#define K_S_LAUNCHER_FORMAT_DISCLAIMER_ALERT        \
    "UWAGA: To usunie wszystkie dane z karty SD!\n" \
    "\nKontynuować?\n\nSTART - kontynuuj\nA - wyjdź"

#define K_S_LAUNCHER_PLEASE_STANDBY K_S_PLEASE_STANDBY

#define K_S_LAUNCHER_FORMAT_ERROR_ALERT      \
    "Nie udało się sformatować karty SD.\n\n" \
    "System zostanie uruchomiony ponownie."

#define K_S_LAUNCHER_FORMAT_SUCCESS_ALLERT        \
    "Formatowanie karty SD zakończone!\n\n"       \
    "System zostanie uruchomiony ponownie."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/wifi_config.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_WIFI_CONFIG_SCANING_NETWORKS    "Skanowanie sieci WiFi..."
#define K_S_WIFI_CONFIG_SCAN_ERROR_CODE_FMT "Nie udało się przeskanować sieci, kod błędu: %d"
#define K_S_WIFI_CONFIG_NETWORKS            "Sieci"

#define K_S_WIFI_CONFIG_ABOUT_NETWORK_FMT \
    "Kanał: %d\n"                         \
    "Siła sygnału: %ddb\n"               \
    "MAC: %s\n"                           \
    "Zabezpieczenia: %s"

#define K_S_WIFI_CONFIG_CONNECTING_TO_OPEN_INSECURE_NETWORK_FMT \
    "Łączysz się z niezabezpieczoną siecią %s\n\n"              \
    "A - kontynuuj\n"                                            \
    "B - wybierz inną sieć"

#define K_S_WIFI_CONFIG_ENTER_PASSWORD              "Wprowadź hasło:"
#define K_S_WIFI_CONFIG_CONNECTING                  "Łączenie..."
#define K_S_WIFI_CONFIG_CONNECTED_TO_NETWORK_FMT    "Połączono z %s"
#define K_S_WIFI_CONFIG_CANT_CONNECT_TO_NETWORK_FMT "Nie udało się połączyć z %s"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/scan_i2c.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_I2C_INIT_ABOUT            "I2C init: SDA=%d, SCL=%d"
#define K_S_I2C_SCANNER_SCAN_START    "Starting I2C scan..."
#define K_S_I2C_SCANNER_SCAN_DONE     "I2C scan done."
#define K_S_I2C_SCANNER_DEVICES_FOUND "Found %d devices."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/transform.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_TRANSFORM_CANT_LOAD_FACE     "Nie można załadować face.bmp z karty SD." // FACEPALM.BMP
#define K_S_TRANFORM_DRAWING_FACE_AT_FMT "Drawing face at %d, %d"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/fmanager.cpp/.h ///////////////////////////////////////////////////////////////////////////////
#define K_S_FMANAGER_COPYING                              "Kopiowanie..."
#define K_S_FMANAGER_MD5_CALC                             "Obliczanie MD5"
#define K_S_FMANAGER_LOADING                              "Ładowanie"
#define K_S_FMANAGER_ENTER_NEW_FOLDER_NAME                "Wprowadź nazwę nowego folderu"
#define K_S_FMANAGER_ENTER_NEW_NAME                       "Wprowadź nową nazwę"
#define K_S_FMANAGER_OPTIONS                              "Opcje"
#define K_S_FMANAGER_OPEN                                 "Otwórz"
#define K_S_FMANAGER_OPEN_WITH                            "Otwórz za pomocą"
#define K_S_FMANAGER_CREATE_FOLDER                        "Utwórz folder"
#define K_S_FMANAGER_RENAME                               "Zmień nazwę"
#define K_S_FMANAGER_DELETE                                "Usuń"
#define K_S_FMANAGER_INFO_ABOUT_FILE                      "Informacje"
#define K_S_FMANAGER_SELECT_APP                           "Wybierz aplikację"
#define K_S_FMANAGER_FILE_MANAGER                         "Menedżer plików"
#define K_S_FMANAGER_NES_EMULATOR                         "Emulator NES"
#define K_S_FMANAGER_FIRMWARE_LOADER                      "Ładowanie firmware"
#define K_S_FMANAGER_LUA                                  "Lua"
#define K_S_FMANAGER_MJS                                  "mJS"
#define K_S_FMANAGER_LILTRACKER                           K_S_LAUNCHER_LILTRACKER
#define K_S_FMANAGER_MAD_PLAYER                           "MadPlayer"
#define K_S_FMANAGER_DYNAPP                               "DynApp (.so)"
#define K_S_FMANAGER_ACTIONS_ON_SELECTED                  "Działania na wybranych"
#define K_S_FMANAGER_COPY_SELECTED                        "Kopiuj wybrane"
#define K_S_FMANAGER_MOVE_SELECTED                        "Przenieś wybrane"
#define K_S_FMANAGER_DELETE_SELECTED                      "Usuń wybrane"
#define K_S_FMANAGER_CLEAR_SELECTION                      "Wyczyść zaznaczenie"
#define K_S_FMANAGER_CALC_INTERRUPTED                     "Nie obliczono"
#define K_S_FMANAGER_ARE_YOU_SURE_ALERT                   "Czy na pewno?"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE_FMT        "Ta operacja usunie plik %s\nKontynuuj: START\nWyjście: B"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_COUNT_FILES_FMT "Ta operacja usunie %d plik(ów)\nKontynuuj: START\nWyjście: B"
#define K_S_FMANAGER_SELECTED_ENTRIES_EXIT_FMT "Wybrano %d plików\nPotwierdź wyjście: START\nPowrót: B"
#define K_S_FMANAGER_ABOUT_DIR_FMT \
    "Typ: katalog\n"               \
    "Ścieżka: %s"

#define K_S_FMANAGER_ABOUT_FILE_FMT \
    "Typ: plik\n"                    \
    "Rozmiar: %s\n"                  \
    "MD5: %s\n"
// TODO: MOVE MULTIBOOT TO SEPARATE APP
#define K_S_FMANAGER_MULTIBOOT_ABOUT_FMT            "%s\n\nRozmiar: %s"

#define K_S_FMANAGER_MULTIBOOT_STARTING             "Uruchamianie..."
#define K_S_FMANAGER_MULTIBOOT_ERROR_FMT            "Etap: %d\nKod: %d"
#define K_S_FMANAGER_SORTING                        "Sortowanie..."
#define K_S_FMANAGER_ALMOST_DONE                    "Prawie gotowe..."
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE      "Ta operacja usunie plik\n"
#define K_S_FMANAGER_NOT_ENOUGH_MEMORY_TO_FINISH_OP "Za mało pamięci, aby zakończyć operację"
#define K_S_FMANAGER_SELECTED_FILES_FMT             "Wybrano %d plik(ów)"
#define K_S_FMANAGER_CANT_DO_OP                     "Nie można wykonać operacji"
#define K_S_FMANAGER_FILE_ADDED_TO_BUFFER_EXCHANGE  "Plik dodany do schowka"
///////////////////////////////////////////////////////////////////////////////////////////////////////
// apps/liltracker/liltracker.cpp ////////////////////////////////////////////
#define K_S_LILTRACKER_SAVE_TRACK             "Zapisz utwór"
#define K_S_LILTRACKER_OPEN_TRACK             "Otwórz utwór"
#define K_S_LILTRACKER_CREATE_NEW_TRACK       "++ Utwórz nowy"
#define K_S_LILTRACKER_ENTER_FILENAME         "Wprowadź nazwę pliku"
#define K_S_LILTRACKER_FILENAME_CANT_BE_EMPTY "Nazwa pliku nie może być pusta"
#define K_S_LILTRACKER_CLEAR_TRACK_DATA       "Wyczyścić wszystkie dane\nutworu?\n[START]Tak\n[B]Nie"
#define K_S_LILTRACKER_OPEN                   "Otwórz"
#define K_S_LILTRACKER_SAVE                   "Zapisz"
#define K_S_LILTRACKER_RESET                  "Resetuj"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/lilcatalog/lilcatalog.cpp /////////////////////////////////////////////////////////////////////
#define K_S_LILCATALOG_APP                           "LilKatalog"

#define K_S_LILCATALOG_APPS                          "Aplikacje"
#define K_S_LILCATALOG_MODS                          "Mody"
#define K_S_LILCATALOG_STOP                          "Wyjście"
#define K_S_LILCATALOG_BACK                          "Wstecz"
#define K_S_LILCATALOG_EMPTY                         ""

#define K_S_LILCATALOG_START                         "Uruchom"
#define K_S_LILCATALOG_INSTALL                       "Zainstaluj"
#define K_S_LILCATALOG_REMOVE                        "Usuń"
#define K_S_LILCATALOG_UPDATE                        "Aktualizuj"
#define K_S_LILCATALOG_SOURCE                        "Źródło"
#define K_S_LILCATALOG_ENTRY_DESCRIPTION             "Opis"
#define K_S_LILCATALOG_LOADING                       "Pobieranie"
#define K_S_LILCATALOG_LOADING_CATALOG               "Ładowanie katalogu..."
#define K_S_LILCATALOG_STARTING                      "Uruchamianie..."
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_NAME        "Nazwa: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_AUTHOR      "Autor: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_DESCRIPTION "Opis: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_FILE        "Plik: "

#define K_S_LILCATALOG_NEXT_PAGE                     ">> Następna strona"
#define K_S_LILCATALOG_PREV_PAGE                     "<< Poprzednia strona"
#define K_S_LILCATALOG_SIZE                          "Rozmiar:"
#define K_S_LILCATALOG_FILE_LOADING                  "Pobieranie pliku..."
#define K_S_LILCATALOG_FILE_LOADING_COMPLETE         "Plik pobrany i zapisany"

#define K_S_LILCATALOG_ARCHIVE_NOTICE                "Archiwa należy rozpakować ręcznie"
#define K_S_LILCATALOG_UNSUPPORTED_TYPE              "Nieobsługiwany typ pliku"
#define K_S_LILCATALOG_CLEAR_CACHE                   "Wyczyść pamięć podręczną"
#define K_S_LILCATALOG_CACHE_CLEARED                 "Pamięć podręczna wyczyszczona"
#define K_S_LILCATALOG_INSTALLED                     "Zainstalowane"
#define K_S_LILCATALOG_NO_INSTALLED                  "Brak zainstalowanych aplikacji"
#define K_S_LILCATALOG_NO_BINARY                     "Ta aplikacja nie ma plików do pobrania"
#define K_S_LILCATALOG_VIEW_SOURCE                   "Pokaż źródło"

#define K_S_LILCATALOG_ERROR_CREATE_FOLDER           "Błąd tworzenia folderu"
#define K_S_LILCATALOG_ERROR_LOAD_CATALOG            "Błąd ładowania katalogu"
#define K_S_LILCATALOG_ERROR_FILE_OPEN               "Błąd otwierania pliku"
#define K_S_LILCATALOG_ERROR_CONNECTION              "Błąd połączenia: "
#define K_S_LILCATALOG_ERROR_DIRETORY_CREATE         "Błąd tworzenia katalogu"
#define K_S_LILCATALOG_ERROR_STAGE1                  "Etap: 1\nKod: "
#define K_S_LILCATALOG_ERROR_STAGE2                  "Etap: 2\nKod: "
#define K_S_LILCATALOG_ERROR_STAGE3                  "Etap: 3\nKod: "
#define K_S_LILCATALOG_SD_NOTFOUND                   "Nie znaleziono karty SD. Nie można kontynuować"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/letris.cpp //////////////////////////////////////////////////////////////////////////////
#define K_S_LETRIS_GAME_OVER      "Game over"
#define K_S_LETRIS_GAME_OVER_LONG "Koniec gry!\nPróbowałeś. :)"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/settings/sound.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SETTINGS_SOUND                "Dźwięk"
#define K_S_SETTINGS_SOUND_VOLUME         "Głośność:"
#define K_S_SETTINGS_SOUND_STARTRUP       "Dźwięk startowy:"
#define K_S_SETTINGS_SOUND_BUZZER_STARTUP "Buzzer przy starcie:"
#define K_S_SETTINGS_SOUND_SAVE           "<< Zapisz"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/screenshot.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SCREENSHOT_SAVED      "Zrzut ekranu zapisany"
#define K_S_SCREENSHOT_SAVE_ERROR "Błąd zapisu zrzutu ekranu"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/network.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_NET_OFFLINE "Utracono połączenie WiFi"
#define K_S_NET_ONLINE  "Połączono z WiFi"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/combo.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_COMBO_FOR_EXIT_HOLD            "Aby wyjść, przytrzymaj"
#define K_S_COMBO_HOLD_SELECT_FOR_TIME_FMT "[SELECT] przez %d s"
#define K_S_COMBO_PRESS_START              "Naciśnij [START]"
#define K_S_COMBO_TO_CONTINUE              "aby kontynuować"
#define K_S_COMBO_PRESS                    "Naciśnij"
#define K_S_COMBO_HOLD_ANY                 "Przytrzymaj dowolny"
#define K_S_COMBO_BUTTON_FOR_ONE_SEC       "przycisk przez 1 s"
#define K_S_COMBO_PRESS_TOGETHER           "Naciśnij razem"
#define K_S_COMBO_TIMEOUT_FMT              "Limit czasu %d ms"
#define K_S_COMBO_END                      "Koniec!"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/weather/weather.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_WEATHER_CLEAR_SKY                     "Bezchmurnie" // 0

#define K_S_WEATHER_MOSTLY_CLEAR                  "Przeważnie bezchmurnie" // 1
#define K_S_WEATHER_PARTLY_CLOUDY                 "Częściowe zachmurzenie" // 2
#define K_S_WEATHER_OVERCAST                      "Pochmurno" //3

#define K_S_WEATHER_FOG                           "Mgła" //45
#define K_S_WEATHER_FREEZING_FOG                  "Marznąca mgła" //48

#define K_S_WEATHER_LIGHT_DRIZZLE                 "Lekka mżawka" //51
#define K_S_WEATHER_DRIZZLE                       "Mżawka" //53
#define K_S_WEATHER_HEAVY_DRIZZLE                 "Silna mżawka" //55

#define K_S_WEATHER_LIGHT_FREEZING_DRIZZLE        "Lekka marznąca mżawka" //56
#define K_S_WEATHER_HEAVY_FREEZING_DRIZZLE        "Silna marznąca mżawka" //57

#define K_S_WEATHER_LIGHT_RAIN                    "Lekki deszcz" //61
#define K_S_WEATHER_RAIN                          "Deszcz" //63
#define K_S_WEATHER_HEAVY_RAIN                    "Ulewny deszcz" // 65

#define K_S_WEATHER_LIGHT_FREEZING_RAIN           "Lekki marznący deszcz" // 66
#define K_S_WEATHER_HEAVY_FREEZING_RAIN           "Silny marznący deszcz" //67

#define K_S_WEATHER_LIGHT_SNOW                    "Lekki śnieg" // 71
#define K_S_WEATHER_SNOW                          "Śnieg" // 73
#define K_S_WEATHER_HEAVY_SNOW                    "Obfity śnieg" // 75

#define K_S_WEATHER_SNOW_GRAINS                   "Ziarna śniegu" // 77

#define K_S_WEATHER_LIGHT_SHOWERS                 "Lekkie opady" // 80
#define K_S_WEATHER_SHOWERS                       "Opady" // 81
#define K_S_WEATHER_HEAVY_SHOWERS                 "Silne opady" // 82

#define K_S_WEATHER_RAIN_AND_SNOW                 "Deszcz ze śniegiem" // 85
#define K_S_WEATHER_HEAVY_RAIN_AND_SNOW           "Silny deszcz ze śniegiem" // 86

#define K_S_WEATHER_THUNDERSTORM                  "Burza" // 95
#define K_S_WEATHER_THUNDERSTORM_WITH_HAIL        "Burza z gradem" // 96
#define K_S_WEATHER_SEVERE_THUNDERSTORM_WITH_HAIL "Silna burza z gradem" // 99

#define K_S_WEATHER_LOADING_DATA                  "Ładowanie danych..."
#define K_S_WEATHER_DATA_PATTERN_ERROR            "Błąd formatu danych"
#define K_S_WEATHER_DATA_LOAD_ERROR_FMT           "Błąd ładowania danych:\nKod statusu: %d"

#define K_S_WEATHER_LOCATION_NOT_SET              "Lokalizacja nie ustawiona"
#define K_S_WEATHER_SELECT_TO_SETUP               "[SELECT] - ustawienia"
#define K_S_WEATHER_A_TO_EXIT                     "[A] - wyjście" // LOL WHY
#define K_S_WEATHER_TEMP_FMT                      "%.1f °C"
#define K_S_WEATHER_WIND_SPEED_FMT                "%.1f km/h"

#define K_S_WEATHER_LATITUDE                      "Szerokość"
#define K_S_WEATHER_LATITUDE_S                    "szerokość"
#define K_S_WEATHER_LONGITUDE                     "Długość"
#define K_S_WEATHER_LONGITUDE_S                   "długość"

#define K_S_WEATHER_INPUT                         "Wprowadź"

#define K_S_WEATHER_SAVE                          "Zapisz"
#define K_S_WEATHER_CANCEL                        "Anuluj"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/usbdrive/usbdrive.cpp /////////////////////////////////////////////////////////////////////////
#define K_S_USB_DRIVE_TITLE                       "Dysk USB"
#define K_S_USB_DRIVE_EXPERIMENTAL_TITLE          "Uwaga"
#define K_S_USB_DRIVE_EXPERIMENTAL_WARNING        "Funkcja eksperymentalna.\n" \
                                                  "Serial zostanie wyłączony.\n\n" \
                                                  "[START] - OK  [A] - Anuluj"
#define K_S_USB_DRIVE_INITIALIZING                "Inicjalizacja USB..."
#define K_S_USB_DRIVE_INIT_ERROR                  "Błąd inicjalizacji USB"
#define K_S_USB_DRIVE_NO_SD                       "Karta SD niedostępna"
#define K_S_USB_DRIVE_CONNECT_USB                 "Podłącz kabel USB do komputera"
#define K_S_USB_DRIVE_CONNECTED                   "Połączono z komputerem"
#define K_S_USB_DRIVE_EJECTED                     "Bezpiecznie wysunięto"
#define K_S_USB_DRIVE_PC_INSTRUCTION              "Karta SD dostępna na komputerze"
#define K_S_USB_DRIVE_SAFE_EJECT                  "Wysuń dysk przed wyjściem"
#define K_S_USB_DRIVE_PRESS_A_TO_EXIT             "[A] - wyjście"
#define K_S_USB_DRIVE_DISCONNECTING               "Rozłączanie..."
#define K_S_USB_DRIVE_REBOOT_REQUIRED             "Urządzenie zostanie uruchomione ponownie..."
#define K_S_USB_DRIVE_NOT_EJECTED                 "Nie wysunięto bezpiecznie!"
#define K_S_USB_DRIVE_EJECT_WARNING               "Dane mogą być uszkodzone"
#define K_S_USB_DRIVE_PRESS_START_CONTINUE        "[START] - kontynuuj"
#define K_S_USB_DRIVE_PRESS_B_CANCEL              "[B] - anuluj"
#define K_S_LAUNCHER_USB_DRIVE                    "Dysk USB"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/pastebin/pastebinApp.cpp //////////////////////////////////////////////////////////////////////
#define K_S_PASTEBIN_CODE                         "Kod"
#define K_S_PASTEBIN_NAME                         "Nazwa"
#define K_S_PASTEBIN_DOWNLOAD                     "Pobierz"
#define K_S_PASTEBIN_ENTER_CODE                   "Wprowadź kod"
#define K_S_PASTEBIN_ENTER_NAME                   "Wprowadź nazwę"
#define K_S_PASTEBIN_ERROR_CREATE_DIR             "Błąd tworzenia katalogu"
#define K_S_PASTEBIN_ERROR_OPEN_FILE              "Błąd otwierania pliku"
#define K_S_PASTEBIN_FILE_DOWNLOADED_FMT          "Plik pobrany do\n%s"
#define K_S_PASTEBIN_REQUEST_FAIL_FMT             "Błąd HTTP\n%s"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/mjs/mjsrunner.cpp /////////////////////////////////////////////////////////////////////////////
#define K_S_MJS_ERROR                             "Błąd: "
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/mjs/luarunner.cpp /////////////////////////////////////////////////////////////////////////////
#define K_S_LUA_ERROR                             "Błąd: "
#define K_S_LUA_AWAIT_CODE_FROM_UART              "Oczekiwanie na kod\nz UART...\n\nNaciśnij [A]\naby wyjść."
#define K_S_LUA_DOWNLOAD                          "Pobieranie..."
#define K_S_LUA_REPL_AWAIT_CODE_FROM_UART         "REPL:\nOczekiwanie na kod\nz UART...\n\nNaciśnij [A]\naby wyjść."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/tests/keyboard/keyboard.cpp //////////////////////////////////////////////////////////////////
#define K_S_KEYBOARD_ENTER_TEXT                    "Wpisz tekst: "
#define K_S_KEYBOARD_YOU_ENTERED                   "Wpisałeś:"
//////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format on
