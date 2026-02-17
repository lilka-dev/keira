#pragma once

#include "keira/app.h"

class StatusBarApp : public App {
public:
    StatusBarApp();

private:
    void run() override;
    void drawClock();
    void drawRam();
    void drawNetwork();
    void drawBattery();
    void formatSize(uint32_t bytes, char* buf, size_t len);
};
