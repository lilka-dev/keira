#include "lilcatalog.h"
#include <HTTPClient.h>
#include <lilka/config.h>

#include "utils/json.h"

LilCatalogApp::LilCatalogApp() : App(K_S_LILCATALOG_APP) {
    setStackSize(16384);
    path_catalog_folder = "/lilcatalog";
}

void LilCatalogApp::run() {
    if (!lilka::fileutils.isSDAvailable()) {
        showAlert(K_S_LILCATALOG_SD_NOTFOUND);
        lilka::serial.log("SD card not found. Cannot continue.");
        return;
    }
    lilka::serial.log("SD card found.");

    // Create catalog folder if needed
    if (!SD.exists(path_catalog_folder)) {
        if (!SD.mkdir(path_catalog_folder.c_str())) {
            showAlert(K_S_LILCATALOG_ERROR_CREATE_FOLDER);
        }
    }

    showTypeSelect();

    while (1) {
        processMenu();
        processBackButton();
        queueDraw();
    }
}

void LilCatalogApp::processMenu() {
    switch (state) {
        case LILCATALOG_TYPE_SELECT:
            typeMenu.update();
            typeMenu.draw(canvas);
            break;
        case LILCATALOG_LIST:
            listMenu.update();
            listMenu.draw(canvas);
            break;
        case LILCATALOG_ENTRY:
            entryMenu.update();
            entryMenu.draw(canvas);
            break;
        default:
            break;
    }
}

void LilCatalogApp::processBackButton() {
    if (lilka::controller.peekState().b.justPressed) {
        lilka::serial.log("B button pressed, going back.");
        switch (state) {
            case LILCATALOG_TYPE_SELECT:
                stop();
                return;
            case LILCATALOG_LIST:
                clearIconCache();
                showTypeSelect();
                break;
            case LILCATALOG_ENTRY:
                showEntryList();
                break;
            case LILCATALOG_ENTRY_DESCRIPTION:
                showEntry();
                break;
            default:
                break;
        }
    }
}

// ================================
// Network Methods
// ================================

String LilCatalogApp::httpGet(const String& url) {
    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();
    http.begin(client, url);
    http.setTimeout(10000);

    int httpCode = http.GET();
    String result = "";

    if (httpCode == HTTP_CODE_OK) {
        result = http.getString();
    } else {
        lilka::serial.err("HTTP GET failed: %s, code: %d", url.c_str(), httpCode);
    }

    http.end();
    return result;
}

bool LilCatalogApp::httpGetBinary(const String& url, uint8_t* buffer, size_t bufferSize, size_t* bytesRead) {
    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();
    http.begin(client, url);
    http.setTimeout(10000);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        WiFiClient* stream = http.getStreamPtr();
        size_t size = http.getSize();

        if (size > bufferSize) {
            lilka::serial.err("Buffer too small: need %d, have %d", size, bufferSize);
            http.end();
            return false;
        }

        size_t read = 0;
        while (http.connected() && (read < size)) {
            size_t available = stream->available();
            if (available) {
                size_t toRead = min(available, size - read);
                size_t actualRead = stream->readBytes(buffer + read, toRead);
                read += actualRead;
            }
            delay(1);
        }

        *bytesRead = read;
        http.end();
        return true;
    }

    lilka::serial.err("HTTP GET binary failed: %s, code: %d", url.c_str(), httpCode);
    http.end();
    return false;
}

bool LilCatalogApp::downloadFile(const String& url, const String& targetPath) {
    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();
    http.begin(client, url);
    http.setTimeout(30000);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        // Create parent directories
        String parentDir = lilka::fileutils.getParentDirectory(targetPath);
        if (!lilka::fileutils.makePath(&SD, parentDir)) {
            lilka::serial.err("Failed to create directory: %s", parentDir.c_str());
            http.end();
            return false;
        }

        fs::File file = SD.open(targetPath.c_str(), FILE_WRITE);
        if (!file) {
            lilka::serial.err("Failed to open file for writing: %s", targetPath.c_str());
            http.end();
            return false;
        }

        WiFiClient* stream = http.getStreamPtr();
        size_t size = http.getSize();
        size_t written = 0;
        uint8_t buffer[1024];

        while (http.connected() && (written < size || size == -1)) {
            size_t available = stream->available();
            if (available) {
                size_t toRead = min(available, sizeof(buffer));
                size_t actualRead = stream->readBytes(buffer, toRead);
                file.write(buffer, actualRead);
                written += actualRead;
            }
            delay(1);
        }

        file.close();
        http.end();
        lilka::serial.log("Downloaded %d bytes to %s", written, targetPath.c_str());
        return true;
    }

    lilka::serial.err("Download failed: %s, code: %d", url.c_str(), httpCode);
    http.end();
    return false;
}

