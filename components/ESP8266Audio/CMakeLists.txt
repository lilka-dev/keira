# ESP8266Audio (ESP8266Audio/ submodule)
set(root "${CMAKE_CURRENT_LIST_DIR}/ESP8266Audio/src")

file(GLOB_RECURSE srcs "${root}/*.c" "${root}/*.cpp")

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${root}"
    REQUIRES arduino driver
)
