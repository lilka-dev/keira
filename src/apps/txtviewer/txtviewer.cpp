#include "txtviewer.h"

// ===== CONFIGURATION
TxtViewerApp::TxtViewerApp(String fPath) : App("Text Viewer") {
    setStackSize(8192);
    fontSetSize(fontSize);

    // Configure TxtViewer
    tView.setCallback(TXTV_CALLBACK_CAST(onTxtViewerButton), TXTV_CALLBACK_PTHIS);

    // Configure Toolbar
    // Note: should be in order specified in TxtToolBarPage enum
    toolBar.addPage(TXTV_TOOLBAR_CALLBACK(onGetStrProgress), TXTV_CALLBACK_PTHIS);
    toolBar.addPage(TXTV_TOOLBAR_CALLBACK(onGetStrFontSize), TXTV_CALLBACK_PTHIS);
    toolBar.addPage(TXTV_TOOLBAR_CALLBACK(onGetStrFontSpacing), TXTV_CALLBACK_PTHIS);

    tView.setTextFile(fPath);
    tView.addActivationButton(lilka::Button::D);
    tView.addActivationButton(lilka::Button::A);
    tView.addActivationButton(lilka::Button::C);
}
// ===== END CONFIGURATION

void TxtViewerApp::onTxtViewerButton() {
    auto aButton = tView.getButton();
    if (aButton == K_BTN_BACK) return;

    if (aButton == lilka::Button::C) toolBar.nextPage();

    // Inc
    if (aButton == lilka::Button::A) {
        auto cur = toolBar.getCursor();
        switch (cur) {
            case TXTV_PROGRESS:

                break;
            case TXTV_FONT_SIZE:
                fontSizeInc();
                break;
            case TXTV_FONT_SPACING:
                fontSpacingInc();
                break;
        }
    }
    // Dec
    if (aButton == lilka::Button::D) {
        auto cur = toolBar.getCursor();
        switch (cur) {
            case TXTV_PROGRESS:

                break;
            case TXTV_FONT_SIZE:
                fontSizeDec();
                break;
            case TXTV_FONT_SPACING:
                fontSpacingDec();
                break;
        }
    }

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

void TxtViewerApp::fontSetSize(int8_t newSize) {
    auto scale = (newSize / TXTV_FONT_COUNT) + 1;
    auto fontIndex = newSize % TXTV_FONT_COUNT;

    tView.setFont(fonts[fontIndex]);
    tView.setTextSize(scale);
}

void TxtViewerApp::fontSetSpacing(uint8_t newSpacing) {
    if (newSpacing > TXTV_MAX_SPACING) return;

    tView.setSpacing(newSpacing);
}

void TxtViewerApp::fontSizeInc() {
    if (fontSize >= TXTV_FONT_MAX_SIZE) return;

    fontSize++;

    fontSetSize(fontSize);
}

void TxtViewerApp::fontSizeDec() {
    if (fontSize <= 0) return;

    fontSize--;

    fontSetSize(fontSize);
}

void TxtViewerApp::fontSpacingInc() {
    if (fontSpacing >= TXTV_MAX_SPACING) return;

    fontSpacing++;

    fontSetSpacing(fontSpacing);
}

void TxtViewerApp::fontSpacingDec() {
    if (fontSpacing == 0) return;

    fontSpacing--;

    fontSetSpacing(fontSpacing);
}

// ===== END FONT SETTINGS

// ===== TOOLBAR

// char * maybe? and static buffer, should be cool :)
String TxtViewerApp::onGetStrProgress() {
    // [===========      ] 80%
    auto max = tView.getFileSize();
    auto curPos = tView.getOffset();

    // TODO: detemine TXTV_PROGRESS_LEN from toolbar width in characters
    size_t countSegments = curPos * TXTV_PROGRESS_LEN / max;
    String progStr = "[";

    for (size_t i = 0; i < countSegments; i++)
        progStr += "=";

    size_t countSpaces = TXTV_PROGRESS_LEN - countSegments;
    for (size_t i = 0; i < countSpaces; i++)
        progStr += " ";

    progStr += "]";

    String offStr = StringFormat(" %ld / %ld", curPos, max);
    progStr += offStr;

    return progStr;
}

String TxtViewerApp::onGetStrFontSize() {
    return StringFormat("Font SIZE %d C(-) A(+)", fontSize);
}
String TxtViewerApp::onGetStrFontSpacing() {
    return StringFormat("Font SPACE %d C(-) A(+)", fontSpacing);
}
// ===== END TOOLBAR