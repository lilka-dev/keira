#include "lualilka_modules.h"
#include "lualilka_hw.h"

#include <new>
#include <string.h>

#ifdef SPI2_NUM

using lilka::mod::Cc1101;
namespace pins = lilka::mod::pins;

#    define CC1101_MT "lilka.cc1101"

struct LuaCc1101 {
    LuaHwClaim claim; // must be first
    Cc1101* dev = nullptr;
};

static LuaCc1101* checkCc(lua_State* L) {
    LuaCc1101* o = static_cast<LuaCc1101*>(luaL_checkudata(L, 1, CC1101_MT));
    if (!o->dev) luaL_error(L, "cc1101: object is closed");
    return o;
}

static int lualilka_cc1101_close(lua_State* L) {
    LuaCc1101* o = static_cast<LuaCc1101*>(luaL_checkudata(L, 1, CC1101_MT));
    delete o->dev; // back to IDLE
    o->dev = nullptr;
    lualilka_hw_endSpi(&o->claim);
    return 0;
}

static bool validFrequency(double mhz) {
    return (mhz >= 300 && mhz <= 348) || (mhz >= 387 && mhz <= 464) || (mhz >= 779 && mhz <= 928);
}

// cc1101.new([{frequency = 433.92, modulation = "gfsk"|"2fsk"|"ask"|"4fsk"|"msk", rate = 38400,
//              deviation = 20000, bandwidth = 100000, power = 0, sync_word = 0xD391, crc = true}])
//   -> cc1101 | nil, error
static int lualilka_cc1101_new(lua_State* L) {
    Cc1101::Config cfg;
    cfg.frequencyMhz = (float)lualilka_opt_num(L, 1, "frequency", cfg.frequencyMhz);
    if (!validFrequency(cfg.frequencyMhz)) {
        return luaL_error(L, "cc1101: frequency must be in 300-348, 387-464 or 779-928 MHz");
    }
    const char* mod = lualilka_opt_str(L, 1, "modulation", "gfsk");
    if (!strcmp(mod, "2fsk")) {
        cfg.modulation = Cc1101::Modulation::FSK2;
    } else if (!strcmp(mod, "gfsk")) {
        cfg.modulation = Cc1101::Modulation::GFSK;
    } else if (!strcmp(mod, "ask") || !strcmp(mod, "ook")) {
        cfg.modulation = Cc1101::Modulation::ASK;
    } else if (!strcmp(mod, "4fsk")) {
        cfg.modulation = Cc1101::Modulation::FSK4;
    } else if (!strcmp(mod, "msk")) {
        cfg.modulation = Cc1101::Modulation::MSK;
    } else {
        return luaL_error(L, "cc1101: unknown modulation '%s'", mod);
    }
    cfg.dataRate = (uint32_t)lualilka_opt_int(L, 1, "rate", cfg.dataRate);
    cfg.deviationHz = (uint32_t)lualilka_opt_int(L, 1, "deviation", cfg.deviationHz);
    cfg.bandwidthHz = (uint32_t)lualilka_opt_int(L, 1, "bandwidth", cfg.bandwidthHz);
    cfg.powerDbm = (int8_t)lualilka_opt_int(L, 1, "power", cfg.powerDbm);
    cfg.syncWord = (uint16_t)lualilka_opt_int(L, 1, "sync_word", cfg.syncWord);
    cfg.crc = lualilka_opt_bool(L, 1, "crc", cfg.crc);
    luaL_argcheck(L, cfg.dataRate >= 600 && cfg.dataRate <= 500000, 1, "rate must be 600-500000");

    LuaCc1101* o = new (lua_newuserdatauv(L, sizeof(LuaCc1101), 0)) LuaCc1101();
    o->dev = nullptr;
    lualilka_hw_initClaim(L, &o->claim);
    luaL_setmetatable(L, CC1101_MT);
    lualilka_hw_claimPins(L, &o->claim, {pins::SPI_SCK, pins::SPI_MISO, pins::SPI_MOSI, cfg.csn, cfg.gdo0, cfg.gdo2});
    lualilka_hw_beginSpi(L, &o->claim, pins::SPI_SCK, pins::SPI_MISO, pins::SPI_MOSI);
    cfg.spi = &lilka::SPI2;

    o->dev = new (std::nothrow) Cc1101();
    if (!o->dev) return luaL_error(L, "cc1101: out of memory");
    if (!o->dev->begin(cfg)) {
        lualilka_hw_closeTop(L, lualilka_cc1101_close);
        lua_pushnil(L);
        lua_pushstring(L, "cc1101: no CC1101 found (check the module and its seating)");
        return 2;
    }
    return 1;
}

