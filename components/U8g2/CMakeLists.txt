# U8g2 (U8g2_Arduino/ submodule)
# Only font data is used, so the u8x8 display drivers are not compiled.
set(root "${CMAKE_CURRENT_LIST_DIR}/U8g2_Arduino/src")

file(GLOB srcs "${root}/*.cpp" "${root}/clib/*.c")
list(FILTER srcs EXCLUDE REGEX "/clib/u8x8[^/]*\\.c$")

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${root}"
    REQUIRES arduino
)
