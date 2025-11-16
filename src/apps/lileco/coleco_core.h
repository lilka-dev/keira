#pragma once

#include <Arduino.h>
#include <lilka.h>

#include <array>
#include <string>
#include <vector>

class LilecoApp;

namespace lileco {

class ColecoCore {
public:
    ColecoCore();
    ~ColecoCore();

    static constexpr int SCREEN_WIDTH = 256;
    static constexpr int SCREEN_HEIGHT = 192;

    bool start(LilecoApp* owner, const String& romPath);
    void stop();
    void step(const lilka::State& controllerState);
    bool isRunning() const;

    // Hooks used from the C bridge
    int loadFile(const char* name, uint8_t* destination, int maxSize);
    void updatePalette(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    void drawLine(const uint8_t* src, int width, int line);
    void drawFrame(const uint8_t* src, int width, int height, int stride);
    void presentFrame();
    uint16_t currentKeyMask() const;
    int keypadValue() const;

    static ColecoCore* getActive();

private:
    void updateInput(const lilka::State& state);
    void queueKeypadValue(int value);
    bool resolveFilePath(const char* name, String& outPath) const;
    void clearFrame();

    LilecoApp* app;
    String romPath;
    String romDirectory;
    bool running;

    std::vector<uint16_t> frameBuffer;
    std::array<uint16_t, 16> palette;
    uint16_t keyMask;
    int keypad;
    int keypadTicks;

    int frameOffsetX;
    int frameOffsetY;
};

void setActiveCore(ColecoCore* core);

} // namespace lileco
