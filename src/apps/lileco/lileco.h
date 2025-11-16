#pragma once

#include "keira/app.h"

class LilecoApp : public App {
public:
    explicit LilecoApp(String path);
    void run() override;

private:
    void drawScreen();

    String selectedFileName;
};