bool LilCatalogApp::downloadFileWithProgress(const String& url, const String& targetPath, const String& displayName) {
    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();
    http.begin(client, url);
    http.setTimeout(60000);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        // Create parent directories
        String parentDir = lilka::fileutils.getParentDirectory(targetPath);
        if (!lilka::fileutils.makePath(&SD, parentDir)) {
            showAlert(K_S_LILCATALOG_ERROR_DIRETORY_CREATE);
            http.end();
            return false;
        }

        fs::File file = SD.open(targetPath.c_str(), FILE_WRITE);
        if (!file) {
            showAlert(K_S_LILCATALOG_ERROR_FILE_OPEN);
            http.end();
            return false;
        }

        WiFiClient* stream = http.getStreamPtr();
        size_t size = http.getSize();
        size_t written = 0;
        uint8_t buffer[2048];

        lilka::ProgressDialog dialog(K_S_LILCATALOG_APP, displayName);

        while (http.connected() && (written < size || size == -1)) {
            size_t available = stream->available();
            if (available) {
                size_t toRead = min(available, sizeof(buffer));
                size_t actualRead = stream->readBytes(buffer, toRead);
                file.write(buffer, actualRead);
                written += actualRead;

                if (size > 0) {
                    int progress = written * 100 / size;
                    dialog.setProgress(progress);
                    dialog.draw(canvas);
                    queueDraw();
                }
            }

            // Check for cancel
            if (lilka::controller.getState().b.justPressed) {
                file.close();
                SD.remove(targetPath.c_str());
                http.end();
                return false;
            }

            delay(1);
        }

        file.close();
        http.end();
        lilka::serial.log("Downloaded %d bytes to %s", written, targetPath.c_str());
        return true;
    }

    showAlert(K_S_LILCATALOG_ERROR_CONNECTION + String(httpCode));
    http.end();
    return false;
}

// ================================
// Catalog Methods
// ================================

bool LilCatalogApp::fetchIndex(int page) {
    lilka::Alert alert(K_S_LILCATALOG_APP, K_S_LILCATALOG_LOADING_CATALOG);
    alert.draw(canvas);
    queueDraw();

    char url[256];
    if (currentType == CATALOG_TYPE_APPS) {
        snprintf(url, sizeof(url), CATALOG_APPS_INDEX_URL, page);
    } else {
        snprintf(url, sizeof(url), CATALOG_MODS_INDEX_URL, page);
    }

    lilka::serial.log("Fetching index: %s", url);
    String json = httpGet(url);

    if (json.length() == 0) {
        showAlert(K_S_LILCATALOG_ERROR_LOAD_CATALOG);
        return false;
    }

    return parseIndex(json);
}

bool LilCatalogApp::parseIndex(const String& json) {
    JsonDocument doc(&spiRamAllocator);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        lilka::serial.err("JSON parse error: %s", error.c_str());
        return false;
    }

    currentPage = doc["page"].as<int>();
    totalPages = doc["total_pages"].as<int>();

    entries.clear();
    clearIconCache();

    JsonArray manifests = doc["manifests"].as<JsonArray>();
    for (JsonVariant v : manifests) {
        String entryId = v.as<String>();
        catalog_entry entry;
        entry.id = entryId;

        // Fetch short manifest for basic info
        if (fetchEntryShortManifest(entryId, entry)) {
            entries.push_back(entry);
            iconCache.push_back(nullptr); // Placeholder for icon
        }
    }

    lilka::serial.log("Loaded %d entries, page %d/%d", entries.size(), currentPage + 1, totalPages);
    return true;
}

bool LilCatalogApp::fetchEntryShortManifest(const String& entryId, catalog_entry& entry) {
    char url[256];
    if (currentType == CATALOG_TYPE_APPS) {
        snprintf(url, sizeof(url), CATALOG_APP_SHORT_MANIFEST_URL, entryId.c_str());
    } else {
        // Mods use same structure
        snprintf(url, sizeof(url), CATALOG_BASE_URL "/mods/%s/index_short.json", entryId.c_str());
    }

    String json = httpGet(url);
    if (json.length() == 0) {
        return false;
    }

    return parseShortManifest(json, entry);
}

