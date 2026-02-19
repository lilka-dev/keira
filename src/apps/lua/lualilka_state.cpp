#include "lualilka_state.h"

#define STATE_METATABLE "state_mt"

// Forward declarations
static int lualilka_state_save_lua(lua_State* L);
static int lualilka_state_reset_lua(lua_State* L);
static int lualilka_state_clear_lua(lua_State* L);

// __index metamethod: provides save() and reset() methods
static int lualilka_state_meta_index(lua_State* L) {
    const char* key = lua_tostring(L, 2);
    if (key) {
        if (strcmp(key, "save") == 0) {
            lua_pushcfunction(L, lualilka_state_save_lua);
            return 1;
        } else if (strcmp(key, "reset") == 0) {
            lua_pushcfunction(L, lualilka_state_reset_lua);
            return 1;
        } else if (strcmp(key, "clear") == 0) {
            lua_pushcfunction(L, lualilka_state_clear_lua);
            return 1;
        } else if (strcmp(key, "path") == 0) {
            lua_getfield(L, LUA_REGISTRYINDEX, "state_path");
            return 1;
        }
    }
    return 0;
}

// __newindex metamethod: protects save/reset from being overwritten
static int lualilka_state_meta_newindex(lua_State* L) {
    // Args: t (1), k (2), v (3)
    const char* key = lua_tostring(L, 2);
    if (key && (strcmp(key, "save") == 0 || strcmp(key, "reset") == 0 || strcmp(key, "clear") == 0 ||
                strcmp(key, "path") == 0)) {
        return luaL_error(L, "Неможливо перезаписати state.%s", key);
    }
    lua_rawset(L, 1);
    return 0;
}

// _G.__newindex hook: intercepts `state = <table>` and sets metatable
static int lualilka_state_global_newindex(lua_State* L) {
    // Args: t (1), k (2), v (3)
    const char* key = lua_tostring(L, 2);
    if (key && strcmp(key, "state") == 0 && lua_istable(L, 3)) {
        luaL_getmetatable(L, STATE_METATABLE);
        lua_setmetatable(L, 3);
    }
    lua_rawset(L, 1);
    return 0;
}

int lualilka_state_register(lua_State* L) {
    // Create metatable for state tables
    luaL_newmetatable(L, STATE_METATABLE);
    lua_pushcfunction(L, lualilka_state_meta_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lualilka_state_meta_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);

    // Set _G metatable with __newindex to intercept state assignments
    lua_pushglobaltable(L);
    if (!lua_getmetatable(L, -1)) {
        lua_newtable(L);
    }
    lua_pushcfunction(L, lualilka_state_global_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);
    lua_pop(L, 1);

    return 0;
}

int lualilka_state_load(lua_State* L, const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        return 1;
    }

    int count = 0;

    // Create state table
    lua_newtable(L);
    // Read state table
    char key[256];
    char type[32];
    while (fscanf(file, "%255s", key) != EOF) {
        // Read value type
        fscanf(file, "%31s", type);
        if (strcmp(type, "number") == 0) {
            // Read number
            double value;
            fscanf(file, "%lf", &value);
            lilka::serial.log("lua: state: load number %s = %lf", key, value);
            lua_pushnumber(L, value);
            lua_setfield(L, -2, key);
            count++;
        } else if (strcmp(type, "string") == 0) {
            // Read string
            char value[256];
            fgetc(file);
            fgets(value, 256, file);
            value[strcspn(value, "\n")] = '\0';
            lilka::serial.log("lua: state: load string %s = %s", key, value);
            lua_pushstring(L, value);
            lua_setfield(L, -2, key);
            count++;
        } else if (strcmp(type, "boolean") == 0) {
            // Read boolean
            int value;
            fscanf(file, "%d", &value);
            lilka::serial.log("lua: state: load boolean %s = %d", key, value);
            lua_pushboolean(L, value);
            lua_setfield(L, -2, key);
            count++;
        } else if (strcmp(type, "nil") == 0) {
            // Read nil
            lilka::serial.log("lua: state: load nil %s", key);
            lua_pushnil(L);
            lua_setfield(L, -2, key);
            count++;
        } else {
            // Skip unsupported types
        }
    }

    lilka::serial.log("lua: state: loaded %d values", count);

    // Set metatable for save/reset methods
    luaL_getmetatable(L, STATE_METATABLE);
    if (!lua_isnil(L, -1)) {
        lua_setmetatable(L, -2);
    } else {
        lua_pop(L, 1);
    }

    // Set state table to global
    lua_setglobal(L, "state");

    fclose(file);
    return 0;
}

// Lua-callable save()
static int lualilka_state_save_lua(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "state_path");
    if (!lua_isstring(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "Збереження стану доступне лише при запуску з файлу");
    }
    const char* path = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "state");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "Таблиця state не визначена");
    }
    lua_pop(L, 1);

    int result = lualilka_state_save(L, path);
    if (result != 0) {
        return luaL_error(L, "Не вдалося зберегти стан у файл %s", path);
    }
    return 0;
}

// Lua-callable reset()
static int lualilka_state_reset_lua(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "state_path");
    if (!lua_isstring(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "Скидання стану доступне лише при запуску з файлу");
    }
    const char* path = lua_tostring(L, -1);
    lua_pop(L, 1);

    int result = lualilka_state_load(L, path);
    if (result != 0) {
        lua_newtable(L);
        luaL_getmetatable(L, STATE_METATABLE);
        lua_setmetatable(L, -2);
        lua_setglobal(L, "state");
    }
    return 0;
}

// Lua-callable clear()
static int lualilka_state_clear_lua(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "state_path");
    if (!lua_isstring(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "Очищення стану доступне лише при запуску з файлу");
    }
    const char* path = lua_tostring(L, -1);
    lua_pop(L, 1);

    // Delete state file
    remove(path);

    // Set state to nil
    lua_pushnil(L);
    lua_setglobal(L, "state");

    return 0;
}

int lualilka_state_save(lua_State* L, const char* path) {
    FILE* file = fopen(path, "w");
    if (!file) {
        return 1;
    }

    int count = 0;

    // Get state global
    lua_getglobal(L, "state");
    // Iterate over state table
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        // Get key
        const char* key = lua_tostring(L, -2);
        fprintf(file, "%s\n", key);
        // Get value type
        int type = lua_type(L, -1);
        if (type == LUA_TNUMBER) {
            // Write number
            double value = lua_tonumber(L, -1);
            lilka::serial.log("lua: state: save number %s = %lf", key, value);
            fprintf(file, "number\n%lf\n", value);
            count++;
        } else if (type == LUA_TSTRING) {
            // Write string
            const char* value = lua_tostring(L, -1);
            String valueStr = value;
            valueStr.replace("\n", "\\n");
            valueStr.replace("\r", "\\r");
            lilka::serial.log("lua: state: save string %s = %s", key, valueStr.c_str());
            fprintf(file, "string\n%s\n", value);
            count++;
        } else if (type == LUA_TBOOLEAN) {
            // Write boolean
            int value = lua_toboolean(L, -1);
            lilka::serial.log("lua: state: save boolean %s = %d", key, value);
            fprintf(file, "boolean\n%d\n", value);
            count++;
        } else if (type == LUA_TNIL) {
            // Write nil
            lilka::serial.log("lua: state: save nil %s", key);
            fprintf(file, "nil\n");
            count++;
        } else {
            // Skip unsupported types
            lilka::serial.log("lua: state: skip %s (cannot serialize)", key);
        }
        // Remove value from stack
        lua_pop(L, 1);
    }

    lilka::serial.log("lua: state: saved %d values", count);

    fclose(file);
    return 0;
}
