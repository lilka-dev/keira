#include <HardwareSerial.h>
#include <new>
#include "lualilka_serial.h"
#include "lualilka_hw.h"

#define DEFAULT_BAUD   115200
#define DEFAULT_CONFIG SERIAL_8N1
#define SERIAL_PORT_MT "lilka.serial.port"

// UART1 state of one script. Lives in the registry, so it is released when the Lua state is closed.
struct LuaSerialPort {
    LuaHwClaim claim; // must be first
    HardwareSerial* port = nullptr;
};

static int lualilka_serial_portGc(lua_State* L) {
    LuaSerialPort* s = static_cast<LuaSerialPort*>(luaL_checkudata(L, 1, SERIAL_PORT_MT));
    if (s->port) {
        s->port->end();
        delete s->port;
        s->port = nullptr;
    }
    lualilka_hw_release(&s->claim);
    return 0;
}

static LuaSerialPort* lualilka_serial_state(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, SERIAL_PORT_MT);
    LuaSerialPort* s = static_cast<LuaSerialPort*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return s;
}

// Port that has been started with serial.begin(); raises an error otherwise.
static HardwareSerial& lualilka_serial_port(lua_State* L) {
    LuaSerialPort* s = lualilka_serial_state(L);
    if (!s || !s->port) luaL_error(L, "serial: call serial.begin() first");
    return *s->port;
}

static int lualilka_serial_available(lua_State* L) {
    int value = lualilka_serial_port(L).available();
    lua_pushinteger(L, value);
    return 1;
}

// serial.begin([baud[, config[, rx, tx]]]) - rx/tx default to GPIO47/48 (header pins 6/5)
static int lualilka_serial_begin(lua_State* L) {
    int baud = luaL_optinteger(L, 1, DEFAULT_BAUD);
    int config = luaL_optinteger(L, 2, DEFAULT_CONFIG);
    int rx = luaL_optinteger(L, 3, LILKA_P4);
    int tx = luaL_optinteger(L, 4, LILKA_P3);
    LuaSerialPort* s = lualilka_serial_state(L);
    if (s->port) {
        s->port->end();
        delete s->port;
        s->port = nullptr;
    }
    lualilka_hw_release(&s->claim);
    lualilka_hw_claimPeriph(L, &s->claim, lilka::mod::Periph::UART1, "UART1");
    lualilka_hw_claimPins(L, &s->claim, {rx, tx});
    s->port = new (std::nothrow) HardwareSerial(1);
    if (!s->port) return luaL_error(L, "serial: out of memory");
    s->port->begin(baud, config, rx, tx);
    return 0;
}

static int lualilka_serial_end(lua_State* L) {
    LuaSerialPort* s = lualilka_serial_state(L);
    if (s->port) {
        s->port->end();
        delete s->port;
        s->port = nullptr;
    }
    lualilka_hw_release(&s->claim);
    return 0;
}

static int lualilka_serial_flush(lua_State* L) {
    lualilka_serial_port(L).flush();
    return 0;
}

static int lualilka_serial_peek(lua_State* L) {
    int value = lualilka_serial_port(L).peek();
    lua_pushinteger(L, value);
    return 1;
}

static int lualilka_serial_print(lua_State* L) {
    int n = lua_gettop(L);
    for (int i = 1; i <= n; i++) {
        if (lua_isstring(L, i)) {
            lualilka_serial_port(L).print(lua_tostring(L, i));
        } else if (lua_isinteger(L, i)) {
            lualilka_serial_port(L).print(lua_tointeger(L, i));
        } else if (lua_isnumber(L, i)) {
            lualilka_serial_port(L).print(lua_tonumber(L, i));
            // } else if (lua_islightuserdata(L, i)) {
            //     lualilka_serial_port(L).print(lua_topointer(L, i));
        } else {
            lualilka_serial_port(L).print(lua_typename(L, lua_type(L, i)));
        }
    }
    return 0;
}

static int lualilka_serial_println(lua_State* L) {
    int n = lua_gettop(L);
    for (int i = 1; i <= n; i++) {
        if (lua_isstring(L, i)) {
            lualilka_serial_port(L).println(lua_tostring(L, i));
        } else if (lua_isinteger(L, i)) {
            lualilka_serial_port(L).println(lua_tointeger(L, i));
        } else if (lua_isnumber(L, i)) {
            lualilka_serial_port(L).println(lua_tonumber(L, i));
            // } else if (lua_islightuserdata(L, i)) {
            //     lualilka_serial_port(L).println(lua_topointer(L, i));
        } else {
            lualilka_serial_port(L).println(lua_typename(L, lua_type(L, i)));
        }
    }
    return 0;
}

static int lualilka_serial_read(lua_State* L) {
    int bytes = luaL_optinteger(L, 1, 0);
    HardwareSerial& port = lualilka_serial_port(L);
    if (bytes <= 0) {
        int value = port.read();
        lua_pushinteger(L, value);
    } else {
        luaL_Buffer b;
        char* buffer = luaL_buffinitsize(L, &b, bytes);
        size_t got = port.read(reinterpret_cast<uint8_t*>(buffer), bytes);
        luaL_pushresultsize(&b, got); // binary-safe: may contain '\0'
    }
    return 1;
}

