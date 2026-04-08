#include "mjsdisplay.h"
#include <lilka.h>
#include "mjs.h"
#include "keira/app.h"

static App* mjs_get_app(struct mjs* mjs) {
    mjs_val_t app_val = mjs_get(mjs, mjs_get_global(mjs), "__app__", ~0);
    return static_cast<App*>(mjs_get_ptr(mjs, app_val));
}

static lilka::Canvas* mjs_get_canvas(struct mjs* mjs) {
    return mjs_get_app(mjs)->canvas;
}

// display.color565(r, g, b) -> number
static void mjs_display_color565(struct mjs* mjs) {
    int r = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int g = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int b = mjs_get_int(mjs, mjs_arg(mjs, 2));
    mjs_return(mjs, mjs_mk_number(mjs, lilka::display.color565(r, g, b)));
}

// display.set_cursor(x, y)
static void mjs_display_set_cursor(struct mjs* mjs) {
    int x = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y = mjs_get_int(mjs, mjs_arg(mjs, 1));
    mjs_get_canvas(mjs)->setCursor(x, y);
    mjs_return(mjs, mjs_mk_undefined());
}

static const uint8_t* fonts[] = {
    u8g2_font_4x6_t_cyrillic,
    u8g2_font_5x7_t_cyrillic,
    u8g2_font_5x8_t_cyrillic,
    u8g2_font_6x12_t_cyrillic,
    u8g2_font_6x13_t_cyrillic,
    u8g2_font_7x13_t_cyrillic,
    u8g2_font_8x13_t_cyrillic,
    u8g2_font_9x15_t_cyrillic,
    u8g2_font_10x20_t_cyrillic,
};

static const char fontNames[][12] = {
    "4x6",
    "5x7",
    "5x8",
    "6x12",
    "6x13",
    "7x13",
    "8x13",
    "9x15",
    "10x20",
};

// display.set_font(name)
static void mjs_display_set_font(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    const char* fontName = mjs_get_cstring(mjs, &arg0);
    for (int i = 0; i < 9; i++) {
        if (strcmp(fontName, fontNames[i]) == 0) {
            mjs_get_canvas(mjs)->setFont(fonts[i]);
            mjs_return(mjs, mjs_mk_undefined());
            return;
        }
    }
    mjs_return(mjs, mjs_mk_undefined());
}

