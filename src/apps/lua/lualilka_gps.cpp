#include "lualilka_modules.h"
#include "lualilka_hw.h"

#include <new>

using lilka::mod::Gps;
using lilka::mod::GpsFix;
using lilka::mod::GpsSatellite;

#define GPS_MT "lilka.gps"

struct LuaGps {
    LuaHwClaim claim; // must be first
    Gps* dev = nullptr;
};

static LuaGps* checkGps(lua_State* L) {
    LuaGps* g = static_cast<LuaGps*>(luaL_checkudata(L, 1, GPS_MT));
    if (!g->dev) luaL_error(L, "gps: object is closed");
    return g;
}

static int lualilka_gps_close(lua_State* L) {
    LuaGps* g = static_cast<LuaGps*>(luaL_checkudata(L, 1, GPS_MT));
    delete g->dev; // ends the UART
    g->dev = nullptr;
    lualilka_hw_release(&g->claim);
    return 0;
}

// gps.new([{baud = 9600, rx = 44, tx = 43}]) -> gps | nil, error
static int lualilka_gps_new(lua_State* L) {
    Gps::Config cfg;
    cfg.baud = (uint32_t)lualilka_opt_int(L, 1, "baud", cfg.baud);
    cfg.rx = lualilka_opt_pin(L, 1, "rx", cfg.rx);
    cfg.tx = lualilka_opt_pin(L, 1, "tx", cfg.tx);
    if (cfg.rx < 0) return luaL_error(L, "gps: rx pin is required");

    LuaGps* g = new (lua_newuserdatauv(L, sizeof(LuaGps), 0)) LuaGps();
    g->dev = nullptr;
    lualilka_hw_initClaim(L, &g->claim);
    luaL_setmetatable(L, GPS_MT);
    lualilka_hw_claimPeriph(L, &g->claim, lilka::mod::Periph::UART2, "UART2");
    lualilka_hw_claimPins(L, &g->claim, {cfg.rx, cfg.tx});

    g->dev = new (std::nothrow) Gps();
    if (!g->dev) return luaL_error(L, "gps: out of memory");
    esp_err_t err = g->dev->begin(cfg);
    if (err != ESP_OK) {
        lualilka_hw_closeTop(L, lualilka_gps_close);
        lua_pushnil(L);
        lua_pushfstring(L, "gps: UART init failed (%s)", esp_err_to_name(err));
        // cppcheck-suppress memleak ; dev is owned by the userdata and freed in close/__gc
        return 2;
    }
    return 1;
}

// gps:update() -> number of new NMEA sentences
static int lualilka_gps_update(lua_State* L) {
    lua_pushinteger(L, checkGps(L)->dev->update());
    return 1;
}

static void setNum(lua_State* L, const char* key, lua_Number v) {
    lua_pushnumber(L, v);
    lua_setfield(L, -2, key);
}

static void setInt(lua_State* L, const char* key, lua_Integer v) {
    lua_pushinteger(L, v);
    lua_setfield(L, -2, key);
}

// gps:fix() -> table
static int lualilka_gps_fix(lua_State* L) {
    const GpsFix& f = checkGps(L)->dev->fix();
    lua_createtable(L, 0, 20);
    lua_pushboolean(L, f.valid);
    lua_setfield(L, -2, "valid");
    setNum(L, "latitude", f.latitude);
    setNum(L, "longitude", f.longitude);
    setNum(L, "altitude", f.altitude);
    setNum(L, "speed", f.speedKmh);
    setNum(L, "course", f.course);
    setInt(L, "satellites", f.satellites);
    setInt(L, "quality", f.quality);
    setInt(L, "fix_type", f.fixType);
    setNum(L, "hdop", f.hdop);
    setNum(L, "vdop", f.vdop);
    setNum(L, "pdop", f.pdop);
    setInt(L, "year", f.year);
    setInt(L, "month", f.month);
    setInt(L, "day", f.day);
    setInt(L, "hour", f.hour);
    setInt(L, "minute", f.minute);
    setInt(L, "second", f.second);
    setInt(L, "millis", f.millis);
    setInt(L, "updates", f.updates);
    return 1;
}

// gps:satellites() -> {{system, prn, elevation, azimuth, snr}, ...}
static int lualilka_gps_satellites(lua_State* L) {
    const lilka::mod::Nmea& nmea = checkGps(L)->dev->nmea();
    size_t n = nmea.satelliteCount();
    lua_createtable(L, (int)n, 0);
    for (size_t i = 0; i < n; i++) {
        const GpsSatellite& s = nmea.satellite(i);
        lua_createtable(L, 0, 5);
        const char* system = "other";
        switch (s.system) {
            case 'P':
                system = "gps";
                break;
            case 'L':
                system = "glonass";
                break;
            case 'A':
                system = "galileo";
                break;
            case 'B':
                system = "beidou";
                break;
            default:
                break;
        }
        lua_pushstring(L, system);
        lua_setfield(L, -2, "system");
        setInt(L, "prn", s.prn);
        setInt(L, "elevation", s.elevation);
        setInt(L, "azimuth", s.azimuth);
        if (s.snr >= 0) setInt(L, "snr", s.snr);
        lua_rawseti(L, -2, (lua_Integer)i + 1);
    }
    return 1;
}

// gps:last_sentence() -> string (without "$" and line end)
static int lualilka_gps_lastSentence(lua_State* L) {
    lua_pushstring(L, checkGps(L)->dev->nmea().lastSentence());
    return 1;
}

// gps:write(data) -> bool
static int lualilka_gps_write(lua_State* L) {
    LuaGps* g = checkGps(L);
    size_t len;
    const char* data = luaL_checklstring(L, 2, &len);
    lua_pushboolean(L, g->dev->write(reinterpret_cast<const uint8_t*>(data), len) == ESP_OK);
    return 1;
}

// gps:send_ubx(class, id[, payload]) -> bool
static int lualilka_gps_sendUbx(lua_State* L) {
    LuaGps* g = checkGps(L);
    uint8_t cls = (uint8_t)luaL_checkinteger(L, 2);
    uint8_t id = (uint8_t)luaL_checkinteger(L, 3);
    size_t len = 0;
    const char* payload = luaL_optlstring(L, 4, "", &len);
    if (len > 0xFFFF) return luaL_error(L, "gps: UBX payload too long");
    bool ok = g->dev->sendUbx(cls, id, reinterpret_cast<const uint8_t*>(payload), (uint16_t)len) == ESP_OK;
    lua_pushboolean(L, ok);
    return 1;
}

static const luaL_Reg gpsMethods[] = {
    {"update", lualilka_gps_update},
    {"fix", lualilka_gps_fix},
    {"satellites", lualilka_gps_satellites},
    {"last_sentence", lualilka_gps_lastSentence},
    {"write", lualilka_gps_write},
    {"send_ubx", lualilka_gps_sendUbx},
    {"close", lualilka_gps_close},
    {NULL, NULL},
};

static const luaL_Reg gpsLib[] = {
    {"new", lualilka_gps_new},
    {NULL, NULL},
};

int lualilka_gps_register(lua_State* L) {
    lualilka_hw_newClass(L, GPS_MT, gpsMethods, lualilka_gps_close);
    luaL_newlib(L, gpsLib);
    lua_setglobal(L, "gps");
    return 0;
}
