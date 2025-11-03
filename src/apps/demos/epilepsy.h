#pragma once

#include "keira/app.h"

class EpilepsyApp : public App {
public:
    EpilepsyApp();

private:
    void run() override;
};
