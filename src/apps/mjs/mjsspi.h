#pragma once

#include "mjs.h"

/// Register the `spi` object in the mJS global scope.
/// Uses the user SPI bus (SPI2 / FSPI) on the expansion connector.
/// Only available on Lilka v2 (ESP32-S3); SPI1 is reserved for display/SD.
/// Provides: spi.begin, spi.transfer, spi.close,
/// and constants: spi.MODE0..spi.MODE3.
///
/// Example usage in JS:
///
///   spi.begin(12, 13, 14);            // SCK, MISO, MOSI
///   gpio.set_mode(21, gpio.OUTPUT);   // chip select
///   gpio.write(21, gpio.LOW);
///   let resp = spi.transfer([0x9F, 0x00, 0x00]);
///   gpio.write(21, gpio.HIGH);
///
void mjs_spi_register(struct mjs* mjs);
