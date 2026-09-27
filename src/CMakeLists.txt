# Keira OS application component
file(GLOB_RECURSE srcs "${CMAKE_CURRENT_LIST_DIR}/*.c" "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "."
    REQUIRES
        # Libraries
        arduino lilka Arduino_GFX U8g2 ArduinoJson ESPTelnet ESP8266Audio AnimatedGIF JPEGDEC lua nofrendo mJS
        SimpleFTPServer qrcode
        # ESP-IDF
        app_update driver esp_http_client esp_http_server esp_wifi fatfs mbedtls mdns mqtt nvs_flash spiffs vfs
)
