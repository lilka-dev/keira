#pragma once

#include "keira/app.h"

class DiskApp : public App {
public:
    DiskApp();

private:
    void run() override;
};
