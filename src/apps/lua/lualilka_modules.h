#pragma once

#include <lua.hpp>

// Lua bindings for the Lilka v2 extension modules (gps, can, ir, lora, nrf24, cc1101, imu, mag).
int lualilka_gps_register(lua_State* L);
int lualilka_can_register(lua_State* L);
int lualilka_ir_register(lua_State* L);
int lualilka_lora_register(lua_State* L);
int lualilka_nrf24_register(lua_State* L);
int lualilka_cc1101_register(lua_State* L);
int lualilka_imu_register(lua_State* L);
int lualilka_mag_register(lua_State* L);

// Registers all of the above.
void lualilka_modules_register(lua_State* L);
