#include "toolbar.h"

// TODO: millis() calls all around keira code use different data types to store time, potential problems expected
// right one unsigned long
void ToolBarWidget::update() {
    updatePage();
}

void ToolBarWidget::updatePage() {
    if (pages.empty()) return;
}

void ToolBarWidget::draw(Arduino_GFX* canvas) {
    if (pages.empty()) return; // nothing to draw
    auto& curPage = pages[cursor];

    unsigned long curTime = millis();

    bool updateNeeded = (curTime - curPage.lastUpdateTime > curPage.updateInterval);
    // TBR_DBG lilka::serial.log(
    //     "cur time %d, lastUpdateTime %d, interval %d ", curTime, curPage.lastUpdateTime, curPage.updateInterval
    // );
    if (updateNeeded) {
        curPage.strVal = curPage.clbkGetStr(curPage.clbkGetStrData);
        curPage.lastUpdateTime = curTime;
        TBR_DBG lilka::serial.log("Updating str data for page %d", cursor);
    }

    canvas->setCursor(TOOLBAR_SAFE_DISTANCE, canvas->height() - 20 / 2); // FONT_Y / 2
    canvas->setFont(FONT_8x13);
    canvas->setTextSize(1);

    canvas->setTextBound(TOOLBAR_SAFE_DISTANCE, canvas->height() - TOOLBAR_HEIGHT, TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    canvas->setTextColor(lilka::colors::White);

    canvas->printf(curPage.strVal.c_str());
}

void ToolBarWidget::nextPage() {
    if (pages.empty()) return;

    auto nextPageIndex = cursor + 1;
    cursor = (nextPageIndex == pages.size()) ? 0 : nextPageIndex;
}

void ToolBarWidget::prevPage() {
    if (pages.empty()) return;

    auto nextPageIndex = cursor - 1;
    cursor = (nextPageIndex < 0) ? pages.size() - 1 : nextPageIndex;
}

size_t ToolBarWidget::getCursor() {
    return cursor;
}

void ToolBarWidget::addPage(PGetStrCallback clbkGetStr, void* clbkGetStrData, unsigned long updateInterval) {
    ToolBarPage newPage;

    newPage.clbkGetStrData = clbkGetStrData;
    newPage.clbkGetStr = clbkGetStr;
    newPage.lastUpdateTime = 0;
    newPage.updateInterval = updateInterval;
    newPage.strVal = "";
    pages.push_back(newPage);
    TBR_DBG lilka::serial.log("Toolbar: Adding a new page. New page count : %d", pages.size());
}