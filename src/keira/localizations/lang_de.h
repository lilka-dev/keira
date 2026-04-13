#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
// GUIDELINE: Keira all strings used in messages, or somehow written to serial, should be defined here

// clang-format off

// Multi purpose strings  /////////////////////////////////////////////////////////////////////////////
#define K_S_CURRENT_LANGUAGE_SHORT      "de"
#define K_S_CURRENT_LANGUAGE_FULL       "Deutsch"

#define K_S_ERROR                       "Fehler"
#define K_S_ATTENTION                   "Achtung"
#define K_S_SUCCESS                     "Erfolg"
#define K_S_PLEASE_STANDBY              "Bitte warten..."
#define K_S_LILKA_V2_OR_HIGHER_REQUIRED "Dieses Programm erfordert Lilka V2 oder höher"
#define K_S_FEATURE_IN_DEVELOPMENT      "Diese Funktion ist in Entwicklung"
#define K_S_CANT_OPEN_DIR_FMT           "Verzeichnis %s kann nicht geöffnet werden"
#define K_S_CANT_OPEN_FILE_FMT          "Datei %s kann nicht geöffnet werden"
#define K_S_CANT_CREATE_DIR_FMT         "Verzeichnis %s kann nicht erstellt werden"
#define K_S_CANT_REMOVE_FILE_FMT        "Datei %s kann nicht gelöscht werden"
#define K_S_DIR_EMPTY_FMT               "Verzeichnis %s ist leer"

#define K_S_MENU_BACK                   "<< Zurück"

#define K_S_PASSWORD                    "Passwort"
#define K_S_USER                        "Benutzer"

#define K_S_SETTINGS                    "Einstellungen"
#define K_S_ON                          "ON"
#define K_S_OFF                         "OFF"
#define K_S_STATUS                      "Status"

