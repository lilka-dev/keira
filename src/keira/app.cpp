
#include "keira/app.h"

#include "keira/servicemanager.h"
#include <lilka/colors565.h>
#include <lilka/display.h>
#include <lilka/controller.h>
#include "services/watchdog/watchdog.h"

App::App(const char* name) :
    name(name),
    taskHandle(NULL),
    isDrawQueued(false),
    backCanvasMutex(xSemaphoreCreateMutex()),
    stackSize(4096),
    appCore(0),
    frame(0) {
    KAPP_DBG lilka::serial.log("Created app on core %d", name, appCore);
}
// Spawn App's main thread
void App::start() {
    // Check if app is already runing
    if (taskHandle != NULL) {
        KAPP_DBG lilka::serial.err("App %s is already running", name);
        return;
    }

    // Set inital drawing flags
    if (!(flags & APP_DRAWING_FLAGS)) {
        setFlags(this->flags | APP_FLAG_DEFAULT);
    }

    KAPP_DBG lilka::serial.log("Starting app %s", name);

    // Spawn new thread
    if (xTaskCreatePinnedToCore(
            reinterpret_cast<TaskFunction_t>(&App::_run), name, stackSize, this, 1, &taskHandle, appCore
        ) != pdPASS) {
        KAPP_DBG lilka::serial.err(
            "Failed to create task for app %s"
            " - not enough memory? Try increasing stack size with setStackSize()",
            name
        );
    }
}

// Actual Apps's main thread
void App::_run() {
#ifdef KEIRA_WATCHDOG
    auto wd = ServiceManager::getInstance()->getService<WatchdogService>("watchdog");
    if (wd != NULL) wd->addCurrentTask(WATCHDOG_TASK_APP);
#endif

    // Reset controller state
    lilka::controller.resetState();

    // Run App's main loop
    run();

    // infinite wait for being stopped by appManager
    exit();
}

void App::suspend() {
    if (taskHandle == NULL) {
        KAPP_DBG lilka::serial.err("App %s is not running, cannot suspend", name);
        return;
    }

    KAPP_DBG lilka::serial.log("Suspending app %s (state = %d)", name, getState());

    onSuspend();
    vTaskSuspend(taskHandle);
}

void App::resume() {
    if (taskHandle == NULL) {
        KAPP_DBG lilka::serial.err("App %s is not running, cannot resume", name);
        return;
    }

    KAPP_DBG lilka::serial.log("Resuming app %s (state = %d)", name, getState());

    // Instance defined actions
    onResume();

    // Invalidate user input
    lilka::controller.resetState();

    // Resume task
    vTaskResume(taskHandle);

    forceRedraw();
}

void App::stop() {
    if (taskHandle == NULL) {
        KAPP_DBG lilka::serial.err("App %s is not running, cannot stop", name);
        return;
    }

    KAPP_DBG lilka::serial.log("Stopping app %s (state = %d)", name, getState());

    vTaskDelete(taskHandle);
    taskHandle = NULL;
}

void App::exit() {
    // Instance defined actions
    onExit();

    // Wait to be stopped by AppManager
    exiting = true;
    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}

App::~App() {
    vSemaphoreDelete(backCanvasMutex);
    delete canvas;
    delete backCanvas;
}

// Swap Canvas, Ask redraw
void App::queueDraw() {
    // Swap the front and back canvases
    // Serial.println("Queuing draw for " + String(name) + ", canvas address = " + String((uint32_t)canvas));
    xSemaphoreTake(backCanvasMutex, portMAX_DELAY);
    lilka::Canvas* temp = canvas;
    canvas = backCanvas;
    backCanvas = temp;
    isDrawQueued = true;
    frame++;
    xSemaphoreGive(backCanvasMutex);
    taskYIELD();
}

void App::setCore(int appCore) {
    this->appCore = appCore;
}

void App::setFlags(AppFlags_t flags) {
    if (this->flags == flags) return;

    this->flags = flags;

    // Keep canvases updated to flags
    initCanvas();
}

AppFlags_t App::getFlags() {
    return flags;
}

bool App::needsRedraw() {
    return isDrawQueued;
}

void App::markClean() {
    isDrawQueued = false;
}

void App::forceRedraw() {
    isDrawQueued = true;
}

const char* App::getName() {
    return name;
}

// Returns exiting flag. To be used in appManager
bool App::getExiting() {
    return exiting;
}

void App::acquireBackCanvas() {
    xSemaphoreTake(backCanvasMutex, portMAX_DELAY);
}

void App::releaseBackCanvas() {
    xSemaphoreGive(backCanvasMutex);
}

eTaskState App::getState() {
    if (taskHandle == NULL) {
        return eTaskState::eInvalid;
    }
    return eTaskGetState(taskHandle);
}

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

// stacksize adjust, no effect in runtime, to be run from app constructor
void App::setStackSize(uint32_t stackSize) {
    this->stackSize = stackSize;
}

// init/reinit canvas depending on flags
void App::initCanvas() {
    // lock
    acquireBackCanvas();

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
        if ((cx == bx) && (cy == by) && (cw == bw) && (ch == bh) && (cy == y) && (cx == x) && (cw == w) && (ch == h))
            return;
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

    // unlock
    releaseBackCanvas();
}
