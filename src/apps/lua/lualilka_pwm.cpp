#include "lualilka_pwm.h"
#include <Arduino.h>

// PWM is backed by the ESP32 LEDC peripheral.
// The buzzer also uses LEDC, so prefer higher channel numbers (e.g. 2..15)
// to avoid clashing with the system buzzer.

// pwm.setup(channel, freq, resolution_bits) -> actual frequency in Hz
static int lualilka_pwm_setup(lua_State* L) {
    int channel = luaL_checkinteger(L, 1);
    uint32_t freq = (uint32_t)luaL_checkinteger(L, 2);
    int resolution = luaL_optinteger(L, 3, 8);
    uint32_t actual = ledcSetup((uint8_t)channel, freq, (uint8_t)resolution);
    lua_pushinteger(L, actual);
    return 1;
}

// pwm.attach_pin(pin, channel)
static int lualilka_pwm_attachPin(lua_State* L) {
    int pin = luaL_checkinteger(L, 1);
    int channel = luaL_checkinteger(L, 2);
    ledcAttachPin((uint8_t)pin, (uint8_t)channel);
    return 0;
}

// pwm.detach_pin(pin)
static int lualilka_pwm_detachPin(lua_State* L) {
    int pin = luaL_checkinteger(L, 1);
    ledcDetachPin((uint8_t)pin);
    return 0;
}

// pwm.write(channel, duty)
static int lualilka_pwm_write(lua_State* L) {
    int channel = luaL_checkinteger(L, 1);
    uint32_t duty = (uint32_t)luaL_checkinteger(L, 2);
    ledcWrite((uint8_t)channel, duty);
    return 0;
}

// pwm.write_tone(channel, freq) -> actual frequency in Hz
static int lualilka_pwm_writeTone(lua_State* L) {
    int channel = luaL_checkinteger(L, 1);
    uint32_t freq = (uint32_t)luaL_checkinteger(L, 2);
    uint32_t actual = ledcWriteTone((uint8_t)channel, freq);
    lua_pushinteger(L, actual);
    return 1;
}

// pwm.read(channel) -> current duty
static int lualilka_pwm_read(lua_State* L) {
    int channel = luaL_checkinteger(L, 1);
    lua_pushinteger(L, ledcRead((uint8_t)channel));
    return 1;
}

static const luaL_Reg lualilka_pwm[] = {
    {"setup", lualilka_pwm_setup},
    {"attach_pin", lualilka_pwm_attachPin},
    {"detach_pin", lualilka_pwm_detachPin},
    {"write", lualilka_pwm_write},
    {"write_tone", lualilka_pwm_writeTone},
    {"read", lualilka_pwm_read},
    {NULL, NULL},
};

int lualilka_pwm_register(lua_State* L) {
    luaL_newlib(L, lualilka_pwm);
    lua_setglobal(L, "pwm");
    return 0;
}
