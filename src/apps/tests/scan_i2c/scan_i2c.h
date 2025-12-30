#pragma once

#include "keira/app.h"

class ScanI2CApp : public App {
public:
    ScanI2CApp();

private:
    void run() override;
};