#define K_S_SERVICES                    "Dienste"
#define K_S_FTP                         "FTP"
#define K_S_WEB                         "Web"
#define K_S_TELNET                      "Telnet"
#define K_S_MDNS                        "mDNS"
#define K_S_LAUNCHER_MDNS               K_S_MDNS
#define K_S_LAUNCHER_MDNS_HOSTNAME      "Hostname"
#define K_S_LAUNCHER_MDNS_ENTER_HOSTNAME "Hostname eingeben:"
#define K_S_CHANGE_ON_NEXT_BOOT         "Änderungen werden beim nächsten Start übernommen"
#define K_S_OS_NAME                     "Keira OS"
#define K_S_OS_DESCRIPTION              "by Anderson & friends"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// service.cpp  ///////////////////////////////////////////////////////////////////////////////////////
#define K_S_SERVICE_STARTUP_FMT "Starting service %s"
#define K_S_SERVICE_DIE_FMT     "Service %s died"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/launcher.cpp //////////////////////////////////////////////////////////////////////////////////
#define K_S_LAUNCHER_MAIN_MENU         "Hauptmenü"
#define K_S_LAUNCHER_APPS              "Anwendungen"
#define K_S_LAUNCHER_DEMOS             "Demo"
#define K_S_LAUNCHER_LINES             "Linien"
#define K_S_LAUNCHER_DISK              "Festplatte"
#define K_S_LAUNCHER_TRANSFORM         "Transformation"
#define K_S_LAUNCHER_BALL              "Ball"
#define K_S_LAUNCHER_CUBE              "Würfel"
#define K_S_LAUNCHER_EPILEPSY          "Epilepsie"
#define K_S_LAUNCHER_PET_PET           "PetPet"
#define K_S_LAUNCHER_TESTS             "Tests"
#define K_S_LAUNCHER_KEYBOARD          "Tastatur"
#define K_S_LAUNCHER_SPI_TEST          "SPI-Test"
#define K_S_LAUNCHER_I2C_SCANNER       "I2C-Scanner"
#define K_S_LAUNCHER_GPIO_MANAGER      "GPIO-Manager"
#define K_S_LAUNCHER_COMBO             "Combo" // wtf?
#define K_S_LAUNCHER_CALLBACK_TEST     "CallbackTest"
#define K_S_LAUNCHER_LILCATALOG        "LilKatalog"
#define K_S_LAUNCHER_LILTRACKER        "LilTracker"
#define K_S_LAUNCHER_LETRIS            "Letris"
#define K_S_LAUNCHER_TAMAGOTCHI        "Tamagotchi"
#define K_S_LAUNCHER_WEATHER           "Wetter"
#define K_S_LAUNCHER_PASTEBIN          "Pastebin"
#define K_S_LAUNCHER_FMANAGER          "Dateimanager"
#define K_S_LAUNCHER_DEV_MENU          "Entwicklung"
#define K_S_LAUNCHER_LIVE_LUA          "Live Lua" // Lua live?
#define K_S_LAUNCHER_LUA_REPL          "Lua REPL"
#define K_S_LAUNCHER_WIFI              "WiFi"
#define K_S_LAUNCHER_WIFI_ADAPTER      "WiFi"
#define K_S_LAUNCHER_WIFI_NETWORKS     "WiFi-Netzwerke"
#define K_S_LAUNCHER_WIFI_TX_POWER     "WiFi-Leistung"
#define K_S_LAUNCHER_SD                "SD"
#define K_S_LAUNCHER_SD_FORMAT         "SD formatieren"
#define K_S_LAUNCHER_SD_SPEED          "SD-Karten-Frequenz"
#define K_S_LAUNCHER_SOUND             "Ton"
#define K_S_LAUNCHER_SERVICES          K_S_SERVICES
#define K_S_LAUNCHER_WEB               K_S_WEB
#define K_S_LAUNCHER_TELNET            K_S_TELNET
#define K_S_LAUNCHER_FTP               K_S_FTP
#define K_S_LAUNCHER_FTP_USER          K_S_USER
#define K_S_LAUNCHER_FTP_PASSWORD      K_S_PASSWORD
#define K_S_LAUNCHER_FTP_IP            "IP"
#define K_S_LAUNCHER_STATUSBAR         "Statusleiste"
#define K_S_LAUNCHER_CLOCK             "Uhr"
#define K_S_LAUNCHER_CLOCK_0           "Keine"
#define K_S_LAUNCHER_CLOCK_1           "HH:MM:SS"
#define K_S_LAUNCHER_CLOCK_2           "HH:MM"
#define K_S_LAUNCHER_CLOCK_3           "*HH:MM"
#define K_S_LAUNCHER_MEM               "Speicher"
#define K_S_LAUNCHER_MEM_0             "Keine"
#define K_S_LAUNCHER_MEM_1             "Symbol"
#define K_S_LAUNCHER_MEM_2             "Text"
#define K_S_LAUNCHER_NETWORK           K_S_LAUNCHER_WIFI
#define K_S_LAUNCHER_NETWORK_0         "Keine"
#define K_S_LAUNCHER_NETWORK_1         "Symbol"
#define K_S_LAUNCHER_BATTERY           "Batterie"
#define K_S_LAUNCHER_BATTERY_0         "Keine"
#define K_S_LAUNCHER_BATTERY_1         "Symbol + Text"
#define K_S_LAUNCHER_BATTERY_2         "Symbol"
#define K_S_LAUNCHER_BATTERY_3         "Text"

#define K_S_LAUNCHER_ABOUT_SYSTEM      "Über das System"
#define K_S_LAUNCHER_DEVICE_INFO       "Über das Gerät"
#define K_S_PARTITION_TABLE            "Partitionstabelle"
#define K_S_LAUNCHER_LIGHT_SLEEP       "Light sleep"
#define K_S_LAUNCHER_DEEP_SLEEP        "Deep sleep"
#define K_S_LAUNCHER_REBOOT            "Neustart"

#define K_S_LAUNCHER_SELECT_TX_POWER   "Leistung einstellen"

#define K_S_LAUNCHER_ENABLE_WIFI_FIRST "Zuerst WiFi aktivieren"

#define K_S_LAUNCHER_DEVICE_INFO_FMT \
    "Modell: %s\n"                   \
    "Revision: %d\n"                 \
    "ESP-IDF-Version: %s\n"         \
    "Frequenz: %d MHz\n"             \
    "Anzahl der Kerne: %d\n"         \
    "IP: %s"

#define K_S_LAUNCHER_PARTITION_FMT "Adresse: 0x%s\nGröße: 0x%s"

#define K_S_LAUNCHER_FORMAT        "Formatierung"

