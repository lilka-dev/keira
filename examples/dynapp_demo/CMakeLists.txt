# CMakeLists.txt — Build Keira DynApp .so files
#
# Prerequisites:
#   - xtensa-esp32s3-elf-gcc toolchain (e.g. from PlatformIO)
#   - CMake 3.16+
#
# Usage:
#   mkdir build && cd build
#   cmake -DCMAKE_TOOLCHAIN_FILE=../xtensa-esp32s3.cmake ..
#   make
#
# Or with a custom toolchain path:
#   cmake -DCMAKE_TOOLCHAIN_FILE=../xtensa-esp32s3.cmake \
#         -DXTENSA_TOOLCHAIN_PATH=/path/to/bin ..
#   make

cmake_minimum_required(VERSION 3.16)

project(keira_dynapp_demo C)

# ── Source files ─────────────────────────────────────────────────────────────
set(SOURCES main.c)

# ── Output name ──────────────────────────────────────────────────────────────
set(OUTPUT_NAME demo.so)

# ── Compile flags ────────────────────────────────────────────────────────────
set(DYNAPP_C_FLAGS
    -c -fPIC -Os -Wall -Wextra -nostdlib -std=gnu11
    -fdata-sections -ffunction-sections -fvisibility=hidden
)

# ── Link flags ───────────────────────────────────────────────────────────────
set(DYNAPP_LINK_FLAGS
    -shared -fPIC -nostdlib -nostartfiles -static-libgcc
    -e app_main
    -fdata-sections -ffunction-sections
    -Wl,--gc-sections
    -fvisibility=hidden
    -Wl,--strip-all -Wl,--strip-debug
    -Wl,--allow-shlib-undefined
)

# ── Compile objects ──────────────────────────────────────────────────────────
# We use custom commands because CMake's Generic platform does not support
# SHARED libraries. This produces a proper ELF shared object for ESP32-S3.

set(OBJECTS "")
foreach(SRC ${SOURCES})
    get_filename_component(OBJ_NAME ${SRC} NAME_WE)
    set(OBJ_FILE "${CMAKE_CURRENT_BINARY_DIR}/${OBJ_NAME}.o")
    add_custom_command(
        OUTPUT  ${OBJ_FILE}
        COMMAND ${CMAKE_C_COMPILER} ${DYNAPP_C_FLAGS}
                -I${CMAKE_CURRENT_SOURCE_DIR}
                -o ${OBJ_FILE}
                ${CMAKE_CURRENT_SOURCE_DIR}/${SRC}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SRC}
                ${CMAKE_CURRENT_SOURCE_DIR}/keira_api.h
        COMMENT "Compiling ${SRC}"
    )
    list(APPEND OBJECTS ${OBJ_FILE})
endforeach()

# ── Link shared object ──────────────────────────────────────────────────────
set(SO_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_NAME}")

add_custom_command(
    OUTPUT  ${SO_OUTPUT}
    COMMAND ${CMAKE_C_COMPILER} ${DYNAPP_LINK_FLAGS}
            -o ${SO_OUTPUT} ${OBJECTS}
    DEPENDS ${OBJECTS}
    COMMENT "Linking ${OUTPUT_NAME}"
)

add_custom_target(${PROJECT_NAME} ALL DEPENDS ${SO_OUTPUT})

# ── Post-build info ─────────────────────────────────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} -A ${SO_OUTPUT} 2>/dev/null || true
    COMMENT "Build complete: ${OUTPUT_NAME}"
)
