#include <Arduino.h>
#include <driver/gpio.h>
#include <esp_heap_caps.h>

#include "services/screenstream/screenstream.h"
#include "keira/ksystem.h"
#include "keira/recclock.h"

static const int8_t kButtonPins[] = {
    LILKA_GPIO_UP,
    LILKA_GPIO_DOWN,
    LILKA_GPIO_LEFT,
    LILKA_GPIO_RIGHT,
    LILKA_GPIO_A,
    LILKA_GPIO_B,
    LILKA_GPIO_C,
    LILKA_GPIO_D,
    LILKA_GPIO_SELECT,
    LILKA_GPIO_START,
};
static constexpr int kButtonCount = sizeof(kButtonPins) / sizeof(kButtonPins[0]);
static constexpr uint32_t kFrameMs = 33; // ~30 fps target
static constexpr uint32_t kKeyEveryMs = 2000;
static constexpr uint32_t kMaxPressMs = 2000;

ScreenStreamService::ScreenStreamService() : Service("screenstream") {
    setktStackSize(8192);
    // above apps: some of them busy-wait (catalog downloads), and a held emulated button must still be released
    setktPriority(KT_PRIO_MAX);
}

// The buttons pull their GPIO to GND. Emulate that with an open-drain output: driving 0 reads as pressed,
// releasing lets the pull-up win again. Never drives the line high, so a real press can't short anything.
void ScreenStreamService::setButton(int button, bool pressed) {
    if (button < 0 || button >= kButtonCount || kButtonPins[button] < 0) {
        return;
    }
    gpio_num_t pin = static_cast<gpio_num_t>(kButtonPins[button]);
    pressedSince[button] = pressed ? millis() : 0;
    buttonsChanged = true;
    if (pressed) {
        gpio_set_level(pin, 0);
        gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT_OD);
    } else {
        gpio_set_level(pin, 1);
        gpio_set_direction(pin, GPIO_MODE_INPUT);
        gpio_pullup_en(pin);
    }
}

void ScreenStreamService::releaseAll() {
    for (int i = 0; i < kButtonCount; i++) {
        setButton(i, false);
    }
    tapButton = -1;
}

void ScreenStreamService::handleCommand(const char* line) {
    switch (line[0]) {
        case 'S':
            streaming = line[1] == '1';
            keyRequested = true;
            releaseAll();
            if (!streaming) {
                RecClock::setActive(false);
            }
            break;
        case 'L':
            // frame-by-frame mode: time only moves on 'F', one frame is sent per 'F'
            RecClock::setActive(line[1] == '1');
            keyRequested = true;
            break;
        case 'F':
            stepRequested = true;
            break;
        case 'K':
            keyRequested = true;
            resendRequested = true; // frame-by-frame: send the current frame again (the last one arrived damaged)
            break;
        case 'B':
            // "B41" = button 4 pressed, "B40" = released
            if (line[1] && line[2]) {
                setButton(line[1] - '0', line[2] == '1');
            }
            break;
        case 'T': {
            char* end = nullptr;
            int n = strtol(line + 1, &end, 10);
            int ms = end ? strtol(end, nullptr, 10) : 80;
            if (tapButton >= 0) {
                setButton(tapButton, false);
            }
            tapButton = n;
            tapUntil = millis() + (ms > 0 ? ms : 80);
            setButton(n, true);
            break;
        }
        default:
            break;
    }
}

size_t ScreenStreamService::encode(const uint16_t* cur, const uint16_t* prev, size_t words, uint8_t* out) {
    uint8_t* o = out;
    size_t i = 0;
    auto token = [&](uint16_t t) {
        *o++ = t & 0xff;
        *o++ = t >> 8;
    };
    auto same = [&](size_t k) { return prev && cur[k] == prev[k]; };
    auto runAt = [&](size_t k) {
        size_t r = 1;
        while (k + r < words && r < 0x3fff && cur[k + r] == cur[k]) {
            r++;
        }
        return r;
    };
    while (i < words) {
        // 1. unchanged since the previous frame
        size_t j = i;
        while (j < words && j - i < 0x7fff && same(j)) {
            j++;
        }
        if (j > i) {
            token(0x8000 | (j - i));
            i = j;
            continue;
        }
        // 2. run of one colour
        size_t r = runAt(i);
        if (r >= 3) {
            token(0x4000 | r);
            token(cur[i]);
            i += r;
            continue;
        }
        // 3. literals until something cheaper starts
        j = i;
        while (j < words && j - i < 0x3fff) {
            if (j + 2 < words && same(j) && same(j + 1) && same(j + 2)) {
                break;
            }
            if (j + 2 < words && cur[j] == cur[j + 1] && cur[j] == cur[j + 2]) {
                break;
            }
            j++;
        }
        token(j - i);
        memcpy(o, cur + i, (j - i) * 2);
        o += (j - i) * 2;
        i = j;
    }
    return o - out;
}

