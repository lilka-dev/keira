#pragma once
#include "keira/app.h"
#include <lilka/partitions.h>
/////////////////////////////////////////////////////////////////////////////
// General settings
/////////////////////////////////////////////////////////////////////////////
#define PART_MGR_BACKUP_PATH "/sd/partmgr"
#define PART_MGR_IMG_EXT     ".img"
#define PART_MGR_MKDIR_MODE  0777
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Buttons:
/////////////////////////////////////////////////////////////////////////////
#define PART_MGR_SELECT_TOGGLE_BUTTON lilka::Button::C
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Icons:
/////////////////////////////////////////////////////////////////////////////
#include "apps/icons/normalfile.h"
#include "apps/icons/selectedfile.h"
#define PART_MGR_PART_ICON          &normalfile_img
#define PART_MGR_SELECTED_PART_ICON &selectedfile_img
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// TODO: Requires unification as additional feature of App
// StatusBar(ToolBar):
/////////////////////////////////////////////////////////////////////////////
#define STATUS_BAR_HEIGHT        30
#define STATUS_BAR_SAFE_DISTANCE 38
#define STATUS_BAR_WIDTH         canvas->width() - STATUS_BAR_SAFE_DISTANCE * 2
#define STATUS_BAR_TEXT_COLOR    lilka::colors::White
#define STATUS_BAR_FILL_COLOR    lilka::colors::Black
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Debug
/////////////////////////////////////////////////////////////////////////////
// Uncomment this line to get some debuging information
//#define PARTMANAGER_DEBUG
#ifdef PARTMANAGER_DEBUG
#    define PM_DBG if (1)
#else
#    define PM_DBG if (0)
#endif
/////////////////////////////////////////////////////////////////////////////

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
    void loadPartListMenu();
    void loadPartOpsListMenu();

    // Actions
    bool backup(const String& path, size_t index);
    bool restore(const String& path, size_t index);
    void selectPart(size_t index);
    void deselectPart(size_t index);

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

    // Callbacks [partitions[i]->erase()/flash]
    bool onBackupRestoreChunk(lilka::Partition* part, const String& filename, size_t offset, long fSize);

    // Drawing loops
    void backupListMenuShow();
    void partOpsListMenuShow();
    void run() override;

    // TODO: Unify it in App
    // Drawing StatusBar //? Toolbar, right?
    void queueDraw();

    size_t lastProgress = 0;
    // cursor of partListMenu as well as index in lilka::partitions
    std::vector<size_t> selectedParts;
};
