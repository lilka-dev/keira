#include "mdns.h"
#include "keira/ksystem.h"
#include "services/network/network.h"

MDNSService::MDNSService() : Service("mdns") {
    NVS_LOCK;
    Preferences prefs;
    prefs.begin(getName(), true);
    hostname = prefs.getString("hostname", MDNS_DEFAULT_HOSTNAME);
    prefs.end();
    NVS_UNLOCK;
}

void MDNSService::run() {
    // Await network service
    while (networkService == NULL) {
        networkService = static_cast<NetworkService*>(ksystem.services["network"]);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    bool wasOnline = false;
    while (true) {
        bool isOnline = networkService->getnetworkState() == NetworkState::NETWORK_STATE_ONLINE;

        if ((getEnabled() && isOnline) && !wasOnline) {
            startMDNS();
            wasOnline = true;
        } else if ((!getEnabled() || !isOnline) && wasOnline) {
            stopMDNS();
            wasOnline = false;
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void MDNSService::startMDNS() {
    if (mdnsStarted) {
        return;
    }

    esp_err_t err = mdns_init();
    if (err != ESP_OK) {
        lilka::serial.log("MDNSService: Failed to init mDNS: %s", esp_err_to_name(err));
        return;
    }

    err = mdns_hostname_set(hostname.c_str());
    if (err != ESP_OK) {
        lilka::serial.log("MDNSService: Failed to set hostname: %s", esp_err_to_name(err));
        mdns_free();
        return;
    }

    lilka::serial.log("MDNSService: mDNS started with hostname %s.local", hostname.c_str());

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
    lilka::serial.log("MDNSService: mDNS stopped");
}

String MDNSService::getHostname() {
    return hostname;
}

void MDNSService::setHostname(const String& newHostname) {
    if (newHostname.isEmpty() || newHostname == hostname) {
        return;
    }

    hostname = newHostname;

    NVS_LOCK;
    Preferences prefs;
    prefs.begin(getName(), false);
    prefs.putString("hostname", hostname);
    prefs.end();
    NVS_UNLOCK;

    // Restart mDNS if it's running
    if (mdnsStarted) {
        stopMDNS();
        startMDNS();
    }
}

String MDNSService::getFullHostname() {
    return hostname + ".local";
}
