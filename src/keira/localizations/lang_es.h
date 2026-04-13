#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
// GUIDELINE: Keira all strings used in messages, or somehow written to serial, should be defined here

// clang-format off

// Multi purpose strings  /////////////////////////////////////////////////////////////////////////////
#define K_S_CURRENT_LANGUAGE_SHORT      "es"
#define K_S_CURRENT_LANGUAGE_FULL       "Español"

#define K_S_ERROR                       "Error"
#define K_S_ATTENTION                   "Atención"
#define K_S_SUCCESS                     "Éxito"
#define K_S_PLEASE_STANDBY              "Por favor espere..."
#define K_S_LILKA_V2_OR_HIGHER_REQUIRED "Este programa requiere Lilka V2 o superior"
#define K_S_FEATURE_IN_DEVELOPMENT      "Esta función está en desarrollo"
#define K_S_CANT_OPEN_DIR_FMT           "No se puede abrir el directorio %s"
#define K_S_CANT_OPEN_FILE_FMT          "No se puede abrir el archivo %s"
#define K_S_CANT_CREATE_DIR_FMT         "No se puede crear el directorio %s"
#define K_S_CANT_REMOVE_FILE_FMT        "No se puede eliminar el archivo %s"
#define K_S_DIR_EMPTY_FMT               "El directorio %s está vacío"

#define K_S_MENU_BACK                   "<< Atrás"

#define K_S_PASSWORD                    "Contraseña"
#define K_S_USER                        "Usuario"

#define K_S_SETTINGS                    "Configuración"
#define K_S_ON                          "ON"
#define K_S_OFF                         "OFF"
#define K_S_STATUS                      "Estado"

