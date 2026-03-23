#include "keira/config.h"
#include "keira/keira_lang.h"
#include "keira/ksystem.h"

static inline esp_err_t nvsWrite(nvs_handle_t handle, KeiraConfigEntry* entry) {
    size_t size = entry->type == KCONFIG_STRING ? entry->s.length() + 1 : sizeof(entry->u64);
    return nvs_set_blob(handle, entry->key, &entry->b, size);
}

static inline esp_err_t nvsRead(nvs_handle_t handle, KeiraConfigEntry* entry) {
    size_t size = entry->type == KCONFIG_STRING ? entry->s.length() : sizeof(entry->u64);
    return nvs_get_blob(handle, entry->key, &entry->b, &size);
}

KeiraConfig::~KeiraConfig() {
    vSemaphoreDelete(configMtx);
}

KeiraConfig::KeiraConfig(const char* scope) {
    if (strlen(scope) + 1 > NVS_NAMESPACE_LEN) {
        // assert here
    }
    strcpy(this->scope, scope);
}

lilka::Menu* KeiraConfig::getMenu() {
    KMTX_LOCK(configMtx);
    if (menuDirty) {
        rebuildMenu();
        menuDirty = false;
    }
    KMTX_UNLOCK(configMtx);
    return &configMenu;
}

void KeiraConfig::init(KeiraConfigEntry entry) {
    NVS_LOCK;
    // We can't place it as a field on class, cause esp idf doesn't support more than 16 open handles
    nvs_handle_t handle;
    if (nvs_open(scope, NVS_READWRITE, &handle) != ESP_OK) {
        NVS_UNLOCK;
        return;
    }

    if (nvsRead(handle, &entry) != ESP_OK) {
        nvsWrite(handle, &entry);
        nvs_commit(handle);
    }

    nvs_close(handle);
    NVS_UNLOCK;

    KMTX_LOCK(configMtx);
    entries.push_back(entry);
    KMTX_UNLOCK(configMtx);

    menuDirty = true;
}

bool KeiraConfig::set(KeiraConfigEntry entry) {
    NVS_LOCK;
    nvs_handle_t handle;
    if (nvs_open(scope, NVS_READWRITE, &handle) != ESP_OK) {
        NVS_UNLOCK;
        return false;
    }
    nvsWrite(handle, &entry);
    nvs_commit(handle);
    nvs_close(handle);
    NVS_UNLOCK;

    KMTX_LOCK(configMtx);
    for (auto& ent : entries) {
        if (strcmp(ent.key, entry.key) == 0) {
            memcpy(&ent.b, &entry.b, sizeof(KeiraConfigEntry) - offsetof(KeiraConfigEntry, b));
            break;
        }
    }
    KMTX_UNLOCK(configMtx);

    menuDirty = true;
    return true;
}

bool KeiraConfig::isKey(const char* key) {
    KMTX_LOCK(configMtx);
    for (auto& entry : entries) {
        if (strcmp(entry.key, key) == 0) {
            KMTX_UNLOCK(configMtx);
            return true;
        }
    }
    KMTX_UNLOCK(configMtx);
    return false;
}

KeiraConfigEntry KeiraConfig::operator[](const char* key) {
    KeiraConfigEntry foundEntry = {};
    KMTX_LOCK(configMtx);
    for (auto& entry : entries) {
        if (strcmp(entry.key, key) == 0) {
            foundEntry = entry;
            break;
        }
    }
    KMTX_UNLOCK(configMtx);
    return foundEntry;
}

void KeiraConfig::rebuildMenu() {
    configMenu.clearItems();
    for (auto& entry : entries) {
        String postfix;
        switch (entry.type) {
            case KCONFIG_BOOL:
                postfix = entry.b ? K_S_ON : K_S_OFF;
                break;
            case KCONFIG_STRING:
                postfix = String(entry.s);
                break;
            case KCONFIG_INT:
                postfix = String(entry.i);
                break;
            case KCONFIG_INT64:
                postfix = String((long)entry.i64);
                break;
            case KCONFIG_UINT:
                postfix = String(entry.u);
                break;
            case KCONFIG_UINT64:
                postfix = String((unsigned long)entry.u64);
                break;
        }
        configMenu.addItem(
            entry.description, nullptr, lilka::colors::White, postfix.c_str(), entry.onClick, entry.onClickData
        );
    }
}