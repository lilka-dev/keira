#include "lualilka_fs.h"
#include "lilka.h"
#include "keira/keira.h"

String luapath_to_path(lua_State* L, const char* path) {
    // Empty path
    if (strlen(path) == 0) return "/";

    // Absolute path
    if (path[0] == '/') return String(path);

    // Relative path
    lua_getfield(L, LUA_REGISTRYINDEX, "dir");
    const char* dir = lua_tostring(L, -1);
    lua_pop(L, 1);

    return String(dir) + "/" + path;
}

static int lualilka_fs_create_object_file(lua_State* L) {
    String res_path = luapath_to_path(L, luaL_checkstring(L, 1));
    const char* path = res_path.c_str();
    const char* mode = luaL_optstring(L, 2, "r");
    *reinterpret_cast<FILE**>(lua_newuserdata(L, sizeof(FILE*))) = fopen(path, mode);
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

static int lualilka_fs_file_size(lua_State* L) {
    FILE* filePointer = *reinterpret_cast<FILE**>(luaL_checkudata(L, 1, FILE_OBJECT));
    if (filePointer) {
        // TODO: use fsize() from pr#120
        size_t initial_offset = ftell(filePointer); // get current file pointer
        fseek(filePointer, 0, SEEK_END); // seek to end of file
        size_t size = ftell(filePointer); // get current file pointer
        fseek(filePointer, initial_offset, SEEK_SET); // seek back to beginning of file

        lua_pushinteger(L, size);
        return 1;
    }
    return luaL_error(L, K_S_LUA_FS_SIZE_READ_ERROR);
}

static int lualilka_fs_file_seek(lua_State* L) {
    FILE* filePointer = *reinterpret_cast<FILE**>(luaL_checkudata(L, 1, FILE_OBJECT));
    if (filePointer) {
        size_t seek = luaL_checknumber(L, 2);
        fseek(filePointer, seek, SEEK_SET);
        return 0;
    }
    return luaL_error(L, K_S_LUA_FS_SEEK_ERROR);
}

static int lualilka_fs_file_read(lua_State* L) {
    FILE* filePointer = *reinterpret_cast<FILE**>(luaL_checkudata(L, 1, FILE_OBJECT));
    if (filePointer) {
        size_t maxBytes = luaL_checknumber(L, 2);

        std::unique_ptr<char[]> bufPtr(new char[maxBytes]);

        size_t bytesRead = fread(bufPtr.get(), 1, maxBytes, filePointer);

        if (bytesRead < maxBytes && ferror(filePointer)) {
            return luaL_error(L, K_S_LUA_FS_READ_FILE_ERROR);
        }

        lua_pushlstring(L, bufPtr.get(), bytesRead);
        return 1;
    }
    return luaL_error(L, K_S_LUA_FS_READ_ERROR);
}

static int lualilka_fs_file_exists(lua_State* L) {
    const FILE* filePointer = *reinterpret_cast<FILE**>(luaL_checkudata(L, 1, FILE_OBJECT));
    lua_pushboolean(L, !!filePointer);
    return 1;
}

static int lualilka_fs_file_write(lua_State* L) {
    FILE* filePointer = *reinterpret_cast<FILE**>(luaL_checkudata(L, 1, FILE_OBJECT));
    if (filePointer) {
        const char* text = luaL_checkstring(L, 2);

        fprintf(filePointer, "%s", text);

        return 0;
    }
    return luaL_error(L, K_S_LUA_FS_WRITE_ERROR);
}

int lualilka_fs_list_dir(lua_State* L) {
    int n = lua_gettop(L);

    if (n != 1) {
        return luaL_error(L, K_S_LUA_FS_ARGS_1_FMT, n);
    }

    String res_path = luapath_to_path(L, luaL_checkstring(L, 1));
    const char* path = res_path.c_str();

    DIR* dir = opendir(path);

    // TODO:Here we can check errno
    if (!dir) return luaL_error(L, K_S_LUA_FS_DIR_EMPTY_OR_ERROR);

    // TODO: use lendir() from pr#120
    size_t _numEntries = 0;
    while ((readdir(dir)) != NULL)
        _numEntries++;
    rewinddir(dir);

    lua_createtable(L, _numEntries, 0);

    const struct dirent* dir_entry = NULL;

    size_t index = 1; /* In lua indices start at 1 */
    while ((dir_entry = readdir(dir)) != NULL) {
        lua_pushstring(L, dir_entry->d_name);
        lua_rawseti(L, -2, index);
        index++;
    }

    closedir(dir);

    return 1;
}

int lualilka_fs_remove(lua_State* L) {
    int n = lua_gettop(L);

    if (n != 1) {
        return luaL_error(L, K_S_LUA_FS_ARGS_1_FMT, n);
    }

    String res_path = luapath_to_path(L, luaL_checkstring(L, 1));
    const char* path = res_path.c_str();

    int ret = remove(path);

    if (ret != 0) {
        return luaL_error(L, K_S_LUA_FS_REMOVE_ERROR_FMT, ret);
    }

    return 0;
}

int lualilka_fs_rename(lua_State* L) {
    int n = lua_gettop(L);

    if (n != 2) {
        return luaL_error(L, K_S_LUA_FS_ARGS_2_FMT, n);
    }
    String old_res_path = luapath_to_path(L, luaL_checkstring(L, 1));
    String new_res_path = luapath_to_path(L, luaL_checkstring(L, 2));

    const char* old_name = old_res_path.c_str();
    const char* new_name = new_res_path.c_str();

    int ret = rename(old_name, new_name);

    if (ret != 0) {
        return luaL_error(L, K_S_LUA_FS_RENAME_ERROR_FMT, ret);
    }

    return 0;
}

int lualilka_fs_join_path(lua_State* L) {
    int n = lua_gettop(L);

    if (n != 2) {
        return luaL_error(L, K_S_LUA_FS_ARGS_2_FMT, n);
    }

    const char* lpath = luaL_checkstring(L, 1);
    const char* rpath = luaL_checkstring(L, 2);

    lua_pushstring(L, lilka::fileutils.joinPath(lpath, rpath).c_str());

    return 1;
}

int lualilka_fs_mk_path(lua_State* L) {
    if (lua_gettop(L) != 1) {
        return luaL_error(L, K_S_LUA_FS_ARGS_1_FMT, lua_gettop(L));
    }
    String res_path = luapath_to_path(L, luaL_checkstring(L, 1));
    const char* path = res_path.c_str();

    char buf[PATH_MAX + 1];

    strncpy(buf, path, PATH_MAX);
    buf[PATH_MAX] = '\0';

    // Skip root "/<mount>"
    const char* p = strchr(buf + 1, '/');
    if (!p) return 0;

    for (;;) {
        char* sep = strchr(p + 1, '/');
        if (sep) *sep = '\0';

        if (mkdir(buf, 0777) != 0 && errno != EEXIST)
            return luaL_error(L, "mkdir('%s') failed: %s", buf, strerror(errno));

        if (!sep) break;

        *sep = '/';
        p = sep;
    }

    errno = 0;
    return 0;
}

static const luaL_Reg lualilka_fs[] = {
    {"ls", lualilka_fs_list_dir},
    {"remove", lualilka_fs_remove},
    {"rename", lualilka_fs_rename},
    {"open", lualilka_fs_create_object_file},
    {"joinpath", lualilka_fs_join_path},
    {"mkpath", lualilka_fs_mk_path},
    {NULL, NULL},
};

int lualilka_fs_register(lua_State* L) {
    luaL_newlib(L, lualilka_fs);
    lua_setglobal(L, "fs");

    luaL_newmetatable(L, FILE_OBJECT);
    lua_pushcfunction(L, lualilka_delete_object_file);
    lua_setfield(L, -2, "__gc");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, lualilka_fs_file_size);
    lua_setfield(L, -2, "size");
    lua_pushcfunction(L, lualilka_fs_file_seek);
    lua_setfield(L, -2, "seek");
    lua_pushcfunction(L, lualilka_fs_file_read);
    lua_setfield(L, -2, "read");
    lua_pushcfunction(L, lualilka_fs_file_write);
    lua_setfield(L, -2, "write");
    lua_pushcfunction(L, lualilka_fs_file_exists);
    lua_setfield(L, -2, "exists");

    lua_pop(L, 1);

    return 0;
}
