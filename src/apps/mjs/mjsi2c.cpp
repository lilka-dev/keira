#include "mjsi2c.h"
#include <Wire.h>
#include "mjs.h"

// Reads an mJS argument that is either a single byte (number) or an array of
// bytes into `buf`. Returns the number of bytes read.
static int mjs_i2c_read_bytes_arg(struct mjs* mjs, mjs_val_t arg, uint8_t* buf, int maxLen) {
    if (mjs_is_number(arg)) {
        if (maxLen < 1) {
            return 0;
        }
        buf[0] = (uint8_t)(mjs_get_int(mjs, arg) & 0xFF);
        return 1;
    }
    if (!mjs_is_array(arg)) {
        return 0;
    }
    int len = mjs_array_length(mjs, arg);
    if (len > maxLen) {
        len = maxLen;
    }
    for (int i = 0; i < len; i++) {
        buf[i] = (uint8_t)(mjs_get_int(mjs, mjs_array_get(mjs, arg, i)) & 0xFF);
    }
    return len;
}

// i2c.begin([sda, scl[, freq]])
static void mjs_i2c_begin(struct mjs* mjs) {
    int n = mjs_nargs(mjs);
    if (n == 0) {
        Wire.begin();
    } else {
        int sda = mjs_get_int(mjs, mjs_arg(mjs, 0));
        int scl = mjs_get_int(mjs, mjs_arg(mjs, 1));
        mjs_val_t freqArg = mjs_arg(mjs, 2);
        uint32_t freq = mjs_is_number(freqArg) ? (uint32_t)mjs_get_int(mjs, freqArg) : 100000;
        Wire.begin(sda, scl, freq);
    }
    mjs_return(mjs, mjs_mk_undefined());
}

// i2c.set_clock(freq)
static void mjs_i2c_set_clock(struct mjs* mjs) {
    uint32_t freq = (uint32_t)mjs_get_int(mjs, mjs_arg(mjs, 0));
    Wire.setClock(freq);
    mjs_return(mjs, mjs_mk_undefined());
}

// i2c.scan() -> array of 7-bit addresses
static void mjs_i2c_scan(struct mjs* mjs) {
    mjs_val_t arr = mjs_mk_array(mjs);
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            mjs_array_push(mjs, arr, mjs_mk_number(mjs, addr));
        }
    }
    mjs_return(mjs, arr);
}

// i2c.write(addr, data) -> status (0 = success)
static void mjs_i2c_write(struct mjs* mjs) {
    int addr = mjs_get_int(mjs, mjs_arg(mjs, 0));
    uint8_t buf[256];
    int len = mjs_i2c_read_bytes_arg(mjs, mjs_arg(mjs, 1), buf, sizeof(buf));
    Wire.beginTransmission((uint8_t)addr);
    if (len > 0) {
        Wire.write(buf, len);
    }
    uint8_t status = Wire.endTransmission();
    mjs_return(mjs, mjs_mk_number(mjs, status));
}

// i2c.read(addr, count) -> array of bytes
static void mjs_i2c_read(struct mjs* mjs) {
    int addr = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int count = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int got = Wire.requestFrom((uint8_t)addr, (uint8_t)count);
    mjs_val_t arr = mjs_mk_array(mjs);
    for (int i = 0; i < got && Wire.available(); i++) {
        mjs_array_push(mjs, arr, mjs_mk_number(mjs, Wire.read()));
    }
    mjs_return(mjs, arr);
}

// i2c.write_read(addr, wdata, count) -> array of bytes
static void mjs_i2c_write_read(struct mjs* mjs) {
    int addr = mjs_get_int(mjs, mjs_arg(mjs, 0));
    uint8_t buf[256];
    int len = mjs_i2c_read_bytes_arg(mjs, mjs_arg(mjs, 1), buf, sizeof(buf));
    int count = mjs_get_int(mjs, mjs_arg(mjs, 2));
    Wire.beginTransmission((uint8_t)addr);
    if (len > 0) {
        Wire.write(buf, len);
    }
    Wire.endTransmission(false); // repeated start
    int got = Wire.requestFrom((uint8_t)addr, (uint8_t)count);
    mjs_val_t arr = mjs_mk_array(mjs);
    for (int i = 0; i < got && Wire.available(); i++) {
        mjs_array_push(mjs, arr, mjs_mk_number(mjs, Wire.read()));
    }
    mjs_return(mjs, arr);
}

void mjs_i2c_register(struct mjs* mjs) {
    mjs_val_t i2c = mjs_mk_object(mjs);
    mjs_set(mjs, i2c, "begin", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_i2c_begin));
    mjs_set(mjs, i2c, "set_clock", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_i2c_set_clock));
    mjs_set(mjs, i2c, "scan", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_i2c_scan));
    mjs_set(mjs, i2c, "write", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_i2c_write));
    mjs_set(mjs, i2c, "read", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_i2c_read));
    mjs_set(mjs, i2c, "write_read", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_i2c_write_read));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "i2c", ~0, i2c);
}
