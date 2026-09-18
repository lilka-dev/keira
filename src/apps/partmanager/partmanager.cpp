#include "partmanager.h"
#include "keira/keira.h"
#include "keira/utils/string.h"
#include "keira/debug.h"
#include <lilka/partitions.h>
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
    loadPartListMenu();
    loadPartOpsListMenu();
}

void PartManagerApp::loadBackupListMenu() {
    DIR* d = opendir(PART_MGR_BACKUP_PATH);
    if (!d) {
        mkdir(PART_MGR_BACKUP_PATH, PART_MGR_MKDIR_MODE);
    }
    // <>
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

    for (const auto& part : lilka::partitions) {
        partListMenu.addItem(
            part->getLabel(),
            0,
            lilka::colors::White,
            part->isRunning() ? "RUN" : "",
            LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListMenu),
            LILKA_MENU_CLBK_DATA_CAST(this)
        );
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

void PartManagerApp::loadPartOpsListMenu() {
    PM_DBG LEP;
    // partOpsListMenu
    partOpsListMenu.clearItems();
    partOpsListMenu.addActivationButton(K_BTN_EXIT);

    partOpsListMenu.addItem(
        "Backup",
        0,
        lilka::colors::White,
        "",
        LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListOpsBackup),
        LILKA_MENU_CLBK_DATA_CAST(this)
    );
    partOpsListMenu.addItem(
        "Restore",
        0,
        lilka::colors::White,
        "",
        LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListOpsRestore),
        LILKA_MENU_CLBK_DATA_CAST(this)
    );

    if ()

    if (selectedParts.size()) {
        for (size_t i = 0; i < selectedParts.size(); i++) {
            if (partOpsListMenu.getCursor() == selectedParts[i]) {
                partOpsListMenu.addItem(
                    "Deselect",
                    0,
                    lilka::colors::White,
                    "",
                    LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListOpsDeselect),
                    LILKA_MENU_CLBK_DATA_CAST(this)
                );
                break;
            }
        }

        partOpsListMenu.addItem(
            "Deselect all",
            0,
            lilka::colors::White,
            "",
            LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListOpsDeselectAll),
            LILKA_MENU_CLBK_DATA_CAST(this)
        );
    }

    partOpsListMenu.addItem(
        "Select",
        0,
        lilka::colors::White,
        "",
        LILKA_MENU_CLBK_CAST(&PartManagerApp::onPartListOpsSelect),
        LILKA_MENU_CLBK_DATA_CAST(this)
    );
    partOpsListMenu.addItem(
        "Select all",
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
void PartManagerApp::backup(const String& path, size_t index) {
    lastProgress = 101; // force first frame to draw :D

    progress.setTitle("Backup...");
    String partFilename = lilka::fileutils.joinPath(path, lilka::partitions[index]->getLabel()) + ".img";

    String message = String(lilka::partitions[index]->getLabel()) + String("\n->\n") + partFilename;
    progress.setMessage(message);

    lilka::partitions[index]->backup(
        partFilename,
        LILKA_PARTITIONS_ON_CHUNK_CLBK_CAST(&PartManagerApp::onBackupRestoreChunk),
        LILKA_PARTITIONS_ON_CHUNK__CLBK_DATA_CAST(this)
    );
}
//---------------------------------------------------------------------------
void PartManagerApp::restore(const String& path, size_t index) {
    lastProgress = 101; // force first frame to draw :D

    progress.setTitle("Restoring...");
    String partFilename = lilka::fileutils.joinPath(path, lilka::partitions[index]->getLabel()) + ".img";

    String message = partFilename + String("\n->\n") + String(lilka::partitions[index]->getLabel());
    progress.setMessage(message);

    lilka::partitions[index]->flash(
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

        lilka::Partition* part = lilka::partitions[cursor];

        alert(
            lilka::partitions[cursor]->getLabel(),
            StringFormat(
                K_S_LAUNCHER_PARTITION_FMT,
                String(lilka::partitions[cursor]->getAddress(), HEX).c_str(),
                String(lilka::partitions[cursor]->getSize(), HEX).c_str()
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

    String backupName = input("Enter backup name");

    // Enlisting all expected changes
    String opCaveats = "This command would backup these partitions:\n";
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
            backup(backupPath, selectedParts[i]);
        }

        // Clear selection
        selectedParts.clear();
    }
}
//---------------------------------------------------------------------------
void PartManagerApp::onPartListOpsRestore() {
    PM_DBG LEP;
    HANDLE_EXIT(partOpsListMenu);
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
    DRAW_MENU(backupListMenu);
}
//---------------------------------------------------------------------------
void PartManagerApp::partOpsListMenuShow() {
    PM_DBG LEP;
    DRAW_MENU(partOpsListMenu);
}
//---------------------------------------------------------------------------
void PartManagerApp::run() {
    PM_DBG LEP;
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
        canvas->printf("Selected %d entries", selectedParts.size());
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
// Partition from which we just boot isn't restorable(need to try that), but backupable
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
// word ``default`` for a backup have a special meaning
// and may be checked on a system launch proposing restoration of modified data
// in case this feature would be ever delivered
/////////////////////////////////////////////////////////////////////////////
