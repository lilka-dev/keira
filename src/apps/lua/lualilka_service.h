#pragma once

#include <lua.hpp>

// Default period of lilka.update() calls in background services, seconds
#define LUALILKA_SERVICE_DEFAULT_INTERVAL 0.1

// Registers global "service" table to start, stop and list background script services.
// When running inside a service, ScriptService* must be stored in registry under "service" key beforehand:
// service.name and service.interval are available then.
int lualilka_service_register(lua_State* L);
