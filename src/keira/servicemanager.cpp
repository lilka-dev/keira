#include "keira/servicemanager.h"
#include "keira/ksystem.h"
#include "keira/mutex.h"

#include <Preferences.h>

ServiceManager::ServiceManager() {
    setktCore(KEIRA_SERVICEMANAGER_CORE);
}

bool ServiceManager::getEnabledFromNVS(const char* name, bool doInit, bool defaultValue) {
    bool enabled = false;

    NVS_LOCK;

    Preferences prefs;
    prefs.begin(name, false); // read-write
    if ((!prefs.isKey("enabled")) && doInit) {
        // Setup inital value
        prefs.putBool("enabled", defaultValue);
    } else {
        // Use stored value
        enabled = prefs.getBool("enabled");
    }
    prefs.end();

    NVS_UNLOCK;

    return enabled;
}

ServiceEntry* ServiceManager::findServiceEntry(const char* name) {
    ServiceEntry* entry = NULL;

    KMTX_LOCK(serviceTableMtx);
    for (auto& serviceTableEntry : serviceTable) {
        if (strcmp(name, serviceTableEntry.name) == 0) entry = &serviceTableEntry;
    }
    KMTX_UNLOCK(serviceTableMtx);

    return entry;
}

bool ServiceManager::getEnabled(const char* name) {
    // K_SM_DBG LEP;

    auto pServiceEntry = findServiceEntry(name);

    if (pServiceEntry) {
        KMTX_LOCK(serviceTableMtx);

        auto enabled = pServiceEntry->enabled;

        KMTX_UNLOCK(serviceTableMtx);

        // K_SM_DBG LXP;
        return enabled;
    }

    // K_SM_DBG LXP;
    return false;
}

void ServiceManager::setEnabled(const char* name, bool enabled) {
    K_SM_DBG LEP;

    auto pServiceTableEntry = findServiceEntry(name);

    // No serviceEntry found in serviceTable
    if (!pServiceTableEntry) return;

    // Check if changes needed
    KMTX_LOCK(serviceTableMtx);
    if (enabled == pServiceTableEntry->enabled) {
        KMTX_UNLOCK(serviceTableMtx);
        return;
    }

    // Change NVS enabled value
    NVS_LOCK;
    Preferences prefs;
    prefs.begin(name, false);
    prefs.putBool("enabled", enabled);
    prefs.end();
    NVS_UNLOCK;

    // Change enabled in service table
    pServiceTableEntry->enabled = enabled;

    // Ensure no changes to threads needed
    if (enabled == (pServiceTableEntry->pService != NULL)) {
        KMTX_UNLOCK(serviceTableMtx);
        K_SM_DBG LXP;
        return;
    }

    // Spawn/delete service
    if (enabled) {
        pServiceTableEntry->pService = pServiceTableEntry->constructor();
        spawn(KT_PCAST(pServiceTableEntry->pService));
    } else {
        if (pServiceTableEntry->pService) {
            K_SM_DBG lilka::serial.log(
                "[ServiceManager] Stoping service %s/%p [ Autorun:%d ]",
                pServiceTableEntry->name,
                pServiceTableEntry->constructor,
                pServiceTableEntry->enabled
            );
            KT_PCAST(pServiceTableEntry->pService)->stop();
            pServiceTableEntry->pService = NULL;
        }
    }

    KMTX_UNLOCK(serviceTableMtx);

    K_SM_DBG LXP;
}

// TODO: Lua script as a service
void ServiceManager::registerService(ServiceEntry serviceEntry) {
    K_SM_DBG LEP;
    // We've to be sure scheduler is runing before using any mutexes
    bool mutexAvailable = (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED);

    // Check NVS for stored enabled state, fall back to default
    if (mutexAvailable) serviceEntry.enabled = getEnabledFromNVS(serviceEntry.name, true, serviceEntry.enabled);

    // Append entry to table
    if (mutexAvailable) KMTX_LOCK(serviceTableMtx);
    K_SM_DBG lilka::serial.log(
        "[ServiceManager] Registering service %s/%p [ Autorun:%d ]",
        serviceEntry.name,
        serviceEntry.constructor,
        serviceEntry.enabled
    );
    serviceTable.push_back(serviceEntry);
    if (mutexAvailable) KMTX_UNLOCK(serviceTableMtx);

    K_SM_DBG LXP;
}

void ServiceManager::run() {
    K_SM_DBG LEP;

    K_SM_DBG lilka::serial.log("Service table have %d services", serviceTable.size());

    // Load enabled from NVS
    KMTX_LOCK(serviceTableMtx);

    for (auto& serviceTableEntry : serviceTable)
        serviceTableEntry.enabled = getEnabledFromNVS(serviceTableEntry.name, true, serviceTableEntry.enabled);

    KMTX_UNLOCK(serviceTableMtx);

    // Spawn services depending on enabled
    KMTX_LOCK(serviceTableMtx);
    for (auto& serviceTableEntry : serviceTable) {
        if (serviceTableEntry.enabled) {
            serviceTableEntry.pService = serviceTableEntry.constructor();
            spawn(KT_PCAST(serviceTableEntry.pService));
            K_SM_DBG lilka::serial.log(
                "[ServiceManager] Spawning service %s/%p [ Enabled:%d ]",
                serviceTableEntry.name,
                serviceTableEntry.constructor,
                serviceTableEntry.enabled
            );
        }
    }
    KMTX_UNLOCK(serviceTableMtx);

    // Handle threads
    ThreadManager::run();

    K_SM_DBG LXP;
}
