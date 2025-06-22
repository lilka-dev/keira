#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
// GUIDELINE: Keira all strings used in messages, or somehow written to serial, should be defined here

// clang-format off

// Multi purpose strings  /////////////////////////////////////////////////////////////////////////////
#define K_S_ERROR                       "Error"
#define K_S_ATTENTION                   "Attention"
#define K_S_SUCCESS                     "Success"
#define K_S_PLEASE_STANDBY              "Please standby..."
#define K_S_LILKA_V2_OR_HIGHER_REQUIRED "This program requires Lilka V2 or above"
#define K_S_FEATURE_IN_DEVELOPMENT      "This feature is in development"
#define K_S_CANT_OPEN_DIR_FMT           "Can't open dir %s"
#define K_S_CANT_OPEN_FILE_FMT          "Can't open file %s"
#define K_S_CANT_CREATE_DIR_FMT         "Can't create dir %s"
#define K_S_CANT_REMOVE_FILE_FMT        "Can't remove file %s"
#define K_S_DIR_EMPTY_FMT               "Directory %s empty"

#define K_S_MENU_BACK                   "<< Back"

#define K_S_PASSWORD                    "Password"
#define K_S_USER                        "User"

#define K_S_SETTINGS                    "Settings"
#define K_S_ON                          "ON"
#define K_S_OFF                         "OFF"
#define K_S_STATUS                      "Status"

#define K_S_SERVICES                    "Services"
#define K_S_FTP                         "FTP"

#define K_S_OS_NAME                     "Keira OS"
#define K_S_OS_DESCRIPTION              "by Андерсон & friends"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// service.cpp  ///////////////////////////////////////////////////////////////////////////////////////
#define K_S_SERVICE_STARTUP_FMT "Starting service %s"
#define K_S_SERVICE_DIE_FMT     "Service %s died"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/launcher.cpp //////////////////////////////////////////////////////////////////////////////////
#define K_S_LAUNCHER_MAIN_MENU         "Main menu"
#define K_S_LAUNCHER_APPS              "Applications"
#define K_S_LAUNCHER_DEMOS             "Demo"
#define K_S_LAUNCHER_LINES             "Lines"
#define K_S_LAUNCHER_DISK              "Disk"
#define K_S_LAUNCHER_TRANSFORM         "Transform"
#define K_S_LAUNCHER_BALL              "Ball"
#define K_S_LAUNCHER_CUBE              "Cube"
#define K_S_LAUNCHER_EPILEPSY          "Epilepsy"
#define K_S_LAUNCHER_PET_PET           "PetPet"
#define K_S_LAUNCHER_TESTS             "Tests"
#define K_S_LAUNCHER_KEYBOARD          "Keyboard"
#define K_S_LAUNCHER_SPI_TEST          "Test SPI"
#define K_S_LAUNCHER_I2C_SCANNER       "I2C-scan"
#define K_S_LAUNCHER_GPIO_MANAGER      "GPIO-manager"
#define K_S_LAUNCHER_COMBO             "Combo" // wtf?
#define K_S_LAUNCHER_CALLBACK_TEST     "CallbackTest"
#define K_S_LAUNCHER_LILCATALOG        "LilCatalogue"
#define K_S_LAUNCHER_LILTRACKER        "LilTracker"
#define K_S_LAUNCHER_LETRIS            "Letris"
#define K_S_LAUNCHER_TAMAGOTCHI        "Tamagotchi"
#define K_S_LAUNCHER_WEATHER           "Weather"
#define K_S_LAUNCHER_PASTEBIN          "Pastebin"
#define K_S_LAUNCHER_SD_BROWSER        "SD Browser"
#define K_S_LAUNCHER_SPIFFS_BROWSER    "SPIFFS Browser"
#define K_S_LAUNCHER_DEV_MENU          "Development"
#define K_S_LAUNCHER_LIVE_LUA          "Live Lua" // Lua live?
#define K_S_LAUNCHER_LUA_REPL          "Lua REPL"
#define K_S_LAUNCHER_WIFI_ADAPTER      "WiFi"
#define K_S_LAUNCHER_WIFI_NETWORKS     "WiFi Networks"
#define K_S_LAUNCHER_WIFI_TX_POWER     "WiFi Power"
#define K_S_LAUNCHER_SOUND             "Sound"
#define K_S_LAUNCHER_SERVICES          K_S_SERVICES
#define K_S_LAUNCHER_FTP               K_S_FTP
#define K_S_LAUNCHER_FTP_STATUS        K_S_STATUS
#define K_S_LAUNCHER_FTP_USER          K_S_USER
#define K_S_LAUNCHER_FTP_PASSWORD      K_S_PASSWORD

#define K_S_LAUNCHER_ABOUT_SYSTEM      "About system"
#define K_S_LAUNCHER_DEVICE_INFO       "About device"
#define K_S_PARTITION_TABLE            "Partition table"
#define K_S_LAUNCHER_SD_FORMAT         "Format SD"
#define K_S_LAUNCHER_LIGHT_SLEEP       "Light sleep"
#define K_S_LAUNCHER_DEEP_SLEEP        "Deep sleep"
#define K_S_LAUNCHER_REBOOT            "Reboot"

