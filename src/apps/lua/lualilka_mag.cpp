#include "lualilka_modules.h"
#include "lualilka_hw.h"

#include <math.h>
#include <new>
#include <string.h>

using lilka::mod::Tmag5273;
namespace pins = lilka::mod::pins;

#define MAG_MT "lilka.mag"

struct LuaMag {
    LuaHwClaim claim; // must be first
    Tmag5273* dev = nullptr;
    float offset[3] = {};
    float minV[3] = {};
    float maxV[3] = {};
    bool calibrating = false;
};

static LuaMag* checkMag(lua_State* L) {
    LuaMag* o = static_cast<LuaMag*>(luaL_checkudata(L, 1, MAG_MT));
    if (!o->dev) luaL_error(L, "mag: object is closed");
    return o;
}

static int lualilka_mag_close(lua_State* L) {
    LuaMag* o = static_cast<LuaMag*>(luaL_checkudata(L, 1, MAG_MT));
    delete o->dev;
    o->dev = nullptr;
    lualilka_hw_endI2c(&o->claim);
    return 0;
}

// mag.new([{sda = 44, scl = 43, address = 0x35, range = "low"|"high", averaging = 5}]) -> mag | nil, error
static int lualilka_mag_new(lua_State* L) {
    Tmag5273::Config cfg;
    int sda = lualilka_opt_pin(L, 1, "sda", pins::I2C_SDA);
    int scl = lualilka_opt_pin(L, 1, "scl", pins::I2C_SCL);
    if (sda < 0 || scl < 0) return luaL_error(L, "mag: sda and scl pins are required");
    cfg.address = (uint8_t)lualilka_opt_int(L, 1, "address", cfg.address);
    const char* range = lualilka_opt_str(L, 1, "range", "low");
    if (strcmp(range, "low") && strcmp(range, "high")) return luaL_error(L, "mag: range must be 'low' or 'high'");
    cfg.highRange = !strcmp(range, "high");
    cfg.averaging = (uint8_t)lualilka_opt_int(L, 1, "averaging", cfg.averaging);
    luaL_argcheck(L, cfg.averaging <= 5, 1, "averaging must be 0-5");

    LuaMag* o = new (lua_newuserdatauv(L, sizeof(LuaMag), 0)) LuaMag();
    lualilka_hw_initClaim(L, &o->claim);
    luaL_setmetatable(L, MAG_MT);
    lualilka_hw_claimPins(L, &o->claim, {sda, scl});
    lualilka_hw_beginI2c(L, &o->claim, sda, scl, 400000);

    o->dev = new (std::nothrow) Tmag5273();
    if (!o->dev) return luaL_error(L, "mag: out of memory");
    if (!o->dev->begin(cfg)) {
        lualilka_hw_closeTop(L, lualilka_mag_close);
        lua_pushnil(L);
        lua_pushstring(L, "mag: no TMAG5273 found");
        return 2;
    }
    return 1;
}

static bool readCorrected(LuaMag* o, float v[3]) {
    Tmag5273::Data d;
    if (!o->dev->read(d)) return false;
    const float raw[3] = {d.x, d.y, d.z};
    for (int i = 0; i < 3; i++) {
        if (o->calibrating) {
            if (raw[i] < o->minV[i]) o->minV[i] = raw[i];
            if (raw[i] > o->maxV[i]) o->maxV[i] = raw[i];
            o->offset[i] = (o->minV[i] + o->maxV[i]) / 2;
        }
        v[i] = raw[i] - o->offset[i];
    }
    return true;
}

// mag:read() -> {x, y, z} in uT (offset applied) | nil
static int lualilka_mag_read(lua_State* L) {
    float v[3];
    if (!readCorrected(checkMag(L), v)) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, 0, 3);
    lua_pushnumber(L, v[0]);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, v[1]);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, v[2]);
    lua_setfield(L, -2, "z");
    return 1;
}

// mag:heading() -> degrees 0-360 from the X/Y plane (hold the Lilka flat) | nil
static int lualilka_mag_heading(lua_State* L) {
    float v[3];
    if (!readCorrected(checkMag(L), v)) {
        lua_pushnil(L);
        return 1;
    }
    float h = atan2f(v[1], v[0]) * 180.0f / (float)M_PI;
    if (h < 0) h += 360.0f;
    lua_pushnumber(L, h);
    return 1;
}

// mag:temperature() -> deg C
static int lualilka_mag_temperature(lua_State* L) {
    lua_pushnumber(L, checkMag(L)->dev->temperature());
    return 1;
}

// mag:set_offset(x, y, z) - hard-iron offset in uT, subtracted from every reading
static int lualilka_mag_setOffset(lua_State* L) {
    LuaMag* o = checkMag(L);
    for (int i = 0; i < 3; i++)
        o->offset[i] = (float)luaL_checknumber(L, 2 + i);
    o->calibrating = false;
    return 0;
}

// mag:get_offset() -> x, y, z
static int lualilka_mag_getOffset(lua_State* L) {
    LuaMag* o = checkMag(L);
    for (int i = 0; i < 3; i++)
        lua_pushnumber(L, o->offset[i]);
    return 3;
}

// mag:start_calibration() - track min/max of every axis while the Lilka is rotated in all directions;
// the offset is updated on each read until stop_calibration()
static int lualilka_mag_startCalibration(lua_State* L) {
    LuaMag* o = checkMag(L);
    for (int i = 0; i < 3; i++) {
        o->minV[i] = INFINITY;
        o->maxV[i] = -INFINITY;
    }
    o->calibrating = true;
    return 0;
}

// mag:stop_calibration() -> x, y, z (the resulting offset)
static int lualilka_mag_stopCalibration(lua_State* L) {
    LuaMag* o = checkMag(L);
    o->calibrating = false;
    for (int i = 0; i < 3; i++)
        lua_pushnumber(L, o->offset[i]);
    return 3;
}

static const luaL_Reg magMethods[] = {
    {"read", lualilka_mag_read},
    {"heading", lualilka_mag_heading},
    {"temperature", lualilka_mag_temperature},
    {"set_offset", lualilka_mag_setOffset},
    {"get_offset", lualilka_mag_getOffset},
    {"start_calibration", lualilka_mag_startCalibration},
    {"stop_calibration", lualilka_mag_stopCalibration},
    {"close", lualilka_mag_close},
    {NULL, NULL},
};

static const luaL_Reg magLib[] = {
    {"new", lualilka_mag_new},
    {NULL, NULL},
};

int lualilka_mag_register(lua_State* L) {
    lualilka_hw_newClass(L, MAG_MT, magMethods, lualilka_mag_close);
    luaL_newlib(L, magLib);
    lua_setglobal(L, "mag");
    return 0;
}
