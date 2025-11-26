#pragma once
#include "keira/app.h"
#include "keira/keira.h"
#include "txtview.h"
#include "toolbar.h"

#define TXTV_CALLBACK_CAST(X)   reinterpret_cast<PTXTViewCallback>(&TxtViewerApp::X)
#define TXTV_CALLBACK_PTHIS     reinterpret_cast<void*>(this) // K_CLBK_PTHIS?
#define TXTV_TOOLBAR_CALLBACK(X)   reinterpret_cast<PGetStrCallback>(&TxtViewerApp::X)
#define TXTV_PROGRESS_LEN 10
#define TXTV_FONT_COUNT 5
#define TXTV_FONT_MAX_SIZE 15
#define TXTV_MAX_SPACING 10 // px



typedef enum{
    TXTV_PROGRESS,
    TXTV_FONT_SIZE,
    TXTV_FONT_SPACING
} TxtToolBarPage;

class TxtViewerApp : public App {
public:
    explicit TxtViewerApp(String fPath);
    TxtView tView;
    ToolBarWidget toolBar;
    void queueDraw();

private:
    TxtViewerApp();
    void run() override;

    // font settings routines

    void fontSetSize(int8_t newSize);
    void fontSetSpacing(uint8_t newSpacing);

    void fontSizeInc();
    void fontSizeDec();
    void fontSpacingInc();
    void fontSpacingDec();

    // tView callbacks:
    void onTxtViewerButton();

    // toolbar callbacks:
    String onGetStrProgress();
    String onGetStrFontSize();
    String onGetStrFontSpacing();


    // scale = (font size / font index)
    // font index should be a list of fonts 
    int8_t fontSize = 1;
    uint8_t fontSpacing = 2;
    const uint8_t* fonts[TXTV_FONT_COUNT] = {FONT_5x8, FONT_6x12, FONT_7x13, FONT_8x13, FONT_9x15};
};
