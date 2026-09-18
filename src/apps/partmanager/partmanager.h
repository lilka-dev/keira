#pragma once
#include "keira/app.h"
#include <lilka/partitions.h>
#define PART_MGR_BACKUP_PATH "/sd/partmgr"
#define PART_MGR_MKDIR_MODE  0777

#define PART_MGR_SELECT_TOGGLE_BUTTON       lilka::Button::C

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
    bool isSelectedPart(size_t index);

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
    void onPartListMenu();

    // Callbacks [partOpsListMenu]
    void onPartListOpsBackup();
    void onPartListOpsRestore();
    void onPartListOpsSelect();
    void onPartListOpsSelectAll();
    void onPartListOpsDeselect();
    void onPartListOpsDeselectAll();

    // Callbacks [anyMenu]
    void onAnyMenuBack();

    // Callbacks [partitions[i]->erase()]
    bool onBackupChunk(lilka::Partition* part, const String& filename, size_t offset, long fSize);

    // Callbacks [partitions[i]->flash()]
    bool onRestoreChunk(lilka::Partition* part, const String& filename, size_t offset, long fSize);

    // Drawing loops
    void backupListMenuShow(); // [DONE]
    void partOpsListMenuShow(); // [DONE]
    void run() override; // [DONE]

    size_t lastProgress = 0;
    std::vector<size_t> selectedParts;
};
