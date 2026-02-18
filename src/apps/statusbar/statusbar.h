#pragma once

#include "keira/app.h"

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

public:
    StatusBarApp();
    void initSystemWidgets();
    uint8_t addWidget(std::function<int(lilka::Canvas*)> drawFn, uint8_t alignment, uint8_t maxWidth = 24);
    bool removeWidget(uint8_t id);

private:
    void run() override;

    void drawLeftWidgets(std::vector<StatusBarWidget>& widgets, int& xOffset, int& availableWidth);
    void drawRightWidgets(std::vector<StatusBarWidget>& widgets, int& xOffset, int& availableWidth);
    int drawWidget(StatusBarWidget* widget, int x, int availableWidth);

    int drawClock(lilka::Canvas* canvas);
    int drawRamText(lilka::Canvas* canvas);
    int drawRamIcon(lilka::Canvas* canvas);
    int drawNetwork(lilka::Canvas* canvas);
    int drawBattery(lilka::Canvas* canvas);

    void formatSize(uint32_t bytes, char* buf, size_t len);
};
