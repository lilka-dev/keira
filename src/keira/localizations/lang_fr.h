#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
// GUIDELINE: Keira all strings used in messages, or somehow written to serial, should be defined here

// clang-format off

// Multi purpose strings  /////////////////////////////////////////////////////////////////////////////
#define K_S_CURRENT_LANGUAGE_SHORT      "fr"
#define K_S_CURRENT_LANGUAGE_FULL       "Français"

#define K_S_ERROR                       "Erreur"
#define K_S_ATTENTION                   "Attention"
#define K_S_SUCCESS                     "Succès"
#define K_S_PLEASE_STANDBY              "Veuillez patienter..."
#define K_S_LILKA_V2_OR_HIGHER_REQUIRED "Ce programme nécessite Lilka V2 ou supérieur"
#define K_S_FEATURE_IN_DEVELOPMENT      "Cette fonctionnalité est en développement"
#define K_S_CANT_OPEN_DIR_FMT           "Impossible d'ouvrir le dossier %s"
#define K_S_CANT_OPEN_FILE_FMT          "Impossible d'ouvrir le fichier %s"
#define K_S_CANT_CREATE_DIR_FMT         "Impossible de créer le dossier %s"
#define K_S_CANT_REMOVE_FILE_FMT        "Impossible de supprimer le fichier %s"
#define K_S_DIR_EMPTY_FMT               "Le dossier %s est vide"

#define K_S_MENU_BACK                   "<< Retour"

#define K_S_PASSWORD                    "Mot de passe"
#define K_S_USER                        "Utilisateur"

#define K_S_SETTINGS                    "Paramètres"
#define K_S_ON                          "ON"
#define K_S_OFF                         "OFF"
#define K_S_STATUS                      "Statut"

