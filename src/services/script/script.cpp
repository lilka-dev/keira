#include "script.h"

#include <unistd.h>
#include "keira/ksystem.h"
#include "keira/utils/string.h"
#include "apps/lua/luaservice.h"
#include "apps/mjs/mjsservice.h"

struct AutorunScript {
    const char* prefix;
    const char* path;
};

static const AutorunScript AUTORUN_SCRIPTS[] = {
    {K_SCRIPT_SERVICE_LUA_PREFIX, K_SCRIPT_AUTORUN_LUA_PATH},
    {K_SCRIPT_SERVICE_JS_PREFIX, K_SCRIPT_AUTORUN_JS_PATH},
};

ScriptService::ScriptService(const char* prefix, const String& path) :
    Service(makeName(prefix, path).c_str()), path(path) {
}

ScriptService::~ScriptService() {
    vSemaphoreDelete(stopSignal);
}

String ScriptService::makeName(const char* prefix, const String& path) {
    String name = path.substring(path.lastIndexOf('/') + 1);
    int dot = name.lastIndexOf('.');
    if (dot > 0) name = name.substring(0, dot);
    name = String(prefix) + name;
    // Thread names are limited, longer ones are silently rejected
    if (name.length() > KT_NAME_MAX - 1) name = name.substring(0, KT_NAME_MAX - 1);
    return name;
}

void ScriptService::requestStop() {
    stopRequested = true;
    xSemaphoreGive(stopSignal);
}

bool ScriptService::isStopRequested() {
    return stopRequested;
}

const String& ScriptService::getPath() {
    return path;
}

String ScriptService::getDir() {
    return path.substring(0, path.lastIndexOf('/'));
}

bool ScriptService::sleep(uint32_t ms) {
    if (stopRequested) return false;
    // Semaphore is given only by requestStop(). Wait at least one tick to let other tasks run
    TickType_t ticks = pdMS_TO_TICKS(ms);
    xSemaphoreTake(stopSignal, ticks > 0 ? ticks : 1);
    return !stopRequested;
}

bool ScriptService::isScriptService(const char* name) {
    return strncmp(name, K_SCRIPT_SERVICE_LUA_PREFIX, strlen(K_SCRIPT_SERVICE_LUA_PREFIX)) == 0 ||
           strncmp(name, K_SCRIPT_SERVICE_JS_PREFIX, strlen(K_SCRIPT_SERVICE_JS_PREFIX)) == 0;
}

String ScriptService::spawn(const String& path, String& err) {
    if (access(path.c_str(), F_OK) != 0) {
        err = StringFormat("file %s not found", path.c_str());
        return "";
    }

    String lowerPath = path;
    lowerPath.toLowerCase();
    bool isLua = lowerPath.endsWith(".lua");
    if (!isLua && !lowerPath.endsWith(".js")) {
        err = StringFormat("unsupported script type: %s", path.c_str());
        return "";
    }

    String name = makeName(isLua ? K_SCRIPT_SERVICE_LUA_PREFIX : K_SCRIPT_SERVICE_JS_PREFIX, path);
    // Service that is still stopping counts as running, two threads with the same name would be ambiguous
    bool exists = false;
    ksystem.services.forEach([&](KeiraThread* thread) {
        if (strcmp(thread->getName(), name.c_str()) == 0 && thread->getState() != KTS_EXITING) exists = true;
    });
    if (exists) {
        err = StringFormat("service %s is already running", name.c_str());
        return "";
    }

    if (isLua) {
        ksystem.services.spawn(new LuaService(path));
    } else {
        ksystem.services.spawn(new MJSService(path));
    }
    lilka::serial.log("Script service %s started from %s", name.c_str(), path.c_str());
    return name;
}

void ScriptService::autorun() {
    if (!getAutorunEnabled()) {
        lilka::serial.log("Autorun is disabled");
        return;
    }
    for (const AutorunScript& script : AUTORUN_SCRIPTS) {
        if (access(script.path, F_OK) != 0) continue;
        String err;
        if (spawn(script.path, err).isEmpty()) {
            lilka::serial.err("Autorun %s: %s", script.path, err.c_str());
        }
    }
}

bool ScriptService::getAutorunEnabled() {
    NVS_LOCK;
    Preferences prefs;
    prefs.begin(K_SCRIPT_AUTORUN_NVS_NAMESPACE, true);
    bool enabled = prefs.getBool("enabled", true);
    prefs.end();
    NVS_UNLOCK;
    return enabled;
}

void ScriptService::setAutorunEnabled(bool enabled) {
    NVS_LOCK;
    Preferences prefs;
    prefs.begin(K_SCRIPT_AUTORUN_NVS_NAMESPACE, false);
    prefs.putBool("enabled", enabled);
    prefs.end();
    NVS_UNLOCK;

    if (enabled) {
        autorun();
    } else {
        for (const AutorunScript& script : AUTORUN_SCRIPTS) {
            stop(makeName(script.prefix, script.path).c_str());
        }
    }
}

bool ScriptService::stop(const char* name) {
    bool found = false;
    ksystem.services.forEach([&](KeiraThread* thread) {
        if (found || !isScriptService(thread->getName()) || strcmp(thread->getName(), name) != 0) return;
        if (thread->getState() == KTS_EXITING) return;
        static_cast<ScriptService*>(thread)->requestStop();
        found = true;
    });
    return found;
}

std::vector<String> ScriptService::list() {
    std::vector<String> names;
    ksystem.services.forEach([&](KeiraThread* thread) {
        if (!isScriptService(thread->getName())) return;
        // Stopped services stay in the manager until it cleans them up
        if (thread->getState() == KTS_EXITING) return;
        if (static_cast<ScriptService*>(thread)->isStopRequested()) return;
        names.push_back(thread->getName());
    });
    return names;
}
