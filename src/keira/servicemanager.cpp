#include "keira/servicemanager.h"
#include "keira/ksystem.h"

ServiceManager::~ServiceManager() {
}

ServiceManager::ServiceManager() {
    setktCore(KEIRA_SERVICEMANAGER_CORE);
}

void ServiceManager::up(const char* name) {
    // Check if already running
    if (status(name)) return;

    // Get registry entry
    KeiraRegistryEntry* entry = ksystem.registry[name];
    if (!entry || entry->type != KREG_SERVICE) return;

    // Check enabled in config
    if (!entry->config->isKey("enabled")) return;
    auto enabledEntry = (*entry->config)["enabled"];
    if (!enabledEntry.b) {
        // Update NVS — service explicitly started means enable it
        enabledEntry.b = true;
        entry->config->set(enabledEntry);
    }

    // Construct and spawn
    ServiceConstructorFunc ctor = (ServiceConstructorFunc)entry->constructor;
    Service* svc = ctor();
    spawn(KT_PCAST(svc));
}

void ServiceManager::down(const char* name) {
    // Find running instance
    KeiraThread* thread = operator[](name);
    if (!thread) return;

    // Update config — service explicitly stopped means disable it
    KeiraRegistryEntry* entry = ksystem.registry[name];
    if (entry && entry->config) {
        auto enabledEntry = (*entry->config)["enabled"];
        enabledEntry.b = false;
        entry->config->set(enabledEntry);
    }

    thread->stop();
}

bool ServiceManager::status(const char* name) {
    KeiraThread* thread = operator[](name);
    return K_PTHREAD_CHECK(thread);
}

void ServiceManager::run() {
    // Iterate registry, spawn autostart services
    ksystem.registry.lock();
    for (auto& entry : ksystem.registry) {
        if (entry.type != KREG_SERVICE) continue;
        if (!entry.config) continue;

        auto enabledEntry = (*entry.config)["enabled"];
        if (enabledEntry.b) {
            ServiceConstructorFunc ctor = (ServiceConstructorFunc)entry.constructor;
            Service* svc = ctor();
            spawn(KT_PCAST(svc));
        }
    }
    ksystem.registry.unlock();

    // Handle threads
    ThreadManager::run();
}