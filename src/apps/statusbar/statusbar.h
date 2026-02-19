#pragma once

#include "keira/app.h"

#define STATUSBAR_KEIRA_NAMESPACE "kstatusbar"

struct StatusBarWidget {
    std::function<int(lilka::Canvas*)> drawFn;
    uint8_t alignment;
    uint8_t maxWidth;
    uint8_t id;
};

class StatusBarApp : public App {
private:
    std::vector<StatusBarWidget> systemWidgets;
    uint8_t appWidgetId = 0;
    std::vector<StatusBarWidget> appWidgets;

    uint8_t clockMode;
    uint8_t memMode;
    uint8_t networkMode;
    uint8_t batteryMode;

public:
    StatusBarApp();

    void initSystemWidgets();

    uint8_t addWidget(std::function<int(lilka::Canvas*)> drawFn, uint8_t alignment, uint8_t maxWidth = 24);
    bool removeWidget(uint8_t id);

    uint8_t getClockMode();
    uint8_t getMemMode();
    uint8_t getNetworkMode();
    uint8_t getBatteryMode();
    void setClockMode(uint8_t mode);
    void setMemMode(uint8_t mode);
    void setNetworkMode(uint8_t mode);
    void setBatteryMode(uint8_t mode);

private:
    void run() override;

    void setMode(const char* key, uint8_t& mode, uint8_t newMode, uint8_t maxMode);
    void loadSettings();

    void drawLeftWidgets(const std::vector<StatusBarWidget>& widgets, int& xOffset, int& availableWidth);
    void drawRightWidgets(const std::vector<StatusBarWidget>& widgets, int& xOffset, int& availableWidth);
    int drawWidget(StatusBarWidget* widget, int x, int availableWidth);

    int drawClock(lilka::Canvas* canvas);
    int drawMem(lilka::Canvas* canvas);
    int drawNetwork(lilka::Canvas* canvas);
    int drawBattery(lilka::Canvas* canvas);

    void formatSize(uint32_t bytes, char* buf, size_t len);
};
