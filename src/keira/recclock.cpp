#include <esp_timer.h>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "keira/recclock.h"
#include "keira/ksystem.h"

namespace RecClock {

static volatile bool sActive = false;
static volatile uint64_t sVirtualUs = 0;
static int64_t sEpochAtStartUs = 0;
static uint64_t sVirtualAtStartUs = 0;

static int64_t realEpochUs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}
static SemaphoreHandle_t sGrant = xSemaphoreCreateBinary();
static SemaphoreHandle_t sDone = xSemaphoreCreateBinary();

void setActive(bool active) {
    if (active && !sActive) {
        sVirtualUs = esp_timer_get_time();
        sVirtualAtStartUs = sVirtualUs;
        sEpochAtStartUs = realEpochUs();
    }
    sActive = active;
    if (!active) {
        xSemaphoreGive(sGrant); // let a waiting app go on in real time
    }
}

bool active() {
    return sActive;
}

uint64_t nowUs() {
    return sActive ? sVirtualUs : esp_timer_get_time();
}

int64_t epochUs() {
    return sActive ? sEpochAtStartUs + static_cast<int64_t>(sVirtualUs - sVirtualAtStartUs) : realEpochUs();
}

void frameBarrier(App* app) {
    if (!sActive || app == ksystem.apps.getpanel()) {
        return;
    }
    xSemaphoreGive(sDone);
    xSemaphoreTake(sGrant, pdMS_TO_TICKS(1000));
}

bool step(uint32_t timeoutMs) {
    xSemaphoreTake(sDone, 0); // drop the report of the frame already captured
    sVirtualUs += kFrameUs;
    xSemaphoreGive(sGrant);
    return xSemaphoreTake(sDone, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}

} // namespace RecClock

// Overrides the weak hook in the Lilka library (menu animations, input dialog cursor blink):
// menus follow the virtual clock while recording, so frame-by-frame video plays them smoothly.
extern "C" unsigned long lilka_ui_millis() {
    return RecClock::nowMs();
}
