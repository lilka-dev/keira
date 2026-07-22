#pragma once

#include <lua.hpp>
#include <Arduino.h>
#define FILE_OBJECT "File"

int lualilka_fs_register(lua_State* L);

// Helper to make shared path traversal across lua
String luapath_to_path(lua_State* L, const char* path);