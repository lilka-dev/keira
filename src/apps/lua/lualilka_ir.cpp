#include "lualilka_modules.h"
#include "lualilka_hw.h"

#include <new>
#include <string.h>

#if CONFIG_IDF_TARGET_ESP32S3

using lilka::mod::Ir;
using lilka::mod::IrCode;
using lilka::mod::IrProtocol;
namespace IrCodec = lilka::mod::IrCodec;

#    define IR_MT "lilka.ir"

struct LuaIr {
    LuaHwClaim claim; // must be first
    LuaHwClaim rxClaim;
    Ir* dev = nullptr;
};

static LuaIr* checkIr(lua_State* L) {
    LuaIr* o = static_cast<LuaIr*>(luaL_checkudata(L, 1, IR_MT));
    if (!o->dev) luaL_error(L, "ir: object is closed");
    return o;
}

static int lualilka_ir_close(lua_State* L) {
    LuaIr* o = static_cast<LuaIr*>(luaL_checkudata(L, 1, IR_MT));
    delete o->dev; // releases the RMT channels
    o->dev = nullptr;
    lualilka_hw_release(&o->claim);
    lualilka_hw_release(&o->rxClaim);
    return 0;
}

static bool parseProtocol(const char* name, IrProtocol& out) {
    if (!strcmp(name, "NEC") || !strcmp(name, "nec")) {
        out = IrProtocol::NEC;
    } else if (!strcmp(name, "SAMSUNG") || !strcmp(name, "samsung")) {
        out = IrProtocol::Samsung;
    } else if (!strcmp(name, "SONY") || !strcmp(name, "sony")) {
        out = IrProtocol::Sony;
    } else {
        return false;
    }
    return true;
}

// ir.new([{tx = 43, rx = 44, carrier = 38000}]) -> ir | nil, error
// Pass tx = false or rx = false to use only one direction.
static int lualilka_ir_new(lua_State* L) {
    Ir::Config cfg;
    cfg.tx = lualilka_opt_pin(L, 1, "tx", cfg.tx);
    cfg.rx = lualilka_opt_pin(L, 1, "rx", cfg.rx);
    cfg.carrierHz = (uint32_t)lualilka_opt_int(L, 1, "carrier", cfg.carrierHz);
    if (cfg.carrierHz < 10000 || cfg.carrierHz > 100000) return luaL_error(L, "ir: carrier must be 10-100 kHz");
    if (cfg.tx < 0 && cfg.rx < 0) return luaL_error(L, "ir: need at least one of tx, rx");

    LuaIr* o = new (lua_newuserdatauv(L, sizeof(LuaIr), 0)) LuaIr();
    o->dev = nullptr;
    lualilka_hw_initClaim(L, &o->claim);
    lualilka_hw_initClaim(L, &o->rxClaim);
    luaL_setmetatable(L, IR_MT);
    if (cfg.tx >= 0) lualilka_hw_claimPeriph(L, &o->claim, lilka::mod::Periph::RMT_TX, "IR transmitter (RMT)");
    if (cfg.rx >= 0) lualilka_hw_claimPeriph(L, &o->rxClaim, lilka::mod::Periph::RMT_RX, "IR receiver (RMT)");
    lualilka_hw_claimPins(L, &o->claim, {cfg.tx, cfg.rx});

    o->dev = new (std::nothrow) Ir();
    if (!o->dev) return luaL_error(L, "ir: out of memory");
    esp_err_t err = o->dev->begin(cfg);
    if (err != ESP_OK) {
        lualilka_hw_closeTop(L, lualilka_ir_close);
        lua_pushnil(L);
        lua_pushfstring(L, "ir: RMT init failed (%s)", esp_err_to_name(err));
        // cppcheck-suppress memleak ; dev is owned by the userdata and freed in close/__gc
        return 2;
    }
    return 1;
}

