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

    if (MDNS.begin(hostname.c_str())) {
        lilka::serial.log("MDNSService: mDNS started with hostname %s.local", hostname.c_str());

        // Advertise services
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("ftp", "tcp", 21);
        MDNS.addService("telnet", "tcp", 23);

        mdnsStarted = true;
    } else {
        lilka::serial.log("MDNSService: Failed to start mDNS");
    }
}

void MDNSService::stopMDNS() {
    if (!mdnsStarted) {
        return;
    }

    MDNS.end();
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
