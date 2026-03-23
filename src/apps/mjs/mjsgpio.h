#pragma once

#include "mjs.h"

/// Register the `gpio` object in the mJS global scope.
/// Provides: gpio.set_mode, gpio.write, gpio.read, gpio.analog_read,
/// and constants: gpio.LOW, gpio.HIGH, gpio.INPUT, gpio.OUTPUT,
/// gpio.INPUT_PULLUP, gpio.INPUT_PULLDOWN.
void mjs_gpio_register(struct mjs* mjs);
