#pragma once
#include <vector>
#include "abstractwidget.h"
// To be reused everywhere

#define TOOLBAR_DEBUG

#ifdef TOOLBAR_DEBUG
#    define TBR_DBG if (1)
#else
#    define TBR_DBG if (0)
#endif

#define TOOLBAR_HEIGHT          30
#define TOOLBAR_SAFE_DISTANCE   38
#define TOOLBAR_WIDTH           canvas->width() - TOOLBAR_SAFE_DISTANCE * 2
#define TOOLBAR_UPDATE_INTERVAL 1000

typedef String (*PGetStrCallback)(void*);

typedef struct {
    // getStr hook
    // activation buttons hook
    PGetStrCallback clbkGetStr;
    void* clbkGetStrData;
    unsigned long lastUpdateTime;
    unsigned long updateInterval;
    String strVal;
} ToolBarPage;

// we can't reuse buttons here, cause default widget would peek state, and
// we would have no access to buttons, sucks but yup

class ToolBarWidget {
public:

    void update();
    void draw(Arduino_GFX* canvas);
    // params -> callback to get str function, data to pass, update str time
    void addPage(PGetStrCallback clbkGetStr, void* clbkGetStrData, unsigned long updateInterval = 1000);
    void nextPage();
    void prevPage();
    void forceUpdate(bool forceCurrentPage = true);
    size_t getCursor();
private:
    size_t cursor = 0;
    void updatePage();
    std::vector<ToolBarPage> pages;
};
