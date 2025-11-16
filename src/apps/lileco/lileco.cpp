#include "lileco.h"

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
        lilka::Alert alert("Lileco", "Unable to start emulator.\nCheck coleco.rom BIOS.");
        lilka::serial.err("Failed to start Coleco emulator for %s", romPath.c_str());
        vTaskDelay(1500 / portTICK_PERIOD_MS);
        return;
    }

    while (1) {
        lilka::State state = lilka::controller.getState();
        bool exitCombo = (state.select.pressed && state.b.justPressed) || (state.b.pressed && state.select.justPressed);
        if (exitCombo) break;

        core.step(state);
    }

    core.stop();
}
