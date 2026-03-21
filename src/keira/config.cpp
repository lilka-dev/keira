#include "keira/config.h"

static inline esp_err_t nvsWrite(nvs_handle_t handle, KeiraConfigEntry* entry) {
    size_t size = entry->type == KCONFIG_STRING ? strlen(entry->s.value) + 1 : sizeof(entry->u64.value);
    return nvs_set_blob(handle, entry->desc.key, &entry->b.value, size);
}

static inline esp_err_t nvsRead(nvs_handle_t handle, KeiraConfigEntry* entry) {
    size_t size = entry->type == KCONFIG_STRING ? NVS_STRING_LEN : sizeof(entry->u64.value);
    return nvs_get_blob(handle, entry->desc.key, &entry->b.value, &size);
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

lilka::Menu* getMenu() {
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
    bool entryValid =
        (!entry.onValidate) || entry.onValidate({.menu = NULL, .entry = &entry, .data = entry.onValidateData});
    if (!entryValid) return false;

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
        if (strcmp(ent.desc.key, entry.desc.key) == 0) {
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
        if (strcmp(entry.desc.key, key) == 0) {
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
        if (strcmp(entry.desc.key, key) == 0) {
            foundEntry = entry;
            break;
        }
    }
    KMTX_UNLOCK(configMtx);
    return foundEntry;
}

void KeiraConfig::rebuildMenu() {
    KMTX_LOCK(configMtx);
    configMenu.clearItems();
    for (auto& entry : entries) {
    }

    KMTX_UNLOCK(configMtx);
}