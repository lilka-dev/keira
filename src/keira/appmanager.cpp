#include "keira/appmanager.h"
#include <lilka/default_splash.h>

AppManager* AppManager::instance = NULL;

AppManager::AppManager() :
    // We're using binary semaphore instead of mutex because we will be releasing the lock from another task (see the hefty explanation in runApp())
    lock(xSemaphoreCreateBinary()), panel(NULL), toastMessage(""), toastStartTime(0), toastEndTime(0) {
    xSemaphoreGive(lock);
}

AppManager::~AppManager() {
    // TODO: Should never be destroyed
}

AppManager* AppManager::getInstance() {
    // TODO: Not thread-safe, but is first called in static context before any tasks are created
    if (instance == NULL) {
        instance = new AppManager();
    }
    return instance;
}

/// Set the panel app.
/// Panel app is drawn separately from the other apps on the top of the screen.
void AppManager::setPanel(App* app) {
    xSemaphoreTake(lock, portMAX_DELAY);
    panel = app;
    panel->start();
    xSemaphoreGive(lock);
}

/// Spawn a new app
void AppManager::runApp(App* app) {
    // lock just in case, though it works without it anyways
    // but let's say, it access data, so why not
    xSemaphoreTake(lock, portMAX_DELAY);

    appsToRun.push_back(app);

    xSemaphoreGive(lock);
}

/// Remove the top app and resume the previous one.
/// Returns new top app.
App* AppManager::removeTopApp() {
    App* topApp = apps.back();
    apps.pop_back();
    delete topApp;
    if (apps.size() == 0) {
        // Panic! No apps left
        lilka::serial.err("appmanager: no apps left! Panic!");
        while (1) {
        }
    } else {
        topApp = apps.back();
        topApp->resume();
        topApp->forceRedraw();
        panel->forceRedraw();
        return topApp;
    }
}

/// Perform one iteration of the main loop.
/// Redraws the panel and the top app if necessary.
/// If the top app has finished, it is removed from the list and the next app is resumed.
void AppManager::loop() {
    xSemaphoreTake(lock, portMAX_DELAY);
    // Single place to control runing/suspending apps
    if (!appsToRun.empty()) {
        for (auto& appToRun : appsToRun) {
            if (!apps.empty()) {
                App* topApp = apps.back();
                topApp->suspend();
            }
            apps.push_back(appToRun);
            appToRun->start();
        }
        appsToRun.clear();
    }

    if (apps.size()) {
        // Check if top app has finished
        App* topApp = apps.back();
        if (topApp->getState() == eDeleted) {
            // Switch to the next app in stack
            topApp = removeTopApp();
        }

        // Draw panel and top app
        for (App* app : {panel, topApp}) {
            if (app == panel) {
                // Check if topApp is fullscreen. If it is, don't draw the panel
                if (topApp->getFlags() & AppFlags::APP_FLAG_FULLSCREEN) {
                    continue;
                }
            }
            app->acquireBackCanvas();
            // Draw toast message on app's canvas to prevent flickering
            if (millis() < toastEndTime) {
                renderToast(topApp->backCanvas);
            }
            if (app->needsRedraw()) {
                if (app->flags & AppFlags::APP_FLAG_INTERLACED) {
                    lilka::display.drawCanvasInterlaced(app->backCanvas, app->frame % 2);
                } else {
                    lilka::display.drawCanvas(app->backCanvas);
                }
                app->markClean();
            }
            app->releaseBackCanvas();
        }
    }

    xSemaphoreGive(lock);

    taskYIELD();
}

void AppManager::renderToast(lilka::Canvas* canvas) {
    int16_t x, y;
    uint16_t w, h;
    lilka::display.setFont(FONT_8x13);
    lilka::display.getTextBounds(toastMessage.c_str(), 0, 0, &x, &y, &w, &h);
    int16_t cx = lilka::display.width() / 2;
    int16_t cy = lilka::display.height() / 7 * 6;
    int16_t yOffset = 0;
    uint64_t time = millis();
    if (time < toastStartTime + 300) {
        // Phase 1: Fade in
        // Offset goes from 100 to 0
        yOffset = 50 - (time - toastStartTime) * 50 / 300;
    } else if (time < toastEndTime - 300) {
        // Phase 2: Fully visible
        yOffset = 0;
    } else {
        // Phase 3: Fade out
        // Offset goes from 0 to 100
        yOffset = (time - toastEndTime + 300) * 50 / 300;
    }

    lilka::Canvas toastCanvas(cx - w / 2 - 5, cy - h - 5 + yOffset, w + 10, h + 10);

    toastCanvas.setFont(FONT_8x13);
    toastCanvas.fillScreen(lilka::colors::Dark_sienna);
    toastCanvas.setTextColor(lilka::colors::White);
    toastCanvas.setCursor(2, h + 2);
    toastCanvas.print(toastMessage.c_str());
    canvas->drawCanvas(&toastCanvas);
}

/// Render panel and top app to the given canvas.
/// Useful for taking screenshots.
void AppManager::renderToCanvas(lilka::Canvas* canvas) {
    xSemaphoreTake(lock, portMAX_DELAY);

    // Draw panel and top app
    for (App* app : {panel, apps.back()}) {
        app->acquireBackCanvas();
        canvas->drawCanvas(app->backCanvas);
        app->releaseBackCanvas();
    }

    xSemaphoreGive(lock);
}

/// Display a toast message.
void AppManager::startToast(String message, uint64_t duration) {
    xSemaphoreTake(lock, portMAX_DELAY);
    toastMessage = message;
    toastStartTime = millis();
    toastEndTime = millis() + duration;
    xSemaphoreGive(lock);
}
