#pragma once
#include <vector>
#include "abstractwidget.h"
typedef String (*PGetStrCallback)(void*);

typedef struct {
    // getStr hook
    // activation buttons hook
    PGetStrCallback clbkGetStr;
    void *clbkGetStrData;
} ToolBarPage;

// we can't reuse buttons here, cause default widget would peek state, and 
// we would have no access to buttons, sucks but yup

class ToolBarWidget{
public:
    size_t currentPage = 0;
    void update();
    void draw(Arduino_GFX* canvas);
    void addPage(PGetStrCallback clbkGetStr, void *clbkGetStrData);
    void nextPage();    
    void prevPage();
private:
    void updatePage();
    std::vector<ToolBarPage> pages;
};
