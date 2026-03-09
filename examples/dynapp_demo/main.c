/**
 * @file main.c
 * @brief Keira DynApp demo — bouncing ball with controller input
 *
 * This demo shows how to write a .so app for KeiraOS:
 *  - Draw shapes and text
 *  - Read controller input
 *  - Animate objects with a simple game loop
 *
 * Build:
 *   make          (requires xtensa-esp32s3-elf-gcc in PATH)
 *
 * Then copy demo.so to the SD card and open it from the file manager.
 */

#include "keira_api.h"

/* We don't have access to the standard library, so we implement what we need. */

static int abs_val(int x) {
    return x < 0 ? -x : x;
}

/* Simple fixed-point sin approximation (returns value in [-256, 256] range, input in degrees). */
static int sin_approx(int deg) {
    /* Normalize to 0-359 */
    deg = deg % 360;
    if (deg < 0) deg += 360;

    /* Use symmetry to reduce to 0-90 */
    int sign = 1;
    if (deg >= 180) {
        sign = -1;
        deg -= 180;
    }
    if (deg > 90) {
        deg = 180 - deg;
    }

    /* Approximate sin(deg) * 256 using a quadratic fit */
    /* sin(x) ≈ (4x(180-x)) / (40500 - x(180-x)) * 256, Bhaskara's approximation */
    int p = deg * (180 - deg);
    int result = (256 * 4 * p) / (40500 - p);
    return sign * result;
}

static int cos_approx(int deg) {
    return sin_approx(deg + 90);
}

/**
 * Entry point for the .so app.
 * argc/argv are provided by Keira: argv[0] is the .so file path.
 */
int app_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    int16_t scr_w = keira_display_width();
    int16_t scr_h = keira_display_height();

    /* Ball state — fixed-point (8-bit fraction) */
    int ball_x = (scr_w / 2) << 8;
    int ball_y = (scr_h / 2) << 8;
    int ball_vx = 384; /* ~1.5 px/frame */
    int ball_vy = 256; /* ~1.0 px/frame */
    int ball_r = 8;

