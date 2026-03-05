
#include "keira/app.h"

#include <lilka/colors565.h>
#include <lilka/display.h>
#include <lilka/controller.h>
#include "services/watchdog/watchdog.h"

//=============================================================================
// App Constructors/Destructors
//=============================================================================
App::App(const char* name) {
    if (name) setName(name);
    else setName(APP_DEFAULT_NAME);
    setStackSize(APP_DEFAULT_STACK);
    setPriority(APP_DEFAULT_PRIO);
    setCore(APP_DEFAULT_CORE);

    // Set inital drawing flags
    if (!(flags & APP_DRAWING_FLAGS)) {
        setFlags(this->flags | APP_FLAG_DEFAULT);
    }
}
//-----------------------------------------------------------------------------
App::~App() {
    xSemaphoreTake(canvasMutex, portMAX_DELAY);

    if (canvas) delete canvas;
    if (backCanvas) delete backCanvas;

    xSemaphoreGive(canvasMutex);
}
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
//  App Flags
//=============================================================================
void App::setFlags(AppFlags_t flags) {
    if (this->flags == flags) return;

    this->flags = flags;

    // Keep canvases updated to flags
    initCanvas();
}
//-----------------------------------------------------------------------------
AppFlags_t App::getFlags() {
    return flags;
}
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// App UI Dialogs:
//=============================================================================
void App::alert(const String& title, const String& description) {
    lilka::Alert alertDialog(title, description);

    alertDialog.addActivationButton(K_BTN_BACK);

    while (!alertDialog.isFinished()) {
        alertDialog.update();
        canvas->fillScreen(lilka::colors::Black);
        alertDialog.draw(canvas);
        queueDraw();
    }
}
//-----------------------------------------------------------------------------
bool App::confirm(const String& title, const String& description) {
    lilka::Alert confirmDialog(title, description);

    confirmDialog.addActivationButton(K_BTN_BACK);
    confirmDialog.addActivationButton(K_BTN_CONFIRM);

    while (!confirmDialog.isFinished()) {
        confirmDialog.update();
        canvas->fillScreen(lilka::colors::Black);
        confirmDialog.draw(canvas);
        queueDraw();
    }

    return confirmDialog.getButton() == K_BTN_CONFIRM;
}
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//  App Canvas management
//=============================================================================
bool App::getRedraw() {
    return redraw; // TODO: atomic?
}
//-----------------------------------------------------------------------------
void App::setRedraw(bool redraw) {
    this->redraw = redraw;
}
//-----------------------------------------------------------------------------
void App::initCanvas() {
    xSemaphoreTake(canvasMutex, portMAX_DELAY);

    auto oldCanvas = canvas;
    auto oldBackCanvas = backCanvas;

    // Default canvas shifts
    uint16_t x = 0;
    uint16_t y = KEIRA_STATUSBAR_HEIGHT;

    // Default canvas sizes
    uint16_t w = lilka::display.width();
    uint16_t h = lilka::display.height() - KEIRA_STATUSBAR_HEIGHT;

    // Determine canvas dimensions based on flags
    if (flags & APP_FLAG_FULLSCREEN) {
        x = 0;
        y = 0;
        w = lilka::display.width();
        h = lilka::display.height();
    }

    if (flags & APP_FLAG_STATUSBAR) {
        x = 0;
        y = 0;
        w = lilka::display.width();
        h = KEIRA_STATUSBAR_HEIGHT;
    }

    // Ensure init/reinit needed
    if (canvas && backCanvas) {
        uint16_t cx = canvas->x();
        uint16_t cy = canvas->y();
        uint16_t bx = backCanvas->x();
        uint16_t by = backCanvas->y();

        uint16_t cw = canvas->width();
        uint16_t ch = canvas->height();
        uint16_t bw = backCanvas->width();
        uint16_t bh = backCanvas->height();

        // how to make it less ugly ? :D
        if ((cx == bx) && (cy == by) && (cw == bw) && (ch == bh) && (cy == y) && (cx == x) && (cw == w) && (ch == h)) {
            xSemaphoreGive(canvasMutex);
            return;
        }
    }

    // Create canvases
    canvas = new lilka::Canvas(x, y, w, h);
    backCanvas = new lilka::Canvas(x, y, w, h);

    // Fill them with black
    canvas->fillScreen(lilka::colors::Black);
    backCanvas->fillScreen(lilka::colors::Black);

    // Cleanup old canvases if them exist
    if (oldCanvas) delete oldCanvas;
    if (oldBackCanvas) delete oldBackCanvas;

    xSemaphoreGive(canvasMutex);
}
//-----------------------------------------------------------------------------
void App::queueDraw() {
    xSemaphoreTake(canvasMutex, portMAX_DELAY);

    // Detect if frame was skipped. Need to readjust priorities in this case
    KAPP_DBG if (frame && redraw) {
        skippedFrames++;
        float percentSkipped = skippedFrames * 100 / frame;
        lilka::serial.log("Skipping frame %d. Skipped %f", frame, percentSkipped);
    }

    // Flip canvas
    auto buf = canvas;
    canvas = backCanvas;
    backCanvas = buf;

    // Increment frameCount
    frame++;

    setRedraw(true);

    xSemaphoreGive(canvasMutex);
}
