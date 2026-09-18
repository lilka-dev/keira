#include "partmanager.h"
#include "keira/keira.h"
#include "keira/utils/string.h"

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
}

/////////////////////////////////////////////////////////////////////////////
// Menu configuration
/////////////////////////////////////////////////////////////////////////////
void PartManagerApp::loadPartListMenu() {
    // Backup cursor
    auto lastCursor = partListMenu.getCursor();
    partListMenu.clearItems();

    // partListMenu
    partListMenu.addActivationButton(K_BTN_EXIT);
    partListMenu.addActivationButton(K_BTN_CONTEXT_MENU);
    partListMenu.setTitle(K_S_PARTITION_TABLE);

    for (const auto& part : lilka::partitions) {
        partListMenu.addItem(
            part->getLabel(),
            0,
            lilka::colors::White,
            "", // TODO: add BOOT flag marking partition from which boot has been done
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
    // partOpsListMenu
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
void PartManagerApp::selectPart(size_t index) {
    for (const auto& partIndex : selectedParts) {
        if (index == partIndex) return;
    }
    selectedParts.push_back(index);
}
//---------------------------------------------------------------------------
void PartManagerApp::deselectPart(size_t index) {
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
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Callbacks [partListMenu]
/////////////////////////////////////////////////////////////////////////////
void PartManagerApp::onPartListMenu() {
    HANDLE_EXIT(partListMenu);

    auto cursor = partListMenu.getCursor();

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

    if (button == K_BTN_CONTEXT_MENU) {
        selectPart(cursor);
        partOpsListMenuShow();
    }
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Callbacks [partOpsListMenu]
/////////////////////////////////////////////////////////////////////////////
void PartManagerApp::onPartListOpsMenu() {
}
//---------------------------------------------------------------------------
void PartManagerApp::onPartListOpsBackup() {
    HANDLE_EXIT(partOpsListMenu);

    // If nothing selected, backup last partition in focus
    if (selectedParts.size() == 0) {
        auto cursor = partOpsListMenu.getCursor();
        selectPart(cursor);
    }

    String backupName = input("Enter backup name");

    String opCaveats = "This command would backup these partitions:\n";
    for (size_t i = 0; i < selectedParts.size(); i++) {
        if (i != selectedParts.size() - 1)
            opCaveats = opCaveats + lilka::partitions[selectedParts[i]]->getLabel() + ", ";
        else opCaveats = opCaveats + lilka::partitions[selectedParts[i]]->getLabel();
    }

    bool userConfirm = confirm(backupName, opCaveats);

    if (userConfirm) {
    }
}
//---------------------------------------------------------------------------
void PartManagerApp::onPartListOpsRestore() {
    HANDLE_EXIT(partOpsListMenu);
}
//---------------------------------------------------------------------------
void PartManagerApp::onPartListOpsSelect() {
    HANDLE_EXIT(partOpsListMenu);
    if (selectedParts.size() == 0) {
        auto cursor = partOpsListMenu.getCursor();
        selectPart(cursor);
    }
}
//---------------------------------------------------------------------------
void PartManagerApp::onPartListOpsSelectAll() {
    HANDLE_EXIT(partOpsListMenu);
}
//---------------------------------------------------------------------------
void PartManagerApp::onPartListOpsDeselect() {
}
//---------------------------------------------------------------------------
void PartManagerApp::onPartListOpsDeselectAll() {
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Callbacks [any]
/////////////////////////////////////////////////////////////////////////////
void PartManagerApp::onAnyMenuBack() {
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Callbacks [partitions[i]->erase()]
/////////////////////////////////////////////////////////////////////////////
bool PartManagerApp::onBackupChunk(lilka::Partition* part, const String& filename, size_t offset, long fSize) {
}
/////////////////////////////////////////////////////////////////////////////
// Callbacks [partitions[i]->flash()]
/////////////////////////////////////////////////////////////////////////////
bool PartManagerApp::onRestoreChunk(lilka::Partition* part, const String& filename, size_t offset, long fSize) {
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Drawing loops
/////////////////////////////////////////////////////////////////////////////
void PartManagerApp::backupListMenuShow() {
    DRAW_MENU(backupListMenu);
}
//---------------------------------------------------------------------------
void PartManagerApp::partOpsListMenuShow() {
    DRAW_MENU(partOpsListMenu);
}
//---------------------------------------------------------------------------
void PartManagerApp::run() {
    DRAW_MENU(partListMenu);
}
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
