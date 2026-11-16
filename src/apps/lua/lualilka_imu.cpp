#include "lualilka_modules.h"
#include "lualilka_hw.h"

#include <stdio.h>
#include <stdlib.h>
#include <new>

using lilka::mod::Bmi260;
namespace pins = lilka::mod::pins;

#define IMU_MT "lilka.imu"

struct LuaImu {
    LuaHwClaim claim; // must be first
    Bmi260* dev = nullptr;
};

static LuaImu* checkImu(lua_State* L) {
    LuaImu* o = static_cast<LuaImu*>(luaL_checkudata(L, 1, IMU_MT));
    if (!o->dev) luaL_error(L, "imu: object is closed");
    return o;
}

static int lualilka_imu_close(lua_State* L) {
    LuaImu* o = static_cast<LuaImu*>(luaL_checkudata(L, 1, IMU_MT));
    delete o->dev;
    o->dev = nullptr;
    lualilka_hw_endI2c(&o->claim);
    return 0;
}

// Reads the whole config file into a malloc'ed buffer. Relative paths are resolved against the script dir.
static uint8_t* loadBlob(lua_State* L, const char* path, size_t* length) {
    String full = path;
    if (path[0] != '/') {
        lua_getfield(L, LUA_REGISTRYINDEX, "dir");
        const char* dir = lua_tostring(L, -1);
        if (dir) full = String(dir) + "/" + path;
        lua_pop(L, 1);
    }
    FILE* f = fopen(full.c_str(), "rb");
    if (!f) return nullptr;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t* buf = nullptr;
    if (size > 0 && size <= 16384) {
        buf = static_cast<uint8_t*>(malloc(size));
        if (buf && fread(buf, 1, size, f) != (size_t)size) {
            free(buf);
            buf = nullptr;
        }
    }
    fclose(f);
    *length = buf ? (size_t)size : 0;
    return buf;
}

// imu.new([{sda = 44, scl = 43, address = nil, config = true | false | "path/to/blob.bin",
//           accel_range = 8, gyro_range = 2000, accel_odr = 100, gyro_odr = 200}]) -> imu | nil, error
static int lualilka_imu_new(lua_State* L) {
    Bmi260::Config cfg;
    int sda = lualilka_opt_pin(L, 1, "sda", pins::get("gyro_magnet", "sda"));
    int scl = lualilka_opt_pin(L, 1, "scl", pins::get("gyro_magnet", "scl"));
    if (sda < 0 || scl < 0) return luaL_error(L, "imu: sda and scl pins are required");
    cfg.address = (uint8_t)lualilka_opt_int(L, 1, "address", 0);
    cfg.accelRangeG = (uint8_t)lualilka_opt_int(L, 1, "accel_range", cfg.accelRangeG);
    cfg.gyroRangeDps = (uint16_t)lualilka_opt_int(L, 1, "gyro_range", cfg.gyroRangeDps);
    cfg.accelOdrHz = (uint16_t)lualilka_opt_int(L, 1, "accel_odr", cfg.accelOdrHz);
    cfg.gyroOdrHz = (uint16_t)lualilka_opt_int(L, 1, "gyro_odr", cfg.gyroOdrHz);

    // config: true (default) = built-in blob, false = none, string = file
    uint8_t* fileBlob = nullptr;
    bool useConfig = true;
    const char* configPath = nullptr;
    if (!lua_isnoneornil(L, 1)) {
        lua_getfield(L, 1, "config");
        if (lua_type(L, -1) == LUA_TSTRING) {
            configPath = lua_tostring(L, -1);
        } else if (!lua_isnil(L, -1)) {
            useConfig = lua_toboolean(L, -1);
        }
        lua_pop(L, 1); // string stays referenced by the options table
    }

    LuaImu* o = new (lua_newuserdatauv(L, sizeof(LuaImu), 0)) LuaImu();
    o->dev = nullptr;
    lualilka_hw_initClaim(L, &o->claim);
    luaL_setmetatable(L, IMU_MT);
    lualilka_hw_claimPins(L, &o->claim, {sda, scl});
    lualilka_hw_beginI2c(L, &o->claim, sda, scl, 400000);

    if (configPath) {
        fileBlob = loadBlob(L, configPath, &cfg.blobLength);
        if (!fileBlob) {
            lualilka_hw_closeTop(L, lualilka_imu_close);
            lua_pushnil(L);
            lua_pushfstring(L, "imu: can't read config file '%s'", configPath);
            return 2;
        }
        cfg.blob = fileBlob;
    } else if (useConfig) {
        cfg.blob = lilka::mod::bmi260DefaultConfig(&cfg.blobLength);
    }

    o->dev = new (std::nothrow) Bmi260();
    if (!o->dev) {
        free(fileBlob);
        return luaL_error(L, "imu: out of memory");
    }
    Bmi260::Result r = o->dev->begin(cfg);
    free(fileBlob);
    switch (r) {
        case Bmi260::Result::OK:
            return 1;
        case Bmi260::Result::NoChip:
            lualilka_hw_closeTop(L, lualilka_imu_close);
            lua_pushnil(L);
            lua_pushstring(L, "imu: no BMI260 found");
            return 2;
        case Bmi260::Result::ConfigFailed: {
            int status = o->dev->internalStatus();
            lualilka_hw_closeTop(L, lualilka_imu_close);
            lua_pushnil(L);
            lua_pushfstring(L, "imu: config upload failed (INTERNAL_STATUS=%d)", status);
            return 2;
        }
        default:
            lualilka_hw_closeTop(L, lualilka_imu_close);
            return luaL_error(L, "imu: invalid range or data rate");
    }
    // cppcheck-suppress memleak ; dev is owned by the userdata and freed in close/__gc
}

// imu:read() -> {ax, ay, az (g), gx, gy, gz (deg/s)} | nil
static int lualilka_imu_read(lua_State* L) {
    Bmi260::Data d;
    if (!checkImu(L)->dev->read(d)) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, 0, 6);
    const struct {
        const char* key;
        float value;
    } fields[] = {{"ax", d.ax}, {"ay", d.ay}, {"az", d.az}, {"gx", d.gx}, {"gy", d.gy}, {"gz", d.gz}};
    for (auto& f : fields) {
        lua_pushnumber(L, f.value);
        lua_setfield(L, -2, f.key);
    }
    return 1;
}

// imu:temperature() -> deg C
static int lualilka_imu_temperature(lua_State* L) {
    lua_pushnumber(L, checkImu(L)->dev->temperature());
    return 1;
}

// imu:initialized() -> bool (config blob uploaded and accepted)
static int lualilka_imu_initialized(lua_State* L) {
    lua_pushboolean(L, checkImu(L)->dev->isInitialized());
    return 1;
}

static const luaL_Reg imuMethods[] = {
    {"read", lualilka_imu_read},
    {"temperature", lualilka_imu_temperature},
    {"initialized", lualilka_imu_initialized},
    {"close", lualilka_imu_close},
    {NULL, NULL},
};

static const luaL_Reg imuLib[] = {
    {"new", lualilka_imu_new},
    {NULL, NULL},
};

int lualilka_imu_register(lua_State* L) {
    lualilka_hw_newClass(L, IMU_MT, imuMethods, lualilka_imu_close);
    luaL_newlib(L, imuLib);
    lua_setglobal(L, "imu");
    return 0;
}
