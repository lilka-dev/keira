#pragma once
// Libraries
#include <Arduino.h>
#include <lilka.h>
// System components
#include "keira/mutex.h"
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
    //========================================================================
    //  Panel(StatusBar
    //========================================================================
    App* getPanel();
    void setPanel(App* app);
    //////////////////////////////////////////////////////////////////////////

    void run() override;
    void spawn(App* app, bool autoSuspend = true);
    void threadsRun() override;
    //========================================================================
    //  Toasts
    //========================================================================
    void renderToCanvas(lilka::Canvas* canvas);
    void startToast(String message, uint64_t duration = 2500);

private:
    void renderToast(lilka::Canvas* canvas);
    String toastMessage = "";
    uint64_t toastStartTime = 0;
    uint64_t toastEndTime = 0;
    //////////////////////////////////////////////////////////////////////////

    App* panel = NULL;
    // Used for capping framerate to A
    TickType_t lastAwake = xTaskGetTickCount();
};
