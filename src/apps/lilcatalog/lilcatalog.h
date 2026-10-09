#pragma once

#include "keira/keira.h"

#include "keira/app.h"
#include "keira/appmanager.h"
// APPS:
#include "../madplayer/madplayer.h"
#include "../liltracker/liltracker.h"
#include "../lua/luarunner.h"
#include "../mjs/mjsrunner.h"
#include "../nes/nesapp.h"
#include "../fmanager/fmanager.h"
#include "../dynapp/dynapp.h"

// Base URL for catalog API
#define CATALOG_BASE_URL "https://catalog.lilka.dev"

// Icon size for mini icons (icon_min.bin is RGB565 raw format, 64x64 px)
#define CATALOG_ICON_WIDTH  64
#define CATALOG_ICON_HEIGHT 64
#define CATALOG_ICON_SIZE   (CATALOG_ICON_WIDTH * CATALOG_ICON_HEIGHT * 2) // 8192 bytes

// Cache folders (relative to category folder)
#define CATALOG_ICON_CACHE_FOLDER           "/icons"
#define CATALOG_MANIFEST_CACHE_FOLDER       "/manifests"
#define CATALOG_SHORT_MANIFEST_CACHE_FOLDER "/short_manifests"

// Home screen Lua wallpaper loaded by launcher
#define CATALOG_WALLPAPER_LUA_PATH "/sd/wallpaper.lua"

// HTTP timeout in milliseconds
#define CATALOG_HTTP_TIMEOUT       10000
#define CATALOG_HTTP_TIMEOUT_SHORT 5000 // For small files like short manifests

// Download buffer size
#define CATALOG_DOWNLOAD_BUFFER_SIZE 2048

// Execution file types
typedef enum {
    EXEC_TYPE_UNKNOWN,
    EXEC_TYPE_LUA,
    EXEC_TYPE_BINARY,
    EXEC_TYPE_ARCHIVE,
    EXEC_TYPE_IMAGE,
    EXEC_TYPE_DYNAPP
} ExecutionType;

// Catalog categories
typedef enum {
    CATALOG_CATEGORY_APPS,
    CATALOG_CATEGORY_WALLPAPERS
} CatalogCategory;

// Source info
typedef struct {
    String type; // "git"
    String origin; // Repository URL
} catalog_source;

// File info (for entryfile and files array)
typedef struct {
    ExecutionType type;
    String location;
    String description; // Optional description for additional files
} catalog_file;

// App entry from manifest
typedef struct {
    String id;
    CatalogCategory category = CATALOG_CATEGORY_APPS;
    String name;
    String short_description;
    String description;
    String author;
    String icon;
    String icon_min;
    catalog_source sources;
    catalog_file entryfile; // Main entry file
    std::vector<catalog_file> files; // Additional files
} catalog_entry;

// UI States
typedef enum {
    LILCATALOG_MAIN_MENU, // Main menu with options
    LILCATALOG_LIST, // Single-item view with left/right scroll
    LILCATALOG_INSTALLED_LIST, // Scrollable list of installed apps
    LILCATALOG_ENTRY, // Entry details menu
    LILCATALOG_DESCRIPTION // Full description view
} LilCatalogState;

class LilCatalogApp : public App {
public:
    LilCatalogApp();

private:
    LilCatalogState state = LILCATALOG_LIST;
    LilCatalogState entryReturnState = LILCATALOG_LIST; // State to return to from LILCATALOG_ENTRY

    String path_catalog_folder;
    CatalogCategory category = CATALOG_CATEGORY_APPS;

    // Pagination
    int currentPage = 0;
    int totalPages = 0;

    // Current entries loaded
    std::vector<catalog_entry> entries;
    catalog_entry currentEntry;

    // Single-item view state
    int currentIndex = 0; // Currently displayed app index
    int loadedIconIndex = -1; // Which entry's icon is in buffer

    // Icon buffer (64x64 RGB565)
    uint16_t iconBuffer[CATALOG_ICON_WIDTH * CATALOG_ICON_HEIGHT];
    bool iconLoaded = false;

    // Loading animation state
    uint8_t loadingFrame = 0;

    // Download buffer (reused across all download operations)
    uint8_t downloadBuffer[CATALOG_DOWNLOAD_BUFFER_SIZE];

    // Menus
    lilka::Menu mainMenu;
    lilka::Menu installedMenu;
    lilka::Menu entryMenu;

    // Category methods
    String getCategoryUrl();
    String getCategoryFolder();
    String getCategoryFolder(CatalogCategory cat);
    const char* getCategoryTitle();
    void openCategory(CatalogCategory cat);

    // Network methods
    String httpGet(const String& url, int timeout = CATALOG_HTTP_TIMEOUT);
    bool httpGetBinary(const String& url, uint8_t* buffer, size_t bufferSize, size_t* bytesRead);
    bool downloadFile(const String& url, const String& targetPath);
    bool downloadFileWithProgress(const String& url, const String& targetPath, const String& displayName);

    // Catalog methods
    bool fetchIndex(int page);
    bool fetchEntryManifest(const String& entryId);
    bool fetchEntryShortManifest(const String& entryId, catalog_entry& entry);
    bool fetchIcon(const String& entryId, const String& iconMinName);

    // Short manifest cache methods
    String getShortManifestCachePath(const String& entryId);
    bool saveShortManifestToCache(const String& entryId, const String& json);
    String loadShortManifestFromCache(const String& entryId);
    void clearShortManifestCache(CatalogCategory cat);

    // Icon cache methods
    String getIconCachePath(const String& entryId);
    bool loadIconFromCache(const String& entryId);
    bool saveIconToCache(const String& entryId);
    void loadCurrentIcon();
    void clearIconCache(CatalogCategory cat);

    // Manifest cache methods
    String getManifestCachePath(const String& entryId);
    bool saveManifestToCache(const String& entryId, const String& json);
    String loadManifestFromCache(const String& entryId);
    bool loadInstalledApps(); // Load apps from cache for offline mode
    void loadInstalledEntries(CatalogCategory cat);
    void clearManifestCache(CatalogCategory cat);
    void clearFolder(const String& path);

    // Parsing
    bool parseIndex(const String& json);
    bool parseManifest(const String& json, catalog_entry& entry);
    bool parseShortManifest(const String& json, catalog_entry& entry);
    ExecutionType parseExecutionType(const String& typeStr);
    ExecutionType detectTypeByExtension(const String& location);
    FileType executionTypeToFileType(ExecutionType type);

    // Entry management
    bool validateEntry();
    String getEntryTargetPath();
    String getEntryExecutablePath();
    void fetchEntry();
    void removeEntry();
    void executeEntry();
    void setWallpaper();

    // UI methods
    void showMainMenu();
    void drawAppView();
    void drawLoadingAnimation();
    void handleInput();
    void showInstalledMenu();
    void showEntry();
    void showDescription();
    void drawDescription();
    void loadNextPage();
    void loadPrevPage();

    void run() override;
    void showAlert(const String& message);
};
