#include "keyboard.h"
#include "keira/keira.h"
KeyboardApp::KeyboardApp() : App("Keyboard") {
}

void KeyboardApp::run() {
    auto inStr = input(K_S_KEYBOARD_ENTER_TEXT);

    alert(K_S_KEYBOARD_YOU_ENTERED, inStr);
}