#define K_S_LAUNCHER_FORMAT_DISCLAIMER_ALERT               \
    "ACHTUNG: Alle Daten auf der SD-Karte werden gelöscht!\n" \
    "\nFortfahren?\n\nSTART - weiter\nA - beenden"

#define K_S_LAUNCHER_PLEASE_STANDBY K_S_PLEASE_STANDBY

#define K_S_LAUNCHER_FORMAT_ERROR_ALERT        \
    "SD-Karte konnte nicht formatiert werden.\n\n" \
    "System wird neu gestartet."

#define K_S_LAUNCHER_FORMAT_SUCCESS_ALLERT        \
    "SD-Karte erfolgreich formatiert!\n\n"        \
    "System wird neu gestartet."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/wifi_config.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_WIFI_CONFIG_SCANING_NETWORKS    "WiFi-Netzwerke werden gescannt..."
#define K_S_WIFI_CONFIG_SCAN_ERROR_CODE_FMT "Netzwerke konnten nicht gescannt werden, Fehlercode: %d"
#define K_S_WIFI_CONFIG_NETWORKS            "Netzwerke"

#define K_S_WIFI_CONFIG_ABOUT_NETWORK_FMT \
    "Kanal: %d\n"                         \
    "Signalstärke: %ddb\n"               \
    "MAC: %s\n"                           \
    "Sicherheit: %s"

#define K_S_WIFI_CONFIG_CONNECTING_TO_OPEN_INSECURE_NETWORK_FMT \
    "Sie verbinden sich mit dem unsicheren Netzwerk %s\n\n"      \
    "A - fortfahren\n"                                            \
    "B - anderes Netzwerk wählen"

#define K_S_WIFI_CONFIG_ENTER_PASSWORD              "Passwort eingeben:"
#define K_S_WIFI_CONFIG_CONNECTING                  "Verbinden..."
#define K_S_WIFI_CONFIG_CONNECTED_TO_NETWORK_FMT    "Verbunden mit %s"
#define K_S_WIFI_CONFIG_CANT_CONNECT_TO_NETWORK_FMT "Verbindung zu %s fehlgeschlagen"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/scan_i2c.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_I2C_INIT_ABOUT            "I2C init: SDA=%d, SCL=%d"
#define K_S_I2C_SCANNER_SCAN_START    "Starting I2C scan..."
#define K_S_I2C_SCANNER_SCAN_DONE     "I2C scan done."
#define K_S_I2C_SCANNER_DEVICES_FOUND "Found %d devices."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/transform.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_TRANSFORM_CANT_LOAD_FACE     "face.bmp konnte nicht von der SD-Karte geladen werden." // FACEPALM.BMP
#define K_S_TRANFORM_DRAWING_FACE_AT_FMT "Drawing face at %d, %d"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/fmanager.cpp/.h ///////////////////////////////////////////////////////////////////////////////
#define K_S_FMANAGER_COPYING                              "Kopieren..."
#define K_S_FMANAGER_MD5_CALC                             "MD5 berechnen"
#define K_S_FMANAGER_LOADING                              "Laden"
#define K_S_FMANAGER_ENTER_NEW_FOLDER_NAME                "Neuen Ordnernamen eingeben"
#define K_S_FMANAGER_ENTER_NEW_NAME                       "Neuen Namen eingeben"
#define K_S_FMANAGER_OPTIONS                              "Optionen"
#define K_S_FMANAGER_OPEN                                 "Öffnen"
#define K_S_FMANAGER_OPEN_WITH                            "Öffnen mit"
#define K_S_FMANAGER_CREATE_FOLDER                        "Ordner erstellen"
#define K_S_FMANAGER_RENAME                               "Umbenennen"
#define K_S_FMANAGER_DELETE                                "Löschen"
#define K_S_FMANAGER_INFO_ABOUT_FILE                      "Informationen"
#define K_S_FMANAGER_SELECT_APP                           "App auswählen"
#define K_S_FMANAGER_FILE_MANAGER                         "Dateimanager"
#define K_S_FMANAGER_NES_EMULATOR                         "NES-Emulator"
#define K_S_FMANAGER_FIRMWARE_LOADER                      "Firmware-Loader"
#define K_S_FMANAGER_LUA                                  "Lua"
#define K_S_FMANAGER_MJS                                  "mJS"
#define K_S_FMANAGER_LILTRACKER                           K_S_LAUNCHER_LILTRACKER
#define K_S_FMANAGER_MAD_PLAYER                           "MadPlayer"
#define K_S_FMANAGER_DYNAPP                               "DynApp (.so)"
#define K_S_FMANAGER_ACTIONS_ON_SELECTED                  "Aktionen für Auswahl"
#define K_S_FMANAGER_COPY_SELECTED                        "Auswahl kopieren"
#define K_S_FMANAGER_MOVE_SELECTED                        "Auswahl verschieben"
#define K_S_FMANAGER_DELETE_SELECTED                      "Auswahl löschen"
#define K_S_FMANAGER_CLEAR_SELECTION                      "Auswahl aufheben"
#define K_S_FMANAGER_CALC_INTERRUPTED                     "Nicht berechnet"
#define K_S_FMANAGER_ARE_YOU_SURE_ALERT                   "Sind Sie sicher?"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE_FMT        "Diese Operation löscht Datei %s\nFortfahren: START\nBeenden: B"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_COUNT_FILES_FMT "Diese Operation löscht %d Datei(en)\nFortfahren: START\nBeenden: B"
#define K_S_FMANAGER_SELECTED_ENTRIES_EXIT_FMT "%d Dateien ausgewählt\nBeenden bestätigen: START\nZurück: B"
#define K_S_FMANAGER_ABOUT_DIR_FMT \
    "Typ: Verzeichnis\n"           \
    "Pfad: %s"

