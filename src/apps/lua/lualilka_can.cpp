#include "lualilka_modules.h"
#include "lualilka_hw.h"

#include <new>
#include <string.h>

using lilka::mod::Can;

#define CAN_MT "lilka.can"

struct LuaCan {
    LuaHwClaim claim; // must be first
    Can* dev = nullptr;
};

static LuaCan* checkCan(lua_State* L) {
    LuaCan* c = static_cast<LuaCan*>(luaL_checkudata(L, 1, CAN_MT));
    if (!c->dev) luaL_error(L, "can: object is closed");
    return c;
}

static int lualilka_can_close(lua_State* L) {
    LuaCan* c = static_cast<LuaCan*>(luaL_checkudata(L, 1, CAN_MT));
    delete c->dev; // stops and uninstalls the TWAI driver
    c->dev = nullptr;
    lualilka_hw_release(&c->claim);
    return 0;
}

// can.new([{bitrate = 500000, mode = "normal"|"listen"|"no_ack", tx = 43, rx = 44,
//           filter = {code = 0, mask = 0xFFFFFFFF, single = true}}]) -> can | nil, error
static int lualilka_can_new(lua_State* L) {
    Can::Config cfg;
    cfg.bitrate = (uint32_t)lualilka_opt_int(L, 1, "bitrate", cfg.bitrate);
    cfg.tx = lualilka_opt_pin(L, 1, "tx", cfg.tx);
    cfg.rx = lualilka_opt_pin(L, 1, "rx", cfg.rx);
    if (cfg.tx < 0 || cfg.rx < 0) return luaL_error(L, "can: tx and rx pins are required");
    const char* mode = lualilka_opt_str(L, 1, "mode", "normal");
    if (!strcmp(mode, "normal")) {
        cfg.mode = Can::Mode::Normal;
    } else if (!strcmp(mode, "listen")) {
        cfg.mode = Can::Mode::ListenOnly;
    } else if (!strcmp(mode, "no_ack")) {
        cfg.mode = Can::Mode::NoAck;
    } else {
        return luaL_error(L, "can: unknown mode '%s' (normal, listen, no_ack)", mode);
    }
    twai_timing_config_t timing;
    if (!Can::timingFor(cfg.bitrate, timing)) {
        return luaL_error(L, "can: unsupported bitrate %d", (int)cfg.bitrate);
    }
    if (!lua_isnoneornil(L, 1)) {
        lua_getfield(L, 1, "filter");
        if (lua_istable(L, -1)) {
            int f = lua_gettop(L);
            cfg.filterCode = (uint32_t)lualilka_opt_int(L, f, "code", 0);
            cfg.filterMask = (uint32_t)lualilka_opt_int(L, f, "mask", 0xFFFFFFFF);
            cfg.singleFilter = lualilka_opt_bool(L, f, "single", true);
        }
        lua_pop(L, 1);
    }

    LuaCan* c = new (lua_newuserdatauv(L, sizeof(LuaCan), 0)) LuaCan();
    c->dev = nullptr;
    lualilka_hw_initClaim(L, &c->claim);
    luaL_setmetatable(L, CAN_MT);
    lualilka_hw_claimPeriph(L, &c->claim, lilka::mod::Periph::TWAI, "CAN (TWAI)");
    lualilka_hw_claimPins(L, &c->claim, {cfg.tx, cfg.rx});

    c->dev = new (std::nothrow) Can();
    if (!c->dev) return luaL_error(L, "can: out of memory");
    esp_err_t err = c->dev->begin(cfg);
    if (err != ESP_OK) {
        lualilka_hw_closeTop(L, lualilka_can_close);
        lua_pushnil(L);
        lua_pushfstring(L, "can: start failed (%s)", esp_err_to_name(err));
        // cppcheck-suppress memleak ; dev is owned by the userdata and freed in close/__gc
        return 2;
    }
    return 1;
}

