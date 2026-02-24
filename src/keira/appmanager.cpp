#include "keira/appmanager.h"
#include <lilka/default_splash.h>
#include "apps/statusbar/statusbar.h"
#include "lilka/controller.h"

AppManager* AppManager::instance = NULL;

// TODO: integrate in ksystem
AppManager* AppManager::getInstance() {
    // TODO: Not thread-safe, but is first called in static context before any tasks are created
    if (instance == NULL) {
        instance = new AppManager();
    }
    return instance;
}

// Get the panel app.
App* AppManager::getPanel() {
    return panel;
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
void AppManager::runApp(App* app, bool autoSuspend) {
    // Retrieve current task handle
    TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();

    xSemaphoreTake(lock, portMAX_DELAY);

    // Add new app to launch
    appsToRun.push_back(app);

    xSemaphoreGive(lock);

    K_AMG_DBG lilka::serial.log("Added app %s in appsToRun", app->getName());

    // Suspend current task immediately.
    // Wait for wakeup in loop()
    if (autoSuspend) vTaskSuspend(currentTask);
}

/// Performs Apps Run/Stop/Suspend/Draw if necessary
void AppManager::loop() {
#define GET_BACK(X) X.empty() ? NULL : X.back()
    xSemaphoreTake(lock, portMAX_DELAY);

    // Retrieve top app
    App* topApp = GET_BACK(apps);

    // Terminate old apps
    while (topApp && topApp->getExiting()) {
        // Terminate main app thread
        topApp->stop();
        K_AMG_DBG lilka::serial.log("Terminating app %s", topApp->getName());

        // Cleanup app related data
        delete topApp;

        // Switch to next app
        apps.pop_back();
        topApp = GET_BACK(apps);
    }

    // Spawn new apps
    if (!appsToRun.empty()) {
        for (auto& appToRun : appsToRun) {
            topApp = GET_BACK(apps);

            if (topApp) {
                topApp->suspend();
            }

            // Launch new app
            topApp = appToRun;
            apps.push_back(appToRun);
            topApp->start();
            K_AMG_DBG lilka::serial.log("Launching app %s", topApp->getName());
        }
        appsToRun.clear();
    }

    // Ensure topApp still exists
    if (!topApp) {
        // shit happened
        K_AMG_DBG lilka::serial.err("No apps to draw");

        // Not sure it would help to restore state :D
        xSemaphoreGive(lock);
        return;
    }

    // Ensure topApp not sleeping
    if (topApp->getState() == eSuspended) {
        // Wake up Neo
        topApp->resume();

        panel->forceRedraw();

        K_AMG_DBG lilka::serial.log("Resuming app %s", topApp->getName());
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

    xSemaphoreGive(lock);

    // Switch to a next thread[task]
    taskYIELD();
#undef GET_BACK
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