// cc1101:send(data[, timeout_ms = 500]) -> bool (max 61 bytes)
static int lualilka_cc1101_send(lua_State* L) {
    LuaCc1101* o = checkCc(L);
    uint8_t buf[Cc1101::MAX_PACKET];
    size_t len = lualilka_check_bytes(L, 2, buf, sizeof(buf));
    uint32_t timeout = (uint32_t)luaL_optinteger(L, 3, 500);
    lua_pushboolean(L, o->dev->send(buf, (uint8_t)len, timeout));
    return 1;
}

static int lualilka_cc1101_listen(lua_State* L) {
    checkCc(L)->dev->startReceive();
    return 0;
}

// cc1101:receive() -> {data, rssi, lqi, crc_ok} | nil
static int lualilka_cc1101_receive(lua_State* L) {
    LuaCc1101* o = checkCc(L);
    if (!o->dev->isReceiving()) o->dev->startReceive();
    Cc1101::Packet p;
    if (!o->dev->receive(p)) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, 0, 4);
    lua_pushlstring(L, reinterpret_cast<const char*>(p.data), p.length);
    lua_setfield(L, -2, "data");
    lua_pushnumber(L, p.rssi);
    lua_setfield(L, -2, "rssi");
    lua_pushinteger(L, p.lqi);
    lua_setfield(L, -2, "lqi");
    lua_pushboolean(L, p.crcOk);
    lua_setfield(L, -2, "crc_ok");
    return 1;
}

// cc1101:rssi() -> dBm
static int lualilka_cc1101_rssi(lua_State* L) {
    LuaCc1101* o = checkCc(L);
    if (!o->dev->isReceiving()) o->dev->startReceive();
    lua_pushnumber(L, o->dev->rssi());
    return 1;
}

// cc1101:set_frequency(mhz) -> bool
static int lualilka_cc1101_setFrequency(lua_State* L) {
    lua_pushboolean(L, checkCc(L)->dev->setFrequency((float)luaL_checknumber(L, 2)));
    return 1;
}

// cc1101:set_power(dbm)
static int lualilka_cc1101_setPower(lua_State* L) {
    checkCc(L)->dev->setPower((int8_t)luaL_checkinteger(L, 2));
    return 0;
}

static int lualilka_cc1101_idle(lua_State* L) {
    checkCc(L)->dev->idle();
    return 0;
}

static const luaL_Reg ccMethods[] = {
    {"send", lualilka_cc1101_send},
    {"listen", lualilka_cc1101_listen},
    {"receive", lualilka_cc1101_receive},
    {"rssi", lualilka_cc1101_rssi},
    {"set_frequency", lualilka_cc1101_setFrequency},
    {"set_power", lualilka_cc1101_setPower},
    {"idle", lualilka_cc1101_idle},
    {"close", lualilka_cc1101_close},
    {NULL, NULL},
};

static const luaL_Reg ccLib[] = {
    {"new", lualilka_cc1101_new},
    {NULL, NULL},
};

int lualilka_cc1101_register(lua_State* L) {
    lualilka_hw_newClass(L, CC1101_MT, ccMethods, lualilka_cc1101_close);
    luaL_newlib(L, ccLib);
    lua_setglobal(L, "cc1101");
    return 0;
}

#else

int lualilka_cc1101_register(lua_State*) {
    return 0;
}

#endif
