# Arduino_GFX (Arduino_GFX/ submodule)
# Only the ST7789 display over ESP32 SPI is used by Lilka, so other drivers are not compiled.
set(root "${CMAKE_CURRENT_LIST_DIR}/Arduino_GFX/src")

file(GLOB srcs "${root}/*.cpp" "${root}/canvas/*.cpp")
# Factory for every supported display, would pull in all drivers
list(REMOVE_ITEM srcs "${root}/Arduino_GFX_Library.cpp")
list(APPEND srcs
    "${root}/databus/Arduino_ESP32SPI.cpp"
    "${root}/databus/Arduino_HWSPI.cpp"
    "${root}/display/Arduino_ST7789.cpp"
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${root}"
    # U8g2 font support is enabled by __has_include(<U8g2lib.h>), so it must be visible here
    REQUIRES arduino U8g2 esp_lcd driver
)
