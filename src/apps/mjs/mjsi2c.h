#pragma once

#include "mjs.h"

/// Register the `i2c` object in the mJS global scope.
/// Provides: i2c.begin, i2c.set_clock, i2c.scan, i2c.write, i2c.read, i2c.write_read.
///
/// Example usage in JS:
///
///   i2c.begin(8, 9);              // SDA=8, SCL=9
///   let found = i2c.scan();       // array of 7-bit addresses
///   i2c.write(0x3C, [0x00, 0xAF]);
///   let data = i2c.read(0x3C, 4); // array of bytes
///
void mjs_i2c_register(struct mjs* mjs);
