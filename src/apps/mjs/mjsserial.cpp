#include "mjsserial.h"
#include <HardwareSerial.h>
#include <lilka.h>
#include "mjs.h"

#define DEFAULT_BAUD   115200
#define DEFAULT_CONFIG SERIAL_8N1

static HardwareSerial MJSSerial(1);

// serial.available() -> number
static void mjs_serial_available(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, MJSSerial.available()));
}

// serial.begin([baud[, config]])
static void mjs_serial_begin(struct mjs* mjs) {
    int baud = DEFAULT_BAUD;
    int config = DEFAULT_CONFIG;
    mjs_val_t baud_arg = mjs_arg(mjs, 0);
    if (mjs_is_number(baud_arg)) {
        baud = mjs_get_int(mjs, baud_arg);
    }
    mjs_val_t config_arg = mjs_arg(mjs, 1);
    if (mjs_is_number(config_arg)) {
        config = mjs_get_int(mjs, config_arg);
    }
    MJSSerial.begin(baud, config, LILKA_P4, LILKA_P3);
    mjs_return(mjs, mjs_mk_undefined());
}

// serial.end()
static void mjs_serial_end(struct mjs* mjs) {
    MJSSerial.end();
    mjs_return(mjs, mjs_mk_undefined());
}

// serial.flush()
static void mjs_serial_flush(struct mjs* mjs) {
    MJSSerial.flush();
    mjs_return(mjs, mjs_mk_undefined());
}

// serial.peek() -> number
static void mjs_serial_peek(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, MJSSerial.peek()));
}

// serial.print(...)
static void mjs_serial_print(struct mjs* mjs) {
    int nargs = mjs_nargs(mjs);
    for (int i = 0; i < nargs; i++) {
        mjs_val_t arg = mjs_arg(mjs, i);
        if (mjs_is_string(arg)) {
            size_t len;
            const char* s = mjs_get_string(mjs, &arg, &len);
            MJSSerial.print(s);
        } else if (mjs_is_number(arg)) {
            MJSSerial.print(mjs_get_double(mjs, arg));
        } else {
            MJSSerial.print("[object]");
        }
    }
    mjs_return(mjs, mjs_mk_undefined());
}

// serial.println(...)
static void mjs_serial_println(struct mjs* mjs) {
    int nargs = mjs_nargs(mjs);
    for (int i = 0; i < nargs; i++) {
        mjs_val_t arg = mjs_arg(mjs, i);
        if (mjs_is_string(arg)) {
            size_t len;
            const char* s = mjs_get_string(mjs, &arg, &len);
            MJSSerial.println(s);
        } else if (mjs_is_number(arg)) {
            MJSSerial.println(mjs_get_double(mjs, arg));
        } else {
            MJSSerial.println("[object]");
        }
    }
    mjs_return(mjs, mjs_mk_undefined());
}

// serial.read([bytes]) -> number or string
static void mjs_serial_read(struct mjs* mjs) {
    int bytes = 0;
    mjs_val_t bytes_arg = mjs_arg(mjs, 0);
    if (mjs_is_number(bytes_arg)) {
        bytes = mjs_get_int(mjs, bytes_arg);
    }
    if (bytes <= 0) {
        mjs_return(mjs, mjs_mk_number(mjs, MJSSerial.read()));
    } else {
        char* buffer = new char[bytes + 1];
        int read = MJSSerial.read(buffer, bytes);
        buffer[read] = 0;
        mjs_return(mjs, mjs_mk_string(mjs, buffer, read, 1));
        delete[] buffer;
    }
}

// serial.setTimeout(timeout)
static void mjs_serial_set_timeout(struct mjs* mjs) {
    int timeout = mjs_get_int(mjs, mjs_arg(mjs, 0));
    MJSSerial.setTimeout(timeout);
    mjs_return(mjs, mjs_mk_undefined());
}

// serial.write(...)
static void mjs_serial_write(struct mjs* mjs) {
    int nargs = mjs_nargs(mjs);
    for (int i = 0; i < nargs; i++) {
        mjs_val_t arg = mjs_arg(mjs, i);
        if (mjs_is_string(arg)) {
            size_t len;
            const char* s = mjs_get_string(mjs, &arg, &len);
            MJSSerial.write(s);
        } else if (mjs_is_number(arg)) {
            MJSSerial.write(mjs_get_int(mjs, arg));
        } else {
            MJSSerial.write("[object]");
        }
    }
    mjs_return(mjs, mjs_mk_undefined());
}

void mjs_serial_register(struct mjs* mjs) {
    mjs_val_t serial = mjs_mk_object(mjs);
    mjs_set(mjs, serial, "available", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_serial_available));
    mjs_set(mjs, serial, "begin", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_serial_begin));
    mjs_set(mjs, serial, "end", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_serial_end));
    mjs_set(mjs, serial, "flush", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_serial_flush));
    mjs_set(mjs, serial, "peek", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_serial_peek));
    mjs_set(mjs, serial, "print", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_serial_print));
    mjs_set(mjs, serial, "println", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_serial_println));
    mjs_set(mjs, serial, "read", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_serial_read));
    mjs_set(mjs, serial, "setTimeout", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_serial_set_timeout));
    mjs_set(mjs, serial, "write", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_serial_write));

    // Serial config constants
    // clang-format off
    const int configValues[] = {
        SERIAL_5N1, SERIAL_6N1, SERIAL_7N1, SERIAL_8N1,
        SERIAL_5N2, SERIAL_6N2, SERIAL_7N2, SERIAL_8N2,
        SERIAL_5E1, SERIAL_6E1, SERIAL_7E1, SERIAL_8E1,
        SERIAL_5E2, SERIAL_6E2, SERIAL_7E2, SERIAL_8E2,
        SERIAL_5O1, SERIAL_6O1, SERIAL_7O1, SERIAL_8O1,
        SERIAL_5O2, SERIAL_6O2, SERIAL_7O2, SERIAL_8O2,
    };
    const char* configNames[] = {
        "SERIAL_5N1", "SERIAL_6N1", "SERIAL_7N1", "SERIAL_8N1",
        "SERIAL_5N2", "SERIAL_6N2", "SERIAL_7N2", "SERIAL_8N2",
        "SERIAL_5E1", "SERIAL_6E1", "SERIAL_7E1", "SERIAL_8E1",
        "SERIAL_5E2", "SERIAL_6E2", "SERIAL_7E2", "SERIAL_8E2",
        "SERIAL_5O1", "SERIAL_6O1", "SERIAL_7O1", "SERIAL_8O1",
        "SERIAL_5O2", "SERIAL_6O2", "SERIAL_7O2", "SERIAL_8O2",
    };
    // clang-format on
    for (int i = 0; i < (int)(sizeof(configValues) / sizeof(configValues[0])); i++) {
        mjs_set(mjs, serial, configNames[i], ~0, mjs_mk_number(mjs, configValues[i]));
    }

    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "serial", ~0, serial);
}
