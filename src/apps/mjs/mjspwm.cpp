#include "mjspwm.h"
#include <Arduino.h>
#include "mjs.h"

// pwm.setup(channel, freq, resolution_bits) -> actual frequency in Hz
static void mjs_pwm_setup(struct mjs* mjs) {
    int channel = mjs_get_int(mjs, mjs_arg(mjs, 0));
    uint32_t freq = (uint32_t)mjs_get_int(mjs, mjs_arg(mjs, 1));
    mjs_val_t resArg = mjs_arg(mjs, 2);
    int resolution = mjs_is_number(resArg) ? mjs_get_int(mjs, resArg) : 8;
    uint32_t actual = ledcSetup((uint8_t)channel, freq, (uint8_t)resolution);
    mjs_return(mjs, mjs_mk_number(mjs, actual));
}

// pwm.attach_pin(pin, channel)
static void mjs_pwm_attach_pin(struct mjs* mjs) {
    int pin = mjs_get_int(mjs, mjs_arg(mjs, 0));
    int channel = mjs_get_int(mjs, mjs_arg(mjs, 1));
    ledcAttachPin((uint8_t)pin, (uint8_t)channel);
    mjs_return(mjs, mjs_mk_undefined());
}

// pwm.detach_pin(pin)
static void mjs_pwm_detach_pin(struct mjs* mjs) {
    int pin = mjs_get_int(mjs, mjs_arg(mjs, 0));
    ledcDetachPin((uint8_t)pin);
    mjs_return(mjs, mjs_mk_undefined());
}

// pwm.write(channel, duty)
static void mjs_pwm_write(struct mjs* mjs) {
    int channel = mjs_get_int(mjs, mjs_arg(mjs, 0));
    uint32_t duty = (uint32_t)mjs_get_int(mjs, mjs_arg(mjs, 1));
    ledcWrite((uint8_t)channel, duty);
    mjs_return(mjs, mjs_mk_undefined());
}

// pwm.write_tone(channel, freq) -> actual frequency in Hz
static void mjs_pwm_write_tone(struct mjs* mjs) {
    int channel = mjs_get_int(mjs, mjs_arg(mjs, 0));
    uint32_t freq = (uint32_t)mjs_get_int(mjs, mjs_arg(mjs, 1));
    uint32_t actual = ledcWriteTone((uint8_t)channel, freq);
    mjs_return(mjs, mjs_mk_number(mjs, actual));
}

// pwm.read(channel) -> current duty
static void mjs_pwm_read(struct mjs* mjs) {
    int channel = mjs_get_int(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, ledcRead((uint8_t)channel)));
}

void mjs_pwm_register(struct mjs* mjs) {
    mjs_val_t pwm = mjs_mk_object(mjs);
    mjs_set(mjs, pwm, "setup", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_pwm_setup));
    mjs_set(mjs, pwm, "attach_pin", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_pwm_attach_pin));
    mjs_set(mjs, pwm, "detach_pin", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_pwm_detach_pin));
    mjs_set(mjs, pwm, "write", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_pwm_write));
    mjs_set(mjs, pwm, "write_tone", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_pwm_write_tone));
    mjs_set(mjs, pwm, "read", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_pwm_read));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "pwm", ~0, pwm);
}
