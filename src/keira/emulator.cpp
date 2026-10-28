// Stubs for hardware that Espressif's QEMU doesn't emulate (board lilka_v2_qemu, see tools/emu/README.md)
#ifdef KEIRA_EMULATOR

#    include <Arduino.h>

// Linked with -Wl,--wrap=analogRead (src/CMakeLists.txt). The real one polls the SAR ADC, which never finishes.
// 0 reads as "no battery" in lilka::battery
extern "C" uint16_t __wrap_analogRead(uint8_t pin) {
    return 0;
}

#endif
