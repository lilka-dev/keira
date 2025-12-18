#include "lilcatalog.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <lilka/config.h>

#include "utils/json.h"

LilCatalogApp::LilCatalogApp() : App(K_S_LILCATALOG_APP), currentEntry{}, iconBuffer{}, downloadBuffer{} {
    setStackSize(16384);
    path_catalog_folder = "/lilcatalog";
}

void LilCatalogApp::run() {
    if (!lilka::fileutils.isSDAvailable()) {
        showAlert(K_S_LILCATALOG_SD_NOTFOUND);
        return;
    }

    // Create catalog folder if needed
    if (!SD.exists(path_catalog_folder)) {
        SD.mkdir(path_catalog_folder.c_str());
    }

    // Show main menu first
    showMainMenu();

    while (1) {
        switch (state) {
            case LILCATALOG_MAIN_MENU:
                mainMenu.update();
                mainMenu.draw(canvas);
                if (lilka::controller.peekState().b.justPressed) {
                    stop();
                    return;
                }
                break;
            case LILCATALOG_LIST:
                handleInput();
                drawAppView();
                break;
            case LILCATALOG_ENTRY:
                entryMenu.update();
                entryMenu.draw(canvas);
                if (lilka::controller.peekState().b.justPressed) {
                    state = LILCATALOG_LIST;
                }
                break;
            case LILCATALOG_DESCRIPTION:
                drawDescription();
                if (lilka::controller.getState().b.justPressed || lilka::controller.getState().a.justPressed) {
                    state = LILCATALOG_ENTRY;
                    showEntry();
                }
                break;
        }
        queueDraw();
    }
}

// ================================
// Network Methods
// ================================

String LilCatalogApp::httpGet(const String& url, int timeout) {
    // No network - return empty
    if (WiFi.status() != WL_CONNECTED) {
        return "";
    }

    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();
    http.begin(client, url);
    http.setTimeout(timeout);

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
    *bytesRead = 0;

    // No network - return false
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();
    http.begin(client, url);
    http.setTimeout(CATALOG_HTTP_TIMEOUT);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        WiFiClient* stream = http.getStreamPtr();
        size_t remaining = bufferSize;

        while (http.connected() && remaining > 0) {
            size_t available = stream->available();
            if (available) {
                size_t toRead = min(available, remaining);
                size_t actualRead = stream->readBytes(buffer + *bytesRead, toRead);
                *bytesRead += actualRead;
                remaining -= actualRead;
            }
            
        }

        http.end();
        return true;
    }

    http.end();
    return false;
}

bool LilCatalogApp::downloadFile(const String& url, const String& targetPath) {
    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();
    http.begin(client, url);
    http.setTimeout(CATALOG_HTTP_TIMEOUT);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String parentDir = lilka::fileutils.getParentDirectory(targetPath);
        if (!lilka::fileutils.makePath(&SD, parentDir)) {
            http.end();
            return false;
        }

        fs::File file = SD.open(targetPath.c_str(), FILE_WRITE);
        if (!file) {
            http.end();
            return false;
        }

        WiFiClient* stream = http.getStreamPtr();
        size_t size = http.getSize();
        size_t written = 0;

        while (http.connected() && (written < size || size == (size_t)-1)) {
            size_t available = stream->available();
            if (available) {
                size_t toRead = min(available, (size_t)CATALOG_DOWNLOAD_BUFFER_SIZE);
                size_t actualRead = stream->readBytes(downloadBuffer, toRead);
                file.write(downloadBuffer, actualRead);
                written += actualRead;
            }
            
        }

        file.close();
        http.end();
        return true;
    }

    http.end();
    return false;
}

