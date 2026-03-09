#include "dynapp.h"
#include "keira/appmanager.h"
#include <lilka.h>

/* ── Global state for the currently running dynamic app ─────────────────── */

static App *g_dynapp = nullptr;

/* ── C API functions exported to .so apps ───────────────────────────────── */

extern "C" {

/* ── Display ──────────────────────────────────────────────────────────────── */

void keira_display_fill_screen(uint16_t color) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->fillScreen(color);
    }
}

void keira_display_draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->drawPixel(x, y, color);
    }
}

void keira_display_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->drawLine(x0, y0, x1, y1, color);
    }
}

void keira_display_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->drawRect(x, y, w, h, color);
    }
}

void keira_display_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->fillRect(x, y, w, h, color);
    }
}

void keira_display_draw_circle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->drawCircle(x, y, r, color);
    }
}

void keira_display_fill_circle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->fillCircle(x, y, r, color);
    }
}

void keira_display_draw_triangle(int16_t x0, int16_t y0,
                                  int16_t x1, int16_t y1,
                                  int16_t x2, int16_t y2,
                                  uint16_t color) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->drawTriangle(x0, y0, x1, y1, x2, y2, color);
    }
}

void keira_display_fill_triangle(int16_t x0, int16_t y0,
                                  int16_t x1, int16_t y1,
                                  int16_t x2, int16_t y2,
                                  uint16_t color) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->fillTriangle(x0, y0, x1, y1, x2, y2, color);
    }
}

void keira_display_set_cursor(int16_t x, int16_t y) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->setCursor(x, y);
    }
}

void keira_display_set_text_color(uint16_t color) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->setTextColor(color);
    }
}

void keira_display_set_text_size(uint8_t size) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->setTextSize(size);
    }
}

void keira_display_print(const char *text) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->print(text);
    }
}

void keira_display_println(const char *text) {
    if (g_dynapp && g_dynapp->canvas) {
        g_dynapp->canvas->println(text);
    }
}

int16_t keira_display_width(void) {
    if (g_dynapp && g_dynapp->canvas) {
        return g_dynapp->canvas->width();
    }
    return 240;
}

int16_t keira_display_height(void) {
    if (g_dynapp && g_dynapp->canvas) {
        return g_dynapp->canvas->height();
    }
    return 280;
}

uint16_t keira_display_color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/* ── Queue draw (swap buffers) ────────────────────────────────────────────── */

void keira_queue_draw(void) {
    if (g_dynapp) {
        g_dynapp->queueDraw();
    }
}

/* ── Controller ───────────────────────────────────────────────────────────── */

/**
 * Read button states into a simple struct suitable for C apps.
 * The struct layout matches keira_buttons_t (defined in the demo SDK header).
 *
 * Buttons are packed as a bitfield in a uint32_t:
 *   bit 0  = UP pressed
 *   bit 1  = DOWN pressed
 *   bit 2  = LEFT pressed
 *   bit 3  = RIGHT pressed
 *   bit 4  = A pressed
 *   bit 5  = B pressed
 *   bit 6  = C pressed
 *   bit 7  = D pressed
 *   bit 8  = SELECT pressed
 *   bit 9  = START pressed
 *   bit 16 = UP justPressed
 *   ... (same order shifted by 16 for justPressed)
 */
uint32_t keira_controller_get_state(void) {
    lilka::State st = lilka::controller.getState();
    uint32_t bits = 0;

    if (st.up.pressed)      bits |= (1 << 0);
    if (st.down.pressed)    bits |= (1 << 1);
    if (st.left.pressed)    bits |= (1 << 2);
    if (st.right.pressed)   bits |= (1 << 3);
    if (st.a.pressed)       bits |= (1 << 4);
    if (st.b.pressed)       bits |= (1 << 5);
    if (st.c.pressed)       bits |= (1 << 6);
    if (st.d.pressed)       bits |= (1 << 7);
    if (st.select.pressed)  bits |= (1 << 8);
    if (st.start.pressed)   bits |= (1 << 9);

    if (st.up.justPressed)      bits |= (1 << 16);
    if (st.down.justPressed)    bits |= (1 << 17);
    if (st.left.justPressed)    bits |= (1 << 18);
    if (st.right.justPressed)   bits |= (1 << 19);
    if (st.a.justPressed)       bits |= (1 << 20);
    if (st.b.justPressed)       bits |= (1 << 21);
    if (st.c.justPressed)       bits |= (1 << 22);
    if (st.d.justPressed)       bits |= (1 << 23);
    if (st.select.justPressed)  bits |= (1 << 24);
    if (st.start.justPressed)   bits |= (1 << 25);

    return bits;
}

