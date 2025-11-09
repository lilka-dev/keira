#include "txtviewer.h"

TxtViewerApp::TxtViewerApp(String fPath) : App("Text Viewer") {
    tView.setTextFile(fPath);
}

void TxtViewerApp::run() {
    while (!tView.isFinished()) {
        tView.update();
        tView.draw(canvas);
        queueDraw();
    }
}

void TxtViewerApp::queueDraw() {
    // show control buttons
    App::queueDraw();
}