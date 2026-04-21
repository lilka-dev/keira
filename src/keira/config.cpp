#include "keira/config.h"
#include "keira/keira_lang.h"
#include "keira/ksystem.h"
#include <nvs_flash.h> // REQUIRES nvs_flash

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
    nvs_flash_init();
    if (strlen(scope) + 1 >= NVS_NAMESPACE_LEN) {
        // assert here
    }
    strcpy(this->scope, scope);
}

void KeiraConfig::init(KeiraConfigEntry& entry) {
    NVS_LOCK;
    // We can't place it as a field on class, cause esp idf doesn't support more than 16 open handles
    nvs_handle_t handle;
    if (nvs_open(scope, NVS_READWRITE, &handle) != ESP_OK) {
        NVS_UNLOCK;
        lilka::serial.err("Can't open nvs for %s", scope);
        return;
    }

    if (nvsRead(handle, &entry) != ESP_OK) {
        lilka::serial.err("Can't read setting for %s[%s]", scope, entry.key);
        nvsWrite(handle, &entry);
        nvs_commit(handle);
    } else {
        lilka::serial.log("Read setting for %s[%s]", scope, entry.key);
    }

    nvs_close(handle);
    NVS_UNLOCK;

    KMTX_LOCK(configMtx);
    entries.push_back(entry);
    KMTX_UNLOCK(configMtx);
}

bool KeiraConfig::set(KeiraConfigEntry& entry) {
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