/* ── Timing ───────────────────────────────────────────────────────────────── */

void keira_delay(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

uint32_t keira_millis(void) {
    return (uint32_t)millis();
}

/* ── Buzzer ───────────────────────────────────────────────────────────────── */

void keira_buzzer_play(float freq, uint32_t duration_ms) {
    lilka::buzzer.play(freq, duration_ms);
}

void keira_buzzer_stop(void) {
    lilka::buzzer.stop();
}

} /* extern "C" */

/* ── Keira API symbol table ─────────────────────────────────────────────── */

static const lilka_dynsym_t g_keira_api_symbols[] = {
    /* Display */
    LILKA_DYNSYM_EXPORT(keira_display_fill_screen),
    LILKA_DYNSYM_EXPORT(keira_display_draw_pixel),
    LILKA_DYNSYM_EXPORT(keira_display_draw_line),
    LILKA_DYNSYM_EXPORT(keira_display_draw_rect),
    LILKA_DYNSYM_EXPORT(keira_display_fill_rect),
    LILKA_DYNSYM_EXPORT(keira_display_draw_circle),
    LILKA_DYNSYM_EXPORT(keira_display_fill_circle),
    LILKA_DYNSYM_EXPORT(keira_display_draw_triangle),
    LILKA_DYNSYM_EXPORT(keira_display_fill_triangle),
    LILKA_DYNSYM_EXPORT(keira_display_set_cursor),
    LILKA_DYNSYM_EXPORT(keira_display_set_text_color),
    LILKA_DYNSYM_EXPORT(keira_display_set_text_size),
    LILKA_DYNSYM_EXPORT(keira_display_print),
    LILKA_DYNSYM_EXPORT(keira_display_println),
    LILKA_DYNSYM_EXPORT(keira_display_width),
    LILKA_DYNSYM_EXPORT(keira_display_height),
    LILKA_DYNSYM_EXPORT(keira_display_color565),

    /* Drawing */
    LILKA_DYNSYM_EXPORT(keira_queue_draw),

    /* Controller */
    LILKA_DYNSYM_EXPORT(keira_controller_get_state),

    /* Timing */
    LILKA_DYNSYM_EXPORT(keira_delay),
    LILKA_DYNSYM_EXPORT(keira_millis),

    /* Buzzer */
    LILKA_DYNSYM_EXPORT(keira_buzzer_play),
    LILKA_DYNSYM_EXPORT(keira_buzzer_stop),

    LILKA_DYNSYM_END
};

/* ── DynApp implementation ──────────────────────────────────────────────── */

DynApp::DynApp(const String& path) : App("DynApp"), filePath(path) {
    setStackSize(16384);
    setFlags(APP_FLAG_FULLSCREEN);
}

void DynApp::registerAppSymbols() {
    lilka_dynloader_register_symbols(g_keira_api_symbols);
}

void DynApp::unregisterAppSymbols() {
    lilka_dynloader_unregister_symbols(g_keira_api_symbols);
}

void DynApp::run() {
    /* Set global app pointer for C API functions */
    g_dynapp = this;
    registerAppSymbols();

    lilka::serial.log("DynApp: loading %s", filePath.c_str());

    lilka::DynLoader loader;
    int ret = loader.load(filePath.c_str());

    if (ret != 0) {
        const char *err = loader.getError();
        lilka::serial.log("DynApp: load failed: %s (ret=%d)", err ? err : "unknown", ret);
        alert("DynApp Error", String("Failed to load:\n") + (err ? err : "Unknown error"));
        g_dynapp = nullptr;
        unregisterAppSymbols();
        return;
    }

    /* Prepare argv: argv[0] = path to the .so file */
    char *argv[1];
    char pathBuf[256];
    strncpy(pathBuf, filePath.c_str(), sizeof(pathBuf) - 1);
    pathBuf[sizeof(pathBuf) - 1] = '\0';
    argv[0] = pathBuf;

    lilka::serial.log("DynApp: executing...");
    ret = loader.execute(1, argv);
    lilka::serial.log("DynApp: app returned %d", ret);

    loader.unload();
    g_dynapp = nullptr;
    unregisterAppSymbols();
}

void DynApp::onExit() {
    g_dynapp = nullptr;
    unregisterAppSymbols();
}