/* Trail positions */
#define TRAIL_LEN 12
    int16_t trail_x[TRAIL_LEN];
    int16_t trail_y[TRAIL_LEN];
    int trail_idx = 0;
    for (int i = 0; i < TRAIL_LEN; i++) {
        trail_x[i] = -1;
        trail_y[i] = -1;
    }

    /* Decoration — spinning polygon */
    int angle = 0;

    /* Play a short startup beep */
    keira_buzzer_play(880.0f, 100);
    keira_delay(120);
    keira_buzzer_stop();

    uint32_t frame = 0;

    for (;;) {
        uint32_t state = keira_controller_get_state();

        /* Exit on B */
        if (state & KEIRA_JUST_B) {
            break;
        }

        /* D-pad moves the ball */
        if (state & KEIRA_BTN_UP) ball_vy -= 64;
        if (state & KEIRA_BTN_DOWN) ball_vy += 64;
        if (state & KEIRA_BTN_LEFT) ball_vx -= 64;
        if (state & KEIRA_BTN_RIGHT) ball_vx += 64;

        /* A button — speed boost */
        if (state & KEIRA_BTN_A) {
            ball_vx = (ball_vx * 280) >> 8; /* 1.09x */
            ball_vy = (ball_vy * 280) >> 8;
        }

        /* Clamp velocity */
        if (ball_vx > 1536) ball_vx = 1536;
        if (ball_vx < -1536) ball_vx = -1536;
        if (ball_vy > 1536) ball_vy = 1536;
        if (ball_vy < -1536) ball_vy = -1536;

        /* Apply friction */
        ball_vx = (ball_vx * 253) / 256; /* 0.988x */
        ball_vy = (ball_vy * 253) / 256;

        /* Update position */
        ball_x += ball_vx;
        ball_y += ball_vy;

        /* Bounce off walls */
        int16_t bx = ball_x >> 8;
        int16_t by = ball_y >> 8;

        if (bx - ball_r < 0) {
            ball_x = ball_r << 8;
            ball_vx = abs_val(ball_vx);
            keira_buzzer_play(440.0f, 30);
        }
        if (bx + ball_r >= scr_w) {
            ball_x = (scr_w - ball_r - 1) << 8;
            ball_vx = -abs_val(ball_vx);
            keira_buzzer_play(440.0f, 30);
        }
        if (by - ball_r < 0) {
            ball_y = ball_r << 8;
            ball_vy = abs_val(ball_vy);
            keira_buzzer_play(523.0f, 30);
        }
        if (by + ball_r >= scr_h) {
            ball_y = (scr_h - ball_r - 1) << 8;
            ball_vy = -abs_val(ball_vy);
            keira_buzzer_play(523.0f, 30);
        }

        bx = ball_x >> 8;
        by = ball_y >> 8;

        /* Store trail */
        trail_x[trail_idx] = bx;
        trail_y[trail_idx] = by;
        trail_idx = (trail_idx + 1) % TRAIL_LEN;

        /* ── Draw ─────────────────────────────────────── */
        keira_display_fill_screen(KEIRA_COLOR_BLACK);

        /* Draw decorative border */
        keira_display_draw_rect(0, 0, scr_w, scr_h, keira_display_color565(40, 40, 80));
        keira_display_draw_rect(2, 2, scr_w - 4, scr_h - 4, keira_display_color565(40, 40, 80));

        /* Draw spinning hexagon in center */
        angle = (angle + 3) % 360;
        {
            int cx = scr_w / 2;
            int cy = scr_h / 2;
            int r = 40;
            int sides = 6;
            int step = 360 / sides;
            for (int i = 0; i < sides; i++) {
                int a1 = angle + i * step;
                int a2 = angle + (i + 1) * step;
                int x1 = cx + (r * cos_approx(a1)) / 256;
                int y1 = cy + (r * sin_approx(a1)) / 256;
                int x2 = cx + (r * cos_approx(a2)) / 256;
                int y2 = cy + (r * sin_approx(a2)) / 256;
                uint16_t hue_color =
                    keira_display_color565(80 + (i * 29) % 176, 40 + (i * 47) % 176, 120 + (i * 37) % 136);
                keira_display_draw_line(x1, y1, x2, y2, hue_color);
            }
        }

        /* Draw trail (fading circles) */
        for (int i = 0; i < TRAIL_LEN; i++) {
            int idx = (trail_idx + i) % TRAIL_LEN;
            if (trail_x[idx] < 0) continue;
            int brightness = (i + 1) * 20;
            if (brightness > 255) brightness = 255;
            uint16_t trail_col = keira_display_color565(0, brightness / 2, brightness);
            int trail_r = 2 + (i * ball_r) / TRAIL_LEN;
            keira_display_fill_circle(trail_x[idx], trail_y[idx], trail_r, trail_col);
        }

        /* Draw ball */
        keira_display_fill_circle(bx, by, ball_r, KEIRA_COLOR_CYAN);
        keira_display_draw_circle(bx, by, ball_r, KEIRA_COLOR_WHITE);

        /* Draw velocity indicator line from ball */
        keira_display_draw_line(bx, by, bx + (ball_vx >> 5), by + (ball_vy >> 5), KEIRA_COLOR_YELLOW);

        /* Draw HUD text */
        keira_display_set_cursor(6, 6);
        keira_display_set_text_color(KEIRA_COLOR_GREEN);
        keira_display_set_text_size(1);
        keira_display_println("DynApp Demo");

        keira_display_set_cursor(6, 18);
        keira_display_set_text_color(keira_display_color565(160, 160, 160));
        keira_display_print("D-Pad:Move A:Boost B:Exit");

        /* Display frame counter at the bottom */
        keira_display_set_cursor(6, scr_h - 12);
        keira_display_set_text_color(keira_display_color565(100, 100, 100));
        keira_display_print("Frame: ");
        /* Convert frame number to string manually */
        {
            char buf[12];
            int n = frame;
            int i = 0;
            if (n == 0) {
                buf[i++] = '0';
            } else {
                char tmp[12];
                int j = 0;
                while (n > 0) {
                    tmp[j++] = '0' + (n % 10);
                    n /= 10;
                }
                while (j > 0) {
                    buf[i++] = tmp[--j];
                }
            }
            buf[i] = '\0';
            keira_display_print(buf);
        }

        keira_queue_draw();

        frame++;
        keira_delay(16); /* ~60 FPS */
    }

    /* Cleanup beep */
    keira_buzzer_play(440.0f, 50);
    keira_delay(60);
    keira_buzzer_stop();

    return 0;
}