// ir:send(protocol, address, command[, repeats = 0]) -> bool
// protocol: "NEC", "SAMSUNG" or "SONY" (SONY: address up to 5 bits = 12-bit frame, 8 bits = 15, 13 bits = 20)
static int lualilka_ir_send(lua_State* L) {
    LuaIr* o = checkIr(L);
    IrCode code = {};
    const char* name = luaL_checkstring(L, 2);
    if (!parseProtocol(name, code.protocol)) return luaL_error(L, "ir: unknown protocol '%s'", name);
    code.address = (uint32_t)luaL_checkinteger(L, 3);
    code.command = (uint32_t)luaL_checkinteger(L, 4);
    int repeats = luaL_optinteger(L, 5, code.protocol == IrProtocol::Sony ? 2 : 0);
    if (repeats < 0 || repeats > 20) return luaL_error(L, "ir: repeats must be 0-20");
    if (code.protocol == IrProtocol::Sony) code.bits = code.address > 0xFF ? 20 : (code.address > 0x1F ? 15 : 12);
    esp_err_t err = o->dev->send(code, (uint8_t)repeats);
    lua_pushboolean(L, err == ESP_OK);
    return 1;
}

// ir:send_raw(durations[, carrier]) -> bool
// durations: table of microseconds, alternating mark/space and starting with a mark.
static int lualilka_ir_sendRaw(lua_State* L) {
    LuaIr* o = checkIr(L);
    luaL_checktype(L, 2, LUA_TTABLE);
    uint32_t carrier = (uint32_t)luaL_optinteger(L, 3, 0);
    size_t n = (size_t)luaL_len(L, 2);
    if (n > Ir::MAX_DURATIONS) return luaL_error(L, "ir: too many durations (max %d)", (int)Ir::MAX_DURATIONS);
    uint16_t* d = static_cast<uint16_t*>(lua_newuserdatauv(L, n * sizeof(uint16_t) + 1, 0));
    for (size_t i = 0; i < n; i++) {
        lua_geti(L, 2, (lua_Integer)i + 1);
        lua_Integer v = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        if (v <= 0 || v > 0xFFFF) return luaL_error(L, "ir: duration #%d out of range", (int)i + 1);
        d[i] = (uint16_t)v;
    }
    lua_pushboolean(L, o->dev->sendRaw(d, n, carrier) == ESP_OK);
    return 1;
}

// ir:receive() -> {protocol, address, command, repeat, bits, raw} | nil
// protocol is "UNKNOWN" if the frame was not recognised; raw is always present.
static int lualilka_ir_receive(lua_State* L) {
    LuaIr* o = checkIr(L);
    uint16_t* d = static_cast<uint16_t*>(lua_newuserdatauv(L, Ir::MAX_DURATIONS * sizeof(uint16_t), 0));
    size_t n = o->dev->receiveRaw(d, Ir::MAX_DURATIONS);
    if (!n) {
        lua_pushnil(L);
        return 1;
    }
    IrCode code;
    bool known = IrCodec::decode(d, n, code);
    lua_createtable(L, 0, 6);
    lua_pushstring(L, IrCodec::name(known ? code.protocol : IrProtocol::Unknown));
    lua_setfield(L, -2, "protocol");
    if (known) {
        lua_pushinteger(L, code.address);
        lua_setfield(L, -2, "address");
        lua_pushinteger(L, code.command);
        lua_setfield(L, -2, "command");
        lua_pushboolean(L, code.repeat);
        lua_setfield(L, -2, "repeat");
        lua_pushinteger(L, code.bits);
        lua_setfield(L, -2, "bits");
    }
    lua_createtable(L, (int)n, 0);
    for (size_t i = 0; i < n; i++) {
        lua_pushinteger(L, d[i]);
        lua_rawseti(L, -2, (lua_Integer)i + 1);
    }
    lua_setfield(L, -2, "raw");
    return 1;
}

static const luaL_Reg irMethods[] = {
    {"send", lualilka_ir_send},
    {"send_raw", lualilka_ir_sendRaw},
    {"receive", lualilka_ir_receive},
    {"close", lualilka_ir_close},
    {NULL, NULL},
};

static const luaL_Reg irLib[] = {
    {"new", lualilka_ir_new},
    {NULL, NULL},
};

int lualilka_ir_register(lua_State* L) {
    lualilka_hw_newClass(L, IR_MT, irMethods, lualilka_ir_close);
    luaL_newlib(L, irLib);
    lua_setglobal(L, "ir");
    return 0;
}

#else

int lualilka_ir_register(lua_State*) {
    return 0;
}

#endif