#define K_S_SERVICES                    "Servicios"
#define K_S_FTP                         "FTP"
#define K_S_WEB                         "Web"
#define K_S_TELNET                      "Telnet"
#define K_S_MDNS                        "mDNS"
#define K_S_LAUNCHER_MDNS               K_S_MDNS
#define K_S_LAUNCHER_MDNS_HOSTNAME      "Nombre de host"
#define K_S_LAUNCHER_MDNS_ENTER_HOSTNAME "Introduzca el nombre de host:"
#define K_S_CHANGE_ON_NEXT_BOOT         "Los cambios se aplicarán en el próximo arranque"
#define K_S_OS_NAME                     "Keira OS"
#define K_S_OS_DESCRIPTION              "by Anderson & friends"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// service.cpp  ///////////////////////////////////////////////////////////////////////////////////////
#define K_S_SERVICE_STARTUP_FMT "Starting service %s"
#define K_S_SERVICE_DIE_FMT     "Service %s died"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/launcher.cpp //////////////////////////////////////////////////////////////////////////////////
#define K_S_LAUNCHER_MAIN_MENU         "Menú principal"
#define K_S_LAUNCHER_APPS              "Aplicaciones"
#define K_S_LAUNCHER_DEMOS             "Demo"
#define K_S_LAUNCHER_LINES             "Líneas"
#define K_S_LAUNCHER_DISK              "Disco"
#define K_S_LAUNCHER_TRANSFORM         "Transformación"
#define K_S_LAUNCHER_BALL              "Pelota"
#define K_S_LAUNCHER_CUBE              "Cubo"
#define K_S_LAUNCHER_EPILEPSY          "Epilepsia"
#define K_S_LAUNCHER_PET_PET           "PetPet"
#define K_S_LAUNCHER_TESTS             "Pruebas"
#define K_S_LAUNCHER_KEYBOARD          "Teclado"
#define K_S_LAUNCHER_SPI_TEST          "Test SPI"
#define K_S_LAUNCHER_I2C_SCANNER       "I2C-escáner"
#define K_S_LAUNCHER_GPIO_MANAGER      "GPIO-gestor"
#define K_S_LAUNCHER_COMBO             "Combo" // wtf?
#define K_S_LAUNCHER_CALLBACK_TEST     "CallbackTest"
#define K_S_LAUNCHER_LILCATALOG        "LilCatálogo"
#define K_S_LAUNCHER_LILTRACKER        "LilTracker"
#define K_S_LAUNCHER_LETRIS            "Letris"
#define K_S_LAUNCHER_TAMAGOTCHI        "Tamagotchi"
#define K_S_LAUNCHER_WEATHER           "Clima"
#define K_S_LAUNCHER_PASTEBIN          "Pastebin"
#define K_S_LAUNCHER_FMANAGER          "Gestor de archivos"
#define K_S_LAUNCHER_DEV_MENU          "Desarrollo"
#define K_S_LAUNCHER_LIVE_LUA          "Live Lua" // Lua live?
#define K_S_LAUNCHER_LUA_REPL          "Lua REPL"
#define K_S_LAUNCHER_WIFI              "WiFi"
#define K_S_LAUNCHER_WIFI_ADAPTER      "WiFi"
#define K_S_LAUNCHER_WIFI_NETWORKS     "Redes WiFi"
#define K_S_LAUNCHER_WIFI_TX_POWER     "Potencia WiFi"
#define K_S_LAUNCHER_SD                "SD"
#define K_S_LAUNCHER_SD_FORMAT         "Formatear SD"
#define K_S_LAUNCHER_SD_SPEED          "Frecuencia de tarjeta SD"
#define K_S_LAUNCHER_SOUND             "Sonido"
#define K_S_LAUNCHER_SERVICES          K_S_SERVICES
#define K_S_LAUNCHER_WEB               K_S_WEB
#define K_S_LAUNCHER_TELNET            K_S_TELNET
#define K_S_LAUNCHER_FTP               K_S_FTP
#define K_S_LAUNCHER_FTP_USER          K_S_USER
#define K_S_LAUNCHER_FTP_PASSWORD      K_S_PASSWORD
#define K_S_LAUNCHER_FTP_IP            "IP"
#define K_S_LAUNCHER_STATUSBAR         "Barra de estado"
#define K_S_LAUNCHER_CLOCK             "Reloj"
#define K_S_LAUNCHER_CLOCK_0           "Ninguno"
#define K_S_LAUNCHER_CLOCK_1           "HH:MM:SS"
#define K_S_LAUNCHER_CLOCK_2           "HH:MM"
#define K_S_LAUNCHER_CLOCK_3           "*HH:MM"
#define K_S_LAUNCHER_MEM               "Memoria"
#define K_S_LAUNCHER_MEM_0             "Ninguno"
#define K_S_LAUNCHER_MEM_1             "Icono"
#define K_S_LAUNCHER_MEM_2             "Texto"
#define K_S_LAUNCHER_NETWORK           K_S_LAUNCHER_WIFI
#define K_S_LAUNCHER_NETWORK_0         "Ninguno"
#define K_S_LAUNCHER_NETWORK_1         "Icono"
#define K_S_LAUNCHER_BATTERY           "Batería"
#define K_S_LAUNCHER_BATTERY_0         "Ninguno"
#define K_S_LAUNCHER_BATTERY_1         "Icono + texto"
#define K_S_LAUNCHER_BATTERY_2         "Icono"
#define K_S_LAUNCHER_BATTERY_3         "Texto"

#define K_S_LAUNCHER_ABOUT_SYSTEM      "Acerca del sistema"
#define K_S_LAUNCHER_DEVICE_INFO       "Acerca del dispositivo"
#define K_S_PARTITION_TABLE            "Tabla de particiones"
#define K_S_LAUNCHER_LIGHT_SLEEP       "Light sleep"
#define K_S_LAUNCHER_DEEP_SLEEP        "Deep sleep"
#define K_S_LAUNCHER_REBOOT            "Reiniciar"

#define K_S_LAUNCHER_SELECT_TX_POWER   "Ajustar potencia"

#define K_S_LAUNCHER_ENABLE_WIFI_FIRST "Primero active el WiFi"

#define K_S_LAUNCHER_DEVICE_INFO_FMT \
    "Modelo: %s\n"                    \
    "Revisión: %d\n"                  \
    "Versión ESP-IDF: %s\n"          \
    "Frecuencia: %d MHz\n"            \
    "Número de núcleos: %d\n"         \
    "IP: %s"

#define K_S_LAUNCHER_PARTITION_FMT "Dirección: 0x%s\nTamaño: 0x%s"

#define K_S_LAUNCHER_FORMAT        "Formateando"

#define K_S_LAUNCHER_FORMAT_DISCLAIMER_ALERT                         \
    "¡ATENCIÓN: Se eliminarán todos los datos de la tarjeta SD!\n"   \
    "\n¿Continuar?\n\nSTART - continuar\nA - salir"

