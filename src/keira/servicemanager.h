#pragma once
#include "keira/bits/servicemanager.h"

#include "keira/threadmanager.h"

#include <vector>

// Uncomment this line to get debugging information
//#define KEIRA_SERVICEMANAGER_DEBUG


// Circular dependency
class Service;

typedef Service* (*ServiceConstructorFunc)();
// Service registration macro
#define REG_SERVICE(NAME, CLASS, DEFAULT_ENABLED)                                                             \
    Service* createService##CLASS() {                                                                         \
        auto pService = new CLASS();                                                                          \
        pService->setName(NAME);                                                                              \
        return pService;                                                                                      \
    }                                                                                                         \
    __attribute__((constructor(3100))) void registerService##CLASS() {                                        \
        ksystem.services.registerService(                                                                     \
            {.name = NAME, .constructor = createService##CLASS, .enabled = DEFAULT_ENABLED, .pService = NULL} \
        );                                                                                                    \
    }


typedef struct {
    // Thread name
    const char* name;
    ServiceConstructorFunc constructor;
    // precached NVS value
    bool enabled;
    Service* pService; //
} ServiceEntry;


class ServiceManager : public ThreadManager {
public:
    ~ServiceManager();
    ServiceManager();

    // Retrives enabled state from service table for a service, note, this doesn't really mean that
    // service is actually runing
    bool getEnabled(const char* name);

    void setEnabled(const char* name, bool enabled);

    // Registers service in service table
    void registerService(ServiceEntry serviceEntry);

    // Runs services in service table according to NVS[service].enabled flag,
    // and does services launch/relaunch
    void run() override;

private:
    ServiceEntry* findServiceEntry(const char* name);

    static bool getEnabledFromNVS(const char* name, bool doInit = false, bool defaultValue = false);

    // List of registered in KeiraOS service constructors
    std::vector<ServiceEntry> serviceTable;

    SemaphoreHandle_t serviceTableMtx = xSemaphoreCreateMutex();
};