bool LilCatalogApp::downloadFileWithProgress(const String& url, const String& targetPath, const String& displayName) {
    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();
    http.begin(client, url);
    http.setTimeout(CATALOG_HTTP_TIMEOUT);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
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

        lilka::ProgressDialog dialog(K_S_LILCATALOG_APP, displayName);

        while (http.connected() && (written < size || size == (size_t)-1)) {
            size_t available = stream->available();
            if (available) {
                size_t toRead = min(available, (size_t)CATALOG_DOWNLOAD_BUFFER_SIZE);
                size_t actualRead = stream->readBytes(downloadBuffer, toRead);
                file.write(downloadBuffer, actualRead);
                written += actualRead;

                if (size > 0) {
                    int progress = written * 100 / size;
                    dialog.setProgress(progress);
                }
                dialog.draw(canvas);
                queueDraw();
            }
            
        }

        file.close();
        http.end();
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

    String url = String(CATALOG_BASE_URL) + "/apps/index_" + String(page) + ".json";

    String json = httpGet(url);

    if (json.length() == 0) {
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
    iconLoaded = false;
    loadedIconIndex = -1;

    JsonArray manifests = doc["manifests"].as<JsonArray>();
    int total = manifests.size();
    int loaded = 0;
    
    lilka::ProgressDialog progress(K_S_LILCATALOG_APP, K_S_LILCATALOG_LOADING_CATALOG);
    
    for (JsonVariant v : manifests) {
        String entryId = v.as<String>();
        catalog_entry entry;
        entry.id = entryId;

        if (fetchEntryShortManifest(entryId, entry)) {
            entries.push_back(entry);
        }
        
        loaded++;
        progress.setProgress(loaded * 100 / total);
        progress.draw(canvas);
        queueDraw();
    }

    lilka::serial.log("Loaded %d entries, page %d/%d", entries.size(), currentPage + 1, totalPages);
    return true;
}

bool LilCatalogApp::fetchEntryShortManifest(const String& entryId, catalog_entry& entry) {
    // Try cache first
    String json = loadShortManifestFromCache(entryId);
    
    // If not cached, fetch from network with shorter timeout
    if (json.length() == 0) {
        String url = String(CATALOG_BASE_URL) + "/apps/" + entryId + "/index_short.json";
        json = httpGet(url, CATALOG_HTTP_TIMEOUT_SHORT);
        
        // Cache for next time
        if (json.length() > 0) {
            saveShortManifestToCache(entryId, json);
        }
    }
    
    if (json.length() == 0) {
        return false;
    }

    return parseShortManifest(json, entry);
}

bool LilCatalogApp::parseShortManifest(const String& json, catalog_entry& entry) {
    JsonDocument doc(&spiRamAllocator);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        return false;
    }

    entry.name = doc["name"].as<String>();
    entry.short_description = doc["short_description"].as<String>();
    
    // Also cache icon_min if available
    if (doc.containsKey("icon_min")) {
        entry.icon_min = doc["icon_min"].as<String>();
    }

    if (doc.containsKey("entryfile")) {
        entry.entryfile.type = parseExecutionType(doc["entryfile"]["type"].as<String>());
        entry.entryfile.location = doc["entryfile"]["location"].as<String>();
    }

    return true;
}

bool LilCatalogApp::fetchEntryManifest(const String& entryId) {
    // Try cache first (for offline support)
    String json = loadManifestFromCache(entryId);

    // If not in cache, try network
    if (json.length() == 0) {
        lilka::Alert alert(K_S_LILCATALOG_APP, K_S_LILCATALOG_FILE_LOADING);
        alert.draw(canvas);
        queueDraw();

        String url = String(CATALOG_BASE_URL) + "/apps/" + entryId + "/index.json";

        json = httpGet(url);
    }

    if (json.length() == 0) {
        return false;
    }

    return parseManifest(json, currentEntry);
}

bool LilCatalogApp::parseManifest(const String& json, catalog_entry& entry) {
    JsonDocument doc(&spiRamAllocator);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        return false;
    }

    entry.name = doc["name"].as<String>();
    entry.short_description = doc["short_description"].as<String>();
    entry.description = doc["description"].as<String>();
    entry.author = doc["author"].as<String>();
    entry.icon = doc["icon"].as<String>();
    entry.icon_min = doc["icon_min"].as<String>();

    if (doc.containsKey("sources")) {
        entry.sources.type = doc["sources"]["type"].as<String>();
        if (doc["sources"].containsKey("location")) {
            entry.sources.origin = doc["sources"]["location"]["origin"].as<String>();
        }
    }

    // Parse entryfile (main execution file)
    if (doc.containsKey("entryfile")) {
        entry.entryfile.type = parseExecutionType(doc["entryfile"]["type"].as<String>());
        entry.entryfile.location = doc["entryfile"]["location"].as<String>();
    }

    // Parse additional files array
    entry.files.clear();
    if (doc.containsKey("files")) {
        JsonArray filesArray = doc["files"].as<JsonArray>();
        for (JsonObject fileObj : filesArray) {
            catalog_file file;
            file.type = parseExecutionType(fileObj["type"].as<String>());
            file.location = fileObj["location"].as<String>();
            if (fileObj.containsKey("description")) {
                file.description = fileObj["description"].as<String>();
            }
            entry.files.push_back(file);
        }
    }

    return true;
}

