#pragma once

#include "mjs.h"

/// Register the `ws2812` object in the mJS global scope.
/// Drives WS2812 ("NeoPixel") addressable LEDs via the ESP32 RMT peripheral.
/// Provides: ws2812.write(pin, colors).
///
/// `colors` is an array of integers, each in 0xRRGGBB form.
///
/// Example usage in JS:
///
///   // Light 3 LEDs on pin 21: red, green, blue
///   ws2812.write(21, [0xFF0000, 0x00FF00, 0x0000FF]);
///
void mjs_ws2812_register(struct mjs* mjs);