#define K_S_LAUNCHER_PLEASE_STANDBY K_S_PLEASE_STANDBY

#define K_S_LAUNCHER_FORMAT_ERROR_ALERT               \
    "No se pudo formatear la tarjeta SD.\n\n"          \
    "El sistema se reiniciará."

#define K_S_LAUNCHER_FORMAT_SUCCESS_ALLERT             \
    "¡Tarjeta SD formateada con éxito!\n\n"            \
    "El sistema se reiniciará."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/wifi_config.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_WIFI_CONFIG_SCANING_NETWORKS    "Buscando redes WiFi..."
#define K_S_WIFI_CONFIG_SCAN_ERROR_CODE_FMT "No se pueden escanear redes, código de error: %d"
#define K_S_WIFI_CONFIG_NETWORKS            "Redes"

#define K_S_WIFI_CONFIG_ABOUT_NETWORK_FMT \
    "Canal: %d\n"                         \
    "Intensidad de señal: %ddb\n"        \
    "MAC: %s\n"                           \
    "Seguridad: %s"

#define K_S_WIFI_CONFIG_CONNECTING_TO_OPEN_INSECURE_NETWORK_FMT \
    "Se está conectando a la red insegura %s\n\n"                \
    "A - continuar\n"                                              \
    "B - elegir otra red"

#define K_S_WIFI_CONFIG_ENTER_PASSWORD              "Introduzca la contraseña:"
#define K_S_WIFI_CONFIG_CONNECTING                  "Conectando..."
#define K_S_WIFI_CONFIG_CONNECTED_TO_NETWORK_FMT    "Conectado a %s"
#define K_S_WIFI_CONFIG_CANT_CONNECT_TO_NETWORK_FMT "No se pudo conectar a %s"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/scan_i2c.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_I2C_INIT_ABOUT            "I2C init: SDA=%d, SCL=%d"
#define K_S_I2C_SCANNER_SCAN_START    "Starting I2C scan..."
#define K_S_I2C_SCANNER_SCAN_DONE     "I2C scan done."
#define K_S_I2C_SCANNER_DEVICES_FOUND "Found %d devices."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/transform.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_TRANSFORM_CANT_LOAD_FACE     "No se puede cargar face.bmp desde la tarjeta SD." // FACEPALM.BMP
#define K_S_TRANFORM_DRAWING_FACE_AT_FMT "Drawing face at %d, %d"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/fmanager.cpp/.h ///////////////////////////////////////////////////////////////////////////////
#define K_S_FMANAGER_COPYING                              "Copiando..."
#define K_S_FMANAGER_MD5_CALC                             "Calculando MD5"
#define K_S_FMANAGER_LOADING                              "Cargando"
#define K_S_FMANAGER_ENTER_NEW_FOLDER_NAME                "Introduzca el nombre de la nueva carpeta"
#define K_S_FMANAGER_ENTER_NEW_NAME                       "Introduzca el nuevo nombre"
#define K_S_FMANAGER_OPTIONS                              "Opciones"
#define K_S_FMANAGER_OPEN                                 "Abrir"
#define K_S_FMANAGER_OPEN_WITH                            "Abrir con"
#define K_S_FMANAGER_CREATE_FOLDER                        "Crear carpeta"
#define K_S_FMANAGER_RENAME                               "Renombrar"
#define K_S_FMANAGER_DELETE                                "Eliminar"
#define K_S_FMANAGER_INFO_ABOUT_FILE                      "Información"
#define K_S_FMANAGER_SELECT_APP                           "Seleccionar aplicación"
#define K_S_FMANAGER_FILE_MANAGER                         "Gestor de archivos"
#define K_S_FMANAGER_NES_EMULATOR                         "Emulador NES"
#define K_S_FMANAGER_FIRMWARE_LOADER                      "Cargador de firmware"
#define K_S_FMANAGER_LUA                                  "Lua"
#define K_S_FMANAGER_MJS                                  "mJS"
#define K_S_FMANAGER_LILTRACKER                           K_S_LAUNCHER_LILTRACKER
#define K_S_FMANAGER_MAD_PLAYER                           "MadPlayer"
#define K_S_FMANAGER_DYNAPP                               "DynApp (.so)"
#define K_S_FMANAGER_ACTIONS_ON_SELECTED                  "Acciones sobre selección"
#define K_S_FMANAGER_COPY_SELECTED                        "Copiar selección"
#define K_S_FMANAGER_MOVE_SELECTED                        "Mover selección"
#define K_S_FMANAGER_DELETE_SELECTED                      "Eliminar selección"
#define K_S_FMANAGER_CLEAR_SELECTION                      "Limpiar selección"
#define K_S_FMANAGER_CALC_INTERRUPTED                     "No calculado"
#define K_S_FMANAGER_ARE_YOU_SURE_ALERT                   "¿Está seguro?"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE_FMT        "Esta operación eliminará el archivo %s\nContinuar: START\nSalir: B"
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_COUNT_FILES_FMT "Esta operación eliminará %d archivo(s)\nContinuar: START\nSalir: B"
#define K_S_FMANAGER_SELECTED_ENTRIES_EXIT_FMT "%d archivos seleccionados\nConfirmar salida: START\nVolver: B"
#define K_S_FMANAGER_ABOUT_DIR_FMT \
    "Tipo: directorio\n"            \
    "Ruta: %s"

