#pragma once

#include "mjs.h"

/// Register the `gpio` object in the mJS global scope.
/// Provides: gpio.set_mode, gpio.write, gpio.read, gpio.analog_read,
/// and constants: gpio.LOW, gpio.HIGH, gpio.INPUT, gpio.OUTPUT,
/// gpio.INPUT_PULLUP, gpio.INPUT_PULLDOWN.
///
/// Example usage in JS:
///
///   // Configure pin 25 as output and set it HIGH
///   gpio.set_mode(25, gpio.OUTPUT);
///   gpio.write(25, gpio.HIGH);
///
///   // Configure pin 34 as input with pull-up and read its value
///   gpio.set_mode(34, gpio.INPUT_PULLUP);
///   let val = gpio.read(34); // returns gpio.LOW or gpio.HIGH
///
///   // Read analog value from pin 36 (0..4095)
///   gpio.set_mode(36, gpio.INPUT);
///   let analog = gpio.analog_read(36);
///
void mjs_gpio_register(struct mjs* mjs);