#define K_S_SERVICES                    "Services"
#define K_S_FTP                         "FTP"
#define K_S_WEB                         "Web"
#define K_S_TELNET                      "Telnet"
#define K_S_MDNS                        "mDNS"
#define K_S_LAUNCHER_MDNS               K_S_MDNS
#define K_S_LAUNCHER_MDNS_HOSTNAME      "Nom d'hôte"
#define K_S_LAUNCHER_MDNS_ENTER_HOSTNAME "Entrez le nom d'hôte :"
#define K_S_CHANGE_ON_NEXT_BOOT         "Les modifications seront appliquées au prochain démarrage"
#define K_S_OS_NAME                     "Keira OS"
#define K_S_OS_DESCRIPTION              "by Anderson & friends"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// service.cpp  ///////////////////////////////////////////////////////////////////////////////////////
#define K_S_SERVICE_STARTUP_FMT "Starting service %s"
#define K_S_SERVICE_DIE_FMT     "Service %s died"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/launcher.cpp //////////////////////////////////////////////////////////////////////////////////
#define K_S_LAUNCHER_MAIN_MENU         "Menu principal"
#define K_S_LAUNCHER_APPS              "Applications"
#define K_S_LAUNCHER_DEMOS             "Démo"
#define K_S_LAUNCHER_LINES             "Lignes"
#define K_S_LAUNCHER_DISK              "Disque"
#define K_S_LAUNCHER_TRANSFORM         "Transformation"
#define K_S_LAUNCHER_BALL              "Balle"
#define K_S_LAUNCHER_CUBE              "Cube"
#define K_S_LAUNCHER_EPILEPSY          "Épilepsie"
#define K_S_LAUNCHER_PET_PET           "PetPet"
#define K_S_LAUNCHER_TESTS             "Tests"
#define K_S_LAUNCHER_KEYBOARD          "Clavier"
#define K_S_LAUNCHER_SPI_TEST          "Test SPI"
#define K_S_LAUNCHER_I2C_SCANNER       "I2C-scan"
#define K_S_LAUNCHER_GPIO_MANAGER      "GPIO-gestionnaire"
#define K_S_LAUNCHER_COMBO             "Combo" // wtf?
#define K_S_LAUNCHER_CALLBACK_TEST     "CallbackTest"
#define K_S_LAUNCHER_LILCATALOG        "LilCatalogue"
#define K_S_LAUNCHER_LILTRACKER        "LilTracker"
#define K_S_LAUNCHER_LETRIS            "Letris"
#define K_S_LAUNCHER_TAMAGOTCHI        "Tamagotchi"
#define K_S_LAUNCHER_WEATHER           "Météo"
#define K_S_LAUNCHER_PASTEBIN          "Pastebin"
#define K_S_LAUNCHER_FMANAGER          "Gestionnaire de fichiers"
#define K_S_LAUNCHER_DEV_MENU          "Développement"
#define K_S_LAUNCHER_LIVE_LUA          "Live Lua" // Lua live?
#define K_S_LAUNCHER_LUA_REPL          "Lua REPL"
#define K_S_LAUNCHER_WIFI              "WiFi"
#define K_S_LAUNCHER_WIFI_ADAPTER      "WiFi"
#define K_S_LAUNCHER_WIFI_NETWORKS     "Réseaux WiFi"
#define K_S_LAUNCHER_WIFI_TX_POWER     "Puissance WiFi"
#define K_S_LAUNCHER_SD                "SD"
#define K_S_LAUNCHER_SD_FORMAT         "Formater SD"
#define K_S_LAUNCHER_SD_SPEED          "Fréquence carte SD"
#define K_S_LAUNCHER_SOUND             "Son"
#define K_S_LAUNCHER_SERVICES          K_S_SERVICES
#define K_S_LAUNCHER_WEB               K_S_WEB
#define K_S_LAUNCHER_TELNET            K_S_TELNET
#define K_S_LAUNCHER_FTP               K_S_FTP
#define K_S_LAUNCHER_FTP_USER          K_S_USER
#define K_S_LAUNCHER_FTP_PASSWORD      K_S_PASSWORD
#define K_S_LAUNCHER_FTP_IP            "IP"
#define K_S_LAUNCHER_STATUSBAR         "Barre d'état"
#define K_S_LAUNCHER_CLOCK             "Horloge"
#define K_S_LAUNCHER_CLOCK_0           "Aucune"
#define K_S_LAUNCHER_CLOCK_1           "HH:MM:SS"
#define K_S_LAUNCHER_CLOCK_2           "HH:MM"
#define K_S_LAUNCHER_CLOCK_3           "*HH:MM"
#define K_S_LAUNCHER_MEM               "Mémoire"
#define K_S_LAUNCHER_MEM_0             "Aucune"
#define K_S_LAUNCHER_MEM_1             "Icône"
#define K_S_LAUNCHER_MEM_2             "Texte"
#define K_S_LAUNCHER_NETWORK           K_S_LAUNCHER_WIFI
#define K_S_LAUNCHER_NETWORK_0         "Aucun"
#define K_S_LAUNCHER_NETWORK_1         "Icône"
#define K_S_LAUNCHER_BATTERY           "Batterie"
#define K_S_LAUNCHER_BATTERY_0         "Aucune"
#define K_S_LAUNCHER_BATTERY_1         "Icône + texte"
#define K_S_LAUNCHER_BATTERY_2         "Icône"
#define K_S_LAUNCHER_BATTERY_3         "Texte"

#define K_S_LAUNCHER_ABOUT_SYSTEM      "À propos du système"
#define K_S_LAUNCHER_DEVICE_INFO       "À propos de l'appareil"
#define K_S_PARTITION_TABLE            "Table de partitions"
#define K_S_LAUNCHER_LIGHT_SLEEP       "Light sleep"
#define K_S_LAUNCHER_DEEP_SLEEP        "Deep sleep"
#define K_S_LAUNCHER_REBOOT            "Redémarrer"

#define K_S_LAUNCHER_SELECT_TX_POWER   "Régler la puissance"

#define K_S_LAUNCHER_ENABLE_WIFI_FIRST "Activez d'abord le WiFi"

#define K_S_LAUNCHER_DEVICE_INFO_FMT \
    "Modèle : %s\n"                  \
    "Révision : %d\n"                \
    "Version ESP-IDF : %s\n"        \
    "Fréquence : %d MHz\n"           \
    "Nombre de cœurs : %d\n"         \
    "IP : %s"

#define K_S_LAUNCHER_PARTITION_FMT "Adresse : 0x%s\nTaille : 0x%s"

#define K_S_LAUNCHER_FORMAT        "Formatage"

