#include "txtviewer.h"


// ===== CONFIGURATION
TxtViewerApp::TxtViewerApp(String fPath) : App("Text Viewer") {
    // Configure TxtViewer
    tView.setCallback(TXTV_CALLBACK_CAST(onTxtViewerButton), TXTV_CALLBACK_PTHIS);

    // Configure Toolbar
    // Note: should be in order specified in TxtToolBarPage enum
    toolBar.addPage(TXTV_TOOLBAR_CALLBACK(onGetStrProgress), TXTV_CALLBACK_PTHIS);
    toolBar.addPage(TXTV_TOOLBAR_CALLBACK(onGetStrFontSize), TXTV_CALLBACK_PTHIS);
    toolBar.addPage(TXTV_TOOLBAR_CALLBACK(onGetStrFontSpacing), TXTV_CALLBACK_PTHIS);

    
    tView.setTextFile(fPath);
}
// ===== END CONFIGURATION

void TxtViewerApp::onTxtViewerButton(){
    auto aButton = tView.getButton();
    if (aButton == K_BTN_BACK)
        return;
    
    if (aButton == lilka::Button::D)
        toolBar.nextPage();
    
    if (aButton == lilka::Button::A); // inc page val
    // TODO: switch across TxtToolBarPage enum, depending on page do different stuff

    if (aButton == lilka::Button::C); // dec page val
    // TODO: switch across TxtToolBarPage enum, depending on page do different stuff


    tView.isFinished(); // clear finished flag
}

// ===== REDRAW
void TxtViewerApp::run() {
    while (!tView.isFinished()) {
        tView.update();
        tView.draw(canvas);
        queueDraw();
    }
}

void TxtViewerApp::queueDraw() {
    // show control buttons
    toolBar.update();
    toolBar.draw(canvas);
    App::queueDraw();
}
// ===== END REDRAW

// ===== FONT SETTINGS

void TxtViewerApp::fontSizeInc(){
    // TODO: font scaling to implement via changing font
    // it would allow more precise scaling without big size jumps
}

void TxtViewerApp::fontSizeDec(){
    // TODO: font scaling to implement via changing font
    // it would allow more precise scaling without big size jumps
}

void TxtViewerApp::fontSpacingInc(){

}

void TxtViewerApp::fontSpacingDec(){

}

// ===== END FONT SETTINGS

// ===== TOOLBAR

// char * maybe? and static buffer, should be cool :)
String TxtViewerApp::onGetStrProgress(){
    // [===========      ] 80%
    auto max = tView.getFileSize();
    auto curPos = tView.getOffset();

    auto piece = curPos/max;




}

String TxtViewerApp::onGetStrFontSize(){
    // SIZE: 10  D(-) A(+)
}
String TxtViewerApp::onGetStrFontSpacing(){
    // SPACE: 10  D(-) A(+)
}
// ===== END TOOLBAR