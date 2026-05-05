#include "dynapp.h"
#include "keira/appmanager.h"
#include <lilka.h>

/* ── DynApp implementation ──────────────────────────────────────────────── */

DynApp::DynApp(const String& path) : App("DynApp"), filePath(path) {
    setktStackSize(16384);
    setFlags(APP_FLAG_FULLSCREEN);
}

void DynApp::run() {
    lilka::serial.log("DynApp: loading %s", filePath.c_str());

    lilka::DynLoader loader;
    int ret = loader.load(filePath.c_str());

    if (ret != 0) {
        const char* err = loader.getError();
        lilka::serial.log("DynApp: load failed: %s (ret=%d)", err ? err : "unknown", ret);
        alert("DynApp Error", String("Failed to load:\n") + (err ? err : "Unknown error"));
        return;
    }

    /* Prepare argv: argv[0] = path to the .so file */
    char* argv[1];
    char pathBuf[256];
    strncpy(pathBuf, filePath.c_str(), sizeof(pathBuf) - 1);
    pathBuf[sizeof(pathBuf) - 1] = '\0';
    argv[0] = pathBuf;

    lilka::serial.log("DynApp: executing...");
    ret = loader.execute(1, argv);
    lilka::serial.log("DynApp: app returned %d", ret);

    loader.unload();
}

void DynApp::onExit() {
    lilka::serial.log("DynApp: exiting");
}
