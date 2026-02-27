#include "keira/appmanager.h"
#include <lilka/default_splash.h>
#include "apps/statusbar/statusbar.h"
#include "lilka/controller.h"

#define MAX_FPS 60

AppManager::AppManager() {
    setPriority(KT_PRIO_MAX);
    setCore(0);
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

#define GET_BACK(X) X.empty() ? NULL : X.back()
void AppManager::threadsRun() {
    xSemaphoreTake(ThreadManager::lock, portMAX_DELAY);

    // Launch new threads
    for (auto& thread : threadsToRun) {
        // Suspend previous thread(app)
        auto topThread = GET_BACK(threads);
        if (topThread) topThread->suspend();
        // Launch new one
        thread->start();
        threads.push_back(thread);
    }
    threadsToRun.clear();

    xSemaphoreGive(ThreadManager::lock);
}

/// Performs Apps Run/Stop/Suspend/Draw if necessary
void AppManager::run() {
    K_AMG_DBG lilka::serial.log("Starting apps update loop");
    while (1) {
        threadsClean();

        threadsRun();

        /// LOCK THREADS LIST
        xSemaphoreTake(ThreadManager::lock, portMAX_DELAY);

        // Retrieve top app[Thread]
        App* topApp = APP_PCAST(GET_BACK(threads));

        // Ensure topApp still exists
        if (!topApp) {
            // shit happened
            K_AMG_DBG lilka::serial.err("No apps to draw");

            // Not sure it would help to restore state :D
            /// UNLOCK THREADS LIST
            xSemaphoreGive(ThreadManager::lock);
            return;
        }

        // Ensure topApp not sleeping
        if (topApp->getState() == KTS_SUSPENDED) {
            // Wake up Neo
            topApp->resume();

            panel->setRedraw(true);
        }

        // Draw panel and top app
        for (App* app : {panel, topApp}) {
            if (app == panel) {
                // Check if topApp is fullscreen. If it is, don't draw the panel
                if (topApp->getFlags() & AppFlags::APP_FLAG_FULLSCREEN) {
                    continue;
                }
            }

            /// LOCK APP CANVAS
            xSemaphoreTake(app->canvasMutex, portMAX_DELAY);

            // Draw toast message on app's canvas to prevent flickering
            if (millis() < toastEndTime) {
                renderToast(topApp->backCanvas);
            }
            // Redraw app
            if (app->getRedraw()) {
                if (app->flags & AppFlags::APP_FLAG_INTERLACED) {
                    lilka::display.drawCanvasInterlaced(app->backCanvas, app->frame % 2);
                } else {
                    lilka::display.drawCanvas(app->backCanvas);
                }
                app->setRedraw(false);
            }
            /// UNLOCK APP CANVAS
            xSemaphoreGive(app->canvasMutex);
        }
        /// UNLOCK THREADS LIST
        xSemaphoreGive(ThreadManager::lock);

        vTaskDelayUntil(&lastAwake, pdMS_TO_TICKS(1000 / MAX_FPS));
        //        K_AMG_DBG lilka::serial.log("Last awake = %d", lastAwake);
        //vTaskDelay(pdMS_TO_TICKS(10));
    }
}
#undef GET_BACK

void AppManager::spawn(App* app, bool autoSuspend) {
    ThreadManager::spawn(app, autoSuspend);
}

// TODO: toasts to be moved in ksystem
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
    //    App *topApp =
    /*
    // Draw panel and top app
    for (App* app : {panel, threads.back()}) {
        xSemaphoreTake(app->canvasMutex, portMAX_DELAY);
        canvas->drawCanvas(app->backCanvas);
        xSemaphoreGive(app->canvasMutex);
    }*/

    xSemaphoreGive(lock);
}

/// Display a toast message.
void AppManager::startToast(String message, uint64_t duration) {
    xSemaphoreTake(lock, portMAX_DELAY);
    /*
    toastMessage = message;
    toastStartTime = millis();
    toastEndTime = millis() + duration;
*/
    xSemaphoreGive(lock);
}
