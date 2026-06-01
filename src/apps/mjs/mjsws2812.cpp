#include "mjsws2812.h"
#include <Arduino.h>
#include <new>
#include "mjs.h"

// WS2812 bit timings at a 100 ns RMT tick:
//   '0' bit: ~400 ns high, ~800 ns low
//   '1' bit: ~800 ns high, ~400 ns low
static const uint32_t WS2812_T0H = 4; // 400 ns
static const uint32_t WS2812_T0L = 8; // 800 ns
static const uint32_t WS2812_T1H = 8; // 800 ns
static const uint32_t WS2812_T1L = 4; // 400 ns

// Encode one 24-bit GRB color into 24 RMT symbols (MSB first).
static void ws2812_encode_color(rmt_data_t* out, uint32_t grb) {
    for (int bit = 23; bit >= 0; bit--) {
        out->level0 = 1;
        out->level1 = 0;
        if (grb & (1u << bit)) {
            out->duration0 = WS2812_T1H;
            out->duration1 = WS2812_T1L;
        } else {
            out->duration0 = WS2812_T0H;
            out->duration1 = WS2812_T0L;
        }
        out++;
    }
}

// ws2812.write(pin, colors)
// `colors` is an array of integers, each in 0xRRGGBB form.
static void mjs_ws2812_write(struct mjs* mjs) {
    int pin = mjs_get_int(mjs, mjs_arg(mjs, 0));
    mjs_val_t colors = mjs_arg(mjs, 1);
    if (!mjs_is_array(colors)) {
        mjs_set_errorf(mjs, MJS_BAD_ARGS_ERROR, "ws2812.write expects (pin, array)");
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    int count = mjs_array_length(mjs, colors);
    if (count <= 0) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    rmt_data_t* data = new (std::nothrow) rmt_data_t[count * 24];
    if (!data) {
        mjs_set_errorf(mjs, MJS_INTERNAL_ERROR, "ws2812: out of memory");
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    for (int i = 0; i < count; i++) {
        uint32_t rgb = (uint32_t)mjs_get_int(mjs, mjs_array_get(mjs, colors, i));
        uint8_t r = (rgb >> 16) & 0xFF;
        uint8_t g = (rgb >> 8) & 0xFF;
        uint8_t b = rgb & 0xFF;
        // WS2812 expects data in GRB order.
        uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
        ws2812_encode_color(&data[i * 24], grb);
    }

    rmt_obj_t* rmt = rmtInit(pin, RMT_TX_MODE, RMT_MEM_64);
    if (!rmt) {
        delete[] data;
        mjs_set_errorf(mjs, MJS_INTERNAL_ERROR, "ws2812: failed to init RMT");
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    rmtSetTick(rmt, 100); // 100 ns per tick
    rmtWriteBlocking(rmt, data, count * 24);
    rmtDeinit(rmt);
    delete[] data;
    mjs_return(mjs, mjs_mk_undefined());
}

void mjs_ws2812_register(struct mjs* mjs) {
    mjs_val_t ws2812 = mjs_mk_object(mjs);
    mjs_set(mjs, ws2812, "write", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_ws2812_write));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "ws2812", ~0, ws2812);
}
