#include "lualilka_hw.h"

#include <Wire.h>
#include <lilka.h>
#include <string.h>

using lilka::mod::Periph;
using lilka::mod::resources;

const void* lualilka_hw_owner(lua_State* L) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
    const void* owner = lua_tothread(L, -1);
    lua_pop(L, 1);
    return owner;
}

void lualilka_hw_initClaim(lua_State* L, LuaHwClaim* claim) {
    claim->owner = lualilka_hw_owner(L);
    claim->pinCount = 0;
    claim->periph = -1;
}

void lualilka_hw_claimPins(lua_State* L, LuaHwClaim* claim, std::initializer_list<int> pins) {
    int conflict = -1;
    if (!resources.claimPins(pins, claim->owner, &conflict)) {
        luaL_error(L, "GPIO%d is used by another script", conflict);
    }
    for (int pin : pins) {
        if (pin < 0) continue;
        if (claim->pinCount >= LuaHwClaim::MAX_PINS) {
            resources.releasePin(pin, claim->owner);
            continue;
        }
        claim->pins[claim->pinCount++] = pin;
    }
}

void lualilka_hw_claimPeriph(lua_State* L, LuaHwClaim* claim, Periph periph, const char* name) {
    if (!resources.claim(periph, claim->owner)) {
        luaL_error(L, "%s is used by another script", name);
    }
    claim->periph = static_cast<int>(periph);
}

uint16_t lualilka_hw_release(LuaHwClaim* claim) {
    for (int i = 0; i < claim->pinCount; i++) {
        resources.releasePin(claim->pins[i], claim->owner);
    }
    claim->pinCount = 0;
    uint16_t left = 0;
    if (claim->periph >= 0) {
        left = resources.release(static_cast<Periph>(claim->periph), claim->owner);
        claim->periph = -1;
    }
    return left;
}

void lualilka_hw_releaseOwner(const void* owner) {
    resources.releaseAll(owner);
}

void lualilka_hw_beginSpi(lua_State* L, LuaHwClaim* claim, int sck, int miso, int mosi) {
#ifdef SPI2_NUM
    bool first = resources.owner(Periph::SPI2) == nullptr;
    lualilka_hw_claimPeriph(L, claim, Periph::SPI2, "SPI2");
    if (first) lilka::SPI2.begin(sck, miso, mosi, -1);
#else
    luaL_error(L, "SPI2 is not available on this board");
#endif
}

void lualilka_hw_endSpi(LuaHwClaim* claim) {
#ifdef SPI2_NUM
    bool held = claim->periph == static_cast<int>(Periph::SPI2);
    if (lualilka_hw_release(claim) == 0 && held) lilka::SPI2.end();
#else
    lualilka_hw_release(claim);
#endif
}

void lualilka_hw_beginI2c(lua_State* L, LuaHwClaim* claim, int sda, int scl, uint32_t freq) {
    bool first = resources.owner(Periph::I2C0) == nullptr;
    lualilka_hw_claimPeriph(L, claim, Periph::I2C0, "I2C0");
    if (first) {
        Wire.end();
        if (!Wire.begin(sda, scl, freq)) {
            lualilka_hw_release(claim);
            luaL_error(L, "failed to start I2C on SDA=%d SCL=%d", sda, scl);
        }
    }
}

void lualilka_hw_endI2c(LuaHwClaim* claim) {
    bool held = claim->periph == static_cast<int>(Periph::I2C0);
    uint16_t left = lualilka_hw_release(claim);
    if (held && left == 0) Wire.end();
}

void lualilka_hw_closeTop(lua_State* L, lua_CFunction close) {
    lua_pushcfunction(L, close);
    lua_insert(L, -2);
    lua_call(L, 1, 0);
}

void lualilka_hw_newClass(lua_State* L, const char* name, const luaL_Reg* methods, lua_CFunction gc) {
    luaL_newmetatable(L, name);
    luaL_newlib(L, methods);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, gc);
    lua_setfield(L, -2, "__close");
    lua_pop(L, 1);
}

static bool pushOpt(lua_State* L, int idx, const char* key) {
    if (lua_isnoneornil(L, idx)) return false;
    luaL_checktype(L, idx, LUA_TTABLE);
    lua_getfield(L, idx, key);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    return true;
}

lua_Integer lualilka_opt_int(lua_State* L, int idx, const char* key, lua_Integer def) {
    if (!pushOpt(L, idx, key)) return def;
    if (!lua_isinteger(L, -1)) luaL_error(L, "option '%s' must be an integer", key);
    lua_Integer v = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return v;
}

lua_Number lualilka_opt_num(lua_State* L, int idx, const char* key, lua_Number def) {
    if (!pushOpt(L, idx, key)) return def;
    if (!lua_isnumber(L, -1)) luaL_error(L, "option '%s' must be a number", key);
    lua_Number v = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return v;
}

bool lualilka_opt_bool(lua_State* L, int idx, const char* key, bool def) {
    if (!pushOpt(L, idx, key)) return def;
    bool v = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return v;
}

const char* lualilka_opt_str(lua_State* L, int idx, const char* key, const char* def) {
    if (!pushOpt(L, idx, key)) return def;
    if (lua_type(L, -1) != LUA_TSTRING) luaL_error(L, "option '%s' must be a string", key);
    const char* v = lua_tostring(L, -1);
    lua_pop(L, 1); // still referenced by the options table
    return v;
}

int lualilka_opt_pin(lua_State* L, int idx, const char* key, int def) {
    if (!pushOpt(L, idx, key)) return def;
    int pin;
    if (lua_isboolean(L, -1) && !lua_toboolean(L, -1)) {
        pin = -1;
    } else if (lua_isinteger(L, -1)) {
        pin = (int)lua_tointeger(L, -1);
        if (pin < 0 || pin >= SOC_GPIO_PIN_COUNT) luaL_error(L, "option '%s': invalid GPIO %d", key, pin);
    } else {
        return luaL_error(L, "option '%s' must be a GPIO number or false", key);
    }
    lua_pop(L, 1);
    return pin;
}

size_t lualilka_check_bytes(lua_State* L, int idx, uint8_t* buf, size_t max) {
    if (lua_type(L, idx) == LUA_TSTRING) {
        size_t len;
        const char* s = lua_tolstring(L, idx, &len);
        if (len > max) luaL_error(L, "data too long (%d bytes, max %d)", (int)len, (int)max);
        memcpy(buf, s, len);
        return len;
    }
    if (lua_isinteger(L, idx)) {
        if (max < 1) luaL_error(L, "data too long");
        buf[0] = (uint8_t)(lua_tointeger(L, idx) & 0xFF);
        return 1;
    }
    luaL_checktype(L, idx, LUA_TTABLE);
    size_t len = (size_t)luaL_len(L, idx);
    if (len > max) luaL_error(L, "data too long (%d bytes, max %d)", (int)len, (int)max);
    for (size_t i = 0; i < len; i++) {
        lua_geti(L, idx, (lua_Integer)i + 1);
        buf[i] = (uint8_t)(luaL_checkinteger(L, -1) & 0xFF);
        lua_pop(L, 1);
    }
    return len;
}
