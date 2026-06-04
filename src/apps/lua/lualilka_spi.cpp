#include "lualilka_spi.h"

// The user SPI bus (SPI2 / FSPI) is only available on Lilka v2 (ESP32-S3).
// SPI1 is reserved for the display and SD card and must not be touched.
#ifdef SPI2_NUM
#    define LUA_SPI_BUS       lilka::SPI2
#    define LUA_SPI_AVAILABLE 1
#else
#    define LUA_SPI_AVAILABLE 0
#endif

// spi.begin([sck, miso, mosi])
static int lualilka_spi_begin(lua_State* L) {
#if LUA_SPI_AVAILABLE
    int n = lua_gettop(L);
    if (n >= 3) {
        int sck = luaL_checkinteger(L, 1);
        int miso = luaL_checkinteger(L, 2);
        int mosi = luaL_checkinteger(L, 3);
        LUA_SPI_BUS.begin(sck, miso, mosi);
    } else {
        LUA_SPI_BUS.begin();
    }
    return 0;
#else
    return luaL_error(L, "spi is not available on this board");
#endif
}

// spi.transfer(data[, frequency[, mode]]) -> received byte(s)
// data may be a single byte (number) or a table of bytes (full-duplex).
// Returns a number for a single byte, or a table for multiple bytes.
static int lualilka_spi_transfer(lua_State* L) {
#if LUA_SPI_AVAILABLE
    uint32_t freq = (uint32_t)luaL_optinteger(L, 2, 4000000);
    int mode = luaL_optinteger(L, 3, SPI_MODE0);
    SPISettings settings(freq, MSBFIRST, mode);

    if (lua_isnumber(L, 1)) {
        uint8_t out = (uint8_t)(luaL_checkinteger(L, 1) & 0xFF);
        LUA_SPI_BUS.beginTransaction(settings);
        uint8_t in = LUA_SPI_BUS.transfer(out);
        LUA_SPI_BUS.endTransaction();
        lua_pushinteger(L, in);
        return 1;
    }

    luaL_checktype(L, 1, LUA_TTABLE);
    int len = (int)luaL_len(L, 1);
    lua_newtable(L);
    LUA_SPI_BUS.beginTransaction(settings);
    for (int i = 0; i < len; i++) {
        lua_geti(L, 1, i + 1);
        uint8_t out = (uint8_t)(luaL_checkinteger(L, -1) & 0xFF);
        lua_pop(L, 1);
        uint8_t in = LUA_SPI_BUS.transfer(out);
        lua_pushinteger(L, in);
        lua_rawseti(L, -2, i + 1);
    }
    LUA_SPI_BUS.endTransaction();
    return 1;
#else
    return luaL_error(L, "spi is not available on this board");
#endif
}

// spi.close()
static int lualilka_spi_close(lua_State* L) {
#if LUA_SPI_AVAILABLE
    LUA_SPI_BUS.end();
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
