#include "keira/config.h"
#include "keira/keira_lang.h"
#include "keira/ksystem.h"

static inline esp_err_t nvsWrite(nvs_handle_t handle, KeiraConfigEntry* entry) {
    switch (entry->type) {
        case KCONFIG_BOOL:
            return nvs_set_u8(handle, entry->key, (uint8_t)entry->b);
        case KCONFIG_STRING:
            return nvs_set_str(handle, entry->key, entry->s.c_str());
        case KCONFIG_INT:
            return nvs_set_i32(handle, entry->key, entry->i);
        case KCONFIG_INT64:
            return nvs_set_i64(handle, entry->key, entry->i64);
        case KCONFIG_UINT:
            return nvs_set_u32(handle, entry->key, entry->u);
        case KCONFIG_UINT64:
            return nvs_set_u64(handle, entry->key, entry->u64);
        default:
            return ESP_ERR_INVALID_ARG;
    }
}

static inline esp_err_t nvsRead(nvs_handle_t handle, KeiraConfigEntry* entry) {
    switch (entry->type) {
        case KCONFIG_BOOL: {
            uint8_t val;
            esp_err_t err = nvs_get_u8(handle, entry->key, &val);
            if (err == ESP_OK) entry->b = (bool)val;
            return err;
        }
        case KCONFIG_STRING: {
            char buf[MAX_STRING_LEN];
            size_t len = sizeof(buf);
            esp_err_t err = nvs_get_str(handle, entry->key, buf, &len);
            if (err == ESP_OK) entry->s = String(buf);
            return err;
        }
        case KCONFIG_INT: {
            esp_err_t err = nvs_get_i32(handle, entry->key, &entry->i);
            return err;
        }
        case KCONFIG_INT64: {
            esp_err_t err = nvs_get_i64(handle, entry->key, &entry->i64);
            return err;
        }
        case KCONFIG_UINT: {
            esp_err_t err = nvs_get_u32(handle, entry->key, &entry->u);
            return err;
        }
        case KCONFIG_UINT64: {
            esp_err_t err = nvs_get_u64(handle, entry->key, &entry->u64);
            return err;
        }
        default:
            return ESP_ERR_INVALID_ARG;
    }
}

KeiraConfig::~KeiraConfig() {
    vSemaphoreDelete(configMtx);
}

KeiraConfig::KeiraConfig(const char* scope) {
    if (strlen(scope) + 1 >= NVS_NAMESPACE_LEN) {
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
            if (entry.type == KCONFIG_STRING) {
                ent.s = entry.s;
            } else {
                memcpy(&ent.b, &entry.b, sizeof(KeiraConfigEntry) - offsetof(KeiraConfigEntry, b));
            }
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

        if (entry.onClick)
            configMenu.addItem(
                entry.description, nullptr, lilka::colors::White, postfix.c_str(), entry.onClick, entry.onClickData
            );
    }
}