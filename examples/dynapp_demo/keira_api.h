/**
 * @file keira_api.h
 * @brief Keira DynApp C API header for .so applications
 *
 * Include this header in your .so app to access Keira display,
 * controller, timing, and buzzer functionality.
 *
 * Build your app with:
 *   xtensa-esp32s3-elf-gcc -c -fPIC -nostdlib -I. -o main.o main.c
 *   xtensa-esp32s3-elf-gcc -shared -nostdlib -fPIC -e app_main \
 *       -Wl,--gc-sections -fvisibility=hidden -o demo.so main.o
 */

#ifndef KEIRA_API_H
#define KEIRA_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Display API ─────────────────────────────────────────────────────────── */

/** Fill the entire screen with a color (RGB565). */
void keira_display_fill_screen(uint16_t color);

/** Draw a single pixel. */
void keira_display_draw_pixel(int16_t x, int16_t y, uint16_t color);

/** Draw a line. */
void keira_display_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

/** Draw a rectangle outline. */
void keira_display_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/** Fill a rectangle. */
void keira_display_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/** Draw a circle outline. */
void keira_display_draw_circle(int16_t x, int16_t y, int16_t r, uint16_t color);

/** Fill a circle. */
void keira_display_fill_circle(int16_t x, int16_t y, int16_t r, uint16_t color);

/** Draw a triangle outline. */
void keira_display_draw_triangle(
    int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color
);

/** Fill a triangle. */
void keira_display_fill_triangle(
    int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color
);

/** Set text cursor position. */
void keira_display_set_cursor(int16_t x, int16_t y);

/** Set text color (RGB565). */
void keira_display_set_text_color(uint16_t color);

/** Set text size multiplier. */
void keira_display_set_text_size(uint8_t size);

/** Print text at current cursor position. */
void keira_display_print(const char* text);

/** Print text with newline. */
void keira_display_println(const char* text);

/** Get display width in pixels. */
int16_t keira_display_width(void);

/** Get display height in pixels. */
int16_t keira_display_height(void);

/** Convert RGB to RGB565 color. */
uint16_t keira_display_color565(uint8_t r, uint8_t g, uint8_t b);

/* ── Drawing control ─────────────────────────────────────────────────────── */

/** Queue the current frame for display (swap front/back buffers). */
void keira_queue_draw(void);

/* ── Controller API ──────────────────────────────────────────────────────── */

/**
 * Read controller state as a packed uint32_t:
 *
 * Bits 0-9:   pressed  (UP=0, DOWN=1, LEFT=2, RIGHT=3, A=4, B=5, C=6, D=7, SELECT=8, START=9)
 * Bits 16-25: justPressed (same order shifted by 16)
 */
uint32_t keira_controller_get_state(void);

/* Button masks for pressed state */
#define KEIRA_BTN_UP     (1 << 0)
#define KEIRA_BTN_DOWN   (1 << 1)
#define KEIRA_BTN_LEFT   (1 << 2)
#define KEIRA_BTN_RIGHT  (1 << 3)
#define KEIRA_BTN_A      (1 << 4)
#define KEIRA_BTN_B      (1 << 5)
#define KEIRA_BTN_C      (1 << 6)
#define KEIRA_BTN_D      (1 << 7)
#define KEIRA_BTN_SELECT (1 << 8)
#define KEIRA_BTN_START  (1 << 9)

/* Button masks for justPressed state (shift by 16) */
#define KEIRA_JUST_UP     (1 << 16)
#define KEIRA_JUST_DOWN   (1 << 17)
#define KEIRA_JUST_LEFT   (1 << 18)
#define KEIRA_JUST_RIGHT  (1 << 19)
#define KEIRA_JUST_A      (1 << 20)
#define KEIRA_JUST_B      (1 << 21)
#define KEIRA_JUST_C      (1 << 22)
#define KEIRA_JUST_D      (1 << 23)
#define KEIRA_JUST_SELECT (1 << 24)
#define KEIRA_JUST_START  (1 << 25)

/* ── Timing API ──────────────────────────────────────────────────────────── */

/** Delay for the specified number of milliseconds. */
void keira_delay(uint32_t ms);

/** Get the number of milliseconds since boot. */
uint32_t keira_millis(void);

/* ── Buzzer API ──────────────────────────────────────────────────────────── */

/** Play a tone at the given frequency for the given duration. */
void keira_buzzer_play(float freq, uint32_t duration_ms);

/** Stop the buzzer. */
void keira_buzzer_stop(void);

/* ── Common colors (RGB565) ──────────────────────────────────────────────── */

#define KEIRA_COLOR_BLACK   0x0000
#define KEIRA_COLOR_WHITE   0xFFFF
#define KEIRA_COLOR_RED     0xF800
#define KEIRA_COLOR_GREEN   0x07E0
#define KEIRA_COLOR_BLUE    0x001F
#define KEIRA_COLOR_YELLOW  0xFFE0
#define KEIRA_COLOR_CYAN    0x07FF
#define KEIRA_COLOR_MAGENTA 0xF81F
#define KEIRA_COLOR_ORANGE  0xFD20

#ifdef __cplusplus
}
#endif

#endif /* KEIRA_API_H */
