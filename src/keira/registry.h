#pragma once
//////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
//////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include <vector>

#define REG_ENTRY_NAME_LEN NVS_NAMESPACE_LEN

class KeiraSystem;
extern KeiraSystem ksystem;

class Service;
typedef Service* (*ServiceConstructorFunc)();

typedef void* (*RegistryUnitContructorFunc)();

// entry->type
typedef enum {
    KREG_APP,
    KREG_SERVICE,
    //KREG_NVS
} KeiraRegistryEntryType;

// Service registration macro
#define REG_SERVICE(NAME, CLASS, DEFAULT_ENABLED)                                                         \
    Service* createService##NAME() {                                                                      \
        auto pService = new CLASS();                                                                      \
        pService->setName(#NAME);                                                                         \
        return pService;                                                                                  \
    }                                                                                                     \
    __attribute__((constructor(3100))) void registerService##NAME() {                                     \
        auto cfg = new KeiraConfig(#NAME);                                                                \
        KeiraConfigEntry enCfgEntry = {                                                                   \
            .key = "enabled", .description = "Enable service", .type = KCONFIG_BOOL, .b = DEFAULT_ENABLED \
        };                                                                                                \
        cfg->init(enCfgEntry);                                                                            \
        KeiraRegistryEntry regEntry = {                                                                   \
            .name = #NAME,                                                                                \
            .type = KREG_SERVICE,                                                                         \
            .constructor = reinterpret_cast<RegistryUnitContructorFunc>(createService##NAME),             \
            .config = cfg                                                                                 \
        };                                                                                                \
        ksystem.registry.reg(regEntry);                                                                   \
    }
// TODO: Reuse provided by KeiraConfig macro to register enabled

typedef struct {
    const char* name; // name of app/service task, scope of settings
    KeiraRegistryEntryType type;
    RegistryUnitContructorFunc constructor;
    KeiraConfig* config; // can be null

} KeiraRegistryEntry;

class KeiraRegistry {
public:
    KeiraRegistry();
    ~KeiraRegistry();
    void lock();
    void unlock();

    void reg(KeiraRegistryEntry& entry);

    KeiraRegistryEntry* operator[](const char* name);
    std::vector<KeiraRegistryEntry>::iterator begin() {
        return entries.begin();
    }
    std::vector<KeiraRegistryEntry>::iterator end() {
        return entries.end();
    }

private:
    std::vector<KeiraRegistryEntry> entries;
    SemaphoreHandle_t registryMtx = xSemaphoreCreateMutex();
};