// can:send(id, data[, {extended = false, rtr = false, timeout = 100}]) -> ok, error
static int lualilka_can_send(lua_State* L) {
    LuaCan* c = checkCan(L);
    twai_message_t msg = {};
    lua_Integer id = luaL_checkinteger(L, 2);
    bool extended = lualilka_opt_bool(L, 4, "extended", id > 0x7FF);
    msg.rtr = lualilka_opt_bool(L, 4, "rtr", false);
    msg.extd = extended;
    msg.identifier = (uint32_t)id & (extended ? 0x1FFFFFFF : 0x7FF);
    uint32_t timeout = (uint32_t)lualilka_opt_int(L, 4, "timeout", 100);
    if (msg.rtr) {
        // Remote frame: data argument is the requested length (number) or ignored
        msg.data_length_code = lua_isinteger(L, 3) ? (uint8_t)lua_tointeger(L, 3) : 0;
        if (msg.data_length_code > 8) return luaL_error(L, "can: DLC must be 0-8");
    } else if (!lua_isnoneornil(L, 3)) {
        msg.data_length_code = (uint8_t)lualilka_check_bytes(L, 3, msg.data, 8);
    }
    esp_err_t err = c->dev->send(msg, timeout);
    lua_pushboolean(L, err == ESP_OK);
    if (err == ESP_OK) return 1;
    lua_pushstring(L, esp_err_to_name(err));
    return 2;
}

// can:receive([timeout_ms = 0]) -> {id, data, extended, rtr, dlc} | nil
static int lualilka_can_receive(lua_State* L) {
    LuaCan* c = checkCan(L);
    uint32_t timeout = (uint32_t)luaL_optinteger(L, 2, 0);
    twai_message_t msg;
    if (c->dev->receive(msg, timeout) != ESP_OK) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, 0, 5);
    lua_pushinteger(L, msg.identifier);
    lua_setfield(L, -2, "id");
    uint8_t len = msg.rtr ? 0 : (msg.data_length_code > 8 ? 8 : msg.data_length_code);
    lua_pushlstring(L, reinterpret_cast<const char*>(msg.data), len);
    lua_setfield(L, -2, "data");
    lua_pushinteger(L, msg.data_length_code);
    lua_setfield(L, -2, "dlc");
    lua_pushboolean(L, msg.extd);
    lua_setfield(L, -2, "extended");
    lua_pushboolean(L, msg.rtr);
    lua_setfield(L, -2, "rtr");
    return 1;
}

// can:status() -> table
static int lualilka_can_status(lua_State* L) {
    LuaCan* c = checkCan(L);
    twai_status_info_t info;
    if (c->dev->status(info) != ESP_OK) {
        lua_pushnil(L);
        return 1;
    }
    static const char* const states[] = {"stopped", "running", "bus_off", "recovering"};
    lua_createtable(L, 0, 9);
    lua_pushstring(L, info.state <= TWAI_STATE_RECOVERING ? states[info.state] : "unknown");
    lua_setfield(L, -2, "state");
    const struct {
        const char* key;
        uint32_t value;
    } fields[] = {
        {"tx_error_counter", info.tx_error_counter},
        {"rx_error_counter", info.rx_error_counter},
        {"tx_failed", info.tx_failed_count},
        {"rx_missed", info.rx_missed_count},
        {"rx_overrun", info.rx_overrun_count},
        {"arbitration_lost", info.arb_lost_count},
        {"bus_errors", info.bus_error_count},
        {"rx_queued", info.msgs_to_rx},
        {"tx_queued", info.msgs_to_tx},
    };
    for (auto& f : fields) {
        lua_pushinteger(L, f.value);
        lua_setfield(L, -2, f.key);
    }
    return 1;
}

// can:recover() -> bool
static int lualilka_can_recover(lua_State* L) {
    lua_pushboolean(L, checkCan(L)->dev->recover() == ESP_OK);
    return 1;
}

static const luaL_Reg canMethods[] = {
    {"send", lualilka_can_send},
    {"receive", lualilka_can_receive},
    {"status", lualilka_can_status},
    {"recover", lualilka_can_recover},
    {"close", lualilka_can_close},
    {NULL, NULL},
};

static const luaL_Reg canLib[] = {
    {"new", lualilka_can_new},
    {NULL, NULL},
};

int lualilka_can_register(lua_State* L) {
    lualilka_hw_newClass(L, CAN_MT, canMethods, lualilka_can_close);
    luaL_newlib(L, canLib);
    lua_setglobal(L, "can");
    return 0;
}
