#pragma once

#include "keira/keira.h"

#include "keira/app.h"
#include "keira/appmanager.h"
// APPS:
#include "../modplayer/modplayer.h"
#include "../liltracker/liltracker.h"
#include "../lua/luarunner.h"
#include "../mjs/mjsrunner.h"
#include "../nes/nesapp.h"
#include "../fmanager/fmanager.h"

// Base URL for catalog API
#define CATALOG_BASE_URL               "https://catalog.lilka.dev"
#define CATALOG_APPS_INDEX_URL         CATALOG_BASE_URL "/apps/index_%d.json"
#define CATALOG_MODS_INDEX_URL         CATALOG_BASE_URL "/mods/index_%d.json"
#define CATALOG_APP_MANIFEST_URL       CATALOG_BASE_URL "/apps/%s/index.json"
#define CATALOG_APP_SHORT_MANIFEST_URL CATALOG_BASE_URL "/apps/%s/index_short.json"
#define CATALOG_APP_STATIC_URL         CATALOG_BASE_URL "/apps/%s/static/%s"
#define CATALOG_MOD_MANIFEST_URL       CATALOG_BASE_URL "/mods/%s/index.json"
#define CATALOG_MOD_STATIC_URL         CATALOG_BASE_URL "/mods/%s/static/%s"

// Icon size for mini icons (icon_min.bin is RGB565 raw format)
#define CATALOG_ICON_MIN_WIDTH  32
#define CATALOG_ICON_MIN_HEIGHT 32
#define CATALOG_ICON_MIN_SIZE   (CATALOG_ICON_MIN_WIDTH * CATALOG_ICON_MIN_HEIGHT * 2)

// Icon cache paths
#define CATALOG_ICON_CACHE_FOLDER "/lilcatalog/icons"

// Catalog types
typedef enum { CATALOG_TYPE_APPS, CATALOG_TYPE_MODS } CatalogType;

// Execution file types from the new catalog API
typedef enum { EXEC_TYPE_UNKNOWN, EXEC_TYPE_LUA, EXEC_TYPE_BINARY, EXEC_TYPE_ARCHIVE, EXEC_TYPE_IMAGE } ExecutionType;

// Source info
typedef struct {
    String type; // "git"
    String origin; // Repository URL
} catalog_source;

// Execution file info
typedef struct {
    ExecutionType type;
    String location; // Filename in static folder or external URL
} catalog_execution_file;

// Mod file info (for mods)
typedef struct {
    String name;
    String location;
} catalog_mod_file;

// App/Mod entry from manifest
typedef struct {
    String id; // Directory name (e.g., "matrix-rain")
    String name; // Display name
    String short_description;
    String description; // Full markdown description
    String changelog;
    String author;
    String icon; // Icon filename
    String icon_min; // Mini icon filename (32x32 RGB565 raw)
    std::vector<String> screenshots;
    catalog_source sources;
    catalog_execution_file executionfile;
    std::vector<catalog_mod_file> modfiles; // For mods only
} catalog_entry;

// Index page data
typedef struct {
    int page;
    int total_pages;
    std::vector<String> manifests;
} catalog_index;

// UI States
typedef enum {
    LILCATALOG_TYPE_SELECT, // Select apps/mods
    LILCATALOG_LIST, // List of entries
    LILCATALOG_ENTRY, // Entry details
    LILCATALOG_ENTRY_DESCRIPTION // Full description view
} LilCatalogState;

class LilCatalogApp : public App {
public:
    LilCatalogApp();

private:
    LilCatalogState state = LILCATALOG_TYPE_SELECT;
    CatalogType currentType = CATALOG_TYPE_APPS;

    String path_catalog_folder;

    // Pagination
    int currentPage = 0;
    int totalPages = 0;

    // Current entries loaded
    std::vector<catalog_entry> entries;
    catalog_entry currentEntry;
    int currentIconIndex = -1;

    // Single shared icon buffer (saves RAM - only one icon loaded at a time)
    uint16_t iconBuffer[CATALOG_ICON_MIN_WIDTH * CATALOG_ICON_MIN_HEIGHT];
    bool iconBufferValid = false;

    // Menus
    lilka::Menu typeMenu;
    lilka::Menu listMenu;
    lilka::Menu entryMenu;

    // Network methods
    String httpGet(const String& url);
    bool httpGetBinary(const String& url, uint8_t* buffer, size_t bufferSize, size_t* bytesRead);
    bool downloadFile(const String& url, const String& targetPath);
    bool downloadFileWithProgress(const String& url, const String& targetPath, const String& displayName);

    // Catalog methods
    bool fetchIndex(int page);
    bool fetchEntryManifest(const String& entryId);
    bool fetchEntryShortManifest(const String& entryId, catalog_entry& entry);
    bool fetchMiniIcon(const String& entryId, const String& iconMinName, uint16_t* buffer);

    // Icon cache methods (stores icons on SD card)
    String getIconCachePath(const String& entryId);
    bool loadIconFromCache(const String& entryId);
    bool saveIconToCache(const String& entryId);
    bool loadIconForEntry(int entryIndex);
    void clearIconCache();
    void clearAllIconCache();

    // Parsing
    bool parseIndex(const String& json);
    bool parseManifest(const String& json, catalog_entry& entry);
    bool parseShortManifest(const String& json, catalog_entry& entry);
    ExecutionType parseExecutionType(const String& typeStr);
    FileType executionTypeToFileType(ExecutionType type);

    // Entry management
    bool validateEntry();
    String getEntryTargetPath();
    String getEntryExecutablePath();
    void fetchEntry();
    void removeEntry();
    void executeEntry();
    void fileLoadAsRom(const String& path);

    // UI methods
    void showTypeSelect();
    void showEntryList();
    void showEntry();
    void showEntryDescription();
    void loadNextPage();
    void loadPrevPage();

    void run() override;
    void processMenu();
    void processBackButton();

    void showAlert(const String& message);
};
