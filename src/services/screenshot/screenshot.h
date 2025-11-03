#pragma once

#include "keira/service.h"

class ScreenshotService : public Service {
public:
    ScreenshotService();

private:
    void run() override;
    bool saveScreenshot(lilka::Canvas* canvas);
    bool writeScreenshot(uint8_t* buffer, uint32_t length, const char* ext);
};
