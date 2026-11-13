#include "lualilka_modules.h"
#include "lualilka_hw.h"

#include <new>
#include <string.h>

#ifdef SPI2_NUM

using lilka::mod::Nrf24;
namespace pins = lilka::mod::pins;

#    define NRF24_MT "lilka.nrf24"

struct LuaNrf24 {
    LuaHwClaim claim; // must be first
    Nrf24* dev = nullptr;
    uint8_t addressWidth = 5;
};

static LuaNrf24* checkNrf(lua_State* L) {
    LuaNrf24* o = static_cast<LuaNrf24*>(luaL_checkudata(L, 1, NRF24_MT));
    if (!o->dev) luaL_error(L, "nrf24: object is closed");
    return o;
}

static int lualilka_nrf24_close(lua_State* L) {
    LuaNrf24* o = static_cast<LuaNrf24*>(luaL_checkudata(L, 1, NRF24_MT));
    delete o->dev; // powers the radio down
    o->dev = nullptr;
    lualilka_hw_endSpi(&o->claim);
    return 0;
}

// Address: a string of `width` bytes (as sent over the air, LSB first) or an integer.
static void checkAddress(lua_State* L, int idx, uint8_t width, uint8_t* out) {
    if (lua_type(L, idx) == LUA_TSTRING) {
        size_t len;
        const char* s = lua_tolstring(L, idx, &len);
        if (len != width) luaL_error(L, "nrf24: address must be %d bytes", width);
        memcpy(out, s, width);
        return;
    }
    lua_Integer v = luaL_checkinteger(L, idx);
    for (uint8_t i = 0; i < width; i++)
        out[i] = (uint8_t)((uint64_t)v >> (8 * i));
}

// nrf24.new([{channel = 76, rate = 1000000, power = 3, payload = 32 | 0 (dynamic), auto_ack = true,
//             retries = 15, retry_delay = 1500, address_width = 5, crc = 2}]) -> nrf24 | nil, error
static int lualilka_nrf24_new(lua_State* L) {
    Nrf24::Config cfg;
    cfg.channel = (uint8_t)lualilka_opt_int(L, 1, "channel", cfg.channel);
    lua_Integer rate = lualilka_opt_int(L, 1, "rate", 1000000);
    if (rate == 250000) {
        cfg.dataRate = Nrf24::DataRate::Rate250K;
    } else if (rate == 1000000) {
        cfg.dataRate = Nrf24::DataRate::Rate1M;
    } else if (rate == 2000000) {
        cfg.dataRate = Nrf24::DataRate::Rate2M;
    } else {
        return luaL_error(L, "nrf24: rate must be 250000, 1000000 or 2000000");
    }
    cfg.paLevel = (uint8_t)lualilka_opt_int(L, 1, "power", cfg.paLevel);
    cfg.payloadSize = (uint8_t)lualilka_opt_int(L, 1, "payload", cfg.payloadSize);
    cfg.autoAck = lualilka_opt_bool(L, 1, "auto_ack", cfg.autoAck);
    cfg.retries = (uint8_t)lualilka_opt_int(L, 1, "retries", cfg.retries);
    cfg.retryDelayUs = (uint16_t)lualilka_opt_int(L, 1, "retry_delay", cfg.retryDelayUs);
    cfg.addressWidth = (uint8_t)lualilka_opt_int(L, 1, "address_width", cfg.addressWidth);
    cfg.crcBytes = (uint8_t)lualilka_opt_int(L, 1, "crc", cfg.crcBytes);
    luaL_argcheck(L, cfg.channel < Nrf24::CHANNELS, 1, "channel must be 0-125");
    luaL_argcheck(L, cfg.paLevel <= 3, 1, "power must be 0-3");
    luaL_argcheck(L, cfg.payloadSize <= 32, 1, "payload must be 0-32");
    luaL_argcheck(L, cfg.addressWidth >= 3 && cfg.addressWidth <= 5, 1, "address_width must be 3-5");
    luaL_argcheck(L, cfg.crcBytes <= 2, 1, "crc must be 0, 1 or 2");

    // Pins come from lilka::config ([rf] in /sd/.lilka/config overrides the built-in v2 values)
    int sck = pins::get("rf", "sck");
    int miso = pins::get("rf", "miso");
    int mosi = pins::get("rf", "mosi");
    if (sck < 0 || miso < 0 || mosi < 0 || cfg.csn < 0) {
        return luaL_error(L, "nrf24: pins are not configured (section [rf] in %s)", LILKA_CONFIG_DIR);
    }

    LuaNrf24* o = new (lua_newuserdatauv(L, sizeof(LuaNrf24), 0)) LuaNrf24();
    o->dev = nullptr;
    o->addressWidth = cfg.addressWidth;
    lualilka_hw_initClaim(L, &o->claim);
    luaL_setmetatable(L, NRF24_MT);
    lualilka_hw_claimPins(L, &o->claim, {sck, miso, mosi, cfg.csn, cfg.ce, cfg.irq});
    lualilka_hw_beginSpi(L, &o->claim, sck, miso, mosi);
    cfg.spi = &lilka::SPI2;

    o->dev = new (std::nothrow) Nrf24();
    if (!o->dev) return luaL_error(L, "nrf24: out of memory");
    if (!o->dev->begin(cfg)) {
        lualilka_hw_closeTop(L, lualilka_nrf24_close);
        lua_pushnil(L);
        lua_pushstring(L, "nrf24: no nRF24L01+ found (check the module and its seating)");
        return 2;
    }
    return 1;
}

