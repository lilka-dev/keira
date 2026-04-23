#pragma once
#include "keira/app.h"

class MultiBootApp : public App {
public:
    explicit MultiBootApp(const String& path);

    void run() override;

private:
    void fileLoadAsRom(const String& path);
    String firmwarePath = "";
};