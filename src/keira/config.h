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

// lol, theoretically we can save float as well, cause NVS do not know a thing about types
typedef enum {
    KCONFIG_BOOL,
    KCONFIG_STRING,
    KCONFIG_INT,
    KCONFIG_INT64,
    KCONFIG_UINT,
    KCONFIG_UINT64
} KeiraConfigEntryType;

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
typedef bool (*KeiraConfigEntryCallback)(KeiraConfigCallbackData* clbkData);

// return false means no changes
typedef bool (*KeiraConfigEntryOnValidateCallback)(KeiraConfigCallbackData* clbkData);

typedef struct {
    // Values Set by user
    KeiraConfigDescription desc;
    KeiraConfigEntryType type;
    KeiraConfigEntryOnValidateCallback onValidate;
    KeiraConfigEntryCallback onClick;
    void* onClickData;
    void* onValidateData;
    union {
        union {
            bool value;
            bool def;
        } b;
        union {
            char value[NVS_STRING_LEN];
            char def[NVS_STRING_LEN];
        } s;
        union {
            int32_t value;
            int32_t def;
        } i;
        union {
            int64_t value;
            int64_t def;
        } i64;
        union {
            uint32_t value;
            uint32_t def;
        } u;
        union {
            uint64_t value;
            uint64_t def;
        } u64;
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
    void init(KeiraConfigEntry entry);
    // Validate value, stores to NVS

    bool set(KeiraConfigEntry entry);

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