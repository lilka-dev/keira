#pragma once

#include <lua.hpp>
#include "services/script/script.h"

// Runs Lua script as a background service.
// Script body is executed first, then lilka.init() and lilka.update(delta) are called if defined.
// lilka.update(delta) is called every service.interval seconds until the script stops or fails.
// Modules that need a canvas or buttons (display, controller, resources, UI, audio) are not available.
// Errors are logged to serial.
class LuaService : public ScriptService {
public:
    explicit LuaService(const String& path);

private:
    void run() override;
    // Runs script and its lilka.init() / lilka.update() callbacks. On error message is left on top of the stack
    int execute(lua_State* L);
};