bool LilCatalogApp::fetchIcon(const String& entryId, const String& iconMinName) {
    String url = String(CATALOG_BASE_URL) + "/apps/" + entryId + "/static/" + iconMinName;

    size_t bytesRead = 0;
    if (httpGetBinary(url, reinterpret_cast<uint8_t*>(iconBuffer), CATALOG_ICON_SIZE, &bytesRead)) {
        return bytesRead == CATALOG_ICON_SIZE;
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
        default:
            return FT_OTHER;
    }
}

// ================================
// Icon Cache Methods
// ================================

String LilCatalogApp::getIconCachePath(const String& entryId) {
    return String(CATALOG_ICON_CACHE_FOLDER) + "/" + entryId + ".bin";
}

bool LilCatalogApp::loadIconFromCache(const String& entryId) {
    String cachePath = getIconCachePath(entryId);

    if (!SD.exists(cachePath.c_str())) {
        return false;
    }

    fs::File file = SD.open(cachePath.c_str(), FILE_READ);
    if (!file) {
        return false;
    }

    size_t bytesRead = file.read(reinterpret_cast<uint8_t*>(iconBuffer), CATALOG_ICON_SIZE);
    file.close();

    return bytesRead == CATALOG_ICON_SIZE;
}

bool LilCatalogApp::saveIconToCache(const String& entryId) {
    if (!iconLoaded) {
        return false;
    }

    if (!SD.exists(CATALOG_ICON_CACHE_FOLDER)) {
        lilka::fileutils.makePath(&SD, CATALOG_ICON_CACHE_FOLDER);
    }

    String cachePath = getIconCachePath(entryId);
    fs::File file = SD.open(cachePath.c_str(), FILE_WRITE);
    if (!file) {
        return false;
    }

    size_t written = file.write(reinterpret_cast<uint8_t*>(iconBuffer), CATALOG_ICON_SIZE);
    file.close();

    return written == CATALOG_ICON_SIZE;
}

void LilCatalogApp::drawLoadingAnimation() {
    // Draw current app view with loading animation in icon area
    canvas->fillScreen(lilka::colors::Black);
    canvas->setFont(FONT_6x13);

    if (entries.empty()) return;

    const catalog_entry& entry = entries[currentIndex];

    // Draw header
    canvas->fillRect(0, 0, canvas->width(), 20, lilka::colors::Black_olive);
    canvas->setTextColor(lilka::colors::White);
    canvas->setCursor(8, 14);
    canvas->print(K_S_LILCATALOG_APPS);

    // Draw loading animation in icon area
    int iconX = (canvas->width() - CATALOG_ICON_WIDTH) / 2;
    int iconY = 28;
    
    canvas->drawRect(iconX, iconY, CATALOG_ICON_WIDTH, CATALOG_ICON_HEIGHT, lilka::colors::Graygrey);
    
    int centerX = iconX + CATALOG_ICON_WIDTH / 2;
    int centerY = iconY + CATALOG_ICON_HEIGHT / 2;
    int radius = 12;
    
    for (int i = 0; i < 8; i++) {
        float angle = (i * 45 + loadingFrame * 45) * 3.14159f / 180.0f;
        int dotX = centerX + cos(angle) * radius;
        int dotY = centerY + sin(angle) * radius;
        uint16_t color = (i == 0) ? lilka::colors::Cyan : lilka::colors::Graygrey;
        canvas->fillCircle(dotX, dotY, 3, color);
    }
    loadingFrame = (loadingFrame + 1) % 8;

    // Draw app name
    int nameY = iconY + CATALOG_ICON_HEIGHT + 12;
    canvas->setTextColor(lilka::colors::White);
    canvas->setCursor(10, nameY);
    canvas->print(entry.name);

    queueDraw();
}

