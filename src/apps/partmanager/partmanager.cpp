#include "partmanager.h"
#include "keira/keira.h"
#include "keira/utils/string.h"

#include <lilka/partitions.h>

PartManagerApp::PartManagerApp() : App("PartManager") {
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
    partListMenu.addItem(
        K_S_MENU_BACK,
        0,
        lilka::colors::White,
        "",
        LILKA_MENU_CLBK_CAST(&PartManagerApp::onAnyMenuBack),
        LILKA_MENU_CLBK_DATA_CAST(this)
    );

    // partOpsListMenu
    partOpsListMenu.addActivationButton(K_BTN_EXIT);

    partOpsListMenu.addItem("Backup", 0, lilka::colors::White, "");
    partOpsListMenu.addItem("Restore", 0, lilka::colors::White, "");
    partOpsListMenu.addItem(
        K_S_MENU_BACK,
        0,
        lilka::colors::White,
        "",
        LILKA_MENU_CLBK_CAST(&PartManagerApp::onAnyMenuBack),
        LILKA_MENU_CLBK_DATA_CAST(this)
    );
}

// TODO: unify somehow?
void PartManagerApp::onAnyMenuBack() {
}

void PartManagerApp::onPartListOpsMenu() {
}

void PartManagerApp::onPartListMenu() {
    auto button = partListMenu.getButton();

    if (button == K_BTN_EXIT) {
        return;
    }

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

        // Draw next frame!
        partListMenu.isFinished();
        return;
    }

    if (button == K_BTN_CONTEXT_MENU) {
        selectedPart = cursor;
        partOpsListMenuShow();
    }
}

void PartManagerApp::partOpsListMenuShow() {
    while (!partOpsListMenu.isFinished()) {
        partOpsListMenu.update();
        partOpsListMenu.draw(canvas);
        queueDraw();
    }
}

void PartManagerApp::run() {
    while (!partListMenu.isFinished()) {
        partListMenu.update();
        partListMenu.draw(canvas);
        queueDraw();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
// BACKUPS
///////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////