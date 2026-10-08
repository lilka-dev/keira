# JPEGDEC (JPEGDEC/ submodule), with ESP32-S3 SIMD routines
set(root "${CMAKE_CURRENT_LIST_DIR}/JPEGDEC/src")

file(GLOB srcs "${root}/*.cpp" "${root}/*.S")

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${root}"
    REQUIRES arduino esp-dsp
)