// display.set_text_size(size)
static void mjs_display_set_text_size(struct mjs* mjs) {
    int size = mjs_get_int(mjs, mjs_arg(mjs, 0));
    mjs_get_canvas(mjs)->setTextSize(size);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.set_text_color(fg[, bg])
static void mjs_display_set_text_color(struct mjs* mjs) {
    uint16_t fg = mjs_get_int(mjs, mjs_arg(mjs, 0));
    mjs_val_t bg_arg = mjs_arg(mjs, 1);
    if (mjs_is_number(bg_arg)) {
        uint16_t bg = mjs_get_int(mjs, bg_arg);
        mjs_get_canvas(mjs)->setTextColor(fg, bg);
    } else {
        mjs_get_canvas(mjs)->setTextColor(fg);
    }
    mjs_return(mjs, mjs_mk_undefined());
}

// display.set_text_bound(x, y, w, h)
static void mjs_display_set_text_bound(struct mjs* mjs) {
    int x = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int w = mjs_get_int(mjs, mjs_arg(mjs, 2));
    int h = mjs_get_int(mjs, mjs_arg(mjs, 3));
    mjs_get_canvas(mjs)->setTextBound(x, y, w, h);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.print(...)
static void mjs_display_print(struct mjs* mjs) {
    int nargs = mjs_nargs(mjs);
    lilka::Canvas* canvas = mjs_get_canvas(mjs);
    for (int i = 0; i < nargs; i++) {
        mjs_val_t arg = mjs_arg(mjs, i);
        if (mjs_is_string(arg)) {
            size_t len;
            const char* s = mjs_get_string(mjs, &arg, &len);
            canvas->print(s);
        } else if (mjs_is_number(arg)) {
            canvas->print(mjs_get_double(mjs, arg));
        } else {
            canvas->print("[object]");
        }
    }
    mjs_return(mjs, mjs_mk_undefined());
}

// display.fill_screen(color)
static void mjs_display_fill_screen(struct mjs* mjs) {
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 0));
    mjs_get_canvas(mjs)->fillScreen(color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.draw_pixel(x, y, color)
static void mjs_display_draw_pixel(struct mjs* mjs) {
    int x = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y = mjs_get_int(mjs, mjs_arg(mjs, 1));
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 2));
    mjs_get_canvas(mjs)->drawPixel(x, y, color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.draw_line(x0, y0, x1, y1, color)
static void mjs_display_draw_line(struct mjs* mjs) {
    int x0 = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y0 = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int x1 = mjs_get_int(mjs, mjs_arg(mjs, 2));
    int y1 = mjs_get_int(mjs, mjs_arg(mjs, 3));
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 4));
    mjs_get_canvas(mjs)->drawLine(x0, y0, x1, y1, color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.draw_rect(x, y, w, h, color)
static void mjs_display_draw_rect(struct mjs* mjs) {
    int x = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int w = mjs_get_int(mjs, mjs_arg(mjs, 2));
    int h = mjs_get_int(mjs, mjs_arg(mjs, 3));
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 4));
    mjs_get_canvas(mjs)->drawRect(x, y, w, h, color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.fill_rect(x, y, w, h, color)
static void mjs_display_fill_rect(struct mjs* mjs) {
    int x = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int w = mjs_get_int(mjs, mjs_arg(mjs, 2));
    int h = mjs_get_int(mjs, mjs_arg(mjs, 3));
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 4));
    mjs_get_canvas(mjs)->fillRect(x, y, w, h, color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.draw_circle(x, y, r, color)
static void mjs_display_draw_circle(struct mjs* mjs) {
    int x = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int r = mjs_get_int(mjs, mjs_arg(mjs, 2));
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 3));
    mjs_get_canvas(mjs)->drawCircle(x, y, r, color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.fill_circle(x, y, r, color)
static void mjs_display_fill_circle(struct mjs* mjs) {
    int x = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int r = mjs_get_int(mjs, mjs_arg(mjs, 2));
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 3));
    mjs_get_canvas(mjs)->fillCircle(x, y, r, color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.draw_triangle(x0, y0, x1, y1, x2, y2, color)
static void mjs_display_draw_triangle(struct mjs* mjs) {
    int x0 = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y0 = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int x1 = mjs_get_int(mjs, mjs_arg(mjs, 2));
    int y1 = mjs_get_int(mjs, mjs_arg(mjs, 3));
    int x2 = mjs_get_int(mjs, mjs_arg(mjs, 4));
    int y2 = mjs_get_int(mjs, mjs_arg(mjs, 5));
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 6));
    mjs_get_canvas(mjs)->drawTriangle(x0, y0, x1, y1, x2, y2, color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.fill_triangle(x0, y0, x1, y1, x2, y2, color)
static void mjs_display_fill_triangle(struct mjs* mjs) {
    int x0 = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y0 = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int x1 = mjs_get_int(mjs, mjs_arg(mjs, 2));
    int y1 = mjs_get_int(mjs, mjs_arg(mjs, 3));
    int x2 = mjs_get_int(mjs, mjs_arg(mjs, 4));
    int y2 = mjs_get_int(mjs, mjs_arg(mjs, 5));
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 6));
    mjs_get_canvas(mjs)->fillTriangle(x0, y0, x1, y1, x2, y2, color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.draw_ellipse(x, y, rx, ry, color)
static void mjs_display_draw_ellipse(struct mjs* mjs) {
    int x = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int rx = mjs_get_int(mjs, mjs_arg(mjs, 2));
    int ry = mjs_get_int(mjs, mjs_arg(mjs, 3));
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 4));
    mjs_get_canvas(mjs)->drawEllipse(x, y, rx, ry, color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.fill_ellipse(x, y, rx, ry, color)
static void mjs_display_fill_ellipse(struct mjs* mjs) {
    int x = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int rx = mjs_get_int(mjs, mjs_arg(mjs, 2));
    int ry = mjs_get_int(mjs, mjs_arg(mjs, 3));
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 4));
    mjs_get_canvas(mjs)->fillEllipse(x, y, rx, ry, color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.draw_arc(x, y, r1, r2, startAngle, endAngle, color)
static void mjs_display_draw_arc(struct mjs* mjs) {
    int x = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int r1 = mjs_get_int(mjs, mjs_arg(mjs, 2));
    int r2 = mjs_get_int(mjs, mjs_arg(mjs, 3));
    int startAngle = mjs_get_int(mjs, mjs_arg(mjs, 4));
    int endAngle = mjs_get_int(mjs, mjs_arg(mjs, 5));
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 6));
    mjs_get_canvas(mjs)->drawArc(x, y, r1, r2, startAngle, endAngle, color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.fill_arc(x, y, r1, r2, startAngle, endAngle, color)
static void mjs_display_fill_arc(struct mjs* mjs) {
    int x = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int y = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int r1 = mjs_get_int(mjs, mjs_arg(mjs, 2));
    int r2 = mjs_get_int(mjs, mjs_arg(mjs, 3));
    int startAngle = mjs_get_int(mjs, mjs_arg(mjs, 4));
    int endAngle = mjs_get_int(mjs, mjs_arg(mjs, 5));
    uint16_t color = mjs_get_int(mjs, mjs_arg(mjs, 6));
    mjs_get_canvas(mjs)->fillArc(x, y, r1, r2, startAngle, endAngle, color);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.draw_image(image, x, y) - image is {width, height, pointer}
static void mjs_display_draw_image(struct mjs* mjs) {
    mjs_val_t img = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, img, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Image* image = static_cast<lilka::Image*>(mjs_get_ptr(mjs, ptr_val));
    int16_t x = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int16_t y = mjs_get_int(mjs, mjs_arg(mjs, 2));
    mjs_get_canvas(mjs)->drawImage(image, x, y);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.draw_image_transformed(image, x, y, transform)
static void mjs_display_draw_image_transformed(struct mjs* mjs) {
    mjs_val_t img = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, img, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Image* image = static_cast<lilka::Image*>(mjs_get_ptr(mjs, ptr_val));
    int16_t x = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int16_t y = mjs_get_int(mjs, mjs_arg(mjs, 2));

    mjs_val_t transform_obj = mjs_arg(mjs, 3);
    mjs_val_t transform_ptr_val = mjs_get(mjs, transform_obj, "pointer", ~0);
    if (!mjs_is_foreign(transform_ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Transform* transform = static_cast<lilka::Transform*>(mjs_get_ptr(mjs, transform_ptr_val));
    mjs_get_canvas(mjs)->drawImageTransformed(image, x, y, *transform);
    mjs_return(mjs, mjs_mk_undefined());
}

// display.queue_draw()
static void mjs_display_queue_draw(struct mjs* mjs) {
    mjs_get_app(mjs)->queueDraw();
    mjs_return(mjs, mjs_mk_undefined());
}

void mjs_display_register(struct mjs* mjs, App* app) {
    // Store App* as global __app__ for use by display/resources/etc
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "__app__", ~0, mjs_mk_foreign(mjs, app));

    mjs_val_t display = mjs_mk_object(mjs);

    mjs_set(mjs, display, "color565", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_color565));
    mjs_set(mjs, display, "set_cursor", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_set_cursor));
    mjs_set(mjs, display, "set_font", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_set_font));
    mjs_set(mjs, display, "set_text_size", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_set_text_size));
    mjs_set(mjs, display, "set_text_color", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_set_text_color));
    mjs_set(mjs, display, "set_text_bound", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_set_text_bound));
    mjs_set(mjs, display, "print", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_print));
    mjs_set(mjs, display, "fill_screen", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_fill_screen));
    mjs_set(mjs, display, "draw_pixel", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_draw_pixel));
    mjs_set(mjs, display, "draw_line", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_draw_line));
    mjs_set(mjs, display, "draw_rect", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_draw_rect));
    mjs_set(mjs, display, "fill_rect", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_fill_rect));
    mjs_set(mjs, display, "draw_circle", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_draw_circle));
    mjs_set(mjs, display, "fill_circle", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_fill_circle));
    mjs_set(mjs, display, "draw_triangle", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_draw_triangle));
    mjs_set(mjs, display, "fill_triangle", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_fill_triangle));
    mjs_set(mjs, display, "draw_ellipse", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_draw_ellipse));
    mjs_set(mjs, display, "fill_ellipse", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_fill_ellipse));
    mjs_set(mjs, display, "draw_arc", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_draw_arc));
    mjs_set(mjs, display, "fill_arc", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_fill_arc));
    mjs_set(mjs, display, "draw_image", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_draw_image));
    mjs_set(
        mjs,
        display,
        "draw_image_transformed",
        ~0,
        mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_draw_image_transformed)
    );
    mjs_set(mjs, display, "queue_draw", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_display_queue_draw));

    // Set display width & height
    mjs_set(mjs, display, "width", ~0, mjs_mk_number(mjs, app->canvas->width()));
    mjs_set(mjs, display, "height", ~0, mjs_mk_number(mjs, app->canvas->height()));

    mjs_set(mjs, global, "display", ~0, display);

    // Register colors
    // clang-format off
    const uint16_t colorValues[] = {
        lilka::colors::Black, lilka::colors::White,
        lilka::colors::Red, lilka::colors::Green, lilka::colors::Blue,
        lilka::colors::Cyan, lilka::colors::Magenta, lilka::colors::Yellow,
        lilka::colors::Midnight_blue, lilka::colors::Orange_red
    };
    const char* colorNames[] = {
        "black", "white",
        "red", "green", "blue",
        "cyan", "magenta", "yellow",
        "midnight_blue", "orange_red"
    };
    // clang-format on
    mjs_val_t colors = mjs_mk_object(mjs);
    for (int i = 0; i < (int)(sizeof(colorValues) / sizeof(colorValues[0])); i++) {
        mjs_set(mjs, colors, colorNames[i], ~0, mjs_mk_number(mjs, colorValues[i]));
    }
    mjs_set(mjs, global, "colors", ~0, colors);
}
