#include "toolbar.h"

void ToolBarWidget::update() {
    updatePage();
}

void ToolBarWidget::updatePage() {
    if (pages.empty()) return;
}

void ToolBarWidget::draw(Arduino_GFX* canvas) {
    if (pages.empty()) return; // nothing to draw
}

void ToolBarWidget::nextPage() {
    if (pages.empty()) return;

    currentPage = (currentPage > pages.size()) ? 0 : currentPage + 1;
}

void ToolBarWidget::prevPage() {
    if (pages.empty()) return;

    currentPage = (currentPage < 1) ? pages.size()-1 : currentPage - 1;
}
void ToolBarWidget::addPage(PGetStrCallback clbkGetStr, void *clbkGetStrData){
    ToolBarPage newPage;
    newPage.clbkGetStrData = clbkGetStrData;
    newPage.clbkGetStr = clbkGetStr;
    pages.push_back(newPage);
}