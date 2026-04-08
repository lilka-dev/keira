#include "mjsutil.h"
#include <Arduino.h>
#include "mjs.h"

// util.time() -> number (seconds since boot)
static void mjs_util_time(struct mjs* mjs) {
    float time = micros() / 1000000.0;
    mjs_return(mjs, mjs_mk_number(mjs, time));
}

// util.sleep(seconds)
static void mjs_util_sleep(struct mjs* mjs) {
    double s = mjs_get_double(mjs, mjs_arg(mjs, 0));
    vTaskDelay(s * 1000 / portTICK_PERIOD_MS);
}

// util.exit()
static void mjs_util_exit(struct mjs* mjs) {
    mjs_prepend_errorf(mjs, MJS_OK, "exit");
}

// util.free_ram() -> number
static void mjs_util_free_ram(struct mjs* mjs) {
    uint32_t freeRAM = ESP.getFreeHeap();
    mjs_return(mjs, mjs_mk_number(mjs, freeRAM));
}

// util.total_ram() -> number
static void mjs_util_total_ram(struct mjs* mjs) {
    uint32_t totalRAM = ESP.getHeapSize();
    mjs_return(mjs, mjs_mk_number(mjs, totalRAM));
}

void mjs_util_register(struct mjs* mjs) {
    mjs_val_t util = mjs_mk_object(mjs);

    mjs_set(mjs, util, "time", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_util_time));
    mjs_set(mjs, util, "sleep", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_util_sleep));
    mjs_set(mjs, util, "exit", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_util_exit));
    mjs_set(mjs, util, "free_ram", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_util_free_ram));
    mjs_set(mjs, util, "total_ram", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_util_total_ram));

    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "util", ~0, util);
}