static int lualilka_serial_setTimeout(lua_State* L) {
    int timeout = luaL_checkinteger(L, 1);
    lualilka_serial_port(L).setTimeout(timeout);
    return 0;
}

static int lualilka_serial_write(lua_State* L) {
    int n = lua_gettop(L);
    for (int i = 1; i <= n; i++) {
        if (lua_type(L, i) == LUA_TSTRING) {
            size_t len;
            const char* str = lua_tolstring(L, i, &len);
            lualilka_serial_port(L).write(reinterpret_cast<const uint8_t*>(str), len);
        } else if (lua_isinteger(L, i)) {
            lualilka_serial_port(L).write((uint8_t)lua_tointeger(L, i));
            // } else if (lua_isnumber(L, i)) {
            //     lualilka_serial_port(L).write(lua_tonumber(L, i));
        } else {
            lualilka_serial_port(L).write(lua_typename(L, lua_type(L, i)));
        }
    }
    return 0;
}

static const struct luaL_Reg lualilka_serial[] = {
    {"available", lualilka_serial_available},
    {"begin", lualilka_serial_begin},
    {"end", lualilka_serial_end},
    //{"find", lualilka_serial_find},
    //{"findUntil", lualilka_serial_findUntil},
    {"flush", lualilka_serial_flush},
    //{"parseFloat", lualilka_serial_parseFloat},
    //{"parseInt", lualilka_serial_parseInt},
    {"peek", lualilka_serial_peek},
    {"print", lualilka_serial_print},
    {"println", lualilka_serial_println},
    {"read", lualilka_serial_read},
    //{"readBytes", lualilka_serial_readBytes},
    //{"readBytesUntil", lualilka_serial_readBytesUntil},
    {"setTimeout", lualilka_serial_setTimeout},
    {"write", lualilka_serial_write},
    {NULL, NULL},
};

int lualilka_serial_register(lua_State* L) {
    LuaSerialPort* s = new (lua_newuserdatauv(L, sizeof(LuaSerialPort), 0)) LuaSerialPort();
    lualilka_hw_initClaim(L, &s->claim);
    s->port = nullptr;
    luaL_newmetatable(L, SERIAL_PORT_MT);
    lua_pushcfunction(L, lualilka_serial_portGc);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, SERIAL_PORT_MT);

    luaL_newlib(L, lualilka_serial);
    // Add SerialConfig constants
    lua_pushinteger(L, SERIAL_5N1);
    lua_setfield(L, -2, "SERIAL_5N1");
    lua_pushinteger(L, SERIAL_6N1);
    lua_setfield(L, -2, "SERIAL_6N1");
    lua_pushinteger(L, SERIAL_7N1);
    lua_setfield(L, -2, "SERIAL_7N1");
    lua_pushinteger(L, SERIAL_8N1);
    lua_setfield(L, -2, "SERIAL_8N1");
    lua_pushinteger(L, SERIAL_5N2);
    lua_setfield(L, -2, "SERIAL_5N2");
    lua_pushinteger(L, SERIAL_6N2);
    lua_setfield(L, -2, "SERIAL_6N2");
    lua_pushinteger(L, SERIAL_7N2);
    lua_setfield(L, -2, "SERIAL_7N2");
    lua_pushinteger(L, SERIAL_8N2);
    lua_setfield(L, -2, "SERIAL_8N2");
    lua_pushinteger(L, SERIAL_5E1);
    lua_setfield(L, -2, "SERIAL_5E1");
    lua_pushinteger(L, SERIAL_6E1);
    lua_setfield(L, -2, "SERIAL_6E1");
    lua_pushinteger(L, SERIAL_7E1);
    lua_setfield(L, -2, "SERIAL_7E1");
    lua_pushinteger(L, SERIAL_8E1);
    lua_setfield(L, -2, "SERIAL_8E1");
    lua_pushinteger(L, SERIAL_5E2);
    lua_setfield(L, -2, "SERIAL_5E2");
    lua_pushinteger(L, SERIAL_6E2);
    lua_setfield(L, -2, "SERIAL_6E2");
    lua_pushinteger(L, SERIAL_7E2);
    lua_setfield(L, -2, "SERIAL_7E2");
    lua_pushinteger(L, SERIAL_8E2);
    lua_setfield(L, -2, "SERIAL_8E2");
    lua_pushinteger(L, SERIAL_5O1);
    lua_setfield(L, -2, "SERIAL_5O1");
    lua_pushinteger(L, SERIAL_6O1);
    lua_setfield(L, -2, "SERIAL_6O1");
    lua_pushinteger(L, SERIAL_7O1);
    lua_setfield(L, -2, "SERIAL_7O1");
    lua_pushinteger(L, SERIAL_8O1);
    lua_setfield(L, -2, "SERIAL_8O1");
    lua_pushinteger(L, SERIAL_5O2);
    lua_setfield(L, -2, "SERIAL_5O2");
    lua_pushinteger(L, SERIAL_6O2);
    lua_setfield(L, -2, "SERIAL_6O2");
    lua_pushinteger(L, SERIAL_7O2);
    lua_setfield(L, -2, "SERIAL_7O2");
    lua_pushinteger(L, SERIAL_8O2);
    lua_setfield(L, -2, "SERIAL_8O2");
    lua_setglobal(L, "serial");
    return 0;
}