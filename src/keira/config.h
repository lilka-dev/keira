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
#define NVS_KEY_LEN       16
#define NVS_STRING_LEN    64

typedef enum { KCONFIG_BOOL, KCONFIG_STRING, KCONFIG_INT, KCONFIG_UINT } KeiraConfigEntryType;

// Forward declaration (circular dependency)
typedef struct KeiraConfigEntry KeiraConfigEntry;

typedef struct {
    char key[NVS_KEY_LEN];
    const char* label;
} KeiraConfigDescription;

typedef struct {
    lilka::Menu* menu; // Can be null on validation check if value set from non ui context
    KeiraConfigEntry* entry;
    void* data;
} KeiraConfigCallbackData;

// return false means menu finished
typedef bool (*KeiraConfigEntryCallback)(KeiraConfigCallbackData* entry);

// return false means no changes
typedef bool (*KeiraConfigEntryOnValidateCallback)(KeiraConfigCallbackData* data);

typedef struct {
    KeiraConfigDescription desc;
    KeiraConfigEntryType type;
    KeiraConfigEntryOnValidateCallback onValidate;
    KeiraConfigEntryCallback onClick;
    union {
        struct {
            bool value;
            bool def;
        } b;
        struct {
            char value[NVS_STRING_LEN];
            const char* def;
        } s;
        struct {
            int32_t value;
            int32_t def;
        } i;
        struct {
            uint32_t value;
            uint32_t def;
        } u;
    };
} KeiraConfigEntry;

class KeiraConfig {
public:
    explicit KeiraConfig(const char* scope);
    ~KeiraConfig();
    // UI
    lilka::Menu* getMenu();
    // Adds entry, setups inital value in NVS
    void add(KeiraConfigEntry* entry);
    // Validate value, stores to NVS
    bool set(KeiraConfigEntry* entry);

    KeiraConfigEntry* operator[](const char* key);
    // TODO: dump settings to json, load settings from json
private:
    // load value from nvs done on add
    void load(KeiraConfigEntry* entry);

    // Rebuild configMenu from entries
    void rebuildMenu();

    std::vector<KeiraConfigEntry> entries;

    //

    nvs_handle_t nvsHandle;
    lilka::Menu configMenu;
    SemaphoreHandle_t configMtx = xSemaphoreCreateMutex();

    char scope[NVS_NAMESPACE_LEN];
};