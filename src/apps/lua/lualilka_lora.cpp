#include "lualilka_modules.h"
#include "lualilka_hw.h"

#include <new>
#include <string.h>

#ifdef SPI2_NUM

using lilka::mod::SX126x;
namespace pins = lilka::mod::pins;

#    define LORA_MT "lilka.lora"

struct LuaLora {
    LuaHwClaim claim; // must be first
    SX126x* dev = nullptr;
};

static const char* resultName(SX126x::Result r) {
    switch (r) {
        case SX126x::Result::OK:
            return "ok";
        case SX126x::Result::NoChip:
            return "no SX126x found (check the module and its seating)";
        case SX126x::Result::BusyTimeout:
            return "BUSY line stuck high";
        case SX126x::Result::DeviceError:
            return "calibration/TCXO error";
        case SX126x::Result::InvalidParam:
            return "invalid parameter";
        case SX126x::Result::Timeout:
            return "timeout";
    }
    return "unknown error";
}

static LuaLora* checkLora(lua_State* L) {
    LuaLora* o = static_cast<LuaLora*>(luaL_checkudata(L, 1, LORA_MT));
    if (!o->dev) luaL_error(L, "lora: object is closed");
    return o;
}

static int lualilka_lora_close(lua_State* L) {
    LuaLora* o = static_cast<LuaLora*>(luaL_checkudata(L, 1, LORA_MT));
    delete o->dev; // puts the radio to sleep
    o->dev = nullptr;
    lualilka_hw_endSpi(&o->claim);
    return 0;
}

// lora.new([{chip = "sx1262"|"sx1268", frequency = 868100000, sf = 7, bandwidth = 125000, cr = 5,
//            power = 0, sync_word = 0x12, preamble = 8, crc = true, invert_iq = false}]) -> lora | nil, error
static int lualilka_lora_new(lua_State* L) {
    SX126x::Config cfg;
    const char* chip = lualilka_opt_str(L, 1, "chip", "sx1262");
    if (!strcmp(chip, "sx1262")) {
        cfg.chip = SX126x::Chip::SX1262;
    } else if (!strcmp(chip, "sx1268")) {
        cfg.chip = SX126x::Chip::SX1268;
        cfg.frequencyHz = 433175000;
    } else {
        return luaL_error(L, "lora: unknown chip '%s' (sx1262, sx1268)", chip);
    }
    cfg.frequencyHz = (uint32_t)lualilka_opt_int(L, 1, "frequency", cfg.frequencyHz);
    cfg.spreadingFactor = (uint8_t)lualilka_opt_int(L, 1, "sf", cfg.spreadingFactor);
    cfg.bandwidthHz = (uint32_t)lualilka_opt_int(L, 1, "bandwidth", cfg.bandwidthHz);
    cfg.codingRate = (uint8_t)lualilka_opt_int(L, 1, "cr", cfg.codingRate);
    cfg.powerDbm = (int8_t)lualilka_opt_int(L, 1, "power", cfg.powerDbm);
    cfg.syncWord = (uint8_t)lualilka_opt_int(L, 1, "sync_word", cfg.syncWord);
    cfg.preambleLength = (uint16_t)lualilka_opt_int(L, 1, "preamble", cfg.preambleLength);
    cfg.crc = lualilka_opt_bool(L, 1, "crc", cfg.crc);
    cfg.invertIq = lualilka_opt_bool(L, 1, "invert_iq", cfg.invertIq);

    // Pins come from lilka::config ([lora] in /sd/.lilka/config overrides the built-in v2 values)
    int sck = pins::get("lora", "sck");
    int miso = pins::get("lora", "miso");
    int mosi = pins::get("lora", "mosi");
    if (sck < 0 || miso < 0 || mosi < 0 || cfg.nss < 0) {
        return luaL_error(L, "lora: pins are not configured (section [lora] in %s)", LILKA_CONFIG_DIR);
    }

    LuaLora* o = new (lua_newuserdatauv(L, sizeof(LuaLora), 0)) LuaLora();
    o->dev = nullptr;
    lualilka_hw_initClaim(L, &o->claim);
    luaL_setmetatable(L, LORA_MT);
    lualilka_hw_claimPins(L, &o->claim, {sck, miso, mosi, cfg.nss, cfg.busy, cfg.nrst, cfg.rxen, cfg.txen});
    lualilka_hw_beginSpi(L, &o->claim, sck, miso, mosi);
    cfg.spi = &lilka::SPI2;

    o->dev = new (std::nothrow) SX126x();
    if (!o->dev) return luaL_error(L, "lora: out of memory");
    SX126x::Result r = o->dev->begin(cfg);
    if (r != SX126x::Result::OK) {
        lualilka_hw_closeTop(L, lualilka_lora_close);
        lua_pushnil(L);
        lua_pushfstring(L, "lora: %s", resultName(r));
        // cppcheck-suppress memleak ; dev is owned by the userdata and freed in close/__gc
        return 2;
    }
    return 1;
}

