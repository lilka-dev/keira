#pragma once
// Libraries
#include <Arduino.h>
#include <lilka.h>

// System components
#include "keira/mutex.h"
#include "keira/bits/appmanager.h"
#include "keira/threadmanager.h"
#include "keira/app.h"

// Uncomment to get debug information
// #define KEIRA_APPMANAGER_DEBUG

typedef struct {
    String message;
    uint64_t startTime;
    uint64_t endTime;
    SemaphoreHandle_t mtx;
} KeiraToast;

class AppManager : public ThreadManager {
public:
    AppManager();
    void run() override;

    // Setups/Retrives currently set panel displayed on a top of the screen
    KMTX_SETER_GETER(App*, panel, panelMtx);
    // Spawns new app
    void spawn(App* app, bool autoSuspend = true);
    // Renders screen to given canvas
    void renderToCanvas(lilka::Canvas* canvas);
    // Starts toast
    void startToast(String message, uint64_t duration = 2500);

private:
    // Performs app runing
    void threadsRun() override;

    // Performs toast rendering to given canvas
    void renderToast(lilka::Canvas* canvas);
    // Storage for toast
    KeiraToast toast = KEIRA_TOAST_INITIALIZER;
    // TopPanel (StatusBarApp)
    App* panel = NULL;
    SemaphoreHandle_t panelMtx = xSemaphoreCreateMutex();
    // Used for capping framerate to A
    TickType_t lastFrameTick = xTaskGetTickCount();
};
