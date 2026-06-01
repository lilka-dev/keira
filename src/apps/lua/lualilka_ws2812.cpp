#include "lualilka_ws2812.h"
#include <Arduino.h>
#include <new>

// WS2812 ("NeoPixel") addressable LEDs are driven via the ESP32 RMT peripheral.
// At a 100 ns RMT tick the WS2812 bit timings are:
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
// `colors` is a table of integers, each in 0xRRGGBB form.
static int lualilka_ws2812_write(lua_State* L) {
    int pin = luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int count = (int)luaL_len(L, 2);
    if (count <= 0) {
        return 0;
    }

    rmt_data_t* data = new (std::nothrow) rmt_data_t[count * 24];
    if (!data) {
        return luaL_error(L, "ws2812: out of memory");
    }

    for (int i = 0; i < count; i++) {
        lua_geti(L, 2, i + 1);
        uint32_t rgb = (uint32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
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
        return luaL_error(L, "ws2812: failed to init RMT on pin %d", pin);
    }
    rmtSetTick(rmt, 100); // 100 ns per tick
    rmtWriteBlocking(rmt, data, count * 24);
    rmtDeinit(rmt);
    delete[] data;
    return 0;
}

static const luaL_Reg lualilka_ws2812[] = {
    {"write", lualilka_ws2812_write},
    {NULL, NULL},
};

int lualilka_ws2812_register(lua_State* L) {
    luaL_newlib(L, lualilka_ws2812);
    lua_setglobal(L, "ws2812");
    return 0;
}
