#include "lileco.h"

LilecoApp::LilecoApp(String path) :
    App("Lileco", 0, 0, lilka::display.width(), lilka::display.height()) {
    setFlags(AppFlags::APP_FLAG_FULLSCREEN);

    String localPath = lilka::fileutils.getLocalPathInfo(path).path;
    if (localPath.isEmpty()) localPath = path;
    int16_t slashIndex = localPath.lastIndexOf('/');
    if (slashIndex >= 0 && slashIndex < localPath.length() - 1) {
        selectedFileName = localPath.substring(slashIndex + 1);
    } else {
        selectedFileName = localPath;
    }
    if (selectedFileName.isEmpty()) {
        selectedFileName = "Unknown file";
    }
}

void LilecoApp::run() {
    while (1) {
        drawScreen();
        lilka::State state = lilka::controller.getState();
        if (state.b.justPressed) {
            break;
        }
        vTaskDelay(16 / portTICK_PERIOD_MS);
    }
}

void LilecoApp::drawScreen() {
    canvas->fillScreen(lilka::colors::Black);
    canvas->setFont(FONT_8x13);
    canvas->setTextColor(lilka::colors::White);
    canvas->drawTextAligned("Lileco", canvas->width() / 2, 40, lilka::ALIGN_CENTER, lilka::ALIGN_CENTER);
    canvas->drawTextAligned(
        "Selected ColecoVision ROM:", canvas->width() / 2, canvas->height() / 2 - 20, lilka::ALIGN_CENTER, lilka::ALIGN_CENTER
    );
    canvas->drawTextAligned(selectedFileName.c_str(), canvas->width() / 2, canvas->height() / 2 + 10, lilka::ALIGN_CENTER, lilka::ALIGN_CENTER);
    canvas->drawTextAligned("Press B to exit", canvas->width() / 2, canvas->height() - 30, lilka::ALIGN_CENTER, lilka::ALIGN_CENTER);
    queueDraw();
}