// nrf24:open_tx(address)
static int lualilka_nrf24_openTx(lua_State* L) {
    LuaNrf24* o = checkNrf(L);
    uint8_t addr[5];
    checkAddress(L, 2, o->addressWidth, addr);
    o->dev->openWritingPipe(addr);
    return 0;
}

// nrf24:open_rx(pipe, address) - pipes 2-5 use only the first byte; the rest comes from pipe 1
static int lualilka_nrf24_openRx(lua_State* L) {
    LuaNrf24* o = checkNrf(L);
    lua_Integer pipe = luaL_checkinteger(L, 2);
    luaL_argcheck(L, pipe >= 0 && pipe <= 5, 2, "pipe must be 0-5");
    uint8_t addr[5] = {};
    if (pipe >= 2 && lua_isinteger(L, 3)) {
        addr[0] = (uint8_t)lua_tointeger(L, 3);
    } else {
        checkAddress(L, 3, pipe >= 2 ? 1 : o->addressWidth, addr);
    }
    o->dev->openReadingPipe((uint8_t)pipe, addr);
    return 0;
}

// nrf24:close_rx(pipe)
static int lualilka_nrf24_closeRx(lua_State* L) {
    LuaNrf24* o = checkNrf(L);
    lua_Integer pipe = luaL_checkinteger(L, 2);
    luaL_argcheck(L, pipe >= 0 && pipe <= 5, 2, "pipe must be 0-5");
    o->dev->closeReadingPipe((uint8_t)pipe);
    return 0;
}

static int lualilka_nrf24_listen(lua_State* L) {
    checkNrf(L)->dev->startListening();
    return 0;
}

static int lualilka_nrf24_stopListening(lua_State* L) {
    checkNrf(L)->dev->stopListening();
    return 0;
}

// nrf24:send(data[, timeout_ms = 100]) -> bool (with auto_ack: delivered and acknowledged)
static int lualilka_nrf24_send(lua_State* L) {
    LuaNrf24* o = checkNrf(L);
    uint8_t buf[Nrf24::MAX_PAYLOAD];
    size_t len = lualilka_check_bytes(L, 2, buf, sizeof(buf));
    uint32_t timeout = (uint32_t)luaL_optinteger(L, 3, 100);
    lua_pushboolean(L, o->dev->send(buf, (uint8_t)len, timeout));
    return 1;
}

