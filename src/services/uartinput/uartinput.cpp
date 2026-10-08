#include "uartinput.h"

typedef struct {
    const char* name = nullptr;
    lilka::Button button = lilka::Button::ANY;
} UARTInputButtonName;

static const UARTInputButtonName buttonNames[] = {
    {"up", lilka::Button::UP},
    {"down", lilka::Button::DOWN},
    {"left", lilka::Button::LEFT},
    {"right", lilka::Button::RIGHT},
    {"a", lilka::Button::A},
    {"b", lilka::Button::B},
    {"c", lilka::Button::C},
    {"d", lilka::Button::D},
    {"select", lilka::Button::SELECT},
    {"start", lilka::Button::START},
};

static bool parseButton(const String& name, lilka::Button& button) {
    for (const auto& entry : buttonNames) {
        if (name.equalsIgnoreCase(entry.name)) {
            button = entry.button;
            return true;
        }
    }
    return false;
}

UARTInputService::UARTInputService() : Service("uartinput") {
}

void UARTInputService::run() {
    while (1) {
        while (Serial.available() > 0) {
            // Only take lines meant for us, leave everything else to stdin / REPLs
            if (lineBuffer.isEmpty() && Serial.peek() != UART_INPUT_PREFIX) {
                break;
            }
            int c = Serial.read();
            if (c < 0 || c == '\r') {
                continue;
            }
            if (c == '\n') {
                handleCommand(lineBuffer.substring(1));
                lineBuffer = "";
                continue;
            }
            if (lineBuffer.length() >= UART_INPUT_MAX_LINE) {
                lilka::serial.err("@err line too long");
                lineBuffer = "";
                continue;
            }
            lineBuffer += static_cast<char>(c);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void UARTInputService::handleCommand(const String& line) {
    String args = line;
    args.trim();
    int space = args.indexOf(' ');
    String command = space < 0 ? args : args.substring(0, space);
    args = space < 0 ? "" : args.substring(space + 1);
    args.trim();
    space = args.indexOf(' ');
    String buttonName = space < 0 ? args : args.substring(0, space);
    String param = space < 0 ? "" : args.substring(space + 1);
    param.trim();

    if (command == "release" && buttonName.isEmpty()) {
        releaseAll();
        lilka::serial.log("@ok %s", line.c_str());
        return;
    }

    lilka::Button button = lilka::Button::ANY;
    if (!parseButton(buttonName, button)) {
        lilka::serial.err("@err unknown button: %s", line.c_str());
        return;
    }

    if (command == "press") {
        int ms = param.isEmpty() ? UART_INPUT_PRESS_MS : param.toInt();
        if (ms < UART_INPUT_MIN_PRESS_MS) {
            lilka::serial.err("@err press must be at least %d ms: %s", UART_INPUT_MIN_PRESS_MS, line.c_str());
            return;
        }
        lilka::controller.setVirtualPressed(button, true);
        vTaskDelay(ms / portTICK_PERIOD_MS);
        setPressed(button, false);
    } else if (command == "hold") {
        setPressed(button, true);
    } else if (command == "release") {
        setPressed(button, false);
    } else {
        lilka::serial.err("@err unknown command: %s", line.c_str());
        return;
    }
    lilka::serial.log("@ok %s", line.c_str());
}

void UARTInputService::setPressed(lilka::Button button, bool pressed) {
    lilka::controller.setVirtualPressed(button, pressed);
    vTaskDelay(UART_INPUT_SETTLE_MS / portTICK_PERIOD_MS);
}

void UARTInputService::releaseAll() {
    for (const auto& entry : buttonNames) {
        lilka::controller.setVirtualPressed(entry.button, false);
    }
    vTaskDelay(UART_INPUT_SETTLE_MS / portTICK_PERIOD_MS);
}