#define K_S_FMANAGER_ABOUT_FILE_FMT \
    "Tipo: archivo\n"                \
    "Tamaño: %s\n"                   \
    "MD5: %s\n"
// TODO: MOVE MULTIBOOT TO SEPARATE APP
#define K_S_FMANAGER_MULTIBOOT_ABOUT_FMT            "%s\n\nTamaño: %s"

#define K_S_FMANAGER_MULTIBOOT_STARTING             "Iniciando..."
#define K_S_FMANAGER_MULTIBOOT_ERROR_FMT            "Etapa: %d\nCódigo: %d"
#define K_S_FMANAGER_SORTING                        "Ordenando..."
#define K_S_FMANAGER_ALMOST_DONE                    "Casi listo..."
#define K_S_FMANAGER_THIS_OP_WOULD_DELETE_FILE      "Esta operación eliminará el archivo\n"
#define K_S_FMANAGER_NOT_ENOUGH_MEMORY_TO_FINISH_OP "No hay suficiente memoria para completar la operación"
#define K_S_FMANAGER_SELECTED_FILES_FMT             "%d archivo(s) seleccionado(s)"
#define K_S_FMANAGER_CANT_DO_OP                     "No se puede completar la operación"
#define K_S_FMANAGER_FILE_ADDED_TO_BUFFER_EXCHANGE  "Archivo agregado al portapapeles"
///////////////////////////////////////////////////////////////////////////////////////////////////////
// apps/liltracker/liltracker.cpp ////////////////////////////////////////////
#define K_S_LILTRACKER_SAVE_TRACK             "Guardar pista"
#define K_S_LILTRACKER_OPEN_TRACK             "Abrir pista"
#define K_S_LILTRACKER_CREATE_NEW_TRACK       "++ Crear nueva"
#define K_S_LILTRACKER_ENTER_FILENAME         "Introduzca nombre de archivo"
#define K_S_LILTRACKER_FILENAME_CANT_BE_EMPTY "El nombre de archivo no puede estar vacío"
#define K_S_LILTRACKER_CLEAR_TRACK_DATA       "¿Borrar todos los datos\nde la pista?\n[START]Sí\n[B]No"
#define K_S_LILTRACKER_OPEN                   "Abrir"
#define K_S_LILTRACKER_SAVE                   "Guardar"
#define K_S_LILTRACKER_RESET                  "Restablecer"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/lilcatalog/lilcatalog.cpp /////////////////////////////////////////////////////////////////////
#define K_S_LILCATALOG_APP                           "LilCatálogo"

#define K_S_LILCATALOG_APPS                          "Aplicaciones"
#define K_S_LILCATALOG_MODS                          "Mods"
#define K_S_LILCATALOG_STOP                          "Salir"
#define K_S_LILCATALOG_BACK                          "Atrás"
#define K_S_LILCATALOG_EMPTY                         ""

