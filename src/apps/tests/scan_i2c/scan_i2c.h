#pragma once

#include "keira/app.h"

class ScanI2CApp : public App {
public:
    ScanI2CApp();

private:
    // Indices into I2C_PINS / I2C_FREQUENCIES
    uint8_t sdaIndex = 0; // LILKA_P3
    uint8_t sclIndex = 1; // LILKA_P4
    uint8_t freqIndex = 0;

    void run() override;
    void scan();
    uint8_t nextPinIndex(uint8_t current, uint8_t other) const;
};
