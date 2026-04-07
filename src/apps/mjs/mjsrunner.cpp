#include "mjsrunner.h"
#include "mjsgpio.h"
#include "mjsutil.h"
#include "mjsconsole.h"
#include "mjsmath.h"
#include "mjscontroller.h"
#include "mjsgeometry.h"
#include "mjsbuzzer.h"
#include "mjsdisplay.h"
#include "mjsresources.h"
#include "mjstransforms.h"
#include "mjswifi.h"
#include "mjshttp.h"
#include "mjsserial.h"
#include "mjssdcard.h"
#include "mjscrypto.h"
#include "mjsaudio.h"
#include "mjsstate.h"
#include "lilka.h"
#include "mjs.h"
#include "keira/keira.h"

MJSApp::MJSApp(String path) : App("mJS"), path(path) {
}

void MJSApp::run() {
    struct mjs* mjs = mjs_create();

    mjs_val_t res;

    mjs_set_ffi_resolver(mjs, ffi_resolver);

    // Register all modules
    mjs_gpio_register(mjs);
    mjs_util_register(mjs);
    mjs_console_register(mjs);
    mjs_math_register(mjs);
    mjs_controller_register(mjs);
    mjs_geometry_register(mjs);
    mjs_buzzer_register(mjs);
    mjs_display_register(mjs, this);
    mjs_transforms_register(mjs);
    mjs_wifi_register(mjs);
    mjs_http_register(mjs);
    mjs_serial_register(mjs);
    mjs_sdcard_register(mjs);
    mjs_crypto_register(mjs);
    mjs_audio_register(mjs);

    // Compute script directory and state path
    String dir = path.substring(0, path.lastIndexOf('/'));
    String statePath = path.substring(0, path.lastIndexOf('.')) + ".state";
    mjs_resources_register(mjs, dir.c_str());
    mjs_state_register(mjs, statePath.c_str());

    mjs_err_t err = mjs_exec_file(mjs, path.c_str(), &res);
    if (err != MJS_OK) {
        const char* error = mjs_strerror(mjs, err);
        (void)error;
        alert("mJS", String(K_S_MJS_ERROR) + err);
    }

    // Cleanup audio
    lilka::audioPlayer.cleanup();
}

void* MJSApp::ffi_resolver(void* handle, const char* name) {
    // TODO: Expose more functions
    if (strcmp(name, "print") == 0) {
        return (void*)printf;
    }
    return NULL;
}
