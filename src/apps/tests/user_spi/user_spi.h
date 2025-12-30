#pragma once

#include "keira/app.h"

class UserSPIApp : public App {
public:
    UserSPIApp();

private:
    void run() override;
};