#define K_S_LAUNCHER_FORMAT_DISCLAIMER_ALERT                  \
    "ATTENTION : Toutes les données de la carte SD seront supprimées !\n" \
    "\nContinuer ?\n\nSTART - continuer\nA - quitter"

#define K_S_LAUNCHER_PLEASE_STANDBY K_S_PLEASE_STANDBY

#define K_S_LAUNCHER_FORMAT_ERROR_ALERT              \
    "Impossible de formater la carte SD.\n\n"         \
    "Le système va redémarrer."

#define K_S_LAUNCHER_FORMAT_SUCCESS_ALLERT            \
    "Formatage de la carte SD réussi !\n\n"           \
    "Le système va redémarrer."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/wifi_config.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_WIFI_CONFIG_SCANING_NETWORKS    "Recherche des réseaux WiFi..."
#define K_S_WIFI_CONFIG_SCAN_ERROR_CODE_FMT "Impossible de scanner les réseaux, code d'erreur : %d"
#define K_S_WIFI_CONFIG_NETWORKS            "Réseaux"

#define K_S_WIFI_CONFIG_ABOUT_NETWORK_FMT \
    "Canal : %d\n"                        \
    "Puissance du signal : %ddb\n"       \
    "MAC : %s\n"                          \
    "Sécurité : %s"

#define K_S_WIFI_CONFIG_CONNECTING_TO_OPEN_INSECURE_NETWORK_FMT \
    "Vous vous connectez au réseau non sécurisé %s\n\n"          \
    "A - continuer\n"                                              \
    "B - choisir un autre réseau"

#define K_S_WIFI_CONFIG_ENTER_PASSWORD              "Entrez le mot de passe :"
#define K_S_WIFI_CONFIG_CONNECTING                  "Connexion..."
#define K_S_WIFI_CONFIG_CONNECTED_TO_NETWORK_FMT    "Connecté à %s"
#define K_S_WIFI_CONFIG_CANT_CONNECT_TO_NETWORK_FMT "Impossible de se connecter à %s"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/scan_i2c.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_I2C_INIT_ABOUT            "I2C init: SDA=%d, SCL=%d"
#define K_S_I2C_SCANNER_SCAN_START    "Starting I2C scan..."
#define K_S_I2C_SCANNER_SCAN_DONE     "I2C scan done."
#define K_S_I2C_SCANNER_DEVICES_FOUND "Found %d devices."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/transform.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_TRANSFORM_CANT_LOAD_FACE     "Impossible de charger face.bmp depuis la carte SD." // FACEPALM.BMP
#define K_S_TRANFORM_DRAWING_FACE_AT_FMT "Drawing face at %d, %d"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/fmanager.cpp/.h ///////////////////////////////////////////////////////////////////////////////
#define K_S_FMANAGER_COPYING                              "Copie..."
#define K_S_FMANAGER_MD5_CALC                             "Calcul MD5"
#define K_S_FMANAGER_LOADING                              "Chargement"
#define K_S_FMANAGER_ENTER_NEW_FOLDER_NAME                "Entrez le nom du nouveau dossier"
#define K_S_FMANAGER_ENTER_NEW_NAME                       "Entrez le nouveau nom"
#define K_S_FMANAGER_OPTIONS                              "Options"
#define K_S_FMANAGER_OPEN                                 "Ouvrir"
#define K_S_FMANAGER_OPEN_WITH                            "Ouvrir avec"
#define K_S_FMANAGER_CREATE_FOLDER                        "Créer un dossier"
#define K_S_FMANAGER_RENAME                               "Renommer"
#define K_S_FMANAGER_DELETE                                "Supprimer"
#define K_S_FMANAGER_INFO_ABOUT_FILE                      "Informations"
#define K_S_FMANAGER_SELECT_APP                           "Choisir l'application"
#define K_S_FMANAGER_FILE_MANAGER                         "Gestionnaire de fichiers"
#define K_S_FMANAGER_NES_EMULATOR                         "Émulateur NES"
#define K_S_FMANAGER_FIRMWARE_LOADER                      "Chargeur de firmware"
#define K_S_FMANAGER_LUA                                  "Lua"
#define K_S_FMANAGER_MJS                                  "mJS"
#define K_S_FMANAGER_LILTRACKER                           K_S_LAUNCHER_LILTRACKER
#define K_S_FMANAGER_MAD_PLAYER                           "MadPlayer"
#define K_S_FMANAGER_DYNAPP                               "DynApp (.so)"
#define K_S_FMANAGER_ACTIONS_ON_SELECTED                  "Actions sur la sélection"
#define K_S_FMANAGER_COPY_SELECTED                        "Copier la sélection"
#define K_S_FMANAGER_MOVE_SELECTED                        "Déplacer la sélection"
#define K_S_FMANAGER_DELETE_SELECTED                      "Supprimer la sélection"
#define K_S_FMANAGER_CLEAR_SELECTION                      "Effacer la sélection"
#define K_S_FMANAGER_CALC_INTERRUPTED                     "Non calculé"
#define K_S_FMANAGER_ARE_YOU_SURE_ALERT                   "Êtes-vous sûr ?"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE_FMT        "Cette opération supprimera le fichier %s\nContinuer : START\nQuitter : B"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_COUNT_FILES_FMT "Cette opération supprimera %d fichier(s)\nContinuer : START\nQuitter : B"
#define K_S_FMANAGER_SELECTED_ENTRIES_EXIT_FMT "%d fichiers sélectionnés\nConfirmer la sortie : START\nRetour : B"
#define K_S_FMANAGER_ABOUT_DIR_FMT \
    "Type : dossier\n"             \
    "Chemin : %s"

