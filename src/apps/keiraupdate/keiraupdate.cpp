#include "keiraupdate.h"
#include "keira/keira.h"
#include "keira/ksystem.h"
#include "keira/keira_version_auto_gen.h"
#include "keira/utils/mem.h"
#include "keira/utils/string.h"
#include "services/network/network.h"

#include <ArduinoJson.h>
#include <esp_http_client.h>
#include <esp_https_ota.h>

// Arduino WiFiClientSecure ships its own esp_crt_bundle.h which shadows esp-idf one,
// so declare esp-idf certificate bundle hook directly
extern "C" esp_err_t esp_crt_bundle_attach(void* conf);

KeiraUpdateApp::KeiraUpdateApp() : App("KeiraUpdate") {
    setktStackSize(16384); // TLS handshake is stack hungry
}

String KeiraUpdateApp::currentLanguage() {
    String lang = K_S_CURRENT_LANGUAGE_SHORT;
    lang.toUpperCase();
    return "LANG_" + lang;
}

String KeiraUpdateApp::currentVersion() {
    return StringFormat("%d.%d.%d", KEIRA_VERSION_MAJOR, KEIRA_VERSION_MINOR, KEIRA_VERSION_PATCH);
}

void KeiraUpdateApp::run() {
    NetworkService* networkService = static_cast<NetworkService*>(ksystem.services["network"]);
    if (networkService->getnetworkState() != NETWORK_STATE_ONLINE) {
        alert(K_S_ERROR, K_S_LAUNCHER_ENABLE_WIFI_FIRST);
        return;
    }

    lilka::ProgressDialog dialog(K_S_KEIRA_UPDATE, K_S_KEIRA_UPDATE_CHECKING);
    dialog.draw(canvas);
    queueDraw();

    String json;
    if (!fetchManifest(json)) {
        alert(K_S_ERROR, K_S_KEIRA_UPDATE_FETCH_FAILED);
        return;
    }
    if (!parseManifest(json)) {
        alert(K_S_ERROR, K_S_KEIRA_UPDATE_PARSE_FAILED);
        return;
    }
    if (versions.empty()) {
        alert(K_S_KEIRA_UPDATE, K_S_KEIRA_UPDATE_NO_VERSIONS);
        return;
    }

    const String current = currentVersion();
    lilka::Menu menu(K_S_KEIRA_UPDATE);
    menu.addActivationButton(K_BTN_BACK);
    for (size_t i = 0; i < versions.size(); i++) {
        const FirmwareVersion& fw = versions[i];
        String postfix;
        if (fw.version == current) postfix = K_S_KEIRA_UPDATE_CURRENT;
        else if (i == 0) postfix = K_S_KEIRA_UPDATE_LATEST;
        else if (fw.prerelease) postfix = K_S_KEIRA_UPDATE_PRERELEASE;
        menu.addItem("v" + fw.version, nullptr, lilka::colors::White, postfix);
    }
    menu.addItem(K_S_MENU_BACK);

    while (1) {
        while (!menu.isFinished()) {
            menu.update();
            menu.draw(canvas);
            queueDraw();
        }
        if (menu.getButton() == K_BTN_BACK) return;
        int16_t index = menu.getCursor();
        if (index < 0 || index >= static_cast<int16_t>(versions.size())) return;

        const FirmwareVersion& fw = versions[index];
        String description = StringFormat(
            K_S_KEIRA_UPDATE_CONFIRM_FMT,
            fw.version.c_str(),
            current.c_str(),
            fw.language.c_str(),
            lilka::fileutils.getHumanFriendlySize(fw.size).c_str(),
            fw.date.substring(0, 10).c_str()
        );
        if (confirm(K_S_KEIRA_UPDATE, description)) {
            install(fw);
        }
    }
}

static esp_err_t manifestHttpEvent(esp_http_client_event_t* evt) {
    // Skip bodies of redirect responses, only final response is a manifest
    if (evt->event_id == HTTP_EVENT_ON_DATA && esp_http_client_get_status_code(evt->client) == 200) {
        String* out = static_cast<String*>(evt->user_data);
        if (out->length() + evt->data_len > KEIRA_UPDATE_MANIFEST_MAX) return ESP_FAIL;
        out->concat(static_cast<const char*>(evt->data), evt->data_len);
    }
    return ESP_OK;
}

