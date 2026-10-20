# Nofrendo NES emulator (arduino-nofrendo/ submodule)
set(root "${CMAKE_CURRENT_LIST_DIR}/arduino-nofrendo/src")

file(GLOB_RECURSE srcs "${root}/*.c")

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${root}"
    # The osd_* platform layer (osd.h) is implemented by Keira in src/apps/nes
    PRIV_REQUIRES src
)