#define K_S_FMANAGER_ABOUT_FILE_FMT \
    "Typ: Datei\n"                   \
    "Größe: %s\n"                    \
    "MD5: %s\n"
// TODO: MOVE MULTIBOOT TO SEPARATE APP
#define K_S_FMANAGER_MULTIBOOT_ABOUT_FMT            "%s\n\nGröße: %s"

#define K_S_FMANAGER_MULTIBOOT_STARTING             "Starten..."
#define K_S_FMANAGER_MULTIBOOT_ERROR_FMT            "Stufe: %d\nCode: %d"
#define K_S_FMANAGER_SORTING                        "Sortieren..."
#define K_S_FMANAGER_ALMOST_DONE                    "Fast fertig..."
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE      "Diese Operation löscht Datei\n"
#define K_S_FMANAGER_NOT_ENOUGH_MEMORY_TO_FINISH_OP "Nicht genug Speicher für die Operation"
#define K_S_FMANAGER_SELECTED_FILES_FMT             "%d Datei(en) ausgewählt"
#define K_S_FMANAGER_CANT_DO_OP                     "Operation kann nicht ausgeführt werden"
#define K_S_FMANAGER_FILE_ADDED_TO_BUFFER_EXCHANGE  "Datei in Zwischenablage kopiert"
///////////////////////////////////////////////////////////////////////////////////////////////////////
// apps/liltracker/liltracker.cpp ////////////////////////////////////////////
#define K_S_LILTRACKER_SAVE_TRACK             "Track speichern"
#define K_S_LILTRACKER_OPEN_TRACK             "Track öffnen"
#define K_S_LILTRACKER_CREATE_NEW_TRACK       "++ Neu erstellen"
#define K_S_LILTRACKER_ENTER_FILENAME         "Dateinamen eingeben"
#define K_S_LILTRACKER_FILENAME_CANT_BE_EMPTY "Dateiname darf nicht leer sein"
#define K_S_LILTRACKER_CLEAR_TRACK_DATA       "Alle Trackdaten löschen?\n[START]Ja\n[B]Nein"
#define K_S_LILTRACKER_OPEN                   "Öffnen"
#define K_S_LILTRACKER_SAVE                   "Speichern"
#define K_S_LILTRACKER_RESET                  "Zurücksetzen"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/lilcatalog/lilcatalog.cpp /////////////////////////////////////////////////////////////////////
#define K_S_LILCATALOG_APP                           "LilKatalog"

#define K_S_LILCATALOG_APPS                          "Apps"
#define K_S_LILCATALOG_MODS                          "Mods"
#define K_S_LILCATALOG_STOP                          "Beenden"
#define K_S_LILCATALOG_BACK                          "Zurück"
#define K_S_LILCATALOG_EMPTY                         ""