#define K_S_FMANAGER_ABOUT_FILE_FMT \
    "Type : fichier\n"               \
    "Taille : %s\n"                  \
    "MD5 : %s\n"
// TODO: MOVE MULTIBOOT TO SEPARATE APP
#define K_S_FMANAGER_MULTIBOOT_ABOUT_FMT            "%s\n\nTaille : %s"

#define K_S_FMANAGER_MULTIBOOT_STARTING             "Démarrage..."
#define K_S_FMANAGER_MULTIBOOT_ERROR_FMT            "Étape : %d\nCode : %d"
#define K_S_FMANAGER_SORTING                        "Tri en cours..."
#define K_S_FMANAGER_ALMOST_DONE                    "Presque prêt..."
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE      "Cette opération supprimera le fichier\n"
#define K_S_FMANAGER_NOT_ENOUGH_MEMORY_TO_FINISH_OP "Mémoire insuffisante pour terminer l'opération"
#define K_S_FMANAGER_SELECTED_FILES_FMT             "%d fichier(s) sélectionné(s)"
#define K_S_FMANAGER_CANT_DO_OP                     "Impossible de terminer l'opération"
#define K_S_FMANAGER_FILE_ADDED_TO_BUFFER_EXCHANGE  "Fichier ajouté au presse-papiers"
///////////////////////////////////////////////////////////////////////////////////////////////////////
// apps/liltracker/liltracker.cpp ////////////////////////////////////////////
#define K_S_LILTRACKER_SAVE_TRACK             "Enregistrer le morceau"
#define K_S_LILTRACKER_OPEN_TRACK             "Ouvrir le morceau"
#define K_S_LILTRACKER_CREATE_NEW_TRACK       "++ Créer nouveau"
#define K_S_LILTRACKER_ENTER_FILENAME         "Entrez le nom du fichier"
#define K_S_LILTRACKER_FILENAME_CANT_BE_EMPTY "Le nom du fichier ne peut pas être vide"
#define K_S_LILTRACKER_CLEAR_TRACK_DATA       "Effacer toutes les données\ndu morceau ?\n[START]Oui\n[B]Non"
#define K_S_LILTRACKER_OPEN                   "Ouvrir"
#define K_S_LILTRACKER_SAVE                   "Enregistrer"
#define K_S_LILTRACKER_RESET                  "Réinitialiser"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/lilcatalog/lilcatalog.cpp /////////////////////////////////////////////////////////////////////
#define K_S_LILCATALOG_APP                           "LilCatalogue"

#define K_S_LILCATALOG_APPS                          "Applications"
#define K_S_LILCATALOG_MODS                          "Mods"
#define K_S_LILCATALOG_STOP                          "Quitter"
#define K_S_LILCATALOG_BACK                          "Retour"
#define K_S_LILCATALOG_EMPTY                         ""

