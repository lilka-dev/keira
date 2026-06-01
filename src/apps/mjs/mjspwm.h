#pragma once

#include "mjs.h"

/// Register the `pwm` object in the mJS global scope.
/// Backed by the ESP32 LEDC peripheral.
/// Provides: pwm.setup, pwm.attach_pin, pwm.detach_pin, pwm.write,
/// pwm.write_tone, pwm.read.
///
/// Note: the system buzzer also uses LEDC, so prefer higher channel
/// numbers (e.g. 2..15) to avoid clashing with it.
///
/// Example usage in JS:
///
///   pwm.setup(2, 5000, 8);   // channel 2, 5 kHz, 8-bit resolution
///   pwm.attach_pin(21, 2);   // route channel 2 to pin 21
///   pwm.write(2, 128);       // 50% duty cycle
///
void mjs_pwm_register(struct mjs* mjs);