#define K_S_LILCATALOG_START                         "Starten"
#define K_S_LILCATALOG_INSTALL                       "Installieren"
#define K_S_LILCATALOG_REMOVE                        "Löschen"
#define K_S_LILCATALOG_UPDATE                        "Aktualisieren"
#define K_S_LILCATALOG_SOURCE                        "Quelle"
#define K_S_LILCATALOG_ENTRY_DESCRIPTION             "Beschreibung"
#define K_S_LILCATALOG_LOADING                       "Herunterladen"
#define K_S_LILCATALOG_LOADING_CATALOG               "Katalog laden..."
#define K_S_LILCATALOG_STARTING                      "Starten..."
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_NAME        "Name: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_AUTHOR      "Autor: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_DESCRIPTION "Beschreibung: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_FILE        "Datei: "

#define K_S_LILCATALOG_NEXT_PAGE                     ">> Nächste Seite"
#define K_S_LILCATALOG_PREV_PAGE                     "<< Vorherige Seite"
#define K_S_LILCATALOG_SIZE                          "Größe:"
#define K_S_LILCATALOG_FILE_LOADING                  "Datei herunterladen..."
#define K_S_LILCATALOG_FILE_LOADING_COMPLETE         "Datei heruntergeladen und gespeichert"

#define K_S_LILCATALOG_ARCHIVE_NOTICE                "Archive müssen manuell entpackt werden"
#define K_S_LILCATALOG_UNSUPPORTED_TYPE              "Nicht unterstützter Dateityp"
#define K_S_LILCATALOG_CLEAR_CACHE                   "Cache leeren"
#define K_S_LILCATALOG_CACHE_CLEARED                 "Cache geleert"
#define K_S_LILCATALOG_INSTALLED                     "Installiert"
#define K_S_LILCATALOG_NO_INSTALLED                  "Keine installierten Apps"
#define K_S_LILCATALOG_NO_BINARY                     "Diese App hat keine herunterladbaren Dateien"
#define K_S_LILCATALOG_VIEW_SOURCE                   "Quelle anzeigen"

#define K_S_LILCATALOG_ERROR_CREATE_FOLDER           "Fehler beim Erstellen des Ordners"
#define K_S_LILCATALOG_ERROR_LOAD_CATALOG            "Fehler beim Laden des Katalogs"
#define K_S_LILCATALOG_ERROR_FILE_OPEN               "Fehler beim Öffnen der Datei"
#define K_S_LILCATALOG_ERROR_CONNECTION              "Verbindungsfehler: "
#define K_S_LILCATALOG_ERROR_DIRETORY_CREATE         "Fehler beim Erstellen des Verzeichnisses"
#define K_S_LILCATALOG_ERROR_STAGE1                  "Stufe: 1\nCode: "
#define K_S_LILCATALOG_ERROR_STAGE2                  "Stufe: 2\nCode: "
#define K_S_LILCATALOG_ERROR_STAGE3                  "Stufe: 3\nCode: "
#define K_S_LILCATALOG_SD_NOTFOUND                   "SD-Karte nicht gefunden. Fortsetzung nicht möglich"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/letris.cpp //////////////////////////////////////////////////////////////////////////////
#define K_S_LETRIS_GAME_OVER      "Game over"
#define K_S_LETRIS_GAME_OVER_LONG "Spiel vorbei!\nDu hast es versucht. :)"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/settings/sound.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SETTINGS_SOUND                "Ton"
#define K_S_SETTINGS_SOUND_VOLUME         "Lautstärke:"
#define K_S_SETTINGS_SOUND_STARTRUP       "Startton:"
#define K_S_SETTINGS_SOUND_BUZZER_STARTUP "Buzzer beim Start:"
#define K_S_SETTINGS_SOUND_SAVE           "<< Speichern"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/screenshot.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SCREENSHOT_SAVED      "Screenshot gespeichert"
#define K_S_SCREENSHOT_SAVE_ERROR "Fehler beim Speichern des Screenshots"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/network.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_NET_OFFLINE "WiFi-Verbindung verloren"
#define K_S_NET_ONLINE  "WiFi verbunden"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/combo.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_COMBO_FOR_EXIT_HOLD            "Zum Beenden halten"
#define K_S_COMBO_HOLD_SELECT_FOR_TIME_FMT "[SELECT] für %d s"
#define K_S_COMBO_PRESS_START              "Drücke [START]"
#define K_S_COMBO_TO_CONTINUE              "zum Fortfahren"
#define K_S_COMBO_PRESS                    "Drücke"
#define K_S_COMBO_HOLD_ANY                 "Beliebige Taste"
#define K_S_COMBO_BUTTON_FOR_ONE_SEC       "1 s halten"
#define K_S_COMBO_PRESS_TOGETHER           "Zusammen drücken"
#define K_S_COMBO_TIMEOUT_FMT              "Zeitlimit %d ms"
#define K_S_COMBO_END                      "Ende!"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/weather/weather.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_WEATHER_CLEAR_SKY                     "Klarer Himmel" // 0

