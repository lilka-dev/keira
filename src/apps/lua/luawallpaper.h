#pragma once

#include <lua.hpp>
#include "keira/app.h"

// Home screen wallpaper driven by a Lua script.
// Script uses the same lilka.init() / lilka.update(delta) / lilka.draw() callbacks as regular Lua apps,
// but only has access to display, resources, math and geometry modules.
// Buttons are not available to the script - they are handled by the launcher.
class LuaWallpaper {
public:
    ~LuaWallpaper();
    // Loads script and calls lilka.init(). Drawing is done on the canvas of the given app.
    // Returns false if file is missing, fails to run or has no lilka.draw() function
    bool open(const char* path, App* app);
    void close();
    bool isOpen();
    // Calls lilka.update(delta) and lilka.draw(). Closes wallpaper and returns false on script error
    bool frame(uint32_t deltaMs);

private:
    // Pushes lilka[name] onto the stack if it's a function
    bool pushCallback(const char* name);
    // Calls function pushed with pushCallback, logs and closes wallpaper on error
    bool call(const char* name, int nargs);

    lua_State* L = nullptr;
};
