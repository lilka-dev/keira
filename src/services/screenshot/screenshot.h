#pragma once

#include <vector>

#include "keira/service.h"

class ScreenshotService : public Service {
public:
    ScreenshotService();
    // Encodes the canvas framebuffer as a PNG image
    static bool encodePNG(lilka::Canvas* canvas, std::vector<uint8_t>& png);

private:
    void run() override;
    bool saveScreenshot(lilka::Canvas* canvas);
    bool writeScreenshot(uint8_t* buffer, uint32_t length, const char* ext);
};
