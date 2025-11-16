#pragma once

#include "keira/app.h"
#include "coleco_core.h"

class LilecoApp : public App {
public:
    explicit LilecoApp(String path);
    void run() override;

private:
    String selectedFileName;
    String romPath;
};
