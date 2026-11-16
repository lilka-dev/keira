#pragma once

#include <initializer_list>
#include <lua.hpp>
#include <lilka_modules.h>

// Shared helpers for Lua bindings that drive hardware on the extension connector.
//
// Every hardware object is a full userdata whose struct starts with LuaHwClaim. The claim records the
// pins and peripherals the object holds in lilka::mod::resources, so two scripts (an app and a service)
// can't drive the same pins, and everything is released when the object is closed or collected.

struct LuaHwClaim {
    static constexpr int MAX_PINS = 10;
    const void* owner = nullptr;
    int pins[MAX_PINS] = {};
    int pinCount = 0;
    int periph = -1; // lilka::mod::Periph or -1
};

// Identifies the script: the main thread of its Lua state (shared by all coroutines).
const void* lualilka_hw_owner(lua_State* L);

void lualilka_hw_initClaim(lua_State* L, LuaHwClaim* claim);
// Claim pins (negative values are skipped). Raises a Lua error if any pin is used by another script.
void lualilka_hw_claimPins(lua_State* L, LuaHwClaim* claim, std::initializer_list<int> pins);
// Claim a peripheral. Raises a Lua error if another script holds it.
void lualilka_hw_claimPeriph(lua_State* L, LuaHwClaim* claim, lilka::mod::Periph periph, const char* name);
// Release what the claim holds. Returns the number of references to its peripheral left for this owner.
uint16_t lualilka_hw_release(LuaHwClaim* claim);
// Release everything a script holds (called after its Lua state is closed).
void lualilka_hw_releaseOwner(const void* owner);

// Shared buses: begin on first use by a script, end when the last object using them is closed.
void lualilka_hw_beginSpi(lua_State* L, LuaHwClaim* claim, int sck, int miso, int mosi);
void lualilka_hw_endSpi(LuaHwClaim* claim);
void lualilka_hw_beginI2c(lua_State* L, LuaHwClaim* claim, int sda, int scl, uint32_t freq);
void lualilka_hw_endI2c(LuaHwClaim* claim);

// Calls `close` on the userdata on top of the stack and pops it: frees pins right away when `new` fails.
void lualilka_hw_closeTop(lua_State* L, lua_CFunction close);

// Creates a metatable `name` with methods in __index, plus __gc/__close = gc and __name.
void lualilka_hw_newClass(lua_State* L, const char* name, const luaL_Reg* methods, lua_CFunction gc);

// Options table helpers: `idx` may be nil/none, then defaults are returned.
lua_Integer lualilka_opt_int(lua_State* L, int idx, const char* key, lua_Integer def);
lua_Number lualilka_opt_num(lua_State* L, int idx, const char* key, lua_Number def);
bool lualilka_opt_bool(lua_State* L, int idx, const char* key, bool def);
// Returns def if missing; the returned pointer is valid while the options table is alive.
const char* lualilka_opt_str(lua_State* L, int idx, const char* key, const char* def);
// Pin option: integer GPIO or false (= not connected, returns -1).
int lualilka_opt_pin(lua_State* L, int idx, const char* key, int def);

// Reads a byte string from a Lua string, a table of integers or a single integer.
// Raises a Lua error if it is longer than `max`. Returns the number of bytes.
size_t lualilka_check_bytes(lua_State* L, int idx, uint8_t* buf, size_t max);