bool LilCatalogApp::parseShortManifest(const String& json, catalog_entry& entry) {
    JsonDocument doc(&spiRamAllocator);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        lilka::serial.err("Short manifest parse error: %s", error.c_str());
        return false;
    }

    entry.name = doc["name"].as<String>();
    entry.short_description = doc["short_description"].as<String>();

    if (doc.containsKey("executionfile")) {
        entry.executionfile.type = parseExecutionType(doc["executionfile"]["type"].as<String>());
        entry.executionfile.location = doc["executionfile"]["location"].as<String>();
    }

    return true;
}

bool LilCatalogApp::fetchEntryManifest(const String& entryId) {
    lilka::Alert alert(K_S_LILCATALOG_APP, K_S_LILCATALOG_FILE_LOADING);
    alert.draw(canvas);
    queueDraw();

    char url[256];
    if (currentType == CATALOG_TYPE_APPS) {
        snprintf(url, sizeof(url), CATALOG_APP_MANIFEST_URL, entryId.c_str());
    } else {
        snprintf(url, sizeof(url), CATALOG_MOD_MANIFEST_URL, entryId.c_str());
    }

    lilka::serial.log("Fetching manifest: %s", url);
    String json = httpGet(url);

    if (json.length() == 0) {
        showAlert(K_S_LILCATALOG_ERROR_LOAD_CATALOG);
        return false;
    }

    return parseManifest(json, currentEntry);
}

bool LilCatalogApp::parseManifest(const String& json, catalog_entry& entry) {
    JsonDocument doc(&spiRamAllocator);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        lilka::serial.err("Manifest parse error: %s", error.c_str());
        return false;
    }

    entry.name = doc["name"].as<String>();
    entry.short_description = doc["short_description"].as<String>();
    entry.description = doc["description"].as<String>();
    entry.changelog = doc["changelog"].as<String>();
    entry.author = doc["author"].as<String>();
    entry.icon = doc["icon"].as<String>();
    entry.icon_min = doc["icon_min"].as<String>();

    // Parse screenshots
    entry.screenshots.clear();
    if (doc.containsKey("screenshots")) {
        JsonArray screenshots = doc["screenshots"].as<JsonArray>();
        for (JsonVariant v : screenshots) {
            entry.screenshots.push_back(v.as<String>());
        }
    }

    // Parse sources
    if (doc.containsKey("sources")) {
        entry.sources.type = doc["sources"]["type"].as<String>();
        if (doc["sources"].containsKey("location")) {
            entry.sources.origin = doc["sources"]["location"]["origin"].as<String>();
        }
    }

    // Parse execution file (for apps)
    if (doc.containsKey("executionfile")) {
        entry.executionfile.type = parseExecutionType(doc["executionfile"]["type"].as<String>());
        entry.executionfile.location = doc["executionfile"]["location"].as<String>();
    }

    // Parse mod files (for mods)
    entry.modfiles.clear();
    if (doc.containsKey("modfiles")) {
        JsonArray modfiles = doc["modfiles"].as<JsonArray>();
        for (JsonVariant v : modfiles) {
            catalog_mod_file mf;
            mf.name = v["name"].as<String>();
            if (v.containsKey("location")) {
                mf.location = v["location"].as<String>();
            }
            entry.modfiles.push_back(mf);
        }
    }

    lilka::serial.log("Parsed manifest: %s by %s", entry.name.c_str(), entry.author.c_str());
    return true;
}

bool LilCatalogApp::fetchMiniIcon(const String& entryId, const String& iconMinName, uint16_t* buffer) {
    char url[256];
    if (currentType == CATALOG_TYPE_APPS) {
        snprintf(url, sizeof(url), CATALOG_APP_STATIC_URL, entryId.c_str(), iconMinName.c_str());
    } else {
        snprintf(url, sizeof(url), CATALOG_MOD_STATIC_URL, entryId.c_str(), iconMinName.c_str());
    }

    size_t bytesRead = 0;
    if (httpGetBinary(url, (uint8_t*)buffer, CATALOG_ICON_MIN_SIZE, &bytesRead)) {
        return bytesRead == CATALOG_ICON_MIN_SIZE;
    }
    return false;
}

