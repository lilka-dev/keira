#include "coleco_core.h"

#include <esp_heap_caps.h>

#include <algorithm>
#include <cstdio>

#include "lileco.h"
#include "core/Colem.h"
#include "core/emuapi.h"

namespace lileco {

namespace {
ColecoCore* g_activeCore = nullptr;
constexpr int KEYPAD_LATCH_TICKS = 6;
} // namespace

ColecoCore::ColecoCore() :
    app(nullptr),
    running(false),
    frameBuffer(SCREEN_WIDTH * SCREEN_HEIGHT, 0),
    palette{},
    keyMask(0),
    keypad(0),
    keypadTicks(0),
    frameOffsetX(0),
    frameOffsetY(0) {
    palette.fill(lilka::colors::Black);
}

ColecoCore::~ColecoCore() {
    if (running) stop();
}

ColecoCore* ColecoCore::getActive() {
    return g_activeCore;
}

void setActiveCore(ColecoCore* core) {
    g_activeCore = core;
}

bool ColecoCore::start(LilecoApp* owner, const String& rom) {
    app = owner;
    romPath = rom;
    romDirectory = lilka::fileutils.getParentDirectory(rom);

    auto canvas = app->canvas;
    frameOffsetX = (canvas->width() - SCREEN_WIDTH) / 2;
    frameOffsetY = (canvas->height() - SCREEN_HEIGHT) / 2;
    clearFrame();

    setActiveCore(this);
    coc_Init();
    running = coc_Start(const_cast<char*>(romPath.c_str()));
    if (!running) {
        setActiveCore(nullptr);
    }
    return running;
}

void ColecoCore::stop() {
    if (!running) return;
    running = false;
    coc_Stop();
    setActiveCore(nullptr);
}

bool ColecoCore::isRunning() const {
    return running;
}

void ColecoCore::step(const lilka::State& controllerState) {
    if (!running) return;
    updateInput(controllerState);
    coc_Step();
}

int ColecoCore::loadFile(const char* name, uint8_t* destination, int maxSize) {
    if (!name || !destination || maxSize <= 0) return 0;

    if (name[0] == '/') {
        FILE* file = fopen(name, "rb");
        if (!file) return 0;
        size_t read = fread(destination, 1, maxSize, file);
        fclose(file);
        return static_cast<int>(read);
    }

    String resolved;
    if (!resolveFilePath(name, resolved)) return 0;

    FILE* file = fopen(resolved.c_str(), "rb");
    if (!file) return 0;
    size_t read = fread(destination, 1, maxSize, file);
    fclose(file);
    return static_cast<int>(read);
}

void ColecoCore::updatePalette(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= palette.size()) return;
    palette[index] = lilka::display.color565(r, g, b);
}

void ColecoCore::drawLine(const uint8_t* src, int width, int line) {
    if (!src) return;
    if (line < 0 || line >= SCREEN_HEIGHT) return;

    uint16_t* dst = frameBuffer.data() + (line * SCREEN_WIDTH);
    const int copyWidth = std::min(width, SCREEN_WIDTH);
    for (int x = 0; x < copyWidth; x++) {
        uint8_t colorIndex = src[x];
        if (colorIndex >= palette.size()) colorIndex &= 0x0F;
        dst[x] = palette[colorIndex];
    }
    for (int x = copyWidth; x < SCREEN_WIDTH; x++) {
        dst[x] = palette[0];
    }
}

void ColecoCore::drawFrame(const uint8_t* src, int width, int height, int stride) {
    if (!src) return;
    for (int y = 0; y < SCREEN_HEIGHT && y < height; y++) {
        const uint8_t* line = src + y * stride;
        drawLine(line, width, y);
    }
}

void ColecoCore::presentFrame() {
    if (!app || !app->canvas) return;

    app->canvas->draw16bitRGBBitmap(frameOffsetX, frameOffsetY, frameBuffer.data(), SCREEN_WIDTH, SCREEN_HEIGHT);
    app->queueDraw();
}

uint16_t ColecoCore::currentKeyMask() const {
    return keyMask;
}

int ColecoCore::keypadValue() const {
    return keypad;
}

void ColecoCore::updateInput(const lilka::State& state) {
    keyMask = 0;

    if (state.left.pressed) keyMask |= MASK_JOY1_LEFT;
    if (state.right.pressed) keyMask |= MASK_JOY1_RIGHT;
    if (state.up.pressed) keyMask |= MASK_JOY1_UP;
    if (state.down.pressed) keyMask |= MASK_JOY1_DOWN;
    if (state.a.pressed) keyMask |= MASK_JOY1_BTN;
    if (state.b.pressed) keyMask |= MASK_JOY1_BTN2;

    if (state.c.pressed) queueKeypadValue(2); // '1'
    if (state.d.pressed) queueKeypadValue(3); // '2'
    if (state.start.pressed) queueKeypadValue(11); // '#'
    if (state.select.pressed) queueKeypadValue(12); // '*'

    if (keypadTicks > 0) {
        keypadTicks--;
        if (keypadTicks == 0) keypad = 0;
    }
}

void ColecoCore::queueKeypadValue(int value) {
    keypad = value;
    keypadTicks = KEYPAD_LATCH_TICKS;
}

bool ColecoCore::resolveFilePath(const char* name, String& outPath) const {
    if (!name) return false;

    String pathCandidates[] = {
        romDirectory + "/" + name,
        "/sd/coleco/" + String(name),
        "/sd/" + String(name),
        "/spiffs/" + String(name),
    };

    for (const auto& candidate : pathCandidates) {
        FILE* file = fopen(candidate.c_str(), "rb");
        if (file) {
            fclose(file);
            outPath = candidate;
            return true;
        }
    }
    return false;
}

void ColecoCore::clearFrame() {
    std::fill(frameBuffer.begin(), frameBuffer.end(), lilka::colors::Black);
    if (app && app->canvas) {
        app->canvas->fillScreen(lilka::colors::Black);
        app->queueDraw();
    }
}

} // namespace lileco
