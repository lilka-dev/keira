#pragma once
//////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
//////////////////////////////////////////////////////////////////////////////
// Unified class for all NVS settings stored across KeiraOS
/////////////////////////////////////////////////////////////////////////////
// Purpose of this component to unify all NVS settings used by Apps/Services
// providing UI to work with them
/////////////////////////////////////////////////////////////////////////////

#include <lilka/ui.h>
#include <nvs.h>
#include "keira/mutex.h"

#define NVS_NAMESPACE_LEN 16
#define MAX_STRING_LEN    256

#define REG_CONFIG(SCOPE, NAME, TYPE, DESCRIPTION, DEFAULT_VALUE)                 \
    __attribute__((constructor(3200))) void registerConfigEntry##SCOPE_##NAME() { \
        KeiraRegistryEntry* entry = ksystem.registry[#SCOPE];                     \
        if (!entry || !entry->config) return;                                     \
        KeiraConfigEntry cfgEntry = {                                             \
            .key = #NAME,                                                         \
            .description = DESCRIPTION,                                           \
            .type = TYPE,                                                         \
            .onClick = nullptr,                                                   \
            .onClickData = nullptr,                                               \
        };                                                                        \
        switch (TYPE) {                                                           \
            case KCONFIG_BOOL:                                                    \
                cfgEntry.b = (bool)DEFAULT_VALUE;                                 \
                break;                                                            \
            case KCONFIG_STRING:                                                  \
                cfgEntry.s = String(DEFAULT_VALUE);                               \
                break;                                                            \
            case KCONFIG_INT:                                                     \
                cfgEntry.i = (int32_t)DEFAULT_VALUE;                              \
                break;                                                            \
            case KCONFIG_UINT:                                                    \
                cfgEntry.u = (uint32_t)DEFAULT_VALUE;                             \
                break;                                                            \
            case KCONFIG_INT64:                                                   \
                cfgEntry.i64 = (int64_t)DEFAULT_VALUE;                            \
                break;                                                            \
            case KCONFIG_UINT64:                                                  \
                cfgEntry.u64 = (uint64_t)DEFAULT_VALUE;                           \
                break;                                                            \
        }                                                                         \
        entry->config->init(cfgEntry);                                            \
    }

// lol, theoretically we can save float as well, cause NVS do not know a thing about types
typedef enum {
    KCONFIG_BOOL,
    KCONFIG_STRING,
    KCONFIG_INT,
    KCONFIG_INT64,
    KCONFIG_UINT,
    KCONFIG_UINT64
} KeiraConfigEntryType;

// return false means menu finished
typedef void (*KeiraConfigEntryCallback)(void* clbkData);

typedef struct {
    // BASE =>
    const char* key;
    const char* description;
    KeiraConfigEntryType type;
    // Callbacks
    KeiraConfigEntryCallback onClick;
    void* onClickData;
    // Entry value
    String s; // empty string if not it
    union {
        bool b;
        int32_t i;
        int64_t i64;
        uint32_t u;
        uint64_t u64;
    };
} KeiraConfigEntry;

class KeiraConfig {
public:
    ~KeiraConfig();
    // Constructs KeiraConfig
    // @param scope NVS scope to be used by KeiraConfig
    explicit KeiraConfig(const char* scope);

    // Retrieves a pointer to lilka::Menu
    lilka::Menu* getMenu();
    // Adds entry, setups inital value in NVS if it doesn't exist
    // or
    // @param entry pointer to KeiraConfigEntry
    void init(KeiraConfigEntry& entry);
    // Validate value, stores to NVS

    bool set(KeiraConfigEntry& entry);

    // checks if key exists
    bool isKey(const char* key);

    // Returns KeiraConfigEntry
    KeiraConfigEntry operator[](const char* key);
    // TODO: dump settings to json, load settings from json
private:
    // Rebuild configMenu from entries
    void rebuildMenu();

    std::vector<KeiraConfigEntry> entries;

    bool menuDirty = false;
    lilka::Menu configMenu;
    // Protects configMenu and entries
    SemaphoreHandle_t configMtx = xSemaphoreCreateMutex();

    char scope[NVS_NAMESPACE_LEN];
};