#include "service.h"

Service::Service(const char* name) : name(name), taskHandle(NULL), stackSize(8192) {
    Preferences prefs;
    prefs.begin(name, true);
    enabled = prefs.getBool("enabled", false);
    prefs.end();
}

Service::~Service() {
}
bool Service::getEnabled() {
    return enabled;
}

void Service::setEnabled(bool enabled) {
    this->enabled = enabled;
    Preferences prefs;
    prefs.begin(name, false);
    prefs.putBool("enabled", enabled);
    prefs.end();
}

void Service::start() {
    lilka::serial.log(K_S_SERVICE_STARTUP_FMT, name);
    xTaskCreate(_run, name, stackSize, this, 1, &taskHandle);
}

void Service::_run(void* arg) {
    Service* service = static_cast<Service*>(arg);
    service->run();
    lilka::serial.err(K_S_SERVICE_DIE_FMT, service->name);
}

void Service::setStackSize(uint32_t stackSize) {
    this->stackSize = stackSize;
}