// nrf24:available() -> pipe | nil
static int lualilka_nrf24_available(lua_State* L) {
    uint8_t pipe;
    if (checkNrf(L)->dev->available(&pipe)) {
        lua_pushinteger(L, pipe);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// nrf24:read() -> data, pipe | nil
static int lualilka_nrf24_read(lua_State* L) {
    LuaNrf24* o = checkNrf(L);
    uint8_t pipe = 0;
    if (!o->dev->available(&pipe)) {
        lua_pushnil(L);
        return 1;
    }
    uint8_t buf[Nrf24::MAX_PAYLOAD];
    uint8_t n = o->dev->read(buf, sizeof(buf));
    lua_pushlstring(L, reinterpret_cast<const char*>(buf), n);
    lua_pushinteger(L, pipe);
    return 2;
}

// nrf24:set_channel(channel)
static int lualilka_nrf24_setChannel(lua_State* L) {
    LuaNrf24* o = checkNrf(L);
    lua_Integer ch = luaL_checkinteger(L, 2);
    luaL_argcheck(L, ch >= 0 && ch < Nrf24::CHANNELS, 2, "channel must be 0-125");
    o->dev->setChannel((uint8_t)ch);
    return 0;
}

// nrf24:carrier() -> bool (signal above -64 dBm on the current channel, while listening)
static int lualilka_nrf24_carrier(lua_State* L) {
    lua_pushboolean(L, checkNrf(L)->dev->carrier());
    return 1;
}

// nrf24:scan([passes = 1[, dwell_us = 260]]) -> {[1] = hits on channel 0, ..., [126] = hits on channel 125}
static int lualilka_nrf24_scan(lua_State* L) {
    LuaNrf24* o = checkNrf(L);
    int passes = luaL_optinteger(L, 2, 1);
    int dwell = luaL_optinteger(L, 3, 260);
    luaL_argcheck(L, passes >= 1 && passes <= 100, 2, "passes must be 1-100");
    luaL_argcheck(L, dwell >= 130 && dwell <= 10000, 3, "dwell_us must be 130-10000");
    uint8_t keep = o->dev->channel();
    uint16_t hits[Nrf24::CHANNELS] = {};
    for (int p = 0; p < passes; p++) {
        for (uint8_t ch = 0; ch < Nrf24::CHANNELS; ch++) {
            if (o->dev->probeChannel(ch, dwell)) hits[ch]++;
        }
    }
    o->dev->setChannel(keep);
    lua_createtable(L, Nrf24::CHANNELS, 0);
    for (int ch = 0; ch < Nrf24::CHANNELS; ch++) {
        lua_pushinteger(L, hits[ch]);
        lua_rawseti(L, -2, ch + 1);
    }
    return 1;
}

static const luaL_Reg nrfMethods[] = {
    {"open_tx", lualilka_nrf24_openTx},
    {"open_rx", lualilka_nrf24_openRx},
    {"close_rx", lualilka_nrf24_closeRx},
    {"listen", lualilka_nrf24_listen},
    {"stop_listening", lualilka_nrf24_stopListening},
    {"send", lualilka_nrf24_send},
    {"available", lualilka_nrf24_available},
    {"read", lualilka_nrf24_read},
    {"set_channel", lualilka_nrf24_setChannel},
    {"carrier", lualilka_nrf24_carrier},
    {"scan", lualilka_nrf24_scan},
    {"close", lualilka_nrf24_close},
    {NULL, NULL},
};

static const luaL_Reg nrfLib[] = {
    {"new", lualilka_nrf24_new},
    {NULL, NULL},
};

int lualilka_nrf24_register(lua_State* L) {
    lualilka_hw_newClass(L, NRF24_MT, nrfMethods, lualilka_nrf24_close);
    luaL_newlib(L, nrfLib);
    lua_setglobal(L, "nrf24");
    return 0;
}

#else

int lualilka_nrf24_register(lua_State*) {
    return 0;
}

#endif
