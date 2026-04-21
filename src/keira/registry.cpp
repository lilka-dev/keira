#include "registry.h"
#include "keira/mutex.h"

KeiraRegistry::KeiraRegistry() {
}

KeiraRegistry::~KeiraRegistry() {
    // Cleanup configs for unit
    lock();
    for (auto& entry : entries) {
        delete entry.config;
    }
    unlock();
    // Remove semaphore
    vSemaphoreDelete(registryMtx);
}

void KeiraRegistry::lock() {
    KMTX_LOCK(registryMtx);
}
void KeiraRegistry::unlock() {
    KMTX_UNLOCK(registryMtx);
}

KeiraRegistryEntry* KeiraRegistry::operator[](const char* name) {
    lock();

    for (auto& entry : entries) {
        if (strcmp(entry.name, name) == 0) {
            unlock();
            return &entry;
        }
    }

    unlock();

    return NULL;
}

void KeiraRegistry::reg(KeiraRegistryEntry& entry) {
    lock();
    entries.push_back(entry);
    unlock();
}