#define K_S_LAUNCHER_SELECT_TX_POWER   "Set power"

#define K_S_LAUNCHER_ENABLE_WIFI_FIRST "Enable WiFi first"

#define K_S_LAUNCHER_DEVICE_INFO_FMT \
    "Model: %s\n"                    \
    "Revision: %d\n"                 \
    "ESP-IDF version: %s\n"          \
    "Frequency: %d Mhz\n"            \
    "Cores count: %d\n"              \
    "IP: %s"

#define K_S_LAUNCHER_PARTITION_FMT "Address: 0x%s\nSize: 0x%s"

#define K_S_LAUNCHER_FORMAT        "Formating"

#define K_S_LAUNCHER_FORMAT_DISCLAIMER_ALERT            \
    "ATTENTION: It would remove all data on SD card!\n" \
    "\nProceed?\n\nSTART - continue\nA - exit"

#define K_S_LAUNCHER_PLEASE_STANDBY K_S_PLEASE_STANDBY

#define K_S_LAUNCHER_FORMAT_ERROR_ALERT \
    "Can't format SD card.\n\n"         \
    "System would reboot."

#define K_S_LAUNCHER_FORMAT_SUCCESS_ALLERT \
    "SD card format success!\n\n"          \
    "System would reboot."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/wifi_config.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_WIFI_CONFIG_SCANING_NETWORKS    "Scanning WiFi neworks..."
#define K_S_WIFI_CONFIG_SCAN_ERROR_CODE_FMT "Can't scan networks, error code: %d"
#define K_S_WIFI_CONFIG_NETWORKS            "Networks"

#define K_S_WIFI_CONFIG_ABOUT_NETWORK_FMT \
    "Channel: %d\n"                       \
    "Signal strength: %ddb\n"             \
    "MAC: %s\n"                           \
    "Security: %s"

#define K_S_WIFI_CONFIG_CONNECTING_TO_OPEN_INSECURE_NETWORK_FMT \
    "You are connecting to insecure network %s\n\n"             \
    "A - continue\n"                                            \
    "B - select another network"

#define K_S_WIFI_CONFIG_ENTER_PASSWORD              "Enter password:"
#define K_S_WIFI_CONFIG_CONNECTING                  "Connecting..."
#define K_S_WIFI_CONFIG_CONNECTED_TO_NETWORK_FMT    "Connected to %s"
#define K_S_WIFI_CONFIG_CANT_CONNECT_TO_NETWORK_FMT "Can't connect to %s"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/scan_i2c.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_I2C_INIT_ABOUT            "I2C init: SDA=%d, SCL=%d"
#define K_S_I2C_SCANNER_SCAN_START    "Starting I2C scan..."
#define K_S_I2C_SCANNER_SCAN_DONE     "I2C scan done."
#define K_S_I2C_SCANNER_DEVICES_FOUND "Found %d devices."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/transform.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_TRANSFORM_CANT_LOAD_FACE     "Can't load face.bmp from SD card." // FACEPALM.BMP
#define K_S_TRANFORM_DRAWING_FACE_AT_FMT "Drawing face at %d, %d"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/fmanager.cpp/.h ///////////////////////////////////////////////////////////////////////////////
#define K_S_FMANAGER_COPYING                       "Copying..."
#define K_S_FMANAGER_MD5_CALC                      "Computing МD5"
#define K_S_FMANAGER_LOADING                       "Loading"
#define K_S_FMANAGER_ENTER_NEW_FOLDER_NAME         "Enter new folder name"
#define K_S_FMANAGER_ENTER_NEW_NAME                "Enter new name"
#define K_S_FMANAGER_OPTIONS                       "Options"
#define K_S_FMANAGER_OPEN                          "Open"
#define K_S_FMANAGER_OPEN_WITH                     "Open with"
#define K_S_FMANAGER_CREATE_FOLDER                 "Make directory"
#define K_S_FMANAGER_RENAME                        "Rename"
#define K_S_FMANAGER_DELETE                        "Delete"
#define K_S_FMANAGER_INFO_ABOUT_FILE               "Information"
#define K_S_FMANAGER_SELECT_APP                    "Select app"
#define K_S_FMANAGER_FILE_MANAGER                  "File Manager"
#define K_S_FMANAGER_NES_EMULATOR                  "NES Emulator"
#define K_S_FMANAGER_FIRMWARE_LOADER               "Flash loader"
#define K_S_FMANAGER_LUA                           "Lua"
#define K_S_FMANAGER_MJS                           "mJS"
#define K_S_FMANAGER_LILTRACKER                    K_S_LAUNCHER_LILTRACKER
#define K_S_FMANAGER_MOD_PLAYER                    "MOD Player"
#define K_S_FMANAGER_ACTIONS_ON_SELECTED           "Actions on selected"
#define K_S_FMANAGER_COPY_SELECTED                 "Copy selected"
#define K_S_FMANAGER_MOVE_SELECTED                 "Move selected"
#define K_S_FMANAGER_DELETE_SELECTED               "Delete selected"
#define K_S_FMANAGER_CLEAR_SELECTION               "Clear selection"
#define K_S_FMANAGER_CALC_INTERRUPTED              "Not computed"
#define K_S_FMANAGER_ARE_YOU_SURE_ALERT            "Are you sure"

