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
#include <cstring>

// Helper function to get the script directory from __dir__ global
static String mjs_get_dir(struct mjs* mjs) {
    mjs_val_t dir_val = mjs_get(mjs, mjs_get_global(mjs), "__dir__", ~0);
    if (mjs_is_string(dir_val)) {
        size_t len;
        const char* s = mjs_get_string(mjs, &dir_val, &len);
        return String(s);
    }
    return "";
}

// Custom load() function that resolves relative paths using __dir__
static void mjs_custom_load(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    if (!mjs_is_string(arg0)) {
        mjs_set_errorf(mjs, MJS_BAD_ARGS_ERROR, "load() requires a string path argument");
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    const char* path = mjs_get_cstring(mjs, &arg0);
    String fullPath;

    // If path starts with '/', it's absolute; otherwise resolve relative to __dir__
    if (path[0] == '/') {
        fullPath = String(path);
    } else {
        String dir = mjs_get_dir(mjs);
        fullPath = dir + "/" + path;
    }

    // Read file and strip any trailing multipart boundary
    FILE* fp = fopen(fullPath.c_str(), "rb");
    if (!fp) {
        mjs_set_errorf(mjs, MJS_FILE_READ_ERROR, "failed to read file \"%s\"", fullPath.c_str());
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* source = (char*)malloc(fileSize + 1);
    if (!source) {
        fclose(fp);
        mjs_set_errorf(mjs, MJS_FILE_READ_ERROR, "out of memory loading \"%s\"", fullPath.c_str());
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    fread(source, 1, fileSize, fp);
    fclose(fp);
    source[fileSize] = '\0';

    mjs_val_t res = mjs_mk_undefined();
    mjs_err_t err = mjs_exec(mjs, source, &res);
    free(source);
    if (err != MJS_OK) {
        mjs_prepend_errorf(mjs, err, "failed to load \"%s\"", fullPath.c_str());
    }
    mjs_return(mjs, res);
}

MJSApp::MJSApp(String path) : App("mJS"), path(path) {
    setktStackSize(16384);
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

    // Override built-in load() with custom version that resolves relative paths
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "load", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_custom_load));

    // Read file and strip any trailing multipart boundary from web uploads
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) {
        alert("mJS", String(K_S_MJS_ERROR) + "\nFailed to read file");
    } else {
        fseek(fp, 0, SEEK_END);
        size_t fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char* source = (char*)malloc(fileSize + 1);
        if (!source) {
            fclose(fp);
            alert("mJS", String(K_S_MJS_ERROR) + "\nOut of memory");
        } else {
            fread(source, 1, fileSize, fp);
            fclose(fp);
            source[fileSize] = '\0';

            mjs_err_t err = mjs_exec(mjs, source, &res);
            free(source);
            if (err != MJS_OK) {
                const char* error = mjs_strerror(mjs, err);
                Serial.printf("mJS error %d: %s\n", err, error ? error : "unknown");
                alert("mJS", String(K_S_MJS_ERROR) + err + "\n" + (error ? error : ""));
            }
        }
    }

    // Run full GC and destroy mJS instance to free all allocated memory
    mjs_gc(mjs, 1);
    mjs_destroy(mjs);

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