#define K_S_LILCATALOG_START                         "Ejecutar"
#define K_S_LILCATALOG_INSTALL                       "Instalar"
#define K_S_LILCATALOG_REMOVE                        "Eliminar"
#define K_S_LILCATALOG_UPDATE                        "Actualizar"
#define K_S_LILCATALOG_SOURCE                        "Fuente"
#define K_S_LILCATALOG_ENTRY_DESCRIPTION             "Descripción"
#define K_S_LILCATALOG_LOADING                       "Descargando"
#define K_S_LILCATALOG_LOADING_CATALOG               "Cargando catálogo..."
#define K_S_LILCATALOG_STARTING                      "Iniciando..."
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_NAME        "Nombre: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_AUTHOR      "Autor: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_DESCRIPTION "Descripción: "
#define K_S_LILCATALOG_ENTRY_DESCRIPTION_FILE        "Archivo: "

#define K_S_LILCATALOG_NEXT_PAGE                     ">> Siguiente página"
#define K_S_LILCATALOG_PREV_PAGE                     "<< Página anterior"
#define K_S_LILCATALOG_SIZE                          "Tamaño:"
#define K_S_LILCATALOG_FILE_LOADING                  "Descargando archivo..."
#define K_S_LILCATALOG_FILE_LOADING_COMPLETE         "Archivo descargado y guardado"

#define K_S_LILCATALOG_ARCHIVE_NOTICE                "Los archivos comprimidos deben extraerse manualmente"
#define K_S_LILCATALOG_UNSUPPORTED_TYPE              "Tipo de archivo no soportado"
#define K_S_LILCATALOG_CLEAR_CACHE                   "Limpiar caché"
#define K_S_LILCATALOG_CACHE_CLEARED                 "Caché limpiada"
#define K_S_LILCATALOG_INSTALLED                     "Instaladas"
#define K_S_LILCATALOG_NO_INSTALLED                  "No hay aplicaciones instaladas"
#define K_S_LILCATALOG_NO_BINARY                     "Esta aplicación no tiene archivos descargables"
#define K_S_LILCATALOG_VIEW_SOURCE                   "Ver fuente"

#define K_S_LILCATALOG_ERROR_CREATE_FOLDER           "Error al crear carpeta"
#define K_S_LILCATALOG_ERROR_LOAD_CATALOG            "Error al cargar catálogo"
#define K_S_LILCATALOG_ERROR_FILE_OPEN               "Error al abrir archivo"
#define K_S_LILCATALOG_ERROR_CONNECTION              "Error de conexión: "
#define K_S_LILCATALOG_ERROR_DIRETORY_CREATE         "Error al crear directorio"
#define K_S_LILCATALOG_ERROR_STAGE1                  "Etapa: 1\nCódigo: "
#define K_S_LILCATALOG_ERROR_STAGE2                  "Etapa: 2\nCódigo: "
#define K_S_LILCATALOG_ERROR_STAGE3                  "Etapa: 3\nCódigo: "
#define K_S_LILCATALOG_SD_NOTFOUND                   "Tarjeta SD no encontrada. No se puede continuar"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/letris.cpp //////////////////////////////////////////////////////////////////////////////
#define K_S_LETRIS_GAME_OVER      "Game over"
#define K_S_LETRIS_GAME_OVER_LONG "¡Fin del juego!\nLo intentaste. :)"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/settings/sound.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SETTINGS_SOUND                "Sonido"
#define K_S_SETTINGS_SOUND_VOLUME         "Volumen:"
#define K_S_SETTINGS_SOUND_STARTRUP       "Sonido de inicio:"
#define K_S_SETTINGS_SOUND_BUZZER_STARTUP "Zumbador al inicio:"
#define K_S_SETTINGS_SOUND_SAVE           "<< Guardar"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/screenshot.cpp ////////////////////////////////////////////////////////////////////////////
#define K_S_SCREENSHOT_SAVED      "Captura de pantalla guardada"
#define K_S_SCREENSHOT_SAVE_ERROR "Error al guardar la captura"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// services/network.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_NET_OFFLINE "Conexión WiFi perdida"
#define K_S_NET_ONLINE  "WiFi conectado"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/demos/combo.cpp ///////////////////////////////////////////////////////////////////////////////
#define K_S_COMBO_FOR_EXIT_HOLD            "Para salir mantenga"
#define K_S_COMBO_HOLD_SELECT_FOR_TIME_FMT "[SELECT] durante %d s"
#define K_S_COMBO_PRESS_START              "Pulse [START]"
#define K_S_COMBO_TO_CONTINUE              "para continuar"
#define K_S_COMBO_PRESS                    "Pulse"
#define K_S_COMBO_HOLD_ANY                 "Mantenga cualquier"
#define K_S_COMBO_BUTTON_FOR_ONE_SEC       "botón durante 1 s"
#define K_S_COMBO_PRESS_TOGETHER           "Pulse juntos"
#define K_S_COMBO_TIMEOUT_FMT              "Tiempo límite %d ms"
#define K_S_COMBO_END                      "¡Fin!"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/weather/weather.cpp ///////////////////////////////////////////////////////////////////////////
#define K_S_WEATHER_CLEAR_SKY                     "Cielo despejado" // 0