// lora:send(data[, timeout_ms = 3000]) -> ok, time_on_air_ms | false, error
static int lualilka_lora_send(lua_State* L) {
    LuaLora* o = checkLora(L);
    uint8_t buf[255];
    size_t len = lualilka_check_bytes(L, 2, buf, sizeof(buf));
    uint32_t timeout = (uint32_t)luaL_optinteger(L, 3, 3000);
    uint32_t air = 0;
    SX126x::Result r = o->dev->send(buf, (uint8_t)len, timeout, &air);
    lua_pushboolean(L, r == SX126x::Result::OK);
    if (r == SX126x::Result::OK) {
        lua_pushinteger(L, air);
    } else {
        lua_pushstring(L, resultName(r));
    }
    return 2;
}

// lora:listen() - continuous receive (receive() also starts it on first call)
static int lualilka_lora_listen(lua_State* L) {
    checkLora(L)->dev->startReceive();
    return 0;
}

// lora:receive() -> {data, rssi, snr, crc_ok} | nil
static int lualilka_lora_receive(lua_State* L) {
    LuaLora* o = checkLora(L);
    if (!o->dev->isReceiving()) o->dev->startReceive();
    SX126x::Packet* p = static_cast<SX126x::Packet*>(lua_newuserdatauv(L, sizeof(SX126x::Packet), 0));
    if (!o->dev->receive(*p)) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, 0, 4);
    lua_pushlstring(L, reinterpret_cast<const char*>(p->data), p->length);
    lua_setfield(L, -2, "data");
    lua_pushnumber(L, p->rssi);
    lua_setfield(L, -2, "rssi");
    lua_pushnumber(L, p->snr);
    lua_setfield(L, -2, "snr");
    lua_pushboolean(L, !p->crcError);
    lua_setfield(L, -2, "crc_ok");
    return 1;
}

// lora:rssi() -> dBm (channel noise/signal right now, in receive mode)
static int lualilka_lora_rssi(lua_State* L) {
    LuaLora* o = checkLora(L);
    if (!o->dev->isReceiving()) o->dev->startReceive();
    lua_pushnumber(L, o->dev->rssi());
    return 1;
}

// lora:set_frequency(hz) -> bool
static int lualilka_lora_setFrequency(lua_State* L) {
    LuaLora* o = checkLora(L);
    lua_pushboolean(L, o->dev->setFrequency((uint32_t)luaL_checkinteger(L, 2)) == SX126x::Result::OK);
    return 1;
}

// lora:set_power(dbm) -> bool
static int lualilka_lora_setPower(lua_State* L) {
    LuaLora* o = checkLora(L);
    lua_pushboolean(L, o->dev->setTxPower((int8_t)luaL_checkinteger(L, 2)) == SX126x::Result::OK);
    return 1;
}

// lora:time_on_air(length) -> ms
static int lualilka_lora_timeOnAir(lua_State* L) {
    LuaLora* o = checkLora(L);
    lua_Integer len = luaL_checkinteger(L, 2);
    luaL_argcheck(L, len >= 0 && len <= 255, 2, "length must be 0-255");
    lua_pushinteger(L, o->dev->timeOnAirMs((uint8_t)len));
    return 1;
}

static int lualilka_lora_standby(lua_State* L) {
    checkLora(L)->dev->standby();
    return 0;
}

static int lualilka_lora_sleep(lua_State* L) {
    checkLora(L)->dev->sleep();
    return 0;
}

static const luaL_Reg loraMethods[] = {
    {"send", lualilka_lora_send},
    {"listen", lualilka_lora_listen},
    {"receive", lualilka_lora_receive},
    {"rssi", lualilka_lora_rssi},
    {"set_frequency", lualilka_lora_setFrequency},
    {"set_power", lualilka_lora_setPower},
    {"time_on_air", lualilka_lora_timeOnAir},
    {"standby", lualilka_lora_standby},
    {"sleep", lualilka_lora_sleep},
    {"close", lualilka_lora_close},
    {NULL, NULL},
};

static const luaL_Reg loraLib[] = {
    {"new", lualilka_lora_new},
    {NULL, NULL},
};

int lualilka_lora_register(lua_State* L) {
    lualilka_hw_newClass(L, LORA_MT, loraMethods, lualilka_lora_close);
    luaL_newlib(L, loraLib);
    lua_setglobal(L, "lora");
    return 0;
}

#else

int lualilka_lora_register(lua_State*) {
    return 0;
}

#endif
