#pragma once

#include <stdint.h>

class App;

/// Frame-by-frame recording clock (used by ScreenStreamService for the release video).
/// While active, animation time stops being wall-clock time: it advances by exactly one video frame
/// each time the host asks for the next frame, and the top app draws exactly one frame per step.
/// Inactive (the default), everything below falls back to real time and never blocks.
namespace RecClock {
static constexpr uint32_t kFrameUs = 33333; // 30 fps

void setActive(bool active);
bool active();
/// Virtual time while recording, esp_timer time otherwise
uint64_t nowUs();
/// Wall-clock epoch time in microseconds that follows the virtual clock while recording (status bar clock)
int64_t epochUs();
inline uint32_t nowMs() {
    return static_cast<uint32_t>(nowUs() / 1000);
}

/// Producer side, called by App::queueDraw(): report the frame and wait for the next step (1 s safety timeout)
void frameBarrier(App* app);

/// Host side, called by ScreenStreamService: advance one frame and wait (up to timeoutMs) until the app drew it
bool step(uint32_t timeoutMs);
} // namespace RecClock