#define K_S_WEATHER_MOSTLY_CLEAR                  "Mayormente despejado" // 1
#define K_S_WEATHER_PARTLY_CLOUDY                 "Parcialmente nublado" // 2
#define K_S_WEATHER_OVERCAST                      "Nublado" //3

#define K_S_WEATHER_FOG                           "Niebla" //45
#define K_S_WEATHER_FREEZING_FOG                  "Niebla helada" //48

#define K_S_WEATHER_LIGHT_DRIZZLE                 "Llovizna ligera" //51
#define K_S_WEATHER_DRIZZLE                       "Llovizna" //53
#define K_S_WEATHER_HEAVY_DRIZZLE                 "Llovizna fuerte" //55

#define K_S_WEATHER_LIGHT_FREEZING_DRIZZLE        "Llovizna helada ligera" //56
#define K_S_WEATHER_HEAVY_FREEZING_DRIZZLE        "Llovizna helada fuerte" //57

#define K_S_WEATHER_LIGHT_RAIN                    "Lluvia ligera" //61
#define K_S_WEATHER_RAIN                          "Lluvia" //63
#define K_S_WEATHER_HEAVY_RAIN                    "Lluvia fuerte" // 65

#define K_S_WEATHER_LIGHT_FREEZING_RAIN           "Lluvia helada ligera" // 66
#define K_S_WEATHER_HEAVY_FREEZING_RAIN           "Lluvia helada fuerte" //67

#define K_S_WEATHER_LIGHT_SNOW                    "Nieve ligera" // 71
#define K_S_WEATHER_SNOW                          "Nieve" // 73
#define K_S_WEATHER_HEAVY_SNOW                    "Nieve intensa" // 75

#define K_S_WEATHER_SNOW_GRAINS                   "Granos de nieve" // 77

#define K_S_WEATHER_LIGHT_SHOWERS                 "Chubascos ligeros" // 80
#define K_S_WEATHER_SHOWERS                       "Chubascos" // 81
#define K_S_WEATHER_HEAVY_SHOWERS                 "Chubascos fuertes" // 82

#define K_S_WEATHER_RAIN_AND_SNOW                 "Lluvia con nieve" // 85
#define K_S_WEATHER_HEAVY_RAIN_AND_SNOW           "Lluvia fuerte con nieve" // 86

#define K_S_WEATHER_THUNDERSTORM                  "Tormenta" // 95
#define K_S_WEATHER_THUNDERSTORM_WITH_HAIL        "Tormenta con granizo" // 96
#define K_S_WEATHER_SEVERE_THUNDERSTORM_WITH_HAIL "Tormenta severa con granizo" // 99

#define K_S_WEATHER_LOADING_DATA                  "Cargando datos..."
#define K_S_WEATHER_DATA_PATTERN_ERROR            "Error en formato de datos"
#define K_S_WEATHER_DATA_LOAD_ERROR_FMT           "Error al cargar datos:\nCódigo de estado: %d"

#define K_S_WEATHER_LOCATION_NOT_SET              "Ubicación no configurada"
#define K_S_WEATHER_SELECT_TO_SETUP               "[SELECT] - configuración"
#define K_S_WEATHER_A_TO_EXIT                     "[A] - salir" // LOL WHY
#define K_S_WEATHER_TEMP_FMT                      "%.1f °C"
#define K_S_WEATHER_WIND_SPEED_FMT                "%.1f km/h"

