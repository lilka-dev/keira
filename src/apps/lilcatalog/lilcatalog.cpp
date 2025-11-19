#include "lilcatalog.h"
#include <HTTPClient.h>
#include <lilka/config.h>
#include "utils/json.h"
#include <sys/stat.h>

#define BASIC_URL         "https://catalog.lilka.dev/apps/"
#define CATALOG_URL       "https://catalog.lilka.dev/apps/index_0.json"
#define CATALOG_BASE_PATH "/sdcard/catalog/apps/"

// Helper function to create directory recursively
static void mkdirRecursive(const char* path) {
    char tmp[256];
    char* p = nullptr;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

LilCatalogApp::LilCatalogApp() : App(K_S_LILCATALOG_APP) {
    setStackSize(8192);
}

void LilCatalogApp::run() {
}

void LilCatalogApp::processMenu() {
}

void LilCatalogApp::doShowCatalog() {
}

void LilCatalogApp::doShowCategory() {
}

void LilCatalogApp::doShowEntry() {
}

void LilCatalogApp::doShowEntryDescription() {
}

void LilCatalogApp::fetchCatalogIndex() {
    String catalogUrl = String(BASIC_URL) + "index_" + String(current_page) + ".json";

    HTTPClient http;
    http.begin(catalogUrl);
    int httpCode = http.GET();

    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            if (catalogDoc) {
                delete catalogDoc;
            }
            catalogDoc = new JsonDocument(&spiRamAllocator);
            DeserializationError error = deserializeJson(*catalogDoc, payload);
            if (error) {
                delete catalogDoc;
                catalogDoc = nullptr;
            }
        }
    }
    http.end();
}

void LilCatalogApp::parseCatalogIndex() {
    if (!catalogDoc || catalogDoc->isNull()) {
        return;
    }

    if ((*catalogDoc).containsKey("total_pages")) {
        num_pages = (*catalogDoc)["total_pages"];
    }

    if ((*catalogDoc).containsKey("manifests")) {
        JsonArray manifests = (*catalogDoc)["manifests"];
        num_entries = manifests.size();

        entries.clear();
        for (JsonVariant manifest : manifests) {
            catalog_entry entry;
            entry.manifestName = manifest.as<String>();
            entries.push_back(entry);
        }
    }
}

void LilCatalogApp::fetchManifestDetails() {
    if (entries.empty() || current_entry >= entries.size()) {
        return;
    }

    catalog_entry& entry = entries[current_entry];

    // Skip if already fetched
    if (!entry.name.isEmpty()) {
        return;
    }

    String manifestUrl = String(BASIC_URL) + entry.manifestName + "/index.json";

    HTTPClient http;
    http.begin(manifestUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        parseManifestDetails(payload, entry);
    }
    http.end();
}

void LilCatalogApp::parseManifestDetails(String payload, catalog_entry& entry) {
    JsonDocument manifestDoc(&spiRamAllocator);
    DeserializationError error = deserializeJson(manifestDoc, payload);

    if (!error) {
        entry.name = manifestDoc["name"].as<String>();
        entry.description = manifestDoc["description"].as<String>();
        entry.short_description = manifestDoc["short_description"].as<String>();
        entry.author = manifestDoc["author"].as<String>();

        if (manifestDoc.containsKey("icon_min")) {
            entry.icon_min = manifestDoc["icon_min"].as<String>();
        }

        if (manifestDoc.containsKey("executionfile")) {
            catalog_entry_file execFile;
            execFile.type = manifestDoc["executionfile"]["type"].as<String>();
            execFile.location = manifestDoc["executionfile"]["location"].as<String>();
            entry.executionfiles.push_back(execFile);
        }
    }
}
void LilCatalogApp::fetchIconForEntry() {
    if (entries.empty() || current_entry >= entries.size()) {
        return;
    }

    catalog_entry& entry = entries[current_entry];
    if (entry.icon_min.isEmpty()) {
        hasIcon = false;
        return;
    }

    // Determine cache filename based on icon_min extension
    String cacheFilename = entry.icon_min;
    // Keep original extension (.bin, .png, etc.)
    String cachePath = String(CATALOG_BASE_PATH) + entry.manifestName + "/cache/" + cacheFilename;

    FILE* cachedFile = fopen(cachePath.c_str(), "rb");

    if (cachedFile) {
        // Load from cache (raw RGB565 binary data)
        size_t bytesRead = fread(iconBuffer, 1, sizeof(iconBuffer), cachedFile);
        fclose(cachedFile);
        hasIcon = (bytesRead == sizeof(iconBuffer)); // Expect full 64x64x2 bytes
        return;
    }

    // Download icon if not cached
    String iconUrl = String(BASIC_URL) + entry.manifestName + "/" + entry.icon_min;

    HTTPClient http;
    http.begin(iconUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        int len = http.getSize();
        // Expect exactly 64x64x2 bytes for RGB565 binary format
        if (len == sizeof(iconBuffer)) {
            WiFiClient* stream = http.getStreamPtr();

            // Read raw RGB565 binary data into buffer
            size_t bytesRead = 0;
            size_t totalBytes = sizeof(iconBuffer);
            while (http.connected() && bytesRead < totalBytes) {
                size_t available = stream->available();
                if (available) {
                    size_t toRead = min(available, totalBytes - bytesRead);
                    bytesRead += stream->readBytes((uint8_t*)(iconBuffer) + bytesRead, toRead);
                }
            }
            hasIcon = (bytesRead == totalBytes);

            // Save to cache
            if (hasIcon) {
                String cacheDir = String(CATALOG_BASE_PATH) + entry.manifestName + "/cache";
                mkdirRecursive(cacheDir.c_str());

                FILE* cacheFile = fopen(cachePath.c_str(), "wb");
                if (cacheFile) {
                    fwrite(iconBuffer, 1, bytesRead, cacheFile);
                    fclose(cacheFile);
                }
            }
        }
    }
    http.end();
}

void LilCatalogApp::downloadAppFiles() {
    if (entries.empty() || current_entry >= entries.size()) {
        return;
    }

    catalog_entry& entry = entries[current_entry];
    if (entry.executionfiles.empty()) {
        return;
    }

    String dirPath = String(CATALOG_BASE_PATH) + entry.manifestName;

    // Create directory structure
    mkdirRecursive(dirPath.c_str());
    HTTPClient http;

    // Download all execution files
    for (const auto& execFile : entry.executionfiles) {
        if (execFile.location.isEmpty()) {
            continue;
        }

        String execFileUrl = String(BASIC_URL) + entry.manifestName + "/" + execFile.location;
        String fileName = execFile.location.substring(execFile.location.lastIndexOf('/') + 1);
        String filePath = dirPath + "/" + fileName;

        http.begin(execFileUrl);
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK) {
            WiFiClient* stream = http.getStreamPtr();
            FILE* file = fopen(filePath.c_str(), "wb");

            if (file) {
                uint8_t buffer[512];
                int len = http.getSize();
                int bytesRead = 0;

                while (http.connected() && (len > 0 || len == -1)) {
                    size_t available = stream->available();
                    if (available) {
                        int toRead = min(available, sizeof(buffer));
                        int c = stream->readBytes(buffer, toRead);
                        fwrite(buffer, 1, c, file);
                        bytesRead += c;
                        if (len > 0) len -= c;
                    }
                }
                fclose(file);
            }
        }
        http.end();
    }
}
bool LilCatalogApp::validateEntry() {
    return false;
}

void LilCatalogApp::removeEntry() {
}
