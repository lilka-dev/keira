# PlatformIO pre-script: build Keira with the lilka-bootloader boot menu (see boot/README.md).
# `board_build.arduino.custom_bootloader` is resolved inside the framework's variants dir,
# so pass it as an absolute path.
from os.path import join

Import("env")

env.BoardConfig().update(
    "build.arduino.custom_bootloader", join(env.subst("$PROJECT_DIR"), "boot", "lilka_v2", "bootloader.bin")
)
