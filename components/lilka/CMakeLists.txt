# Lilka SDK (sdk/ submodule)
set(root "${CMAKE_CURRENT_LIST_DIR}/sdk/lib/lilka/src")

file(GLOB_RECURSE srcs "${root}/*.c" "${root}/*.cpp")
# Weak default setup()/loop(); Keira provides its own
list(REMOVE_ITEM srcs "${root}/main.cpp")

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${root}"
    REQUIRES arduino Arduino_GFX U8g2 driver fatfs spiffs app_update esp_wifi nvs_flash
)

# The SDK sources carry a placeholder version header (0.0.0/Dev). The real version is pinned here and
# force-included; it defines the same guard, so the placeholder is skipped.
# Keep in sync with the components/lilka/sdk submodule; checked against its tag when tags are fetched.
set(LILKA_SDK_VERSION "2.5.15")
set(LILKA_SDK_VERSION_TYPE SDK_VERSION_TYPE_RELEASE)

execute_process(
    COMMAND git describe --tags --exact-match
    WORKING_DIRECTORY "${root}"
    OUTPUT_VARIABLE sdk_tag
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(sdk_tag AND NOT sdk_tag STREQUAL "v${LILKA_SDK_VERSION}")
    message(FATAL_ERROR "components/lilka/sdk is at ${sdk_tag}, but LILKA_SDK_VERSION is ${LILKA_SDK_VERSION}. "
                        "Update LILKA_SDK_VERSION in ${CMAKE_CURRENT_LIST_FILE}.")
endif()

string(REPLACE "." ";" sdk_version_parts "${LILKA_SDK_VERSION}")
list(GET sdk_version_parts 0 SDK_VERSION_MAJOR)
list(GET sdk_version_parts 1 SDK_VERSION_MINOR)
list(GET sdk_version_parts 2 SDK_VERSION_PATCH)
set(SDK_VERSION_TYPE ${LILKA_SDK_VERSION_TYPE})

set(version_header "${CMAKE_CURRENT_BINARY_DIR}/lilka_sdk_version.h")
configure_file("${CMAKE_CURRENT_LIST_DIR}/lilka_sdk_version.h.in" "${version_header}")
target_compile_options(${COMPONENT_LIB} PUBLIC "-include${version_header}")
