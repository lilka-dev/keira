#pragma once

#include <Arduino.h>
#include <lilka.h>

#include "keira/threadmanager.h"
#include "keira/app.h"

// Uncomment to get debug information
#define KEIRA_APPMANAGER_DEBUG

#ifdef KEIRA_APPMANAGER_DEBUG
#    define K_AMG_DBG if (1)
#else
#    define K_AMG_DBG if (0)
#endif

class AppManager : public ThreadManager {
public:
    AppManager();

    App* getPanel();
    void setPanel(App* app);
    // adds app to appsToRun list, actuall runing happens
    // inside loop(). automatically suspends execution of current task
    //   void runApp(App* app, bool autoSuspend = true);

    void run() override;
    void spawn(App* app, bool autoSuspend = true);
    void threadsRun() override;
    void renderToCanvas(lilka::Canvas* canvas);
    void startToast(String message, uint64_t duration = 2500);

private:
    void renderToast(lilka::Canvas* canvas);

    App* panel = NULL;
    static AppManager* instance;
    TickType_t lastAwake = xTaskGetTickCount();
    //    SemaphoreHandle_t lock = xSemaphoreCreateMutex();
    // Using threads/threadsToRun from ThreadManager to store apps

    String toastMessage = "";
    uint64_t toastStartTime = 0;
    uint64_t toastEndTime = 0;
};
