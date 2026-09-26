#pragma once

#include <atomic>
#include "keira/service.h"

#define K_SCRIPT_SERVICE_LUA_PREFIX "lua:"
#define K_SCRIPT_SERVICE_JS_PREFIX  "js:"

// Scripts from SD card root that are started as background services on boot
#define K_SCRIPT_AUTORUN_LUA_PATH      "/sd/autorun.lua"
#define K_SCRIPT_AUTORUN_JS_PATH       "/sd/autorun.js"
#define K_SCRIPT_AUTORUN_NVS_NAMESPACE "autorun"

// Background service that runs a user script (Lua or mJS). Has no canvas and no access to buttons.
// Service name is "<prefix><script file name without extension>", e.g. "lua:autorun".
// Only one service with the same name can run at a time.
class ScriptService : public Service {
public:
    ScriptService(const char* prefix, const String& path);
    ~ScriptService();

    // Asks script to stop. Script is interrupted on its next sleep (or next instructions for Lua)
    void requestStop();
    bool isStopRequested();
    const String& getPath();

    // Sleeps for given time, wakes up earlier if stop is requested. Returns false if stop is requested
    bool sleep(uint32_t ms);

    // True if thread with this name is a script service
    static bool isScriptService(const char* name);
    // Spawns Lua or mJS service depending on file extension.
    // Returns service name, or empty string with err set if service can't be started
    static String spawn(const String& path, String& err);
    // Starts autorun scripts from SD card root, if they exist and autorun is enabled
    static void autorun();
    // Autorun is enabled by default, the setting is kept in NVS
    static bool getAutorunEnabled();
    // Remembers the setting and starts or stops autorun scripts right away
    static void setAutorunEnabled(bool enabled);
    // Asks script service with given name to stop. Returns false if there's no such service
    static bool stop(const char* name);
    // Returns names of all script services
    static std::vector<String> list();

protected:
    // Directory of the script, without trailing slash
    String getDir();

private:
    static String makeName(const char* prefix, const String& path);

    String path;
    std::atomic<bool> stopRequested{false};
    SemaphoreHandle_t stopSignal = xSemaphoreCreateBinary();
};
