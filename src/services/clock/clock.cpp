#include "keira/recclock.h"
#include "clock.h"
#include "services/network/network.h"
#include "keira/ksystem.h"
ClockService::ClockService() : Service("clock") {
}

void ClockService::run() {
    while (1) {
        NetworkService* network = reinterpret_cast<NetworkService*>(ksystem.services["network"]);
        if (network == NULL) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        if (network->getnetworkState() == NetworkState::NETWORK_STATE_ONLINE) {
            lilka::serial.log("ClockService: Setting time from NTP server");
            configTzTime(MYTZ, "ua.pool.ntp.org", "pool.ntp.org");
            // Delay for 12 hours
            vTaskDelay(1000 * 60 * 60 * 12 / portTICK_PERIOD_MS);
        } else {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

struct tm ClockService::getTime() {
    struct tm timeinfo;
    time_t now = static_cast<time_t>(RecClock::epochUs() / 1000000); // follows the video clock while recording
    localtime_r(&now, &timeinfo);
    return timeinfo;
}