void ScreenStreamService::run() {
    const int16_t w = lilka::display.width();
    const int16_t h = lilka::display.height();
    const size_t words = static_cast<size_t>(w) * h;
    lilka::Canvas canvas(w, h);
    uint16_t* prev = static_cast<uint16_t*>(heap_caps_malloc(words * 2, MALLOC_CAP_SPIRAM));
    const size_t outCap = 24 + words * 2 + words / 0x7fff * 2 + 1024;
    uint8_t* out = static_cast<uint8_t*>(heap_caps_malloc(outCap, MALLOC_CAP_SPIRAM));
    if (!prev || !out) {
        lilka::serial.err("screenstream: out of memory");
        vTaskDelete(NULL);
        return;
    }
    releaseAll();
    char line[32];
    size_t lineLen = 0;
    uint32_t lastKey = 0;
    TickType_t lastWake = xTaskGetTickCount();

    while (1) {
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                line[lineLen] = 0;
                if (lineLen) {
                    handleCommand(line);
                }
                lineLen = 0;
            } else if (lineLen < sizeof(line) - 1) {
                line[lineLen++] = c;
            }
        }
        if (tapButton >= 0 && static_cast<int32_t>(millis() - tapUntil) >= 0) {
            setButton(tapButton, false);
            tapButton = -1;
        }
        // fail-safe: a press from the host never outlives kMaxPressMs, even if the release got lost
        for (int i = 0; i < kButtonCount; i++) {
            if (pressedSince[i] && !RecClock::active() && millis() - pressedSince[i] > kMaxPressMs) {
                setButton(i, false);
            }
        }

        bool capture = streaming && !RecClock::active();
        if (streaming && RecClock::active() && resendRequested) {
            capture = true;
        }
        resendRequested = false;
        if (streaming && RecClock::active() && stepRequested) {
            stepRequested = false;
            if (buttonsChanged) {
                // let the controller task debounce the new button state before the app reads it
                vTaskDelay(pdMS_TO_TICKS(25));
                buttonsChanged = false;
            }
            RecClock::step(400);
            capture = true;
        }
        if (capture) {
            canvas.fillScreen(0);
            ksystem.apps.renderToCanvas(&canvas);
            const uint16_t* cur = canvas.getFramebuffer();
            uint32_t now = RecClock::nowMs();
            bool key = keyRequested || now - lastKey >= kKeyEveryMs;
            uint8_t* p = out + 24;
            size_t len = encode(cur, key ? nullptr : prev, words, p);
            uint32_t sum = 0;
            for (size_t k = 0; k < len; k++) {
                sum += p[k];
            }
            uint8_t kind = key ? 0 : 1;
            uint32_t s = seq++;
            memcpy(out, "LKF1", 4);
            memcpy(out + 4, &s, 4);
            memcpy(out + 8, &now, 4);
            memcpy(out + 12, &w, 2);
            memcpy(out + 14, &h, 2);
            out[16] = kind;
            uint32_t l32 = len;
            memcpy(out + 17, &l32, 4);
            out[21] = out[22] = out[23] = 0; // padding: the payload starts at offset 24
            memcpy(p + len, &sum, 4);
            // HWCDC returns early when its TX buffer stays full past the timeout; keep pushing so frames stay whole
            size_t total = 24 + len + 4, sent = 0;
            while (sent < total) {
                size_t n = Serial.write(out + sent, total - sent);
                sent += n;
                if (!n) {
                    if (!Serial) {
                        break; // host went away
                    }
                    vTaskDelay(1);
                }
            }
            memcpy(prev, cur, words * 2);
            if (key) {
                lastKey = now;
                keyRequested = false;
            }
        }
        if (RecClock::active()) {
            vTaskDelay(1); // stepping: react to the next 'F' right away
            lastWake = xTaskGetTickCount();
        } else {
            vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(streaming ? kFrameMs : 10));
        }
    }
}
