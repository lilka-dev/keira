#include "lualilka_spi.h"
#include "lualilka_hw.h"

// The user SPI bus (SPI2 / FSPI) is only available on Lilka v2 (ESP32-S3).
// SPI1 is reserved for the display and SD card and must not be touched.
#ifdef SPI2_NUM
#    define LUA_SPI_BUS       lilka::SPI2
#    define LUA_SPI_AVAILABLE 1
#else
#    define LUA_SPI_AVAILABLE 0
#endif

#define SPI_STATE_MT "lilka.spi.state"
#define SPI_MAX_LEN  4096

// SPI2 claim of one script. Lives in the registry, so it is released when the Lua state is closed.
struct LuaSpiState {
    LuaHwClaim claim; // must be first
};

static int lualilka_spi_stateGc(lua_State* L) {
    LuaSpiState* s = static_cast<LuaSpiState*>(luaL_checkudata(L, 1, SPI_STATE_MT));
    lualilka_hw_endSpi(&s->claim);
    return 0;
}

static LuaSpiState* lualilka_spi_state(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, SPI_STATE_MT);
    LuaSpiState* s = static_cast<LuaSpiState*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return s;
}

// spi.begin([sck, miso, mosi])
static int lualilka_spi_begin(lua_State* L) {
#if LUA_SPI_AVAILABLE
    LuaSpiState* s = lualilka_spi_state(L);
    lualilka_hw_endSpi(&s->claim);
    if (lua_gettop(L) >= 3) {
        int sck = luaL_checkinteger(L, 1);
        int miso = luaL_checkinteger(L, 2);
        int mosi = luaL_checkinteger(L, 3);
        lualilka_hw_claimPins(L, &s->claim, {sck, miso, mosi});
        lualilka_hw_beginSpi(L, &s->claim, sck, miso, mosi);
    } else {
        lualilka_hw_beginSpi(L, &s->claim, -1, -1, -1);
    }
    return 0;
#else
    return luaL_error(L, "spi is not available on this board");
#endif
}

// spi.transfer(data[, frequency[, mode[, cs]]]) -> received data
// data may be a single byte (number), a string or a table of bytes (full-duplex).
// The result has the same type as data. If cs is given, it is driven low for the whole transfer.
static int lualilka_spi_transfer(lua_State* L) {
#if LUA_SPI_AVAILABLE
    uint32_t freq = (uint32_t)luaL_optinteger(L, 2, 4000000);
    int mode = luaL_optinteger(L, 3, SPI_MODE0);
    int cs = luaL_optinteger(L, 4, -1);
    SPISettings settings(freq, MSBFIRST, mode);
    int type = lua_type(L, 1);

    uint8_t single;
    uint8_t* buf = &single;
    size_t len = 1;
    luaL_Buffer b;
    if (type == LUA_TNUMBER) {
        single = (uint8_t)(luaL_checkinteger(L, 1) & 0xFF);
    } else {
        size_t n = type == LUA_TSTRING ? lua_rawlen(L, 1) : (size_t)luaL_len(L, 1);
        if (n > SPI_MAX_LEN) return luaL_error(L, "spi: transfer too long (max %d bytes)", SPI_MAX_LEN);
        buf = reinterpret_cast<uint8_t*>(luaL_buffinitsize(L, &b, n));
        len = lualilka_check_bytes(L, 1, buf, n);
    }

    if (cs >= 0) {
        pinMode(cs, OUTPUT);
        digitalWrite(cs, LOW);
    }
    LUA_SPI_BUS.beginTransaction(settings);
    LUA_SPI_BUS.transfer(buf, len); // in place
    LUA_SPI_BUS.endTransaction();
    if (cs >= 0) digitalWrite(cs, HIGH);

    if (type == LUA_TNUMBER) {
        lua_pushinteger(L, single);
    } else if (type == LUA_TSTRING) {
        luaL_pushresultsize(&b, len);
    } else {
        luaL_pushresultsize(&b, len);
        const uint8_t* in = reinterpret_cast<const uint8_t*>(lua_tostring(L, -1));
        lua_createtable(L, (int)len, 0);
        for (size_t i = 0; i < len; i++) {
            lua_pushinteger(L, in[i]);
            lua_rawseti(L, -2, (lua_Integer)i + 1);
        }
    }
    return 1;
#else
    return luaL_error(L, "spi is not available on this board");
#endif
}

// spi.close()
static int lualilka_spi_close(lua_State* L) {
#if LUA_SPI_AVAILABLE
    lualilka_hw_endSpi(&lualilka_spi_state(L)->claim);
    return 0;
#else
    return luaL_error(L, "spi is not available on this board");
#endif
}

static const luaL_Reg lualilka_spi[] = {
    {"begin", lualilka_spi_begin},
    {"transfer", lualilka_spi_transfer},
    {"close", lualilka_spi_close},
    {NULL, NULL},
};

int lualilka_spi_register(lua_State* L) {
    LuaSpiState* s = static_cast<LuaSpiState*>(lua_newuserdatauv(L, sizeof(LuaSpiState), 0));
    lualilka_hw_initClaim(L, &s->claim);
    luaL_newmetatable(L, SPI_STATE_MT);
    lua_pushcfunction(L, lualilka_spi_stateGc);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, SPI_STATE_MT);

    luaL_newlib(L, lualilka_spi);
    // SPI modes as constants for convenience
    lua_pushinteger(L, SPI_MODE0);
    lua_setfield(L, -2, "MODE0");
    lua_pushinteger(L, SPI_MODE1);
    lua_setfield(L, -2, "MODE1");
    lua_pushinteger(L, SPI_MODE2);
    lua_setfield(L, -2, "MODE2");
    lua_pushinteger(L, SPI_MODE3);
    lua_setfield(L, -2, "MODE3");
    lua_setglobal(L, "spi");
    return 0;
}