bool KeiraUpdateApp::fetchManifest(String& out) {
    out = "";
    esp_http_client_config_t config = {};
    config.url = KEIRA_UPDATE_MANIFEST_URL;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    config.timeout_ms = KEIRA_UPDATE_HTTP_TIMEOUT;
    config.buffer_size = KEIRA_UPDATE_RX_BUFFER;
    config.buffer_size_tx = KEIRA_UPDATE_TX_BUFFER;
    config.event_handler = manifestHttpEvent;
    config.user_data = &out;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) return false;
    esp_err_t err = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    if (err != ESP_OK || status != 200) {
        lilka::serial.err("Keira update: manifest fetch failed, err=%s, status=%d", esp_err_to_name(err), status);
        return false;
    }
    return !out.isEmpty();
}

// Missing keys are converted to "null" by as<String>(), use empty string instead
static String jsonStr(JsonVariantConst value) {
    const char* str = value.as<const char*>();
    return str ? String(str) : String();
}

bool KeiraUpdateApp::parseManifest(const String& json) {
    JsonDocument doc(&spiRamAllocator);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        lilka::serial.err("Keira update: manifest parse failed: %s", error.c_str());
        return false;
    }

    const String lang = currentLanguage();
    versions.clear();
    for (JsonObject entry : doc["versions"].as<JsonArray>()) {
        JsonObject firmware = entry["firmware"].as<JsonObject>();
        if (firmware.isNull() || firmware.size() == 0) continue;

        FirmwareVersion fw;
        fw.version = jsonStr(entry["version"]);
        fw.tag = jsonStr(entry["tag"]);
        fw.prerelease = entry["prerelease"].as<bool>();
        fw.date = jsonStr(entry["date"]);

        // Prefer firmware built for the current language
        JsonObject bin = firmware[lang].as<JsonObject>();
        if (!bin.isNull()) {
            fw.language = lang;
        } else {
            JsonPair first = *firmware.begin();
            fw.language = first.key().c_str();
            bin = first.value().as<JsonObject>();
        }
        fw.url = jsonStr(bin["url"]);
        fw.size = bin["size"].as<uint32_t>();

        if (fw.version.isEmpty() || fw.url.isEmpty()) continue;
        versions.push_back(fw);
    }
    return true;
}

void KeiraUpdateApp::install(const FirmwareVersion& fw) {
    lilka::ProgressDialog dialog(K_S_KEIRA_UPDATE, StringFormat(K_S_KEIRA_UPDATE_DOWNLOADING_FMT, fw.version.c_str()));
    dialog.draw(canvas);
    queueDraw();

    esp_http_client_config_t httpConfig = {};
    httpConfig.url = fw.url.c_str();
    httpConfig.crt_bundle_attach = esp_crt_bundle_attach;
    httpConfig.timeout_ms = KEIRA_UPDATE_HTTP_TIMEOUT;
    httpConfig.buffer_size = KEIRA_UPDATE_RX_BUFFER;
    httpConfig.buffer_size_tx = KEIRA_UPDATE_TX_BUFFER;
    httpConfig.keep_alive_enable = true;

    esp_https_ota_config_t otaConfig = {};
    otaConfig.http_config = &httpConfig;

    esp_https_ota_handle_t handle = nullptr;
    esp_err_t err = esp_https_ota_begin(&otaConfig, &handle);
    if (err != ESP_OK) {
        alert(K_S_ERROR, StringFormat(K_S_KEIRA_UPDATE_ERROR_FMT, esp_err_to_name(err)));
        return;
    }

    int total = esp_https_ota_get_image_size(handle);
    if (total <= 0) total = fw.size;
    int lastProgress = -1;

    while ((err = esp_https_ota_perform(handle)) == ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
        if (lilka::controller.getState().b.justPressed) {
            esp_https_ota_abort(handle);
            return;
        }
        if (total > 0) {
            int progress = esp_https_ota_get_image_len_read(handle) * 100LL / total;
            if (progress != lastProgress) {
                lastProgress = progress;
                dialog.setProgress(progress);
                dialog.draw(canvas);
                queueDraw();
            }
        }
    }

    if (err != ESP_OK || !esp_https_ota_is_complete_data_received(handle)) {
        esp_https_ota_abort(handle);
        alert(K_S_ERROR, StringFormat(K_S_KEIRA_UPDATE_ERROR_FMT, esp_err_to_name(err)));
        return;
    }

    // Validates image and switches boot partition, KeiraSystem::verifyOTA() approves it on next boot
    err = esp_https_ota_finish(handle);
    if (err != ESP_OK) {
        alert(K_S_ERROR, StringFormat(K_S_KEIRA_UPDATE_ERROR_FMT, esp_err_to_name(err)));
        return;
    }

    alert(K_S_KEIRA_UPDATE, K_S_KEIRA_UPDATE_SUCCESS);
    esp_restart();
}
