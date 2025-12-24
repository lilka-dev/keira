#include "keyboard.h"
#include "keira/keira.h"
KeyboardApp::KeyboardApp() : App("Keyboard") {
}

void KeyboardApp::run() {
    lilka::InputDialog dialog(K_S_KEYBOARD_ENTER_TEXT);

    while (true) {
        dialog.update();
        dialog.draw(canvas);
        queueDraw();
        if (dialog.isFinished()) {
            break;
        }
    }

    alert(K_S_KEYBOARD_YOU_ENTERED, dialog.getValue());
}