ExecutionType LilCatalogApp::parseExecutionType(const String& typeStr) {
    if (typeStr == "lua") return EXEC_TYPE_LUA;
    if (typeStr == "binary") return EXEC_TYPE_BINARY;
    if (typeStr == "archive") return EXEC_TYPE_ARCHIVE;
    if (typeStr == "image") return EXEC_TYPE_IMAGE;
    return EXEC_TYPE_UNKNOWN;
}

FileType LilCatalogApp::executionTypeToFileType(ExecutionType type) {
    switch (type) {
        case EXEC_TYPE_LUA:
            return FT_LUA_SCRIPT;
        case EXEC_TYPE_BINARY:
            return FT_BIN;
        case EXEC_TYPE_ARCHIVE:
            return FT_OTHER;
        case EXEC_TYPE_IMAGE:
            return FT_OTHER;
        default:
            return FT_OTHER;
    }
}

// ================================
// Entry Management
// ================================

String LilCatalogApp::getEntryTargetPath() {
    return path_catalog_folder + "/" + currentEntry.id;
}

String LilCatalogApp::getEntryExecutablePath() {
    return getEntryTargetPath() + "/" + currentEntry.executionfile.location;
}

bool LilCatalogApp::validateEntry() {
    String execPath = getEntryExecutablePath();
    return SD.exists(execPath.c_str());
}

void LilCatalogApp::fetchEntry() {
    String targetDir = getEntryTargetPath();

    // Create entry directory
    if (!lilka::fileutils.makePath(&SD, targetDir)) {
        showAlert(K_S_LILCATALOG_ERROR_DIRETORY_CREATE);
        return;
    }

    // Build download URL
    char url[512];
    if (currentType == CATALOG_TYPE_APPS) {
        snprintf(
            url,
            sizeof(url),
            CATALOG_APP_STATIC_URL,
            currentEntry.id.c_str(),
            currentEntry.executionfile.location.c_str()
        );
    } else {
        snprintf(
            url,
            sizeof(url),
            CATALOG_MOD_STATIC_URL,
            currentEntry.id.c_str(),
            currentEntry.executionfile.location.c_str()
        );
    }

    String targetPath = getEntryExecutablePath();

    lilka::serial.log("Downloading: %s -> %s", url, targetPath.c_str());

    if (downloadFileWithProgress(url, targetPath, currentEntry.name)) {
        showAlert(K_S_LILCATALOG_FILE_LOADING_COMPLETE);
        // Refresh the entry menu to show new options
        showEntry();
    }
}

void LilCatalogApp::removeEntry() {
    String execPath = getEntryExecutablePath();
    String targetDir = getEntryTargetPath();

    // Remove executable file
    if (SD.exists(execPath.c_str())) {
        SD.remove(execPath.c_str());
    }

    // Try to remove directory (will only work if empty)
    SD.rmdir(targetDir.c_str());

    // Refresh entry menu
    showEntry();
}

void LilCatalogApp::executeEntry() {
    String execPath = getEntryExecutablePath();
    String canonicalPath = lilka::fileutils.getCannonicalPath(&SD, execPath);

    lilka::serial.log("Executing: %s (type: %d)", canonicalPath.c_str(), currentEntry.executionfile.type);

    switch (currentEntry.executionfile.type) {
        case EXEC_TYPE_LUA:
            K_FT_LUA_SCRIPT_HANDLER(canonicalPath);
            break;
        case EXEC_TYPE_BINARY:
            fileLoadAsRom(canonicalPath);
            break;
        case EXEC_TYPE_ARCHIVE:
            showAlert(K_S_LILCATALOG_ARCHIVE_NOTICE);
            break;
        case EXEC_TYPE_IMAGE:
        case EXEC_TYPE_UNKNOWN:
        default:
            showAlert(K_S_LILCATALOG_UNSUPPORTED_TYPE);
            break;
    }

    vTaskDelay(100 / portTICK_RATE_MS);
}