#define K_S_WEATHER_MOSTLY_CLEAR                  "Überwiegend klar" // 1
#define K_S_WEATHER_PARTLY_CLOUDY                 "Teilweise bewölkt" // 2
#define K_S_WEATHER_OVERCAST                      "Bewölkt" //3

#define K_S_WEATHER_FOG                           "Nebel" //45
#define K_S_WEATHER_FREEZING_FOG                  "Gefrierender Nebel" //48

#define K_S_WEATHER_LIGHT_DRIZZLE                 "Leichter Nieselregen" //51
#define K_S_WEATHER_DRIZZLE                       "Nieselregen" //53
#define K_S_WEATHER_HEAVY_DRIZZLE                 "Starker Nieselregen" //55

#define K_S_WEATHER_LIGHT_FREEZING_DRIZZLE        "Leichter gefrierender Nieselregen" //56
#define K_S_WEATHER_HEAVY_FREEZING_DRIZZLE        "Starker gefrierender Nieselregen" //57

#define K_S_WEATHER_LIGHT_RAIN                    "Leichter Regen" //61
#define K_S_WEATHER_RAIN                          "Regen" //63
#define K_S_WEATHER_HEAVY_RAIN                    "Starker Regen" // 65

#define K_S_WEATHER_LIGHT_FREEZING_RAIN           "Leichter gefrierender Regen" // 66
#define K_S_WEATHER_HEAVY_FREEZING_RAIN           "Starker gefrierender Regen" //67

#define K_S_WEATHER_LIGHT_SNOW                    "Leichter Schnee" // 71
#define K_S_WEATHER_SNOW                          "Schnee" // 73
#define K_S_WEATHER_HEAVY_SNOW                    "Starker Schnee" // 75

#define K_S_WEATHER_SNOW_GRAINS                   "Schneegriesel" // 77

#define K_S_WEATHER_LIGHT_SHOWERS                 "Leichte Schauer" // 80
#define K_S_WEATHER_SHOWERS                       "Schauer" // 81
#define K_S_WEATHER_HEAVY_SHOWERS                 "Starke Schauer" // 82

#define K_S_WEATHER_RAIN_AND_SNOW                 "Schneeregen" // 85
#define K_S_WEATHER_HEAVY_RAIN_AND_SNOW           "Starker Schneeregen" // 86

#define K_S_WEATHER_THUNDERSTORM                  "Gewitter" // 95
#define K_S_WEATHER_THUNDERSTORM_WITH_HAIL        "Gewitter mit Hagel" // 96
#define K_S_WEATHER_SEVERE_THUNDERSTORM_WITH_HAIL "Schweres Gewitter mit Hagel" // 99

#define K_S_WEATHER_LOADING_DATA                  "Daten laden..."
#define K_S_WEATHER_DATA_PATTERN_ERROR            "Datenformatfehler"
#define K_S_WEATHER_DATA_LOAD_ERROR_FMT           "Fehler beim Laden der Daten:\nStatuscode: %d"

#define K_S_WEATHER_LOCATION_NOT_SET              "Standort nicht festgelegt"
#define K_S_WEATHER_SELECT_TO_SETUP               "[SELECT] - Einstellungen"
#define K_S_WEATHER_A_TO_EXIT                     "[A] - Beenden" // LOL WHY
#define K_S_WEATHER_TEMP_FMT                      "%.1f °C"
#define K_S_WEATHER_WIND_SPEED_FMT                "%.1f km/h"

#define K_S_WEATHER_LATITUDE                      "Breitengrad"
#define K_S_WEATHER_LATITUDE_S                    "Breitengrad"
#define K_S_WEATHER_LONGITUDE                     "Längengrad"
#define K_S_WEATHER_LONGITUDE_S                   "Längengrad"

