#pragma once
#include "keira/app.h"

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

    // Callbacks
    void onPartListMenu();
    void onPartListOpsMenu();
    void onAnyMenuBack();
    // Drawing loops
    void partOpsListMenuShow();
    void run() override;

    std::vector<size_t> selectedParts;
};
