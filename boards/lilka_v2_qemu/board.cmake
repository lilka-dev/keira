# Lilka v2 firmware for Espressif's QEMU (qemu-system-xtensa -M esp32s3), for automated UI tests.
# Same as lilka_v2, except:
#   - Serial is UART0 (QEMU has no USB-Serial/JTAG), so the host can talk to it through QEMU's -serial
#   - UARTInputService is enabled: buttons and screenshots are driven over that serial port
#   - no boot sounds: QEMU has no LEDC, Arduino's tone() divides by its zero clock and panics
#   - quad PSRAM (sdkconfig.defaults): octal PSRAM init hangs in QEMU
#   - KEIRA_EMULATOR: stubs for hardware QEMU lacks, see src/keira/emulator.cpp
# See tools/emu/README.md

include("${CMAKE_CURRENT_LIST_DIR}/../lilka_v2/board.cmake")

set(KEIRA_EMULATOR ON)

list(REMOVE_ITEM KEIRA_BOARD_DEFINITIONS ARDUINO_USB_CDC_ON_BOOT=1)
list(APPEND KEIRA_BOARD_DEFINITIONS
    ARDUINO_USB_CDC_ON_BOOT=0
    KEIRA_UART_INPUT
    LILKA_NO_BUZZER_HELLO
    LILKA_NO_AUDIO_HELLO
    KEIRA_EMULATOR
)

set(KEIRA_SDKCONFIG_DEFAULTS
    "${CMAKE_CURRENT_LIST_DIR}/../lilka_v2/sdkconfig.defaults"
    "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults"
)