#define K_S_WEATHER_INPUT                         "Eingabe"

#define K_S_WEATHER_SAVE                          "Speichern"
#define K_S_WEATHER_CANCEL                        "Abbrechen"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/usbdrive/usbdrive.cpp /////////////////////////////////////////////////////////////////////////
#define K_S_USB_DRIVE_TITLE                       "USB-Laufwerk"
#define K_S_USB_DRIVE_EXPERIMENTAL_TITLE          "Warnung"
#define K_S_USB_DRIVE_EXPERIMENTAL_WARNING        "Experimentelle Funktion.\n" \
                                                  "Serial wird deaktiviert.\n\n" \
                                                  "[START] - OK  [A] - Abbrechen"
#define K_S_USB_DRIVE_INITIALIZING                "USB wird initialisiert..."
#define K_S_USB_DRIVE_INIT_ERROR                  "USB-Initialisierungsfehler"
#define K_S_USB_DRIVE_NO_SD                       "SD-Karte nicht verfügbar"
#define K_S_USB_DRIVE_CONNECT_USB                 "USB-Kabel mit PC verbinden"
#define K_S_USB_DRIVE_CONNECTED                   "Mit PC verbunden"
#define K_S_USB_DRIVE_EJECTED                     "Sicher ausgeworfen"
#define K_S_USB_DRIVE_PC_INSTRUCTION              "SD-Karte ist am PC verfügbar"
#define K_S_USB_DRIVE_SAFE_EJECT                  "Vor dem Beenden sicher auswerfen"
#define K_S_USB_DRIVE_PRESS_A_TO_EXIT             "[A] - Beenden"
#define K_S_USB_DRIVE_DISCONNECTING               "Trennen..."
#define K_S_USB_DRIVE_REBOOT_REQUIRED             "Gerät wird neu gestartet..."
#define K_S_USB_DRIVE_NOT_EJECTED                 "Nicht sicher ausgeworfen!"
#define K_S_USB_DRIVE_EJECT_WARNING               "Daten könnten beschädigt sein"
#define K_S_USB_DRIVE_PRESS_START_CONTINUE        "[START] - fortfahren"
#define K_S_USB_DRIVE_PRESS_B_CANCEL              "[B] - Abbrechen"
#define K_S_LAUNCHER_USB_DRIVE                    "USB-Laufwerk"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/pastebin/pastebinApp.cpp //////////////////////////////////////////////////////////////////////
#define K_S_PASTEBIN_CODE                         "Code"
#define K_S_PASTEBIN_NAME                         "Name"
#define K_S_PASTEBIN_DOWNLOAD                     "Herunterladen"
#define K_S_PASTEBIN_ENTER_CODE                   "Code eingeben"
#define K_S_PASTEBIN_ENTER_NAME                   "Namen eingeben"
#define K_S_PASTEBIN_ERROR_CREATE_DIR             "Fehler beim Erstellen des Verzeichnisses"
#define K_S_PASTEBIN_ERROR_OPEN_FILE              "Fehler beim Öffnen der Datei"
#define K_S_PASTEBIN_FILE_DOWNLOADED_FMT          "Datei heruntergeladen nach\n%s"
#define K_S_PASTEBIN_REQUEST_FAIL_FMT             "HTTP-Fehler\n%s"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/mjs/mjsrunner.cpp /////////////////////////////////////////////////////////////////////////////
#define K_S_MJS_ERROR                             "Fehler: "
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/mjs/luarunner.cpp /////////////////////////////////////////////////////////////////////////////
#define K_S_LUA_ERROR                             "Fehler: "
#define K_S_LUA_AWAIT_CODE_FROM_UART              "Warte auf Code\nvon UART...\n\nDrücke [A]\nzum Beenden."
#define K_S_LUA_DOWNLOAD                          "Herunterladen..."
#define K_S_LUA_REPL_AWAIT_CODE_FROM_UART         "REPL:\nWarte auf Code\nvon UART...\n\nDrücke [A]\nzum Beenden."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/tests/keyboard/keyboard.cpp //////////////////////////////////////////////////////////////////
#define K_S_KEYBOARD_ENTER_TEXT                    "Text eingeben: "
#define K_S_KEYBOARD_YOU_ENTERED                   "Sie haben eingegeben:"
//////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format on
