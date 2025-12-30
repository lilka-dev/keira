#pragma once

#include "keira/app.h"

class KeyboardApp : public App {
public:
    KeyboardApp();

private:
    void run() override;
};
