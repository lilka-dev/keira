#include "mjsservice.h"

#include "mjsrunner.h"
#include "mjsgpio.h"
#include "mjsi2c.h"
#include "mjsspi.h"
#include "mjspwm.h"
#include "mjsws2812.h"
#include "mjsutil.h"
#include "mjsconsole.h"
#include "mjsmath.h"
#include "mjsgeometry.h"
#include "mjsbuzzer.h"
#include "mjswifi.h"
#include "mjshttp.h"
#include "mjsserial.h"
#include "mjssdcard.h"
#include "mjscrypto.h"
#include "mjsstate.h"
#include "mjs.h"

static ScriptService* mjsservice_get(struct mjs* mjs) {
    return static_cast<ScriptService*>(mjs_get_ptr(mjs, mjs_get(mjs, mjs_get_global(mjs), "__service__", ~0)));
}

// util.sleep() replacement that aborts the script when stop is requested
static void mjsservice_sleep(struct mjs* mjs) {
    double s = mjs_get_double(mjs, mjs_arg(mjs, 0));
    if (!mjsservice_get(mjs)->sleep(s * 1000)) {
        mjs_set_errorf(mjs, MJS_INTERNAL_ERROR, "service stopped");
    }
    mjs_return(mjs, mjs_mk_undefined());
}

// util.exit() replacement that aborts the script
static void mjsservice_exit(struct mjs* mjs) {
    mjsservice_get(mjs)->requestStop();
    mjs_set_errorf(mjs, MJS_INTERNAL_ERROR, "service stopped");
    mjs_return(mjs, mjs_mk_undefined());
}

MJSService::MJSService(const String& path) : ScriptService(K_SCRIPT_SERVICE_JS_PREFIX, path) {
    setktStackSize(16384);
}

void MJSService::run() {
    struct mjs* mjs = mjs_create();
    mjs_val_t global = mjs_get_global(mjs);

    mjs_set(mjs, global, "__service__", ~0, mjs_mk_foreign(mjs, static_cast<ScriptService*>(this)));
    // Normally set by resources module, needed by load() to resolve relative paths
    String dir = getDir();
    mjs_set(mjs, global, "__dir__", ~0, mjs_mk_string(mjs, dir.c_str(), ~0, 1));

    mjs_gpio_register(mjs);
    mjs_i2c_register(mjs);
    mjs_spi_register(mjs);
    mjs_pwm_register(mjs);
    mjs_ws2812_register(mjs);
    mjs_util_register(mjs);
    mjs_console_register(mjs);
    mjs_math_register(mjs);
    mjs_geometry_register(mjs);
    mjs_buzzer_register(mjs);
    mjs_wifi_register(mjs);
    mjs_http_register(mjs);
    mjs_serial_register(mjs);
    mjs_sdcard_register(mjs);
    mjs_crypto_register(mjs);

    const String& path = getPath();
    String statePath = path.substring(0, path.lastIndexOf('.')) + ".state";
    mjs_state_register(mjs, statePath.c_str());

    mjs_val_t util = mjs_get(mjs, global, "util", ~0);
    mjs_set(mjs, util, "sleep", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjsservice_sleep));
    mjs_set(mjs, util, "exit", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjsservice_exit));
    mjs_set(mjs, global, "load", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_custom_load));

    lilka::serial.log("mJS service %s: running %s", getName(), path.c_str());
    mjs_val_t res;
    mjs_err_t err = mjs_exec_file(mjs, path.c_str(), &res);
    if (err != MJS_OK && !isStopRequested()) {
        const char* error = mjs_strerror(mjs, err);
        lilka::serial.err("mJS service %s: error %d: %s", getName(), err, error ? error : "unknown");
    } else {
        lilka::serial.log("mJS service %s: stopped", getName());
    }

    mjs_gc(mjs, 1);
    mjs_destroy(mjs);
}
