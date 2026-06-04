#include "lualilka_i2c.h"
#include <Wire.h>

// Reads a Lua argument (at index `idx`) that is either a single byte (number)
// or a table/array of bytes into `buf`. Returns the number of bytes read.
static int lualilka_i2c_readBytesArg(lua_State* L, int idx, uint8_t* buf, int maxLen) {
    if (lua_isnumber(L, idx)) {
        if (maxLen < 1) {
            return 0;
        }
        buf[0] = (uint8_t)(luaL_checkinteger(L, idx) & 0xFF);
        return 1;
    }
    luaL_checktype(L, idx, LUA_TTABLE);
    int len = (int)luaL_len(L, idx);
    if (len > maxLen) {
        len = maxLen;
    }
    for (int i = 0; i < len; i++) {
        lua_geti(L, idx, i + 1);
        buf[i] = (uint8_t)(luaL_checkinteger(L, -1) & 0xFF);
        lua_pop(L, 1);
    }
    return len;
}

// i2c.begin([sda, scl[, freq]])
static int lualilka_i2c_begin(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 0) {
        Wire.begin();
    } else {
        int sda = luaL_checkinteger(L, 1);
        int scl = luaL_checkinteger(L, 2);
        uint32_t freq = (uint32_t)luaL_optinteger(L, 3, 100000);
        Wire.begin(sda, scl, freq);
    }
    return 0;
}

// i2c.set_clock(freq)
static int lualilka_i2c_setClock(lua_State* L) {
    uint32_t freq = (uint32_t)luaL_checkinteger(L, 1);
    Wire.setClock(freq);
    return 0;
}

// i2c.scan() -> table of 7-bit addresses found on the bus
static int lualilka_i2c_scan(lua_State* L) {
    lua_newtable(L);
    int idx = 1;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            lua_pushinteger(L, addr);
            lua_rawseti(L, -2, idx++);
        }
    }
    return 1;
}

// i2c.write(addr, data) -> status (0 = success)
// data may be a single byte (number) or a table of bytes.
static int lualilka_i2c_write(lua_State* L) {
    int addr = luaL_checkinteger(L, 1);
    uint8_t buf[256];
    int len = lualilka_i2c_readBytesArg(L, 2, buf, sizeof(buf));
    Wire.beginTransmission((uint8_t)addr);
    if (len > 0) {
        Wire.write(buf, len);
    }
    uint8_t status = Wire.endTransmission();
    lua_pushinteger(L, status);
    return 1;
}

// i2c.read(addr, count) -> table of bytes
static int lualilka_i2c_read(lua_State* L) {
    int addr = luaL_checkinteger(L, 1);
    int count = luaL_checkinteger(L, 2);
    int got = Wire.requestFrom((uint8_t)addr, (uint8_t)count);
    lua_newtable(L);
    for (int i = 0; i < got && Wire.available(); i++) {
        lua_pushinteger(L, Wire.read());
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

// i2c.write_read(addr, wdata, count) -> table of bytes
// Writes wdata (number or table) then performs a repeated-start read of `count` bytes.
static int lualilka_i2c_writeRead(lua_State* L) {
    int addr = luaL_checkinteger(L, 1);
    uint8_t buf[256];
    int len = lualilka_i2c_readBytesArg(L, 2, buf, sizeof(buf));
    int count = luaL_checkinteger(L, 3);
    Wire.beginTransmission((uint8_t)addr);
    if (len > 0) {
        Wire.write(buf, len);
    }
    // Keep the bus active to issue a repeated start (no stop condition).
    Wire.endTransmission(false);
    int got = Wire.requestFrom((uint8_t)addr, (uint8_t)count);
    lua_newtable(L);
    for (int i = 0; i < got && Wire.available(); i++) {
        lua_pushinteger(L, Wire.read());
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

static const luaL_Reg lualilka_i2c[] = {
    {"begin", lualilka_i2c_begin},
    {"set_clock", lualilka_i2c_setClock},
    {"scan", lualilka_i2c_scan},
    {"write", lualilka_i2c_write},
    {"read", lualilka_i2c_read},
    {"write_read", lualilka_i2c_writeRead},
    {NULL, NULL},
};

int lualilka_i2c_register(lua_State* L) {
    luaL_newlib(L, lualilka_i2c);
    lua_setglobal(L, "i2c");
    return 0;
}
