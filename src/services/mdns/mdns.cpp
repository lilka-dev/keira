#include "mdns.h"
#include "keira/ksystem.h"
#include "services/network/network.h"

REG_SERVICE(mdns, MDNSService, true);

MDNSService::~MDNSService() {
    stopMDNS();
}

void MDNSService::run() {
    // Await network service
    while (networkService == NULL) {
        networkService = static_cast<NetworkService*>(ksystem.services["network"]);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    startMDNS();

    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}

void MDNSService::startMDNS() {
    if (mdnsStarted) {
        return;
    }

    esp_err_t err = mdns_init();
    if (err != ESP_OK) {
        MDNS_DBG lilka::serial.log("MDNSService: Failed to init mDNS: %s", esp_err_to_name(err));
        return;
    }
    // TODO: set hostname from network service
    err = mdns_hostname_set(hostname.c_str());
    if (err != ESP_OK) {
        MDNS_DBG lilka::serial.log("MDNSService: Failed to set hostname: %s", esp_err_to_name(err));
        mdns_free();
        return;
    }

    MDNS_DBG lilka::serial.log("MDNSService: mDNS started with hostname %s.local", hostname.c_str());

    // Advertise services
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
    mdns_service_add(NULL, "_ftp", "_tcp", 21, NULL, 0);
    mdns_service_add(NULL, "_telnet", "_tcp", 23, NULL, 0);

    mdnsStarted = true;
}

void MDNSService::stopMDNS() {
    if (!mdnsStarted) {
        return;
    }

    mdns_service_remove_all();
    mdns_free();
    mdnsStarted = false;
    MDNS_DBG lilka::serial.log("MDNSService: mDNS stopped");
}
