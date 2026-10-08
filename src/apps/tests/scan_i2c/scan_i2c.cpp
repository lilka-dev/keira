#include <Arduino.h>
#include <Wire.h>
#include "keira/keira.h"
#include "scan_i2c.h"

#include "keira/utils/string.h"

#if LILKA_VERSION >= 2
static constexpr uint32_t I2C_FREQUENCIES[] = {100000, 400000, 1000000};
static constexpr uint8_t I2C_FREQUENCY_COUNT = sizeof(I2C_FREQUENCIES) / sizeof(I2C_FREQUENCIES[0]);

struct ScanI2CPin {
    const char* name;
    uint8_t gpio;
};

// Extension header pins usable for I2C (TX/RX are free: console runs over USB CDC)
static const ScanI2CPin I2C_PINS[] = {
    {"P3", LILKA_P3},
    {"P4", LILKA_P4},
    {"P5", LILKA_P5},
    {"P6", LILKA_P6},
    {"P7", LILKA_P7},
    {"P8", LILKA_P8},
    {"TX", TX},
    {"RX", RX},
};
static constexpr uint8_t I2C_PIN_COUNT = sizeof(I2C_PINS) / sizeof(I2C_PINS[0]);

enum ScanI2CMenuItem {
    SCAN_I2C_MENU_SDA,
    SCAN_I2C_MENU_SCL,
    SCAN_I2C_MENU_FREQUENCY,
    SCAN_I2C_MENU_SCAN,
    SCAN_I2C_MENU_BACK,
};
#endif

ScanI2CApp::ScanI2CApp() : App("I2C Scanner") {
}

void ScanI2CApp::run() {
#if LILKA_VERSION >= 2
    lilka::Menu menu("I2C");
    menu.addActivationButton(K_BTN_BACK);

    while (true) {
        int16_t cursor = menu.getCursor();
        menu.clearItems();
        menu.addItem(
            K_S_I2C_SCANNER_SDA,
            0,
            lilka::colors::White,
            StringFormat("%s (GPIO %d)", I2C_PINS[sdaIndex].name, I2C_PINS[sdaIndex].gpio)
        );
        menu.addItem(
            K_S_I2C_SCANNER_SCL,
            0,
            lilka::colors::White,
            StringFormat("%s (GPIO %d)", I2C_PINS[sclIndex].name, I2C_PINS[sclIndex].gpio)
        );
        menu.addItem(
            K_S_I2C_SCANNER_FREQUENCY,
            0,
            lilka::colors::White,
            StringFormat("%d kHz", static_cast<int>(I2C_FREQUENCIES[freqIndex] / 1000))
        );
        menu.addItem(K_S_I2C_SCANNER_SCAN);
        menu.addItem(K_S_MENU_BACK);
        menu.setCursor(cursor);

        while (!menu.isFinished()) {
            menu.update();
            menu.draw(canvas);
            queueDraw();
        }

        if (menu.getButton() == K_BTN_BACK) return;

        switch (menu.getCursor()) {
            case SCAN_I2C_MENU_SDA:
                sdaIndex = nextPinIndex(sdaIndex, sclIndex);
                break;
            case SCAN_I2C_MENU_SCL:
                sclIndex = nextPinIndex(sclIndex, sdaIndex);
                break;
            case SCAN_I2C_MENU_FREQUENCY:
                freqIndex = (freqIndex + 1) % I2C_FREQUENCY_COUNT;
                break;
            case SCAN_I2C_MENU_SCAN:
                scan();
                break;
            default:
                return;
        }
    }
#else
    lilka::Alert alert(K_S_ERROR, K_S_LILKA_V2_OR_HIGHER_REQUIRED);
    alert.draw(canvas);
    queueDraw();
    while (!alert.isFinished()) {
        alert.update();
    }
#endif
}

uint8_t ScanI2CApp::nextPinIndex(uint8_t current, uint8_t other) const {
#if LILKA_VERSION >= 2
    uint8_t next = (current + 1) % I2C_PIN_COUNT;
    if (next == other) next = (next + 1) % I2C_PIN_COUNT;
    return next;
#else
    return current;
#endif
}

void ScanI2CApp::scan() {
#if LILKA_VERSION >= 2
    const uint8_t sda = I2C_PINS[sdaIndex].gpio;
    const uint8_t scl = I2C_PINS[sclIndex].gpio;
    const uint32_t freq = I2C_FREQUENCIES[freqIndex];

    lilka::Canvas buffer(canvas->width(), canvas->height());
    buffer.begin();
    buffer.setFont(FONT_9x15);

    buffer.fillScreen(lilka::colors::Black);
    buffer.setTextBound(4, 0, canvas->width() - 8, canvas->height());
    buffer.setCursor(4, 20);

    buffer.println(StringFormat(K_S_I2C_INIT_ABOUT, sda, scl));
    Wire.begin(sda, scl, freq);
    buffer.println(K_S_I2C_SCANNER_SCAN_START);
    canvas->drawCanvas(&buffer);
    queueDraw();

    uint8_t found = 0;
    for (uint16_t address = 1; address <= 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            buffer.printf("%02X", address);
            found++;
        } else {
            buffer.print(".");
        }
        if (address % 24 == 0 || address == 127) {
            buffer.println();
        }
    }

    buffer.println(K_S_I2C_SCANNER_SCAN_DONE);
    buffer.printf(K_S_I2C_SCANNER_DEVICES_FOUND, found);
    canvas->drawCanvas(&buffer);
    queueDraw();

    Wire.end();

    while (true) {
        lilka::State state = lilka::controller.getState();
        if (state.a.justPressed || state.b.justPressed) break;
        taskYIELD();
    }
#endif
}