#define K_S_LILCATALOG_START                         "Lancer"
#define K_S_LILCATALOG_INSTALL                       "Installer"
#define K_S_LILCATALOG_REMOVE                        "Supprimer"
#define K_S_LILCATALOG_UPDATE                        "Mettre à jour"
#define K_S_LILCATALOG_SOURCE                        "Source"
#define K_S_LILCATALOG_ENTRY_DESCRIPTION             "Description"
#define K_S_LILCATALOG_LOADING                       "Téléchargement"
#define K_S_LILCATALOG_LOADING_CATALOG               "Chargement du catalogue..."
#define K_S_LILCATALOG_STARTING                      "Démarrage..."
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_NAME        "Nom : "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_AUTHOR      "Auteur : "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_DESCRIPTION "Description : "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_FILE        "Fichier : "

#define K_S_LILCATALOG_NEXT_PAGE                     ">> Page suivante"
#define K_S_LILCATALOG_PREV_PAGE                     "<< Page précédente"
#define K_S_LILCATALOG_SIZE                          "Taille :"
#define K_S_LILCATALOG_FILE_LOADING                  "Téléchargement du fichier..."
#define K_S_LILCATALOG_FILE_LOADING_COMPLETE         "Fichier téléchargé et enregistré"

#define K_S_LILCATALOG_ARCHIVE_NOTICE                "Les archives doivent être extraites manuellement"
#define K_S_LILCATALOG_UNSUPPORTED_TYPE              "Type de fichier non pris en charge"
#define K_S_LILCATALOG_CLEAR_CACHE                   "Vider le cache"
#define K_S_LILCATALOG_CACHE_CLEARED                 "Cache vidé"
#define K_S_LILCATALOG_INSTALLED                     "Installées"
#define K_S_LILCATALOG_NO_INSTALLED                  "Aucune application installée"
#define K_S_LILCATALOG_NO_BINARY                     "Cette application n'a pas de fichiers téléchargeables"
#define K_S_LILCATALOG_VIEW_SOURCE                   "Voir la source"

#define K_S_LILCATALOG_ERROR_CREATE_FOLDER           "Erreur de création du dossier"
#define K_S_LILCATALOG_ERROR_LOAD_CATALOG            "Erreur de chargement du catalogue"
#define K_S_LILCATALOG_ERROR_FILE_OPEN               "Erreur d'ouverture du fichier"
#define K_S_LILCATALOG_ERROR_CONNECTION              "Erreur de connexion : "
#define K_S_LILCATALOG_ERROR_DIRETORY_CREATE         "Erreur de création du répertoire"
#define K_S_LILCATALOG_ERROR_STAGE1                  "Étape : 1\nCode : "
#define K_S_LILCATALOG_ERROR_STAGE2                  "Étape : 2\nCode : "
#define K_S_LILCATALOG_ERROR_STAGE3                  "Étape : 3\nCode : "
#define K_S_LILCATALOG_SD_NOTFOUND                   "Carte SD introuvable. Impossible de continuer"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/letris.cpp //////////////////////////////////////////////////////////////////////////////
#define K_S_LETRIS_GAME_OVER      "Game over"
#define K_S_LETRIS_GAME_OVER_LONG "Partie terminée !\nVous avez essayé. :)"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/settings/sound.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SETTINGS_SOUND                "Son"
#define K_S_SETTINGS_SOUND_VOLUME         "Volume :"
#define K_S_SETTINGS_SOUND_STARTRUP       "Son au démarrage :"
#define K_S_SETTINGS_SOUND_BUZZER_STARTUP "Buzzer au démarrage :"
#define K_S_SETTINGS_SOUND_SAVE           "<< Enregistrer"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/screenshot.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SCREENSHOT_SAVED      "Capture d'écran enregistrée"
#define K_S_SCREENSHOT_SAVE_ERROR "Erreur de sauvegarde de la capture"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/network.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_NET_OFFLINE "Connexion WiFi perdue"
#define K_S_NET_ONLINE  "WiFi connecté"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/combo.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_COMBO_FOR_EXIT_HOLD            "Pour quitter, maintenez"
#define K_S_COMBO_HOLD_SELECT_FOR_TIME_FMT "[SELECT] pendant %d s"
#define K_S_COMBO_PRESS_START              "Appuyez sur [START]"
#define K_S_COMBO_TO_CONTINUE              "pour continuer"
#define K_S_COMBO_PRESS                    "Appuyez"
#define K_S_COMBO_HOLD_ANY                 "Maintenez n'importe quel"
#define K_S_COMBO_BUTTON_FOR_ONE_SEC       "bouton pendant 1 s"
#define K_S_COMBO_PRESS_TOGETHER           "Appuyez ensemble"
#define K_S_COMBO_TIMEOUT_FMT              "Délai %d ms"
#define K_S_COMBO_END                      "Fin !"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/weather/weather.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_WEATHER_CLEAR_SKY                     "Ciel dégagé" // 0

