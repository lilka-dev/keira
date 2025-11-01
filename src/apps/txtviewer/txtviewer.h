#pragma once
#include "app.h"
#include "keira/keira.h"
#include "txtview.h"

class TxtViewerApp : public App {
public:
    TxtViewerApp(String fPath);
    TxtView tView;

private:
    TxtViewerApp();
    void run() override;
};
