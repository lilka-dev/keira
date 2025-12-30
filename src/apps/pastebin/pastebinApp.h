#pragma once
#include "keira/keira.h"
#include "keira/app.h"

class pastebinApp : public App {
public:
    pastebinApp();

private:
    void run() override;
    void uiLoop();
};