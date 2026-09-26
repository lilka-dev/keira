#include "luawallpaper.h"

#include <stdio.h>
#include <lilka.h>
#include "luarunner.h"
#include "lualilka_display.h"
#include "lualilka_resources.h"
#include "lualilka_math.h"
#include "lualilka_geometry.h"
#include "keira/ksound/sound.h"

LuaWallpaper::~LuaWallpaper() {
    close();
}

bool LuaWallpaper::open(const char* path, App* app) {
    close();

    FILE* file = fopen(path, "r");
    if (file == nullptr) return false;
    fclose(file);

    L = lua_newstate(lua_smart_alloc, NULL);
    if (L == nullptr) {
        lilka::serial.err("Lua wallpaper: failed to create Lua state");
        return false;
    }
    luaL_openlibs(L);

    // Relative paths and require() are resolved against the script directory
    String dir = String(path).substring(0, String(path).lastIndexOf('/'));
    lua_pushstring(L, dir.c_str());
    lua_setfield(L, LUA_REGISTRYINDEX, "dir");
    lua_getglobal(L, "package");
    lua_pushstring(L, (dir + "/?.lua").c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);

    // Display functions draw on the canvas of this app
    lua_pushlightuserdata(L, app);
    lua_setfield(L, LUA_REGISTRYINDEX, "app");

    // Controller, util (util.exit) and buzzer are left out on purpose: buttons belong to the launcher,
    // and these modules escape through longjmp, which is only safe inside Lua runner apps
    lualilka_display_register(L);
    lualilka_resources_register(L);
    lualilka_math_register(L);
    lualilka_geometry_register(L);

    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "images");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "sounds");

    lua_newtable(L);
    lua_setglobal(L, "lilka");

    if (luaL_loadfile(L, path) != LUA_OK || lua_pcall(L, 0, 0, 0) != LUA_OK) {
        lilka::serial.err("Lua wallpaper: %s", lua_tostring(L, -1));
        close();
        return false;
    }

    if (!pushCallback("draw")) {
        lilka::serial.err("Lua wallpaper: %s has no lilka.draw() function", path);
        close();
        return false;
    }
    lua_pop(L, 1);

    if (pushCallback("init") && !call("init", 0)) return false;
    return true;
}

void LuaWallpaper::close() {
    if (L == nullptr) return;

    // Free images and sounds loaded by the script
    lua_getfield(L, LUA_REGISTRYINDEX, "images");
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        delete static_cast<lilka::Image*>(lua_touserdata(L, -1));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, "sounds");
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        delete static_cast<lilka::Sound*>(lua_touserdata(L, -1));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    lua_close(L);
    L = nullptr;
}

bool LuaWallpaper::isOpen() {
    return L != nullptr;
}

bool LuaWallpaper::frame(uint32_t deltaMs) {
    if (L == nullptr) return false;

    if (pushCallback("update")) {
        lua_pushnumber(L, static_cast<float>(deltaMs) / 1000.0);
        if (!call("update", 1)) return false;
    }
    if (!pushCallback("draw") || !call("draw", 0)) {
        close();
        return false;
    }

    lua_gc(L, LUA_GCSTEP, 0);
    return true;
}

bool LuaWallpaper::pushCallback(const char* name) {
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

bool LuaWallpaper::call(const char* name, int nargs) {
    if (lua_pcall(L, nargs, 0, 0) != LUA_OK) {
        lilka::serial.err("Lua wallpaper: lilka.%s(): %s", name, lua_tostring(L, -1));
        close();
        return false;
    }
    return true;
}
