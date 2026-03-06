#include "keira/appmanager.h"
#include <lilka/default_splash.h>
#include "apps/statusbar/statusbar.h"
#include <lilka/controller.h>
#include "keira/thread.h"
#define MAX_FPS 60

AppManager::AppManager() {
    setktPriority(KT_PRIO_IDLE);
    setktCore(0);
}

#define GET_BACK(X) X.empty() ? NULL : X.back()
void AppManager::threadsRun() {
    KMTX_LOCK(ThreadManager::lock);

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

    KMTX_UNLOCK(ThreadManager::lock);
}

/// Performs Apps Run/Stop/Suspend/Draw if necessary
void AppManager::run() {
    K_AMG_DBG lilka::serial.log("Starting apps update loop");
    while (1) {
        threadsRun();

        ThreadManager::threadsClean();

        /// LOCK THREADS LIST
        KMTX_LOCK(ThreadManager::lock);

        // Retrieve top app[Thread]
        App* topApp = APP_PCAST(GET_BACK(threads));

        // Ensure topApp and panel exists
        if (!(topApp && panel)) {
            // Absolutely possible situation and can happen

            /// UNLOCK THREADS LIST
            KMTX_UNLOCK(ThreadManager::lock);
            continue;
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
                KMTX_LOCK(panelMtx);
                // Check if topApp is fullscreen. If it is, don't draw the panel
                if (topApp->getFlags() & AppFlags::APP_FLAG_FULLSCREEN) {
                    KMTX_UNLOCK(panelMtx);
                    continue;
                }
                KMTX_UNLOCK(panelMtx);
            }

            /// LOCK APP CANVAS
            KMTX_LOCK(app->canvasMutex);

            // Draw toast message on app's canvas to prevent flickering
            if (millis() < toast.endTime) {
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
            KMTX_UNLOCK(app->canvasMutex);
        }
        /// UNLOCK THREADS LIST

        KMTX_UNLOCK(ThreadManager::lock);
        vTaskDelayUntil(&lastFrameTick, pdMS_TO_TICKS(1000 / MAX_FPS));
        //K_AMG_DBG lilka::serial.log("Last frame tick = %d", lastFrameTick);
    }
}
/// Render panel and top app to the given canvas.
/// Useful for taking screenshots.
void AppManager::renderToCanvas(lilka::Canvas* canvas) {
    KMTX_LOCK(ThreadManager::lock);

    App* topApp = APP_PCAST(GET_BACK(threads));

    // Ensure topApp exists
    if (topApp == NULL) {
        KMTX_UNLOCK(ThreadManager::lock);
        return;
    }

    // Draw panel and top app
    KMTX_LOCK(panelMtx);
    for (App* app : {panel, topApp}) {
        KMTX_LOCK(app->canvasMutex);
        canvas->drawCanvas(app->backCanvas);
        KMTX_UNLOCK(app->canvasMutex);
    }
    KMTX_UNLOCK(panelMtx);

    KMTX_UNLOCK(ThreadManager::lock);
}
#undef GET_BACK

void AppManager::spawn(App* app, bool autoSuspend) {
    // Reset controller state on launch
    app->setupOnEntryCallback(KT_CLBK_CAST(&lilka::Controller::resetState), KT_CLBK_DATA_CAST(&lilka::controller));
    // Reset controler state on resume
    app->setupOnResumeCallback(KT_CLBK_CAST(&lilka::Controller::resetState), KT_CLBK_DATA_CAST(&lilka::controller));
    // Do spawn
    // if (!(this->operator[](app->getName()))) {
    ThreadManager::spawn(app, autoSuspend);
    // } else {
    //     // Delete app immediately
    //     delete app;
    //     K_AMG_DBG lilka::serial.err("Trying to launch already run app. Skipping...");
    // }
}

void AppManager::renderToast(lilka::Canvas* canvas) {
    int16_t x, y;
    uint16_t w, h;
    KMTX_LOCK(toast.mtx);

    lilka::display.setFont(FONT_8x13);
    lilka::display.getTextBounds(toast.message.c_str(), 0, 0, &x, &y, &w, &h);
    int16_t cx = lilka::display.width() / 2;
    int16_t cy = lilka::display.height() / 7 * 6;
    int16_t yOffset = 0;
    uint64_t time = millis();

    if (time < toast.startTime + 300) {
        // Phase 1: Fade in
        // Offset goes from 100 to 0
        yOffset = 50 - (time - toast.startTime) * 50 / 300;
    } else if (time < toast.endTime - 300) {
        // Phase 2: Fully visible
        yOffset = 0;
    } else {
        // Phase 3: Fade out
        // Offset goes from 0 to 100
        yOffset = (time - toast.endTime + 300) * 50 / 300;
    }

    lilka::Canvas toastCanvas(cx - w / 2 - 5, cy - h - 5 + yOffset, w + 10, h + 10);

    toastCanvas.setFont(FONT_8x13);
    toastCanvas.fillScreen(lilka::colors::Dark_sienna);
    toastCanvas.setTextColor(lilka::colors::White);
    toastCanvas.setCursor(2, h + 2);
    toastCanvas.print(toast.message.c_str());

    KMTX_UNLOCK(toast.mtx);

    // canvas mtx already locked in run
    canvas->drawCanvas(&toastCanvas);
}

/// Display a toast message.
void AppManager::startToast(String message, uint64_t duration) {
    KMTX_LOCK(toast.mtx);

    // TODO: is millis equialent to xTaskGetTickCount() ?
    toast.message = message;
    toast.startTime = millis();
    toast.endTime = millis() + duration;

    KMTX_UNLOCK(toast.mtx);
}