#define K_S_WEATHER_MOSTLY_CLEAR                  "Ciel dégagé" // 1
#define K_S_WEATHER_PARTLY_CLOUDY                 "Partiellement nuageux" // 2
#define K_S_WEATHER_OVERCAST                      "Couvert" //3

#define K_S_WEATHER_FOG                           "Brouillard" //45
#define K_S_WEATHER_FREEZING_FOG                  "Brouillard givrant" //48

#define K_S_WEATHER_LIGHT_DRIZZLE                 "Bruine légère" //51
#define K_S_WEATHER_DRIZZLE                       "Bruine" //53
#define K_S_WEATHER_HEAVY_DRIZZLE                 "Forte bruine" //55

#define K_S_WEATHER_LIGHT_FREEZING_DRIZZLE        "Bruine verglaçante légère" //56
#define K_S_WEATHER_HEAVY_FREEZING_DRIZZLE        "Forte bruine verglaçante" //57

#define K_S_WEATHER_LIGHT_RAIN                    "Pluie légère" //61
#define K_S_WEATHER_RAIN                          "Pluie" //63
#define K_S_WEATHER_HEAVY_RAIN                    "Forte pluie" // 65

#define K_S_WEATHER_LIGHT_FREEZING_RAIN           "Pluie verglaçante légère" // 66
#define K_S_WEATHER_HEAVY_FREEZING_RAIN           "Forte pluie verglaçante" //67

#define K_S_WEATHER_LIGHT_SNOW                    "Neige légère" // 71
#define K_S_WEATHER_SNOW                          "Neige" // 73
#define K_S_WEATHER_HEAVY_SNOW                    "Forte neige" // 75

#define K_S_WEATHER_SNOW_GRAINS                   "Grains de neige" // 77

#define K_S_WEATHER_LIGHT_SHOWERS                 "Averses légères" // 80
#define K_S_WEATHER_SHOWERS                       "Averses" // 81
#define K_S_WEATHER_HEAVY_SHOWERS                 "Fortes averses" // 82

#define K_S_WEATHER_RAIN_AND_SNOW                 "Pluie et neige" // 85
#define K_S_WEATHER_HEAVY_RAIN_AND_SNOW           "Forte pluie et neige" // 86

#define K_S_WEATHER_THUNDERSTORM                  "Orage" // 95
#define K_S_WEATHER_THUNDERSTORM_WITH_HAIL        "Orage avec grêle" // 96
#define K_S_WEATHER_SEVERE_THUNDERSTORM_WITH_HAIL "Violent orage avec grêle" // 99

#define K_S_WEATHER_LOADING_DATA                  "Chargement des données..."
#define K_S_WEATHER_DATA_PATTERN_ERROR            "Erreur de format des données"
#define K_S_WEATHER_DATA_LOAD_ERROR_FMT           "Erreur de chargement :\nCode de statut : %d"

#define K_S_WEATHER_LOCATION_NOT_SET              "Emplacement non défini"
#define K_S_WEATHER_SELECT_TO_SETUP               "[SELECT] - paramètres"
#define K_S_WEATHER_A_TO_EXIT                     "[A] - quitter" // LOL WHY
#define K_S_WEATHER_TEMP_FMT                      "%.1f °C"
#define K_S_WEATHER_WIND_SPEED_FMT                "%.1f km/h"

#define K_S_WEATHER_LATITUDE                      "Latitude"
#define K_S_WEATHER_LATITUDE_S                    "latitude"
#define K_S_WEATHER_LONGITUDE                     "Longitude"
#define K_S_WEATHER_LONGITUDE_S                   "longitude"

