#pragma once

#include "keira/service.h"

/// Streams the composed screen (panel + top app + toast) over the USB serial port and accepts
/// button presses from the host. Used to record the KeiraOS release video without LCD-TAP.
///
/// Host -> device, one command per line:
///   S1 / S0        start / stop streaming
///   K              send a key frame next
///   B<n><0|1>      press / release button n (lilka::Button: 0 up 1 down 2 left 3 right 4 a 5 b 6 c 7 d 8 select 9 start)
///   T<n> <ms>      tap button n for ms milliseconds
///   L1 / L0        frame-by-frame mode on / off (virtual clock, see keira/recclock.h)
///   F              frame-by-frame: advance one frame (1/30 s) and send it
///
/// Device -> host frame: "LKF1" u32 seq, u32 t_ms, u16 w, u16 h, u8 kind, u32 len, 3 pad, payload[len], u32 sum (all LE)
///   kind 0 = key frame, 1 = delta vs previous frame. Payload is a sequence of u16 tokens over RGB565 words:
///   bit15 set -> skip (token & 0x7fff) unchanged words; bit14 set -> next u16 repeated (token & 0x3fff) times;
///   else (token) literal words follow.
///   sum = byte sum of the payload. Log lines may appear between frames; the host skips them.
class ScreenStreamService : public Service {
public:
    ScreenStreamService();

private:
    void run() override;
    void handleCommand(const char* line);
    void setButton(int button, bool pressed);
    void releaseAll();
    size_t encode(const uint16_t* cur, const uint16_t* prev, size_t words, uint8_t* out);

    bool streaming = false;
    bool keyRequested = true;
    uint32_t seq = 0;
    int tapButton = -1;
    uint32_t tapUntil = 0;
    uint32_t pressedSince[10] = {};
    bool stepRequested = false;
    bool resendRequested = false;
    bool buttonsChanged = false;
};
