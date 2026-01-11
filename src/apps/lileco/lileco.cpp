/** ColecoVision Emulator for Lilka - Main app: implementations ********/
/**                                                                   **/
/** Author: Oleksii "Alder" Derkach  [https://github.com/alder/]      **/
/**                                                                   **/
/***********************************************************************/
#include "lileco.h"
#include "keira/keira.h"

LilecoApp::LilecoApp(String path) :
    App("Lileco", 0, 0, lilka::display.width(), lilka::display.height()), romPath(path) {
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
    lileco::ColecoCore core;
    if (!core.start(this, romPath)) {
        lilka::Alert alert(K_S_FMANAGER_LILECO, K_S_LILECO_FAILED_COLECO_ROM);
        lilka::serial.err("Failed to start Coleco emulator for %s", romPath.c_str());
        vTaskDelay(1500 / portTICK_PERIOD_MS);
        return;
    }

    while (1) {
        lilka::State state = lilka::controller.getState();
        bool exitCombo = (state.select.pressed && state.b.justPressed) || (state.b.pressed && state.select.justPressed);
        bool helpCombo = (state.select.pressed && state.a.justPressed) || (state.a.pressed && state.select.justPressed);
        if (exitCombo) break;
        if (helpCombo) {
            showControlsOverlay(core);
            continue;
        }

        core.step(state);
    }

    core.stop();
}

void LilecoApp::drawHelpHint() {
    const int16_t barHeight = 20;
    const int16_t y = canvas->height() - barHeight;
    canvas->fillRect(0, y, canvas->width(), barHeight, lilka::colors::Black);
    canvas->setFont(FONT_8x13);
    canvas->setTextColor(lilka::colors::White);
    canvas->setCursor(50, y + 15);
    canvas->print(K_S_LILECO_HELP_HINT);
}

void LilecoApp::showControlsOverlay(lileco::ColecoCore& core) {
    lilka::Alert alert(K_S_LILECO_HELP_TITLE, K_S_LILECO_HELP);
    alert.addActivationButton(lilka::Button::A);
    alert.addActivationButton(lilka::Button::B);
    alert.addActivationButton(lilka::Button::C);
    alert.addActivationButton(lilka::Button::D);
    alert.addActivationButton(lilka::Button::START);
    alert.addActivationButton(lilka::Button::SELECT);
    alert.addActivationButton(lilka::Button::UP);
    alert.addActivationButton(lilka::Button::DOWN);
    alert.addActivationButton(lilka::Button::LEFT);
    alert.addActivationButton(lilka::Button::RIGHT);

    while (!alert.isFinished()) {
        alert.update();
        alert.draw(canvas);
        drawHelpHint();
        queueDraw();
        vTaskDelay(16 / portTICK_PERIOD_MS);
    }

    while (true) {
        lilka::State state = lilka::controller.getState();
        if (!state.select.pressed && !state.a.pressed) break;
        vTaskDelay(16 / portTICK_PERIOD_MS);
    }

    core.presentFrame();
}
