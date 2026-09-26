#include "luaservice.h"

#include <unistd.h>
#include <lilka.h>
#include "luarunner.h"
#include "lualilka_console.h"
#include "lualilka_math.h"
#include "lualilka_geometry.h"
#include "lualilka_gpio.h"
#include "lualilka_i2c.h"
#include "lualilka_spi.h"
#include "lualilka_pwm.h"
#include "lualilka_ws2812.h"
#include "lualilka_util.h"
#include "lualilka_buzzer.h"
#include "lualilka_sdcard.h"
#include "lualilka_fs.h"
#include "lualilka_wifi.h"
#include "lualilka_serial.h"
#include "lualilka_http.h"
#include "lualilka_crypto.h"
#include "lualilka_socket.h"
#include "lualilka_mqtt.h"
#include "lualilka_httpserver.h"
#include "lualilka_state.h"
#include "lualilka_service.h"
#include "keira/utils/defer.h"

// Number of VM instructions between checks for a stop request
#define LUA_SERVICE_STOP_CHECK_COUNT 1000

static ScriptService* luaservice_get(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "service");
    ScriptService* service = static_cast<ScriptService*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return service;
}

// Interrupts script once stop is requested, even if it never sleeps
static void luaservice_stopHook(lua_State* L, lua_Debug* ar) {
    (void)ar;
    if (luaservice_get(L)->isStopRequested()) luaL_error(L, "service stopped");
}

// util.sleep() replacement that wakes up when stop is requested
static int luaservice_sleep(lua_State* L) {
    float s = luaL_checknumber(L, 1);
    if (!luaservice_get(L)->sleep(s * 1000)) return luaL_error(L, "service stopped");
    return 0;
}

// util.exit() replacement. Original one longjmps to the Lua runner app, which doesn't exist here
static int luaservice_exit(lua_State* L) {
    luaservice_get(L)->requestStop();
    return luaL_error(L, "service stopped");
}

// Pushes lilka[name] onto the stack if it's a function
static bool luaservice_pushCallback(lua_State* L, const char* name) {
    lua_getglobal(L, "lilka");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    lua_getfield(L, -1, name);
    lua_remove(L, -2);
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    return true;
}

// Returns service.interval in milliseconds
static uint32_t luaservice_getInterval(lua_State* L) {
    double interval = LUALILKA_SERVICE_DEFAULT_INTERVAL;
    lua_getglobal(L, "service");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "interval");
        if (lua_isnumber(L, -1)) interval = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return interval > 0 ? interval * 1000 : 0;
}

LuaService::LuaService(const String& path) : ScriptService(K_SCRIPT_SERVICE_LUA_PREFIX, path) {
    setktStackSize(8192);
}

void LuaService::run() {
#ifndef LILKA_NO_LUA
    const String& path = getPath();
    String dir = getDir();

    lua_State* L = lua_newstate(lua_smart_alloc, NULL);
    if (L == nullptr) {
        lilka::serial.err("Lua service %s: failed to create Lua state", getName());
        return;
    }
    Defer closeState([L]() { lua_close(L); });
    luaL_openlibs(L);

    // Relative paths and require() are resolved against the script directory
    lua_pushstring(L, dir.c_str());
    lua_setfield(L, LUA_REGISTRYINDEX, "dir");
    lua_getglobal(L, "package");
    lua_pushstring(L, (dir + "/?.lua").c_str());
    lua_setfield(L, -2, "path");
    lua_pushstring(L, (dir + "/?.so").c_str());
    lua_setfield(L, -2, "cpath");
    lua_pop(L, 1);

    lua_pushlightuserdata(L, static_cast<ScriptService*>(this));
    lua_setfield(L, LUA_REGISTRYINDEX, "service");

    // Display, controller, resources, UI and audio are left out on purpose: services have no canvas,
    // buttons belong to the foreground app, and audio playback is shared with it
    lualilka_console_register(L);
    lualilka_math_register(L);
    lualilka_geometry_register(L);
    lualilka_gpio_register(L);
    lualilka_i2c_register(L);
    lualilka_spi_register(L);
    lualilka_pwm_register(L);
    lualilka_ws2812_register(L);
    lualilka_util_register(L);
    lualilka_buzzer_register(L);
    lualilka_sdcard_register(L);
    lualilka_fs_register(L);
    lualilka_wifi_register(L);
    lualilka_serial_register(L);
    lualilka_http_register(L);
    lualilka_crypto_register(L);
    lualilka_socket_register(L);
    lualilka_mqtt_register(L);
    lualilka_httpserver_register(L);
    lualilka_state_register(L);
    lualilka_service_register(L);

    lua_getglobal(L, "util");
    lua_pushcfunction(L, luaservice_sleep);
    lua_setfield(L, -2, "sleep");
    lua_pushcfunction(L, luaservice_exit);
    lua_setfield(L, -2, "exit");
    lua_pop(L, 1);

    lua_newtable(L);
    lua_setglobal(L, "lilka");

    // State is kept next to the script, same as for Lua apps
    String statePath = path.substring(0, path.lastIndexOf('.')) + ".state";
    lua_pushstring(L, statePath.c_str());
    lua_setfield(L, LUA_REGISTRYINDEX, "state_path");
    if (access(statePath.c_str(), F_OK) != -1) {
        lualilka_state_load(L, statePath.c_str());
    }

    lua_sethook(L, luaservice_stopHook, LUA_MASKCOUNT, LUA_SERVICE_STOP_CHECK_COUNT);

    lilka::serial.log("Lua service %s: running %s", getName(), path.c_str());
    if (execute(L) != LUA_OK && !isStopRequested()) {
        lilka::serial.err("Lua service %s: %s", getName(), lua_tostring(L, -1));
    } else {
        lilka::serial.log("Lua service %s: stopped", getName());
    }

    // Saving state must not be interrupted by the stop hook
    lua_sethook(L, nullptr, 0, 0);
    lua_getglobal(L, "state");
    bool hasState = lua_istable(L, -1);
    lua_pop(L, 1);
    if (hasState) {
        lualilka_state_save(L, statePath.c_str());
    }
#endif
}

int LuaService::execute(lua_State* L) {
    int ret = luaL_loadfile(L, getPath().c_str());
    if (ret != LUA_OK) return ret;
    ret = lua_pcall(L, 0, 0, 0);
    if (ret != LUA_OK) return ret;

    if (luaservice_pushCallback(L, "init")) {
        ret = lua_pcall(L, 0, 0, 0);
        if (ret != LUA_OK) return ret;
    }

    TickType_t lastUpdate = xTaskGetTickCount();
    while (luaservice_pushCallback(L, "update")) {
        TickType_t now = xTaskGetTickCount();
        lua_pushnumber(L, pdTICKS_TO_MS(now - lastUpdate) / 1000.0);
        lastUpdate = now;
        ret = lua_pcall(L, 1, 0, 0);
        if (ret != LUA_OK) return ret;

        lua_gc(L, LUA_GCSTEP, 0);
        if (!sleep(luaservice_getInterval(L))) break;
    }
    return LUA_OK;
}
