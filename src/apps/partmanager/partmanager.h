#pragma once
#include "keira/app.h"
#define PART_MGR_BACKUP_PATH "/sd/partmgr"
#define PART_MGR_MKDIR_MODE  0777

class PartManagerApp : public App {
public:
    explicit PartManagerApp();

private:
    // Dialogs:
    lilka::ProgressDialog backupProgress;
    lilka::ProgressDialog flashProgress;

    // Menus
    lilka::Menu backupListMenu;
    lilka::Menu partListMenu;
    lilka::Menu partOpsListMenu;

    // Menu configuration
    void loadBackupListMenu();
    void loadPartListMenu(); // [DONE]
    void loadPartOpsListMenu();

    // Actions
    void selectPart(size_t index); // [DONE]
    void deselectPart(size_t index); // [DONE]

    // Callbacks [backupListMenu]
    void onBackupListMenu();

    // Callbacks [partListMenu]
    void onPartListMenu();

    // Callbacks [partOpsListMenu]
    void onPartListOpsMenu();
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

    std::vector<size_t> selectedParts;
};
