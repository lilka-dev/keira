# Keira OS — ESP-IDF project.
#
# Build:   idf.py build                       (board defaults to lilka_v2)
#          idf.py -DKEIRA_BOARD=<name> build  (any directory under boards/)
#          or via make, see make help
#
# Layout: src/ is the Keira component; components/<name>/ wraps each library, with the
# upstream code as a pinned git submodule inside. The ESP-IDF version is pinned per
# board in boards/<name>/board.cmake and checked below.

cmake_minimum_required(VERSION 3.16)

set(KEIRA_BOARD "lilka_v2" CACHE STRING "Target board, one of boards/<name>")
set(KEIRA_BOARD_DIR "${CMAKE_CURRENT_LIST_DIR}/boards/${KEIRA_BOARD}")
if(NOT EXISTS "${KEIRA_BOARD_DIR}/board.cmake")
    message(FATAL_ERROR "Unknown board '${KEIRA_BOARD}': ${KEIRA_BOARD_DIR}/board.cmake not found")
endif()

# Sets IDF_TARGET, KEIRA_IDF_VERSION and KEIRA_BOARD_DEFINITIONS
include("${KEIRA_BOARD_DIR}/board.cmake")

# Extra flags for a single build, e.g. -DKEIRA_BUILD_FLAGS="-DLANG_EN;-DFMANAGER_DEBUG"
set(KEIRA_BUILD_FLAGS "" CACHE STRING "Extra compile flags (semicolon separated)")

set(SDKCONFIG_DEFAULTS "${KEIRA_BOARD_DIR}/sdkconfig.defaults")
# Keep the generated sdkconfig next to the build, so switching boards is just switching build dirs
set(SDKCONFIG "${CMAKE_BINARY_DIR}/sdkconfig")

# components/* is picked up automatically
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/src"
    "$ENV{IDF_PATH}/examples/common_components/qrcode"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(idf_version "${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}.${IDF_VERSION_PATCH}")
if(NOT idf_version VERSION_EQUAL KEIRA_IDF_VERSION AND NOT "$ENV{KEIRA_SKIP_IDF_VERSION_CHECK}")
    message(FATAL_ERROR "Board ${KEIRA_BOARD} requires ESP-IDF v${KEIRA_IDF_VERSION}, "
                        "but IDF_PATH points to v${idf_version}. "
                        "Set KEIRA_SKIP_IDF_VERSION_CHECK=1 to build anyway.")
endif()

foreach(def ${KEIRA_BOARD_DEFINITIONS})
    idf_build_set_property(COMPILE_DEFINITIONS "-D${def}" APPEND)
endforeach()
foreach(flag ${KEIRA_BUILD_FLAGS})
    idf_build_set_property(COMPILE_OPTIONS "${flag}" APPEND)
endforeach()
idf_build_set_property(CXX_COMPILE_OPTIONS "-Wno-pmf-conversions" APPEND)

project(keira)

# IDF builds with -Werror=all; Keira and its libraries were never built that way under PlatformIO.
# The option list is evaluated at generate time, so dropping it here applies to every component.
idf_build_get_property(compile_options COMPILE_OPTIONS)
list(REMOVE_ITEM compile_options "-Werror=all")
idf_build_set_property(COMPILE_OPTIONS "${compile_options}")

# Filesystem image from data/spiffs/, flashed on demand only (idf.py spiffs-flash) so user files are not wiped
spiffs_create_partition_image(spiffs "${CMAKE_CURRENT_LIST_DIR}/data/spiffs")