#define K_S_WEATHER_LATITUDE                      "Latitud"
#define K_S_WEATHER_LATITUDE_S                    "latitud"
#define K_S_WEATHER_LONGITUDE                     "Longitud"
#define K_S_WEATHER_LONGITUDE_S                   "longitud"

#define K_S_WEATHER_INPUT                         "Introducir"

#define K_S_WEATHER_SAVE                          "Guardar"
#define K_S_WEATHER_CANCEL                        "Cancelar"

///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/usbdrive/usbdrive.cpp /////////////////////////////////////////////////////////////////////////
#define K_S_USB_DRIVE_TITLE                       "Unidad USB"
#define K_S_USB_DRIVE_EXPERIMENTAL_TITLE          "Advertencia"
#define K_S_USB_DRIVE_EXPERIMENTAL_WARNING        "Función experimental.\n" \
                                                  "Serial se desactivará.\n\n" \
                                                  "[START] - OK  [A] - Cancelar"
#define K_S_USB_DRIVE_INITIALIZING                "Inicializando USB..."
#define K_S_USB_DRIVE_INIT_ERROR                  "Error de inicialización USB"
#define K_S_USB_DRIVE_NO_SD                       "Tarjeta SD no disponible"
#define K_S_USB_DRIVE_CONNECT_USB                 "Conecte el cable USB al PC"
#define K_S_USB_DRIVE_CONNECTED                   "Conectado al PC"
#define K_S_USB_DRIVE_EJECTED                     "Expulsado de forma segura"
#define K_S_USB_DRIVE_PC_INSTRUCTION              "Tarjeta SD disponible en el PC"
#define K_S_USB_DRIVE_SAFE_EJECT                  "Expulse antes de salir"
#define K_S_USB_DRIVE_PRESS_A_TO_EXIT             "[A] - salir"
#define K_S_USB_DRIVE_DISCONNECTING               "Desconectando..."
#define K_S_USB_DRIVE_REBOOT_REQUIRED             "El dispositivo se reiniciará..."
#define K_S_USB_DRIVE_NOT_EJECTED                 "¡No se expulsó de forma segura!"
#define K_S_USB_DRIVE_EJECT_WARNING               "Los datos pueden estar dañados"
#define K_S_USB_DRIVE_PRESS_START_CONTINUE        "[START] - continuar"
#define K_S_USB_DRIVE_PRESS_B_CANCEL              "[B] - cancelar"
#define K_S_LAUNCHER_USB_DRIVE                    "Unidad USB"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/pastebin/pastebinApp.cpp //////////////////////////////////////////////////////////////////////
#define K_S_PASTEBIN_CODE                         "Código"
#define K_S_PASTEBIN_NAME                         "Nombre"
#define K_S_PASTEBIN_DOWNLOAD                     "Descargar"
#define K_S_PASTEBIN_ENTER_CODE                   "Introduzca el código"
#define K_S_PASTEBIN_ENTER_NAME                   "Introduzca el nombre"
#define K_S_PASTEBIN_ERROR_CREATE_DIR             "Error al crear directorio"
#define K_S_PASTEBIN_ERROR_OPEN_FILE              "Error al abrir archivo"
#define K_S_PASTEBIN_FILE_DOWNLOADED_FMT          "Archivo descargado en\n%s"
#define K_S_PASTEBIN_REQUEST_FAIL_FMT             "Error HTTP\n%s"
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/mjs/mjsrunner.cpp /////////////////////////////////////////////////////////////////////////////
#define K_S_MJS_ERROR                             "Error: "
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/mjs/luarunner.cpp /////////////////////////////////////////////////////////////////////////////
#define K_S_LUA_ERROR                             "Error: "
#define K_S_LUA_AWAIT_CODE_FROM_UART              "Esperando código\nde UART...\n\nPulse [A]\npara salir."
#define K_S_LUA_DOWNLOAD                          "Descargando..."
#define K_S_LUA_REPL_AWAIT_CODE_FROM_UART         "REPL:\nEsperando código\nde UART...\n\nPulse [A]\npara salir."
///////////////////////////////////////////////////////////////////////////////////////////////////////

// apps/tests/keyboard/keyboard.cpp //////////////////////////////////////////////////////////////////
#define K_S_KEYBOARD_ENTER_TEXT                    "Introduzca texto: "
#define K_S_KEYBOARD_YOU_ENTERED                   "Ha introducido:"
//////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format on