void LilCatalogApp::fileLoadAsRom(const String& path) {
    lilka::ProgressDialog dialog(K_S_LILCATALOG_LOADING, path + "\n\n" + K_S_LILCATALOG_STARTING);
    dialog.draw(canvas);
    queueDraw();

    int error = lilka::multiboot.start(path);
    if (error) {
        showAlert(String(K_S_LILCATALOG_ERROR_STAGE1) + error);
        return;
    }

    dialog.setMessage(path + "\n\n" + K_S_LILCATALOG_SIZE + String(lilka::multiboot.getBytesTotal()));
    dialog.draw(canvas);
    queueDraw();

    while ((error = lilka::multiboot.process()) > 0) {
        int progress = lilka::multiboot.getBytesWritten() * 100 / lilka::multiboot.getBytesTotal();
        dialog.setProgress(progress);
        dialog.draw(canvas);
        queueDraw();
        if (lilka::controller.getState().a.justPressed) {
            lilka::multiboot.cancel();
            return;
        }
    }

    if (error < 0) {
        showAlert(String(K_S_LILCATALOG_ERROR_STAGE2) + error);
        return;
    }

    error = lilka::multiboot.finishAndReboot();
    if (error) {
        showAlert(String(K_S_LILCATALOG_ERROR_STAGE3) + error);
        return;
    }
}

// ================================
// UI Methods
// ================================

void LilCatalogApp::showTypeSelect() {
    state = LILCATALOG_TYPE_SELECT;
    currentPage = 0;
    totalPages = 0;

    typeMenu.clearItems();
    typeMenu.setTitle(K_S_LILCATALOG_APP);

    typeMenu.addItem(
        K_S_LILCATALOG_APPS,
        nullptr,
        lilka::colors::White,
        K_S_LILCATALOG_EMPTY,
        [](void* ctx) {
            LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
            app->currentType = CATALOG_TYPE_APPS;
            if (app->fetchIndex(0)) {
                app->showEntryList();
            }
        },
        this
    );

    typeMenu.addItem(
        K_S_LILCATALOG_MODS,
        nullptr,
        lilka::colors::White,
        K_S_LILCATALOG_EMPTY,
        [](void* ctx) {
            LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
            app->currentType = CATALOG_TYPE_MODS;
            if (app->fetchIndex(0)) {
                app->showEntryList();
            }
        },
        this
    );

    typeMenu.addItem(
        K_S_LILCATALOG_STOP,
        nullptr,
        lilka::colors::White,
        K_S_LILCATALOG_EMPTY,
        [](void* ctx) {
            LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
            app->stop();
        },
        this
    );

    typeMenu.setCursor(0);
    typeMenu.update();
}

void LilCatalogApp::showEntryList() {
    state = LILCATALOG_LIST;

    listMenu.clearItems();

    String title = (currentType == CATALOG_TYPE_APPS) ? K_S_LILCATALOG_APPS : K_S_LILCATALOG_MODS;
    title += " (" + String(currentPage + 1) + "/" + String(totalPages) + ")";
    listMenu.setTitle(title);

    for (size_t i = 0; i < entries.size(); i++) {
        const catalog_entry& entry = entries[i];

        // Create callback with entry index
        listMenu.addItem(
            entry.name,
            nullptr,
            lilka::colors::White,
            entry.short_description,
            [](void* ctx) {
                // This is a simplified callback - we'll handle selection in processMenu
            },
            nullptr
        );
    }

    // Add pagination controls
    if (currentPage > 0) {
        listMenu.addItem(
            K_S_LILCATALOG_PREV_PAGE,
            nullptr,
            lilka::colors::Cyan,
            K_S_LILCATALOG_EMPTY,
            [](void* ctx) {
                LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
                app->loadPrevPage();
            },
            this
        );
    }

    if (currentPage < totalPages - 1) {
        listMenu.addItem(
            K_S_LILCATALOG_NEXT_PAGE,
            nullptr,
            lilka::colors::Cyan,
            K_S_LILCATALOG_EMPTY,
            [](void* ctx) {
                LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
                app->loadNextPage();
            },
            this
        );
    }

    listMenu.addItem(
        K_S_LILCATALOG_BACK,
        nullptr,
        lilka::colors::White,
        K_S_LILCATALOG_EMPTY,
        [](void* ctx) {
            LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
            app->clearIconCache();
            app->showTypeSelect();
        },
        this
    );

    listMenu.setCursor(0);
    listMenu.update();

    // Handle entry selection via A button
    while (state == LILCATALOG_LIST) {
        listMenu.update();
        listMenu.draw(canvas);
        queueDraw();

        if (lilka::controller.peekState().a.justPressed) {
            int cursor = listMenu.getCursor();
            if (cursor < (int)entries.size()) {
                // Selected an entry
                currentEntry = entries[cursor];
                currentEntry.id = entries[cursor].id; // Preserve ID

                // Fetch full manifest
                if (fetchEntryManifest(currentEntry.id)) {
                    showEntry();
                }
            }
            // Pagination and back buttons handled by callbacks
        }

        if (lilka::controller.peekState().b.justPressed) {
            clearIconCache();
            showTypeSelect();
            return;
        }
    }
}

