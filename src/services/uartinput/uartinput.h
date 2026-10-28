#pragma once

#include "keira/service.h"

// Lets a host drive Lilka buttons over the USB serial port (e.g. for automated tests).
//
// Protocol: one command per line, each line starts with UART_INPUT_PREFIX. Lines not starting with the
// prefix are left untouched for stdin / REPLs.
//
//   @press <button> [ms]   press, hold for ms (default UART_INPUT_PRESS_MS), release
//   @hold <button>         press and keep pressed
//   @release [button]      release button, or all buttons when omitted
//   @screenshot            send what is on the screen as a PNG image: "@png <base64>" lines,
//                          then "@png end <size in bytes>"
//
// Buttons: up, down, left, right, a, b, c, d, select, start
// Replies (via lilka::serial.log): "@ok <command>" or "@err <reason>: <command>"

#define UART_INPUT_PREFIX   '@'
#define UART_INPUT_MAX_LINE 64
#define UART_INPUT_PRESS_MS 100
// Debounce + controller poll period; shorter state changes may be missed
#define UART_INPUT_MIN_PRESS_MS 20
// Pause after each state change so the controller sees every edge
#define UART_INPUT_SETTLE_MS 30
// Raw PNG bytes per "@png" line, a multiple of 3 that fits lilka::serial's line buffer once base64 encoded
#define UART_INPUT_PNG_CHUNK 144

class UARTInputService : public Service {
public:
    UARTInputService();

private:
    void run() override;
    void handleCommand(const String& line);
    void setPressed(lilka::Button button, bool pressed);
    void releaseAll();
    bool sendScreenshot();
    String lineBuffer;
};
