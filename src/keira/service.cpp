#include "keira/service.h"
#include "services/watchdog/watchdog.h"
#include "keira/ksystem.h"
Service::Service(const char* name) {
    setName(name);
    setStackSize(4096);
    setPriority(KT_PRIO_IDLE);
    setType(KT_SERVICE);
    NVS_LOCK;
    Preferences prefs;
    prefs.begin(name, true);
    enabled = prefs.getBool("enabled", false);
    prefs.end();
    NVS_UNLOCK;
}

bool Service::getEnabled() {
    return enabled;
}

void Service::setEnabled(bool enabled) {
    this->enabled = enabled;
    NVS_LOCK;
    Preferences prefs;
    prefs.begin(name, false);
    prefs.putBool("enabled", enabled);
    prefs.end();
    NVS_UNLOCK;
}
