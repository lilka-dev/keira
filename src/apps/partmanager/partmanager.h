#pragma once
#include "keira/app.h"
#include <lilka/partitions.h>
#define PART_MGR_BACKUP_PATH          "/sd/partmgr"
#define PART_MGR_IMG_EXT              ".img"
#define PART_MGR_MKDIR_MODE           0777

#define PART_MGR_SELECT_TOGGLE_BUTTON lilka::Button::C

// STATUS BAR SETTINGS:  /////////////////////////////////////////////////////
#define STATUS_BAR_HEIGHT        30
#define STATUS_BAR_SAFE_DISTANCE 38
#define STATUS_BAR_WIDTH         canvas->width() - STATUS_BAR_SAFE_DISTANCE * 2
#define STATUS_BAR_TEXT_COLOR    lilka::colors::White
#define STATUS_BAR_FILL_COLOR    lilka::colors::Black

#define PARTMANAGER_DEBUG

#ifdef PARTMANAGER_DEBUG
#    define PM_DBG if (1)
#else
#    define PM_DBG if (0)
#endif

class PartManagerApp : public App {
public:
    explicit PartManagerApp();

private:
    // Dialogs:
    lilka::ProgressDialog progress;

    // Menus
    lilka::Menu backupListMenu;
    lilka::Menu partListMenu;
    lilka::Menu partOpsListMenu;

    // checks
    bool isSelectedPart(size_t index); // [DONE]

    // Menu configuration
    void loadBackupListMenu();
    void loadPartListMenu(); // [DONE]
    void loadPartOpsListMenu(); // [DONE]

    // Actions
    void backup(const String& path, size_t index);
    void restore(const String& path, size_t index);
    void selectPart(size_t index); // [DONE]
    void deselectPart(size_t index); // [DONE]

    // Callbacks [backupListMenu]
    void onBackupListMenu();

    // Callbacks [partListMenu]
    void onPartListMenu(); // [DONE]

    // Callbacks [partOpsListMenu]
    void onPartListOpsBackup(); // [DONE]
    void onPartListOpsRestore();
    void onPartListOpsSelect(); // [DONE]
    void onPartListOpsSelectAll(); // [DONE]
    void onPartListOpsDeselect(); // [DONE]
    void onPartListOpsDeselectAll(); // [DONE]

    // Callbacks [anyMenu]
    void onAnyMenuBack();

    // Callbacks [partitions[i]->erase()/flash]
    bool onBackupRestoreChunk(lilka::Partition* part, const String& filename, size_t offset, long fSize); // [DONE]

    // Drawing loops
    void backupListMenuShow(); // [DONE]
    void partOpsListMenuShow(); // [DONE]
    void run() override; // [DONE]

    // TODO: Unify it in App
    // Drawing StatusBar //? Toolbar, right?
    void queueDraw(); // [DONE]

    size_t lastProgress = 0;
    std::vector<size_t> selectedParts;
};
