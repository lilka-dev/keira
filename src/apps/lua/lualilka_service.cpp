#include "lualilka_service.h"

#include "services/script/script.h"

// Resolves path relative to the script directory
static String lualilka_service_resolvePath(lua_State* L, const char* path) {
    if (path[0] == '/') return path;
    lua_getfield(L, LUA_REGISTRYINDEX, "dir");
    String dir = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
    lua_pop(L, 1);
    return dir + "/" + path;
}

int lualilka_service_start(lua_State* L) {
    String path = lualilka_service_resolvePath(L, luaL_checkstring(L, 1));
    String err;
    String name = ScriptService::spawn(path, err);
    if (name.isEmpty()) {
        lua_pushnil(L);
        lua_pushstring(L, err.c_str());
        return 2;
    }
    lua_pushstring(L, name.c_str());
    return 1;
}

int lualilka_service_stop(lua_State* L) {
    lua_pushboolean(L, ScriptService::stop(luaL_checkstring(L, 1)));
    return 1;
}

int lualilka_service_list(lua_State* L) {
    std::vector<String> names = ScriptService::list();
    lua_createtable(L, names.size(), 0);
    for (size_t i = 0; i < names.size(); i++) {
        lua_pushstring(L, names[i].c_str());
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

int lualilka_service_isRunning(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    bool running = false;
    for (const String& service : ScriptService::list()) {
        if (service == name) running = true;
    }
    lua_pushboolean(L, running);
    return 1;
}

static const luaL_Reg lualilka_service[] = {
    {"start", lualilka_service_start},
    {"stop", lualilka_service_stop},
    {"list", lualilka_service_list},
    {"is_running", lualilka_service_isRunning},
    {NULL, NULL},
};

int lualilka_service_register(lua_State* L) {
    // Create global "service" table that contains all service functions
    luaL_newlib(L, lualilka_service);

    lua_getfield(L, LUA_REGISTRYINDEX, "service");
    ScriptService* service = static_cast<ScriptService*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (service != nullptr) {
        lua_pushstring(L, service->getName());
        lua_setfield(L, -2, "name");
        lua_pushnumber(L, LUALILKA_SERVICE_DEFAULT_INTERVAL);
        lua_setfield(L, -2, "interval");
    }

    lua_setglobal(L, "service");
    return 0;
}