void LilCatalogApp::loadCurrentIcon() {
    if (currentIndex < 0 || currentIndex >= (int)entries.size()) {
        iconLoaded = false;
        return;
    }

    // Already loaded?
    if (loadedIconIndex == currentIndex && iconLoaded) {
        return;
    }

    const catalog_entry& entry = entries[currentIndex];
    iconLoaded = false;
    loadedIconIndex = currentIndex;

    // Try cache first (fast, no animation needed)
    if (loadIconFromCache(entry.id)) {
        iconLoaded = true;
        return;
    }

    // Show loading animation before network request
    drawLoadingAnimation();

    // Need to get icon_min name - fetch full manifest
    String iconMinName = entry.icon_min;
    if (iconMinName.isEmpty()) {
        drawLoadingAnimation();
        String url = String(CATALOG_BASE_URL) + "/apps/" + entry.id + "/index.json";
        String json = httpGet(url);
        if (json.length() > 0) {
            JsonDocument doc(&spiRamAllocator);
            if (!deserializeJson(doc, json)) {
                if (doc.containsKey("icon_min")) {
                    iconMinName = doc["icon_min"].as<String>();
                }
            }
        }
    }

    if (iconMinName.isEmpty()) {
        return;
    }

    // Show animation before fetching icon
    drawLoadingAnimation();

    // Fetch from network
    if (fetchIcon(entry.id, iconMinName)) {
        iconLoaded = true;
        saveIconToCache(entry.id);
    }
}

void LilCatalogApp::clearIconCache() {
    iconLoaded = false;
    loadedIconIndex = -1;

    if (SD.exists(CATALOG_ICON_CACHE_FOLDER)) {
        fs::File dir = SD.open(CATALOG_ICON_CACHE_FOLDER);
        if (dir && dir.isDirectory()) {
            fs::File entry = dir.openNextFile();
            while (entry) {
                String path = String(CATALOG_ICON_CACHE_FOLDER) + "/" + entry.name();
                entry.close();
                SD.remove(path.c_str());
                entry = dir.openNextFile();
            }
            dir.close();
        }
        SD.rmdir(CATALOG_ICON_CACHE_FOLDER);
    }
}

// ================================
// Short Manifest Cache Methods
// ================================

String LilCatalogApp::getShortManifestCachePath(const String& entryId) {
    return String(CATALOG_SHORT_MANIFEST_CACHE_FOLDER) + "/" + entryId + ".json";
}

bool LilCatalogApp::saveShortManifestToCache(const String& entryId, const String& json) {
    if (!lilka::fileutils.makePath(&SD, CATALOG_SHORT_MANIFEST_CACHE_FOLDER)) {
        return false;
    }

    String cachePath = getShortManifestCachePath(entryId);
    fs::File file = SD.open(cachePath.c_str(), FILE_WRITE);
    if (!file) {
        return false;
    }

    file.print(json);
    file.close();
    return true;
}

String LilCatalogApp::loadShortManifestFromCache(const String& entryId) {
    String cachePath = getShortManifestCachePath(entryId);

    if (!SD.exists(cachePath.c_str())) {
        return "";
    }

    fs::File file = SD.open(cachePath.c_str(), FILE_READ);
    if (!file) {
        return "";
    }

    String json = file.readString();
    file.close();
    return json;
}

void LilCatalogApp::clearShortManifestCache() {
    if (SD.exists(CATALOG_SHORT_MANIFEST_CACHE_FOLDER)) {
        fs::File dir = SD.open(CATALOG_SHORT_MANIFEST_CACHE_FOLDER);
        if (dir && dir.isDirectory()) {
            fs::File entry = dir.openNextFile();
            while (entry) {
                String path = String(CATALOG_SHORT_MANIFEST_CACHE_FOLDER) + "/" + entry.name();
                entry.close();
                SD.remove(path.c_str());
                entry = dir.openNextFile();
            }
            dir.close();
        }
        SD.rmdir(CATALOG_SHORT_MANIFEST_CACHE_FOLDER);
    }
}

// ================================
// Manifest Cache Methods
// ================================

String LilCatalogApp::getManifestCachePath(const String& entryId) {
    return String(CATALOG_MANIFEST_CACHE_FOLDER) + "/" + entryId + ".json";
}

bool LilCatalogApp::saveManifestToCache(const String& entryId, const String& json) {
    if (!lilka::fileutils.makePath(&SD, CATALOG_MANIFEST_CACHE_FOLDER)) {
        return false;
    }

    String cachePath = getManifestCachePath(entryId);
    fs::File file = SD.open(cachePath.c_str(), FILE_WRITE);
    if (!file) {
        return false;
    }

    file.print(json);
    file.close();
    return true;
}

String LilCatalogApp::loadManifestFromCache(const String& entryId) {
    String cachePath = getManifestCachePath(entryId);

    if (!SD.exists(cachePath.c_str())) {
        return "";
    }

    fs::File file = SD.open(cachePath.c_str(), FILE_READ);
    if (!file) {
        return "";
    }

    String json = file.readString();
    file.close();
    return json;
}