#define K_S_WEATHER_INPUT                         "Saisir"

#define K_S_WEATHER_SAVE                          "Enregistrer"
#define K_S_WEATHER_CANCEL                        "Annuler"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/usbdrive/usbdrive.cpp /////////////////////////////////////////////////////////////////////////
#define K_S_USB_DRIVE_TITLE                       "Clé USB"
#define K_S_USB_DRIVE_EXPERIMENTAL_TITLE          "Avertissement"
#define K_S_USB_DRIVE_EXPERIMENTAL_WARNING        "Fonctionnalité expérimentale.\n" \
                                                  "Serial sera désactivé.\n\n" \
                                                  "[START] - OK  [A] - Annuler"
#define K_S_USB_DRIVE_INITIALIZING                "Initialisation USB..."
#define K_S_USB_DRIVE_INIT_ERROR                  "Erreur d'initialisation USB"
#define K_S_USB_DRIVE_NO_SD                       "Carte SD non disponible"
#define K_S_USB_DRIVE_CONNECT_USB                 "Connectez le câble USB au PC"
#define K_S_USB_DRIVE_CONNECTED                   "Connecté au PC"
#define K_S_USB_DRIVE_EJECTED                     "Éjecté en toute sécurité"
#define K_S_USB_DRIVE_PC_INSTRUCTION              "Carte SD disponible sur le PC"
#define K_S_USB_DRIVE_SAFE_EJECT                  "Éjectez avant de quitter"
#define K_S_USB_DRIVE_PRESS_A_TO_EXIT             "[A] - quitter"
#define K_S_USB_DRIVE_DISCONNECTING               "Déconnexion..."
#define K_S_USB_DRIVE_REBOOT_REQUIRED             "L'appareil va redémarrer..."
#define K_S_USB_DRIVE_NOT_EJECTED                 "Non éjecté en toute sécurité !"
#define K_S_USB_DRIVE_EJECT_WARNING               "Les données pourraient être corrompues"
#define K_S_USB_DRIVE_PRESS_START_CONTINUE        "[START] - continuer"
#define K_S_USB_DRIVE_PRESS_B_CANCEL              "[B] - annuler"
#define K_S_LAUNCHER_USB_DRIVE                    "Clé USB"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/pastebin/pastebinApp.cpp //////////////////////////////////////////////////////////////////////
#define K_S_PASTEBIN_CODE                         "Code"
#define K_S_PASTEBIN_NAME                         "Nom"
#define K_S_PASTEBIN_DOWNLOAD                     "Télécharger"
#define K_S_PASTEBIN_ENTER_CODE                   "Entrez le code"
#define K_S_PASTEBIN_ENTER_NAME                   "Entrez le nom"
#define K_S_PASTEBIN_ERROR_CREATE_DIR             "Erreur de création du répertoire"
#define K_S_PASTEBIN_ERROR_OPEN_FILE              "Erreur d'ouverture du fichier"
#define K_S_PASTEBIN_FILE_DOWNLOADED_FMT          "Fichier téléchargé dans\n%s"
#define K_S_PASTEBIN_REQUEST_FAIL_FMT             "Erreur HTTP\n%s"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/mjs/mjsrunner.cpp /////////////////////////////////////////////////////////////////////////////
#define K_S_MJS_ERROR                             "Erreur : "
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/mjs/luarunner.cpp /////////////////////////////////////////////////////////////////////////////
#define K_S_LUA_ERROR                             "Erreur : "
#define K_S_LUA_AWAIT_CODE_FROM_UART              "En attente du code\nde UART...\n\nAppuyez sur [A]\npour quitter."
#define K_S_LUA_DOWNLOAD                          "Téléchargement..."
#define K_S_LUA_REPL_AWAIT_CODE_FROM_UART         "REPL:\nEn attente du code\nde UART...\n\nAppuyez sur [A]\npour quitter."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/tests/keyboard/keyboard.cpp //////////////////////////////////////////////////////////////////
#define K_S_KEYBOARD_ENTER_TEXT                    "Saisissez le texte : "
#define K_S_KEYBOARD_YOU_ENTERED                   "Vous avez saisi :"
//////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format on
