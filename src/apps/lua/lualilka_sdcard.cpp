#include "lualilka_sdcard.h"
#include "lilka.h"
#include "keira/keira.h"

static int lualilka_create_object_file(lua_State* L) {
    String path = luaL_checkstring(L, 1);
    String mode = luaL_optstring(L, 2, "r");
    *reinterpret_cast<FILE**>(lua_newuserdata(L, sizeof(FILE*))) =
        fopen((lilka::fileutils.getSDRoot() + path).c_str(), mode.c_str());
    luaL_setmetatable(L, FILE_OBJECT);
    return 1;
}

static int lualilka_delete_object_file(lua_State* L) {
    FILE* filePointer = *reinterpret_cast<FILE**>(luaL_checkudata(L, 1, FILE_OBJECT));
    if (filePointer) {
        fclose(filePointer);
    }
    return 0;
}

static int lualilka_file_size(lua_State* L) {
    FILE* filePointer = *reinterpret_cast<FILE**>(luaL_checkudata(L, 1, FILE_OBJECT));
    if (filePointer) {
        size_t initial_offset = ftell(filePointer); // get current file pointer
        fseek(filePointer, 0, SEEK_END); // seek to end of file
        size_t size = ftell(filePointer); // get current file pointer
        fseek(filePointer, initial_offset, SEEK_SET); // seek back to beginning of file

        lua_pushinteger(L, size);
        return 1;
    }
    return luaL_error(L, K_S_LUA_SDCARD_SIZE_READ_ERROR);
}

static int lualilka_file_seek(lua_State* L) {
    FILE* filePointer = *reinterpret_cast<FILE**>(luaL_checkudata(L, 1, FILE_OBJECT));
    if (filePointer) {
        size_t seek = luaL_checknumber(L, 2);
        fseek(filePointer, seek, SEEK_SET);
        return 0;
    }
    return luaL_error(L, K_S_LUA_SDCARD_SEEK_ERROR);
}

static int lualilka_file_read(lua_State* L) {
    FILE* filePointer = *reinterpret_cast<FILE**>(luaL_checkudata(L, 1, FILE_OBJECT));
    if (filePointer) {
        size_t maxBytes = luaL_checknumber(L, 2);

        std::unique_ptr<char[]> bufPtr(new char[maxBytes]);

        size_t bytesRead = fread(bufPtr.get(), 1, maxBytes, filePointer);

        if (bytesRead < maxBytes && ferror(filePointer)) {
            return luaL_error(L, K_S_LUA_SDCARD_READ_FILE_ERROR);
        }

        lua_pushlstring(L, bufPtr.get(), bytesRead);
        return 1;
    }
    return luaL_error(L, K_S_LUA_SDCARD_READ_ERROR);
}

static int lualilka_file_exists(lua_State* L) {
    const FILE* filePointer = *reinterpret_cast<FILE**>(luaL_checkudata(L, 1, FILE_OBJECT));
    lua_pushboolean(L, !!filePointer);
    return 1;
}

static int lualilka_file_write(lua_State* L) {
    FILE* filePointer = *reinterpret_cast<FILE**>(luaL_checkudata(L, 1, FILE_OBJECT));
    if (filePointer) {
        String text = luaL_checkstring(L, 2);

        fprintf(filePointer, "%s", text.c_str());

        return 0;
    }
    return luaL_error(L, K_S_LUA_SDCARD_WRITE_ERROR);
}

int lualilka_sdcard_list_dir(lua_State* L) {
    int n = lua_gettop(L);

    if (n != 1) {
        return luaL_error(L, K_S_LUA_SDCARD_ARGS_1_FMT, n);
    }

    if (!lilka::fileutils.isSDAvailable()) {
        return luaL_error(L, K_S_LUA_SDCARD_NOT_FOUND);
    }

    String path = lua_tostring(L, 1);

    size_t _numEntries = lilka::fileutils.getEntryCount(&SD, path);
    if (_numEntries == 0) {
        return luaL_error(L, K_S_LUA_SDCARD_DIR_EMPTY_OR_ERROR);
    }

    lilka::Entry* entries = new lilka::Entry[_numEntries];

    int numEntries = lilka::fileutils.listDir(&SD, path, entries);
    std::unique_ptr<lilka::Entry[]> entriesPtr(entries);

    if (_numEntries != numEntries) {
        return luaL_error(L, K_S_LUA_SDCARD_DIR_READ_ERROR);
    }

    lua_createtable(L, numEntries, 0);

    for (int i = 0; i < numEntries; i++) {
        lua_pushstring(L, entries[i].name.c_str());
        lua_rawseti(L, -2, i + 1); /* In lua indices start at 1 */
    }

    return 1;
}

int lualilka_sdcard_remove(lua_State* L) {
    int n = lua_gettop(L);

    if (n != 1) {
        return luaL_error(L, K_S_LUA_SDCARD_ARGS_1_FMT, n);
    }

    if (!lilka::fileutils.isSDAvailable()) {
        return luaL_error(L, K_S_LUA_SDCARD_NOT_FOUND);
    }

    String path = lua_tostring(L, 1);

    int ret = remove((lilka::fileutils.getSDRoot() + path).c_str());

    if (ret != 0) {
        return luaL_error(L, K_S_LUA_SDCARD_REMOVE_ERROR_FMT, ret);
    }

    return 0;
}

int lualilka_sdcard_rename(lua_State* L) {
    int n = lua_gettop(L);

    if (n != 2) {
        return luaL_error(L, K_S_LUA_SDCARD_ARGS_2_FMT, n);
    }

    if (!lilka::fileutils.isSDAvailable()) {
        return luaL_error(L, K_S_LUA_SDCARD_NOT_FOUND);
    }

    String old_name = lua_tostring(L, 1);
    String new_name = lua_tostring(L, 2);

    int ret =
        rename((lilka::fileutils.getSDRoot() + old_name).c_str(), (lilka::fileutils.getSDRoot() + new_name).c_str());

    if (ret != 0) {
        return luaL_error(L, K_S_LUA_SDCARD_RENAME_ERROR_FMT, ret);
    }

    return 0;
}

static const luaL_Reg lualilka_sdcard[] = {
    {"ls", lualilka_sdcard_list_dir},
    {"remove", lualilka_sdcard_remove},
    {"rename", lualilka_sdcard_rename},
    {"open", lualilka_create_object_file},
    {NULL, NULL},
};

int lualilka_sdcard_register(lua_State* L) {
    luaL_newlib(L, lualilka_sdcard);
    lua_setglobal(L, "sdcard");

    luaL_newmetatable(L, FILE_OBJECT);
    lua_pushcfunction(L, lualilka_delete_object_file);
    lua_setfield(L, -2, "__gc");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, lualilka_file_size);
    lua_setfield(L, -2, "size");
    lua_pushcfunction(L, lualilka_file_seek);
    lua_setfield(L, -2, "seek");
    lua_pushcfunction(L, lualilka_file_read);
    lua_setfield(L, -2, "read");
    lua_pushcfunction(L, lualilka_file_write);
    lua_setfield(L, -2, "write");
    lua_pushcfunction(L, lualilka_file_exists);
    lua_setfield(L, -2, "exists");

    lua_pop(L, 1);

    return 0;
}
