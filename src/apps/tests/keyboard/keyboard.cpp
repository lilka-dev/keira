#include "keyboard.h"

KeyboardApp::KeyboardApp() : App("Keyboard") {
}

void KeyboardApp::run() {
    lilka::InputDialog dialog("Введіть текст:");

    while (true) {
        dialog.update();
        dialog.draw(canvas);
        queueDraw();
        if (dialog.isFinished()) {
            break;
        }
    }

    alert("Ви ввели:", dialog.getValue());
}