bool LilCatalogApp::loadInstalledApps() {
    entries.clear();
    iconLoaded = false;
    loadedIconIndex = -1;
    currentIndex = 0;
    currentPage = 0;
    totalPages = 1;

    // Scan manifest cache folder for installed apps
    if (!SD.exists(CATALOG_MANIFEST_CACHE_FOLDER)) {
        return false;
    }

    fs::File dir = SD.open(CATALOG_MANIFEST_CACHE_FOLDER);
    if (!dir || !dir.isDirectory()) {
        return false;
    }

    fs::File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String filename = file.name();
            if (filename.endsWith(".json")) {
                // Extract entry ID from filename
                String entryId = filename.substring(0, filename.length() - 5);

                // Check if this app is actually installed (has execution file)
                String execPath = path_catalog_folder + "/" + entryId;
                if (SD.exists(execPath.c_str())) {
                    // Load manifest from cache
                    String json = file.readString();
                    file.close();

                    catalog_entry entry;
                    entry.id = entryId;
                    if (parseManifest(json, entry)) {
                        entries.push_back(entry);
                    }
                } else {
                    file.close();
                }
            } else {
                file.close();
            }
        } else {
            file.close();
        }
        file = dir.openNextFile();
    }
    dir.close();

    return !entries.empty();
}

void LilCatalogApp::clearManifestCache() {
    if (SD.exists(CATALOG_MANIFEST_CACHE_FOLDER)) {
        fs::File dir = SD.open(CATALOG_MANIFEST_CACHE_FOLDER);
        if (dir && dir.isDirectory()) {
            fs::File entry = dir.openNextFile();
            while (entry) {
                String path = String(CATALOG_MANIFEST_CACHE_FOLDER) + "/" + entry.name();
                entry.close();
                SD.remove(path.c_str());
                entry = dir.openNextFile();
            }
            dir.close();
        }
        SD.rmdir(CATALOG_MANIFEST_CACHE_FOLDER);
    }
}

// ================================
// Entry Management
// ================================

String LilCatalogApp::getEntryTargetPath() {
    return path_catalog_folder + "/" + currentEntry.id;
}

String LilCatalogApp::getEntryExecutablePath() {
    return getEntryTargetPath() + "/" + currentEntry.entryfile.location;
}

bool LilCatalogApp::validateEntry() {
    String execPath = getEntryExecutablePath();
    return SD.exists(execPath.c_str());
}

void LilCatalogApp::fetchEntry() {
    // Check if app has downloadable files
    if (currentEntry.entryfile.location.isEmpty()) {
        showAlert(K_S_LILCATALOG_NO_BINARY);
        return;
    }

    String targetDir = getEntryTargetPath();

    if (!lilka::fileutils.makePath(&SD, targetDir)) {
        showAlert(K_S_LILCATALOG_ERROR_DIRETORY_CREATE);
        return;
    }

    // Download entryfile (main file)
    String url = String(CATALOG_BASE_URL) + "/apps/" + currentEntry.id + "/static/" + currentEntry.entryfile.location;

    String targetPath = getEntryExecutablePath();

    if (!downloadFileWithProgress(url, targetPath, currentEntry.name)) {
        return;
    }

    // Download additional files
    for (const auto& file : currentEntry.files) {
        url = String(CATALOG_BASE_URL) + "/apps/" + currentEntry.id + "/static/" + file.location;

        String filePath = getEntryTargetPath() + "/" + file.location;

        if (!downloadFileWithProgress(url, filePath, file.location)) {
            // Continue with other files even if one fails
            lilka::serial.err("Failed to download: %s", file.location.c_str());
        }
    }

    // Save manifest to cache for offline use
    String manifestUrl = String(CATALOG_BASE_URL) + "/apps/" + currentEntry.id + "/index.json";
    String manifestJson = httpGet(manifestUrl);
    if (manifestJson.length() > 0) {
        saveManifestToCache(currentEntry.id, manifestJson);
    }

    showAlert(K_S_LILCATALOG_FILE_LOADING_COMPLETE);
    showEntry();
}

void LilCatalogApp::removeEntry() {
    String targetDir = getEntryTargetPath();

    // Remove entryfile
    String execPath = getEntryExecutablePath();
    if (SD.exists(execPath.c_str())) {
        SD.remove(execPath.c_str());
    }

    // Remove additional files
    for (const auto& file : currentEntry.files) {
        String filePath = targetDir + "/" + file.location;
        if (SD.exists(filePath.c_str())) {
            SD.remove(filePath.c_str());
        }
    }

    SD.rmdir(targetDir.c_str());

    // Also remove cached manifest
    String manifestPath = getManifestCachePath(currentEntry.id);
    if (SD.exists(manifestPath.c_str())) {
        SD.remove(manifestPath.c_str());
    }

    showEntry();
}

