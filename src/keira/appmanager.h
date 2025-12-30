#pragma once

#include <Arduino.h>
#include <lilka.h>

#include "keira/app.h"

class AppManager {
public:
    ~AppManager();
    void setPanel(App* app);
    // adds app to appsToRun list, actuall runing happens
    // inside loop
    void runApp(App* app);
    void loop();
    void renderToCanvas(lilka::Canvas* canvas);
    void startToast(String message, uint64_t duration = 2500);

    static AppManager* getInstance();

private:
    AppManager();
    App* removeTopApp();
    void renderToast(lilka::Canvas* canvas);

    App* panel;
    std::vector<App*> apps;
    std::vector<App*> appsToRun;
    static AppManager* instance;
    SemaphoreHandle_t lock;

    String toastMessage;
    uint64_t toastStartTime;
    uint64_t toastEndTime;
};
