#pragma once

#include "keira/keira.h"
#include <ArduinoJson.h>

#include "keira/app.h"
#include "keira/appmanager.h"
// APPS:
#include "../modplayer/modplayer.h"
#include "../liltracker/liltracker.h"
#include "../lua/luarunner.h"
#include "../mjs/mjsrunner.h"
#include "../nes/nesapp.h"
#include "../fmanager/fmanager.h"

typedef struct {
    String type;
    String location;
} catalog_entry_file;

typedef struct {
    String origin;
} catalog_source_location;

typedef struct {
    catalog_source_location location;
} catalog_sources;

typedef struct {
    String name;
    String description;
    String short_description;
    String author;
    catalog_sources sources;
    std::vector<String> screenshots;
    String icon;
    String icon_min;
    String manifestName; // Store manifest directory name
    std::vector<catalog_entry_file> executionfiles;
} catalog_entry;

class LilCatalogApp : public App {
public:
    LilCatalogApp();

private:
    int num_pages = 0;
    int current_page = 0;
    int current_entry = 0;
    int num_entries = 0;
    JsonDocument* catalogDoc = nullptr;
    std::vector<catalog_entry> entries;
    uint16_t iconBuffer[64 * 64]; // Single static icon buffer for current entry
    bool hasIcon = false;

    void fetchCatalogIndex(); // Fetch index_{page}.json
    void parseCatalogIndex(); // Parse index and get manifest names
    void fetchManifestDetails(); // Fetch full details for current entry on demand
    void parseManifestDetails(String payload, catalog_entry& entry);
    void fetchIconForEntry(); // Download icon for current entry
    void downloadAppFiles(); // Download all execution files for installation

    bool validateEntry();
    void removeEntry();

    void doShowCatalog();
    void doShowCategory();
    void doShowEntry();
    void doShowEntryDescription();

    void run() override;

    void processMenu();
    void processBackButton();
};