#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE_FMT "This operation would delete file\n %s\nProceed: START\nExit: B"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_COUNT_FILES_FMT \
    "This operation would delete %d file(s)\nProceed: START\nExit: B"
#define K_S_FMANAGER_ABOUT_DIR_FMT \
    "Type: directory\n"            \
    "Path: %s"

#define K_S_FMANAGER_ABOUT_FILE_FMT \
    "Type: file\n"                  \
    "Size: %s\n" \ 
    "MD5: %s\n"
// TODO: MOVE MULTIBOOT TO SEPARATE APP
#define K_S_FMANAGER_MULTIBOOT_ABOUT_FMT            "%s\n\nSize: %s"

#define K_S_FMANAGER_MULTIBOOT_STARTING             "Starting..."
#define K_S_FMANAGER_MULTIBOOT_ERROR_FMT            "Stage: %d\nCode: %d"
#define K_S_FMANAGER_SORTING                        "Sorting..."
#define K_S_FMANAGER_ALMOST_DONE                    "Almost ready..."
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE      "This operation would delete file\n"
#define K_S_FMANAGER_NOT_ENOUGH_MEMORY_TO_FINISH_OP "Not enough memory to finish operation"
#define K_S_FMANAGER_SELECTED_FILES_FMT             "Selected %d file(s)"
#define K_S_FMANAGER_CANT_DO_OP                     "Can't finish operation"
#define K_S_FMANAGER_FILE_ADDED_TO_BUFFER_EXCHANGE  "File added to buffer exchange"
///////////////////////////////////////////////////////////////////////////////////////////////////////
// apps/liltracker/liltracker.cpp ////////////////////////////////////////////
#define K_S_LILTRACKER_SAVE_TRACK             "Save track"
#define K_S_LILTRACKER_OPEN_TRACK             "Open track"
#define K_S_LILTRACKER_CREATE_NEW_TRACK       "++ Create new"
#define K_S_LILTRACKER_ENTER_FILENAME         "Enter filename"
#define K_S_LILTRACKER_FILENAME_CANT_BE_EMPTY "Filename can't be empty"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/lilcatalog/lilcatalog.cpp /////////////////////////////////////////////////////////////////////
#define K_S_LILCATALOG_LANGUAGE                      "uk"
#define K_S_LILCATALOG_APP                           "LilCatalogue"

#define K_S_LILCATALOG_FETCH_CATALOG                 "Update catalogue"
#define K_S_LILCATALOG_STOP                          "Exit"
#define K_S_LILCATALOG_BACK                          "Back"
#define K_S_LILCATALOG_EMPTY                         ""

#define K_S_LILCATALOG_START                         "Run"
#define K_S_LILCATALOG_INSTALL                       "Install"
#define K_S_LILCATALOG_REMOVE                        "Delete"
#define K_S_LILCATALOG_UPDATE                        "Update"
#define K_S_LILCATALOG_ENTRY_DESCRIPTION             "Description"
#define K_S_LILCATALOG_LOADING                       "Downloading" // ?
#define K_S_LILCATALOG_STARTING                      "Starting..."
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_NAME        "Name: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_AUTHOR      "Author: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_DESCRIPTION "Description: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_FILES       "Files: "

#define K_S_LILCATALOG_CATEGORY_POSTFIX              " elements" // 4 elements? :D
#define K_S_LILCATALOG_SIZE                          "Size:"
#define K_S_LILCATALOG_FILE_LOADING                  "Downloading file..."
#define K_S_LILCATALOG_FILE_LOADING_COMPLETE         "File downloaded and saved" // simplify this bullshit

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
#define K_S_LETRIS_GAME_OVER_LONG "Game over!\nYou tried. :)"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/settings/sound.cpp /////////////////////////////////////////////////////////////////////
#define K_S_SETTINGS_SOUND                "Sound"
#define K_S_SETTINGS_SOUND_VOLUME         "Volume:"
#define K_S_SETTINGS_SOUND_STARTRUP       "Startup on Boot:"
#define K_S_SETTINGS_SOUND_BUZZER_STARTUP "Buzzer on Boot:"
#define K_S_SETTINGS_SOUND_SAVE           "<< Save"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/screenshot.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SCREENSHOT_SAVED      "Screenshot saved"
#define K_S_SCREENSHOT_SAVE_ERROR "Screenshot save error"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format on