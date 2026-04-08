#pragma once

#include <mdns.h>
#include "services/network/network.h"
#include "keira/service.h"

#define MDNS_DEFAULT_HOSTNAME "lilka"

class MDNSService : public Service {
private:
    String hostname;
    NetworkService* networkService = NULL;
    bool mdnsStarted = false;

public:
    MDNSService();

    String getHostname();
    void setHostname(const String& newHostname);
    String getFullHostname(); // Returns hostname.local

private:
    void run() override;
    void startMDNS();
    void stopMDNS();
};
