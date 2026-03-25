#include "keira/service.h"
#include <assert.h>

Service::Service() {
    setktStackSize(4096);
    setktPriority(KT_PRIO_IDLE);
    setktType(KT_SERVICE);
}

KeiraConfig& Service::getConfig() {
    auto* entry = ksystem.registry[getName()];
    assert(entry && entry->config);
    return *entry->config;
}