void LilCatalogApp::showEntry() {
    state = LILCATALOG_ENTRY;

    entryMenu.clearItems();
    entryMenu.setTitle(currentEntry.name);

    bool installed = validateEntry();

    if (installed) {
        entryMenu.addItem(
            K_S_LILCATALOG_START,
            nullptr,
            lilka::colors::Green,
            K_S_LILCATALOG_EMPTY,
            [](void* ctx) {
                LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
                app->executeEntry();
            },
            this
        );

        entryMenu.addItem(
            K_S_LILCATALOG_REMOVE,
            nullptr,
            lilka::colors::Red,
            K_S_LILCATALOG_EMPTY,
            [](void* ctx) {
                LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
                app->removeEntry();
            },
            this
        );

        entryMenu.addItem(
            K_S_LILCATALOG_UPDATE,
            nullptr,
            lilka::colors::Yellow,
            K_S_LILCATALOG_EMPTY,
            [](void* ctx) {
                LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
                app->fetchEntry();
            },
            this
        );
    } else {
        entryMenu.addItem(
            K_S_LILCATALOG_INSTALL,
            nullptr,
            lilka::colors::Green,
            K_S_LILCATALOG_EMPTY,
            [](void* ctx) {
                LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
                app->fetchEntry();
            },
            this
        );
    }

    entryMenu.addItem(
        K_S_LILCATALOG_ENTRY_DESCRIPTION,
        nullptr,
        lilka::colors::White,
        K_S_LILCATALOG_EMPTY,
        [](void* ctx) {
            LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
            app->showEntryDescription();
        },
        this
    );

    if (!currentEntry.sources.origin.isEmpty()) {
        entryMenu.addItem(
            K_S_LILCATALOG_SOURCE, nullptr, lilka::colors::Cyan, currentEntry.sources.origin, nullptr, nullptr
        );
    }

    entryMenu.addItem(
        K_S_LILCATALOG_BACK,
        nullptr,
        lilka::colors::White,
        K_S_LILCATALOG_EMPTY,
        [](void* ctx) {
            LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
            app->showEntryList();
        },
        this
    );

    entryMenu.setCursor(0);
    entryMenu.update();
}

void LilCatalogApp::showEntryDescription() {
    state = LILCATALOG_ENTRY_DESCRIPTION;

    String description = K_S_LILCATALOG_ENTRY_DESCRIPTION_NAME + currentEntry.name + "\n" +
                         K_S_LILCATALOG_ENTRY_DESCRIPTION_AUTHOR + currentEntry.author + "\n\n" +
                         K_S_LILCATALOG_ENTRY_DESCRIPTION_DESCRIPTION + "\n" + currentEntry.short_description;

    if (!currentEntry.executionfile.location.isEmpty()) {
        description += "\n\n" + String(K_S_LILCATALOG_ENTRY_DESCRIPTION_FILE) + currentEntry.executionfile.location;
    }

    showAlert(description);
    state = LILCATALOG_ENTRY;
}

void LilCatalogApp::loadNextPage() {
    if (currentPage < totalPages - 1) {
        if (fetchIndex(currentPage + 1)) {
            showEntryList();
        }
    }
}

void LilCatalogApp::loadPrevPage() {
    if (currentPage > 0) {
        if (fetchIndex(currentPage - 1)) {
            showEntryList();
        }
    }
}

void LilCatalogApp::clearIconCache() {
    for (uint16_t* icon : iconCache) {
        if (icon != nullptr) {
            free(icon);
        }
    }
    iconCache.clear();
}

void LilCatalogApp::showAlert(const String& message) {
    lilka::Alert alert(K_S_LILCATALOG_APP, message);
    alert.draw(canvas);
    queueDraw();
    while (!alert.isFinished()) {
        alert.update();
    }
    lilka::serial.log(message.c_str());
}
