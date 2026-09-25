#include "partmanager.h"
#include "keira/keira.h"
#include "keira/utils/string.h"
#include "keira/debug.h"
#include <lilka/partitions.h>
#include <lilka/controller.h>
#include <dirent.h>

//TODO: do not expose button
#define HANDLE_EXIT(MENU)           \
    auto button = MENU.getButton(); \
    if (button == K_BTN_EXIT) {     \
        return;                     \
    }

#define DRAW_MENU(MENU)          \
    while (!MENU.isFinished()) { \
        MENU.update();           \
        MENU.draw(canvas);       \
        queueDraw();             \
    }

#define DRAW_MENU_CONTINUE(MENU) MENU.isFinished()

PartManagerApp::PartManagerApp() : App("PartManager") {
}

/////////////////////////////////////////////////////////////////////////////
// checks
/////////////////////////////////////////////////////////////////////////////
bool PartManagerApp::isSelectedPart(size_t index) {
    for (const auto& partIndex : selectedParts) {
        if (index == partIndex) return true;
    }

    return false;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Menu configuration
/////////////////////////////////////////////////////////////////////////////
void PartManagerApp::loadBackupListMenu() {
    backupListMenu.setTitle(K_S_PART_MGR_BACKUPS);

    backupListMenu.clearItems();

    // Check if backup directory exists
    DIR* d = opendir(PART_MGR_BACKUP_PATH);
    if (!d) return;

    // Build list of required partition images
    std::vector<String> partImageList;
    for (size_t i = 0; i < selectedParts.size(); i++) {
        String partImage = lilka::partitions[selectedParts[i]]->getLabel();
        partImage += PART_MGR_IMG_EXT;
        partImageList.push_back(partImage);
    }

    // Iterate over directory entries[backups]
    struct dirent* backupEntry = NULL;
    while (backupEntry = readdir(d)) {
        // skip all except dirs
        if (backupEntry->d_type != DT_DIR) continue;

        String backupDirPath = lilka::fileutils.joinPath(PART_MGR_BACKUP_PATH, backupEntry->d_name);
        // skip non openable backups
        DIR* d2 = opendir(backupDirPath.c_str());
        if (!d2) continue;

        // build image list for current backup
        std::vector<String> backupImageList;
        struct dirent* backupFileEntry = NULL;
        size_t extLen = strlen(PART_MGR_IMG_EXT);
        while (backupFileEntry = readdir(d2)) {
            // skip all except files
            if (backupFileEntry->d_type != DT_REG) continue;

            // check extension
            char* mbExt = backupFileEntry->d_name + strlen(backupFileEntry->d_name) - extLen;
            if (strcasecmp(mbExt, PART_MGR_IMG_EXT) == 0) {
                backupImageList.push_back(String(backupFileEntry->d_name));
            }
        }
        closedir(d2);

        // Determine if backup contains all needed images for restore
        bool backupComplete = true;
        for (size_t i = 0; i < partImageList.size(); i++) {
            bool imageFound = false;
            for (size_t j = 0; j < backupImageList.size(); j++) {
                if (strcasecmp(partImageList[i].c_str(), backupImageList[j].c_str()) == 0) {
                    imageFound = true;
                    break;
                }
            }
            if (!imageFound) {
                backupComplete = false;
                break;
            }
        }

        // If satisfies demands, add to list
        if (backupComplete) {
            backupListMenu.addItem(
                backupEntry->d_name,
                0,
                lilka::colors::White,
                "",
                LILKA_MENU_CLBK_CAST(&PartManagerApp::onBackupListMenu),
                LILKA_MENU_CLBK_DATA_CAST(this)
            );
        }
        // TODO: it seems that process can take a while
        // Add into it notification/interruption
    }

    // Back
    backupListMenu.addItem(
        K_S_MENU_BACK,
        0,
        lilka::colors::White,
        "",
        LILKA_MENU_CLBK_CAST(&PartManagerApp::onAnyMenuBack),
        LILKA_MENU_CLBK_DATA_CAST(this)
    );

    closedir(d);
}
//---------------------------------------------------------------------------
void PartManagerApp::loadPartListMenu() {
    PM_DBG LEP;
    // Backup cursor
    auto lastCursor = partListMenu.getCursor();
    partListMenu.clearItems();

    // partListMenu
    partListMenu.addActivationButton(K_BTN_EXIT);
    partListMenu.addActivationButton(K_BTN_CONTEXT_MENU);
    partListMenu.addActivationButton(PART_MGR_SELECT_TOGGLE_BUTTON);
    partListMenu.setTitle(K_S_PARTITION_TABLE);

    size_t i = 0;
    for (const auto& part : lilka::partitions) {
        partListMenu.addItem(
            part->getLabel(),
            isSelectedPart(i) ? PART_MGR_SELECTED_PART_ICON : PART_MGR_PART_ICON,
            lilka::colors::White,
            part->isRunning() ? "RUN" : "",
            LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListMenu),
            LILKA_MENU_CLBK_DATA_CAST(this)
        );
        i++;
    }
    // Back
    partListMenu.addItem(
        K_S_MENU_BACK,
        0,
        lilka::colors::White,
        "",
        LILKA_MENU_CLBK_CAST(&PartManagerApp::onAnyMenuBack),
        LILKA_MENU_CLBK_DATA_CAST(this)
    );

    // Restore cursor
    bool isCursorValid = ((lastCursor > 0) && (lastCursor <= lilka::partitions.size()));
    if (isCursorValid) partListMenu.setCursor(lastCursor);
}
//---------------------------------------------------------------------------
void PartManagerApp::loadPartOpsListMenu() {
    PM_DBG LEP;
    // partOpsListMenu
    partOpsListMenu.setTitle(K_S_PART_MGR_OPTIONS);
    partOpsListMenu.clearItems();
    partOpsListMenu.addActivationButton(K_BTN_EXIT);

    auto partCursor = partListMenu.getCursor();
    auto curSelected = isSelectedPart(partCursor);
    auto countSelected = selectedParts.size();

    partOpsListMenu.addItem(
        K_S_PART_MGR_OPT_BACKUP,
        0,
        lilka::colors::White,
        "",
        LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListOpsBackup),
        LILKA_MENU_CLBK_DATA_CAST(this)
    );

    partOpsListMenu.addItem(
        K_S_PART_MGR_OPT_RESTORE,
        0,
        lilka::colors::White,
        "",
        LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListOpsRestore),
        LILKA_MENU_CLBK_DATA_CAST(this)
    );

    if (curSelected)
        partOpsListMenu.addItem(
            K_S_PART_MGR_OPT_DESELECT,
            0,
            lilka::colors::White,
            "",
            LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListOpsDeselect),
            LILKA_MENU_CLBK_DATA_CAST(this)
        );

    if (countSelected)
        partOpsListMenu.addItem(
            K_S_PART_MGR_OPT_DESELECT_ALL,
            0,
            lilka::colors::White,
            "",
            LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListOpsDeselectAll),
            LILKA_MENU_CLBK_DATA_CAST(this)
        );

    if (!curSelected)
        partOpsListMenu.addItem(
            K_S_PART_MGR_OPT_SELECT,
            0,
            lilka::colors::White,
            "",
            LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListOpsSelect),
            LILKA_MENU_CLBK_DATA_CAST(this)
        );

    if (countSelected != lilka::partitions.size())
        partOpsListMenu.addItem(
            K_S_PART_MGR_OPT_SELECT_ALL,
            0,
            lilka::colors::White,
            "",
            LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListOpsSelectAll),
            LILKA_MENU_CLBK_DATA_CAST(this)
        );

    // Back
    partOpsListMenu.addItem(
        K_S_MENU_BACK,
        0,
        lilka::colors::White,
        "",
        LILKA_MENU_CLBK_CAST(&PartManagerApp::onAnyMenuBack),
        LILKA_MENU_CLBK_DATA_CAST(this)
    );
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Actions
/////////////////////////////////////////////////////////////////////////////
bool PartManagerApp::backup(const String& path, size_t index) {
    lastProgress = 101; // force first frame to draw :D

    progress.setTitle(K_S_PART_MGR_BACKUP);
    String partFilename = lilka::fileutils.joinPath(path, lilka::partitions[index]->getLabel()) + PART_MGR_IMG_EXT;

    String message = String(lilka::partitions[index]->getLabel()) + String("\n->\n") + partFilename;
    progress.setMessage(message);

    return lilka::partitions[index]->backup(
        partFilename,
        LILKA_PARTITIONS_ON_CHUNK_CLBK_CAST(&PartManagerApp::onBackupRestoreChunk),
        LILKA_PARTITIONS_ON_CHUNK__CLBK_DATA_CAST(this)
    );
}
//---------------------------------------------------------------------------
bool PartManagerApp::restore(const String& path, size_t index) {
    lastProgress = 101; // force first frame to draw :D

    progress.setTitle(K_S_PART_MGR_OPT_RESTORE);
    String partFilename = lilka::fileutils.joinPath(path, lilka::partitions[index]->getLabel()) + PART_MGR_IMG_EXT;

    String message = partFilename + String("\n->\n") + String(lilka::partitions[index]->getLabel());
    progress.setMessage(message);

    return lilka::partitions[index]->flash(
        partFilename,
        LILKA_PARTITIONS_ON_CHUNK_CLBK_CAST(&PartManagerApp::onBackupRestoreChunk),
        LILKA_PARTITIONS_ON_CHUNK__CLBK_DATA_CAST(this)
    );
}
//---------------------------------------------------------------------------
void PartManagerApp::selectPart(size_t index) {
    PM_DBG LEP;
    if (index >= lilka::partitions.size()) return;

    for (const auto& partIndex : selectedParts) {
        if (index == partIndex) return;
    }
    selectedParts.push_back(index);
}
//---------------------------------------------------------------------------
void PartManagerApp::deselectPart(size_t index) {
    PM_DBG LEP;
    for (size_t i = 0; i < selectedParts.size(); i++) {
        if (index == selectedParts[i]) {
            selectedParts.erase(selectedParts.begin() + i);
            break;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Callbacks [backupListMenu]
/////////////////////////////////////////////////////////////////////////////
void PartManagerApp::onBackupListMenu() {
    PM_DBG LEP;
    HANDLE_EXIT(backupListMenu);

    // Determine selected backup name
    auto cursor = backupListMenu.getCursor();
    lilka::MenuItem backupNameItem;
    backupListMenu.getItem(cursor, &backupNameItem);
    String backupName = backupNameItem.title;

    String backupPath = lilka::fileutils.joinPath(PART_MGR_BACKUP_PATH, backupName);

    if (button == K_BTN_OPEN) {
        // Enlisting all expected changes
        String opCaveats = K_S_PART_MGR_BACKUP_CAVEATS;
        for (size_t i = 0; i < selectedParts.size(); i++) {
            if (i != selectedParts.size() - 1)
                opCaveats = opCaveats + lilka::partitions[selectedParts[i]]->getLabel() + ", ";
            else opCaveats = opCaveats + lilka::partitions[selectedParts[i]]->getLabel();
        }

        // Ask for confirmation
        bool userConfirm = confirm(backupName, opCaveats);

        if (userConfirm) {
            // Restore each selected partition
            for (size_t i = 0; i < selectedParts.size(); i++) {
                // Break on error or user interrupt
                if (!restore(backupPath, selectedParts[i])) break;
            }

            // Clear selection
            selectedParts.clear();
            // Update icons
            loadPartListMenu();
        }
    }
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Callbacks [partListMenu]
/////////////////////////////////////////////////////////////////////////////
void PartManagerApp::onPartListMenu() {
    PM_DBG LEP;
    HANDLE_EXIT(partListMenu);

    auto cursor = partListMenu.getCursor();
    // Information about partition
    if (button == K_BTN_OPEN) {
        if (cursor >= lilka::partitions.size()) return;

        const lilka::Partition* part = lilka::partitions[cursor];

        alert(
            part->getLabel(),
            StringFormat(
                K_S_LAUNCHER_PARTITION_FMT,
                String(part->getAddress(), HEX).c_str(),
                String(part->getSize(), HEX).c_str()
            )
        );

        DRAW_MENU_CONTINUE(partListMenu);
        return;
    }
    // Context menu
    if (button == K_BTN_CONTEXT_MENU) {
        // selectPart(cursor);
        partOpsListMenuShow();
        DRAW_MENU_CONTINUE(partListMenu);
        return;
    }

    // Select/Deselect toggle
    if (button == PART_MGR_SELECT_TOGGLE_BUTTON) {
        if (isSelectedPart(cursor)) deselectPart(cursor);
        else selectPart(cursor);

        // Reload part list for icons change
        loadPartListMenu();

        DRAW_MENU_CONTINUE(partListMenu);
        return;
    }
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Callbacks [partOpsListMenu]
/////////////////////////////////////////////////////////////////////////////
void PartManagerApp::onPartListOpsBackup() {
    PM_DBG LEP;
    HANDLE_EXIT(partOpsListMenu);

    // If nothing selected, backup last partition in focus
    if (selectedParts.size() == 0) {
        auto cursor = partListMenu.getCursor();
        selectPart(cursor);
    }

    String backupName = input(K_S_PART_MGR_ENTER_BACKUP_NAME);

    // Enlisting all expected changes
    String opCaveats = K_S_PART_MGR_BACKUP_CAVEAETS;
    for (size_t i = 0; i < selectedParts.size(); i++) {
        if (i != selectedParts.size() - 1)
            opCaveats = opCaveats + lilka::partitions[selectedParts[i]]->getLabel() + ", ";
        else opCaveats = opCaveats + lilka::partitions[selectedParts[i]]->getLabel();
    }

    // Ask for confirmation
    bool userConfirm = confirm(backupName, opCaveats);

    if (userConfirm) {
        // Ensure backup folder even exist
        mkdir(PART_MGR_BACKUP_PATH, PART_MGR_MKDIR_MODE);
        // Create folder for backup
        String backupPath = lilka::fileutils.joinPath(PART_MGR_BACKUP_PATH, backupName);
        mkdir(backupPath.c_str(), PART_MGR_MKDIR_MODE);

        String backupPartPath;

        // Backup each selected partition
        for (size_t i = 0; i < selectedParts.size(); i++) {
            // Break on error or user interrupt
            if (!backup(backupPath, selectedParts[i])) break;
        }

        // Clear selection
        selectedParts.clear();
        // Update icons
        loadPartListMenu();
    }
}
//---------------------------------------------------------------------------
void PartManagerApp::onPartListOpsRestore() {
    PM_DBG LEP;
    HANDLE_EXIT(partOpsListMenu);

    // If nothing selected, select last partition in focus
    if (selectedParts.size() == 0) {
        auto cursor = partListMenu.getCursor();
        selectPart(cursor);
    }

    // Find out if running partition selected
    for (size_t i = 0; i < selectedParts.size(); i++) {
        auto partIndex = selectedParts[i];
        lilka::Partition* curPart = lilka::partitions[partIndex];

        if (curPart->isRunning()) {
            String alertMessage = StringFormat(K_S_PART_MGR_PART_RUNNING_FMT, curPart->getLabel());
            alert("", alertMessage);

            deselectPart(partIndex);

            break;
        }
    }

    // Load backups satisfying list of selected partitions
    loadBackupListMenu();

    // Display them
    backupListMenuShow();

    // Ensure we have a clean selection list before return
    selectedParts.clear();
    // Update icons
    loadPartListMenu();
}
//---------------------------------------------------------------------------
void PartManagerApp::onPartListOpsSelect() {
    PM_DBG LEP;
    HANDLE_EXIT(partOpsListMenu);

    auto cursor = partListMenu.getCursor();

    selectPart(cursor);
}
//---------------------------------------------------------------------------
void PartManagerApp::onPartListOpsSelectAll() {
    PM_DBG LEP;
    HANDLE_EXIT(partOpsListMenu);

    for (size_t i = 0; i < lilka::partitions.size(); i++)
        selectPart(i);
}
//---------------------------------------------------------------------------
void PartManagerApp::onPartListOpsDeselect() {
    PM_DBG LEP;
    HANDLE_EXIT(partOpsListMenu);

    auto cursor = partListMenu.getCursor();

    deselectPart(cursor);
}
//---------------------------------------------------------------------------
void PartManagerApp::onPartListOpsDeselectAll() {
    PM_DBG LEP;
    HANDLE_EXIT(partOpsListMenu);

    for (size_t i = 0; i < lilka::partitions.size(); i++)
        deselectPart(i);
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Callbacks [any]
/////////////////////////////////////////////////////////////////////////////
void PartManagerApp::onAnyMenuBack() {
    PM_DBG LEP;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Callbacks [partitions[i]->erase()/flash]
/////////////////////////////////////////////////////////////////////////////
bool PartManagerApp::onBackupRestoreChunk(lilka::Partition* part, const String& filename, size_t offset, long fSize) {
    // PM_DBG LEP;
    auto ctrlState = lilka::controller.getState();
    if (ctrlState.a.justPressed) {
        if (confirm(K_S_ARE_YOU_SURE, K_S_PART_MGR_INTERRUPT_CONFIRM)) return false;
    }

    size_t currentProgress = (offset * 100) / fSize;

    if (lastProgress != currentProgress) {
        progress.setProgress(currentProgress);
        lastProgress = currentProgress;
        progress.draw(canvas);

        queueDraw();
    }

    return true;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Drawing loops
/////////////////////////////////////////////////////////////////////////////
void PartManagerApp::backupListMenuShow() {
    PM_DBG LEP;
    loadBackupListMenu();
    DRAW_MENU(backupListMenu);
}
//---------------------------------------------------------------------------
void PartManagerApp::partOpsListMenuShow() {
    PM_DBG LEP;
    loadPartOpsListMenu();
    DRAW_MENU(partOpsListMenu);
}
//---------------------------------------------------------------------------
void PartManagerApp::run() {
    PM_DBG LEP;
    loadPartListMenu();
    DRAW_MENU(partListMenu);
}
/////////////////////////////////////////////////////////////////////////////
// Drawing StatusBar //? Toolbar, right?
/////////////////////////////////////////////////////////////////////////////
void PartManagerApp::queueDraw() {
    canvas->fillRect(
        0, canvas->height() - STATUS_BAR_HEIGHT, canvas->width(), STATUS_BAR_HEIGHT, STATUS_BAR_FILL_COLOR
    );

    canvas->setCursor(STATUS_BAR_SAFE_DISTANCE, canvas->height() - 20 / 2); // FONT_Y / 2
    canvas->setFont(FONT_8x13);

    canvas->setTextBound(
        STATUS_BAR_SAFE_DISTANCE, canvas->height() - STATUS_BAR_HEIGHT, STATUS_BAR_WIDTH, STATUS_BAR_HEIGHT
    );
    if (selectedParts.size() > 0) {
        canvas->printf(K_S_PART_MGR_SELECTED_ENTRIES_FMT, selectedParts.size());
    }
    App::queueDraw();
}
/////////////////////////////////////////////////////////////////////////////

#undef DRAW_MENU
#undef HANDLE_EXIT
#undef DRAW_MENU_CONTINUE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// BACKUPS
/////////////////////////////////////////////////////////////////////////////
// User can select one or mutiple partitions and create a backup
//
// Each backup have a name, which allows us to distinguish backups between each other
// since we can't just use datetime assuming it's always year 1970,
// due to our RTC clock behaviour
//
// On a filesystem it's just a folder, which consists from one or multiple *.img files
// Name of each file is a label of partition from which backup was taken from
//
// Partition from which we just boot isn't restorable, but backupable
// anyways
//
// NOTE: Backups use no encryption, therefore any secrets(wifi password in nvs) can be
// exposed
/////////////////////////////////////////////////////////////////////////////
//
// Backup flow->
//
// User selects one or multiple items on partListMenu -> partOpsMenu -> Backup
// for each selectedParts we do make backups
//
// Restore flow->
//
// User selects one or multiple items on partListMenu -> partOpsMenu -> Restore
// -> iterate over backups and filter ones whose match all selectedParts criteria
// -> restore
//
// word ``default`` for a backup may have a special meaning in future, in case
// we would like to add auto restore option on each boot, via validation of
// partition checksum in case this feature would be ever delivered
/////////////////////////////////////////////////////////////////////////////
