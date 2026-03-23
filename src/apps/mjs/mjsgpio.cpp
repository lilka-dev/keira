#include "mjsgpio.h"
#include <Arduino.h>
#include "mjs.h"

// gpio.set_mode(pin, mode)
static void mjs_gpio_set_mode(struct mjs* mjs) {
    int pin = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int mode = mjs_get_int(mjs, mjs_arg(mjs, 1));
    pinMode(pin, mode);
}

// gpio.write(pin, value)
static void mjs_gpio_write(struct mjs* mjs) {
    int pin = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int value = mjs_get_int(mjs, mjs_arg(mjs, 1));
    digitalWrite(pin, value);
}

// gpio.read(pin) -> number
static void mjs_gpio_read(struct mjs* mjs) {
    int pin = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int value = digitalRead(pin);
    mjs_return(mjs, mjs_mk_number(mjs, value));
}

// gpio.analog_read(pin) -> number (0..4095)
static void mjs_gpio_analog_read(struct mjs* mjs) {
    int pin = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int value = analogRead(pin);
    mjs_return(mjs, mjs_mk_number(mjs, value));
}

void mjs_gpio_register(struct mjs* mjs) {
    mjs_val_t gpio = mjs_mk_object(mjs);

    // Methods
    mjs_set(mjs, gpio, "set_mode", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_gpio_set_mode));
    mjs_set(mjs, gpio, "write", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_gpio_write));
    mjs_set(mjs, gpio, "read", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_gpio_read));
    mjs_set(mjs, gpio, "analog_read", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_gpio_analog_read));

    // Constants
    mjs_set(mjs, gpio, "LOW", ~0, mjs_mk_number(mjs, LOW));
    mjs_set(mjs, gpio, "HIGH", ~0, mjs_mk_number(mjs, HIGH));
    mjs_set(mjs, gpio, "INPUT", ~0, mjs_mk_number(mjs, INPUT));
    mjs_set(mjs, gpio, "OUTPUT", ~0, mjs_mk_number(mjs, OUTPUT));
    mjs_set(mjs, gpio, "INPUT_PULLUP", ~0, mjs_mk_number(mjs, INPUT_PULLUP));
    mjs_set(mjs, gpio, "INPUT_PULLDOWN", ~0, mjs_mk_number(mjs, INPUT_PULLDOWN));

    // Register as global "gpio"
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "gpio", ~0, gpio);
}
