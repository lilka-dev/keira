#include "lualilka_modules.h"

void lualilka_modules_register(lua_State* L) {
    lualilka_gps_register(L);
    lualilka_can_register(L);
    lualilka_ir_register(L);
    lualilka_lora_register(L);
    lualilka_nrf24_register(L);
    lualilka_cc1101_register(L);
    lualilka_imu_register(L);
    lualilka_mag_register(L);
}