void LilCatalogApp::executeEntry() {
    String execPath = getEntryExecutablePath();
    String canonicalPath = lilka::fileutils.getCannonicalPath(&SD, execPath);

    switch (currentEntry.entryfile.type) {
        case EXEC_TYPE_LUA:
            K_FT_LUA_SCRIPT_HANDLER(canonicalPath);
            break;
        case EXEC_TYPE_BINARY:
            fileLoadAsRom(canonicalPath);
            break;
        case EXEC_TYPE_ARCHIVE:
            showAlert(K_S_LILCATALOG_ARCHIVE_NOTICE);
            break;
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
    }
}

// ================================
// UI Methods
// ================================

void LilCatalogApp::showMainMenu() {
    state = LILCATALOG_MAIN_MENU;

    mainMenu.clearItems();
    mainMenu.setTitle(K_S_LILCATALOG_APP);

    mainMenu.addItem(
        K_S_LILCATALOG_APPS,
        nullptr,
        lilka::colors::White,
        K_S_LILCATALOG_EMPTY,
        [](void* ctx) {
            LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
            if (app->fetchIndex(0)) {
                app->loadCurrentIcon();
                app->state = LILCATALOG_LIST;
            }
        },
        this
    );

    mainMenu.addItem(
        K_S_LILCATALOG_INSTALLED,
        nullptr,
        lilka::colors::Green,
        K_S_LILCATALOG_EMPTY,
        [](void* ctx) {
            LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
            if (app->loadInstalledApps()) {
                app->loadCurrentIcon();
                app->state = LILCATALOG_LIST;
            } else {
                app->showAlert(K_S_LILCATALOG_NO_INSTALLED);
            }
        },
        this
    );

    mainMenu.addItem(
        K_S_LILCATALOG_CLEAR_CACHE,
        nullptr,
        lilka::colors::Orange,
        K_S_LILCATALOG_EMPTY,
        [](void* ctx) {
            LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
            app->clearIconCache();
            app->clearShortManifestCache();
            app->clearManifestCache();
            app->showAlert(K_S_LILCATALOG_CACHE_CLEARED);
        },
        this
    );

    mainMenu.addItem(
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

    mainMenu.setCursor(0);
}

void LilCatalogApp::drawAppView() {
    canvas->fillScreen(lilka::colors::Black);
    canvas->setFont(FONT_6x13);

    if (entries.empty()) {
        canvas->setTextColor(lilka::colors::White);
        canvas->setCursor(canvas->width() / 2 - 40, canvas->height() / 2);
        canvas->print(K_S_LILCATALOG_EMPTY);
        return;
    }

    const catalog_entry& entry = entries[currentIndex];

    // Draw header
    canvas->fillRect(0, 0, canvas->width(), 20, lilka::colors::Black_olive);
    canvas->setTextColor(lilka::colors::White);
    canvas->setCursor(8, 14);
    canvas->print(K_S_LILCATALOG_APPS);

    // Draw 64x64 icon centered
    int iconX = (canvas->width() - CATALOG_ICON_WIDTH) / 2;
    int iconY = 28;

    if (iconLoaded && loadedIconIndex == currentIndex) {
        canvas->draw16bitRGBBitmapWithTranColor(
            iconX, iconY, iconBuffer, lilka::colors::Black, CATALOG_ICON_WIDTH, CATALOG_ICON_HEIGHT
        );
    } else {
        // Draw loading animation (spinning dots)
        canvas->drawRect(iconX, iconY, CATALOG_ICON_WIDTH, CATALOG_ICON_HEIGHT, lilka::colors::Graygrey);
        
        int centerX = iconX + CATALOG_ICON_WIDTH / 2;
        int centerY = iconY + CATALOG_ICON_HEIGHT / 2;
        int radius = 12;
        
        for (int i = 0; i < 8; i++) {
            float angle = (i * 45 + loadingFrame * 45) * 3.14159f / 180.0f;
            int dotX = centerX + cos(angle) * radius;
            int dotY = centerY + sin(angle) * radius;
            uint16_t color = (i == 0) ? lilka::colors::Cyan : lilka::colors::Graygrey;
            canvas->fillCircle(dotX, dotY, 3, color);
        }
        loadingFrame = (loadingFrame + 1) % 8;
    }

    // Draw navigation arrows
    int arrowY = iconY + CATALOG_ICON_HEIGHT / 2;

    if (currentIndex > 0 || currentPage > 0) {
        canvas->fillTriangle(15, arrowY, 25, arrowY - 10, 25, arrowY + 10, lilka::colors::Cyan);
    }

    int totalItems = entries.size();
    if (currentIndex < totalItems - 1 || currentPage < totalPages - 1) {
        canvas->fillTriangle(
            canvas->width() - 15,
            arrowY,
            canvas->width() - 25,
            arrowY - 10,
            canvas->width() - 25,
            arrowY + 10,
            lilka::colors::Cyan
        );
    }

    // Draw app name
    canvas->setFont(FONT_6x13);
    int nameY = iconY + CATALOG_ICON_HEIGHT + 12;
    canvas->setTextColor(lilka::colors::White);
    canvas->setCursor(10, nameY);
    canvas->print(entry.name);

    // Draw type badge
    const char* typeStr = nullptr;
    switch (entry.entryfile.type) {
        case EXEC_TYPE_LUA:
            typeStr = "LUA";
            break;
        case EXEC_TYPE_BINARY:
            typeStr = "BIN";
            break;
        default:
            break;
    }

    if (typeStr) {
        int badgeX = 10;
        int badgeY = nameY + 3;
        canvas->fillRoundRect(badgeX, badgeY, 30, 12, 2, lilka::colors::Dark_cyan);
        canvas->setTextColor(lilka::colors::White);
        canvas->setCursor(badgeX + 4, badgeY + 10);
        canvas->print(typeStr);
    }

    // Draw description with text wrap
    int descY = nameY + 22;
    int descHeight = canvas->height() - descY - 20; // Leave space for bottom bar
    canvas->setTextColor(lilka::colors::Light_gray);
    canvas->setTextBound(10, descY - 10, canvas->width() - 20, descHeight);
    canvas->setTextWrap(true);
    canvas->setCursor(10, descY);
    canvas->print(entry.short_description);
    canvas->setTextWrap(false);
    canvas->setTextBound(0, 0, canvas->width(), canvas->height());

    // Cover any overflow with black bar at bottom
    int bottomBarY = canvas->height() - 18;
    canvas->fillRect(0, bottomBarY, canvas->width(), 18, lilka::colors::Black);

    // Draw item counter and page counter
    canvas->setTextColor(lilka::colors::Graygrey);
    String counterStr = String(currentIndex + 1) + "/" + String(entries.size());
    if (totalPages > 1) {
        counterStr += " [" + String(currentPage + 1) + "/" + String(totalPages) + "]";
    }
    canvas->setCursor(canvas->width() / 2 - 40, canvas->height() - 6);
    canvas->print(counterStr);

    // Draw hint
    canvas->setCursor(10, canvas->height() - 6);
    canvas->setTextColor(lilka::colors::Cyan);
    canvas->print("A");
    canvas->setTextColor(lilka::colors::Graygrey);
    canvas->print("-OK B-Back");
}

void LilCatalogApp::handleInput() {
    lilka::State st = lilka::controller.getState();

    // Left - previous app (with wrap-around)
    if (st.left.justPressed) {
        if (currentIndex > 0) {
            currentIndex--;
            loadCurrentIcon();
        } else if (currentPage > 0) {
            loadPrevPage();
            currentIndex = entries.size() - 1;
            loadCurrentIcon();
        } else if (totalPages > 1) {
            // Wrap to last page, last app
            if (fetchIndex(totalPages - 1)) {
                currentIndex = entries.size() - 1;
                loadCurrentIcon();
            }
        } else {
            // Single page - wrap to last app
            currentIndex = entries.size() - 1;
            loadCurrentIcon();
        }
    }

    // Right - next app (with wrap-around)
    if (st.right.justPressed) {
        if (currentIndex < (int)entries.size() - 1) {
            currentIndex++;
            loadCurrentIcon();
        } else if (currentPage < totalPages - 1) {
            loadNextPage();
            currentIndex = 0;
            loadCurrentIcon();
        } else if (totalPages > 1) {
            // Wrap to first page, first app
            if (fetchIndex(0)) {
                currentIndex = 0;
                loadCurrentIcon();
            }
        } else {
            // Single page - wrap to first app
            currentIndex = 0;
            loadCurrentIcon();
        }
    }

    // A - select app
    if (st.a.justPressed && !entries.empty()) {
        currentEntry = entries[currentIndex];
        if (fetchEntryManifest(currentEntry.id)) {
            showEntry();
        }
    }

    // B - back to main menu
    if (st.b.justPressed) {
        showMainMenu();
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
        // Check if app has downloadable files
        if (currentEntry.entryfile.location.isEmpty()) {
            // No binary available - show info message
            entryMenu.addItem(
                K_S_LILCATALOG_NO_BINARY,
                nullptr,
                lilka::colors::Graygrey,
                K_S_LILCATALOG_EMPTY,
                [](void* ctx) {
                    LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
                    app->showAlert(K_S_LILCATALOG_NO_BINARY);
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
    }

    entryMenu.addItem(
        K_S_LILCATALOG_ENTRY_DESCRIPTION,
        nullptr,
        lilka::colors::Cyan,
        K_S_LILCATALOG_EMPTY,
        [](void* ctx) {
            LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
            app->showDescription();
        },
        this
    );

    entryMenu.addItem(
        K_S_LILCATALOG_BACK,
        nullptr,
        lilka::colors::White,
        K_S_LILCATALOG_EMPTY,
        [](void* ctx) {
            LilCatalogApp* app = static_cast<LilCatalogApp*>(ctx);
            app->state = LILCATALOG_LIST;
        },
        this
    );

    entryMenu.setCursor(0);
}

void LilCatalogApp::showDescription() {
    state = LILCATALOG_DESCRIPTION;
}

void LilCatalogApp::drawDescription() {
    canvas->fillScreen(lilka::colors::Black);
    canvas->setFont(FONT_6x13);

    // Title
    canvas->setTextColor(lilka::colors::White);
    canvas->setCursor(8, 14);

    char nameBuf[28];
    strncpy(nameBuf, currentEntry.name.c_str(), 24);
    nameBuf[24] = '\0';
    canvas->print(nameBuf);

    // Author
    canvas->setCursor(8, 30);
    canvas->setTextColor(lilka::colors::Cyan);
    canvas->print(K_S_LILCATALOG_ENTRY_DESCRIPTION_AUTHOR);
    canvas->setTextColor(lilka::colors::Light_gray);

    char authorBuf[20];
    strncpy(authorBuf, currentEntry.author.c_str(), 16);
    authorBuf[16] = '\0';
    canvas->print(authorBuf);

    // Description - simple fixed-width lines
    canvas->setTextColor(lilka::colors::White);

    String desc = currentEntry.short_description;
    if (desc.isEmpty()) {
        desc = currentEntry.description;
    }

    int lineY = 50;
    int maxCharsPerLine = 28; // ~168px at 6px per char
    int maxLines = 10;
    int charIndex = 0;
    int linesDrawn = 0;

    while (charIndex < (int)desc.length() && linesDrawn < maxLines) {
        // Find line end
        int lineEnd = charIndex + maxCharsPerLine;
        if (lineEnd > (int)desc.length()) {
            lineEnd = desc.length();
        }

        // Check for newline before that
        int newlinePos = desc.indexOf('\n', charIndex);
        if (newlinePos >= 0 && newlinePos < lineEnd) {
            lineEnd = newlinePos;
        }

        // Extract and print line
        char lineBuf[32];
        int lineLen = lineEnd - charIndex;
        if (lineLen > 28) lineLen = 28;
        strncpy(lineBuf, desc.c_str() + charIndex, lineLen);
        lineBuf[lineLen] = '\0';

        canvas->setCursor(8, lineY);
        canvas->print(lineBuf);

        lineY += 14;
        linesDrawn++;
        charIndex = lineEnd;
        if (charIndex < (int)desc.length() && desc[charIndex] == '\n') {
            charIndex++; // Skip newline
        }
    }

    // Hint at bottom
    canvas->setCursor(8, canvas->height() - 10);
    canvas->setTextColor(lilka::colors::Graygrey);
    canvas->print("A/B - Back");
}

void LilCatalogApp::loadNextPage() {
    if (currentPage < totalPages - 1) {
        if (fetchIndex(currentPage + 1)) {
            currentIndex = 0;
        }
    }
}

void LilCatalogApp::loadPrevPage() {
    if (currentPage > 0) {
        if (fetchIndex(currentPage - 1)) {
            currentIndex = entries.size() - 1;
        }
    }
}

void LilCatalogApp::showAlert(const String& message) {
    lilka::Alert alert(K_S_LILCATALOG_APP, message);
    alert.draw(canvas);
    queueDraw();
    while (!alert.isFinished()) {
        alert.update();
    }
}
