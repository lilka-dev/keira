#pragma once

#include <mdns.h>
#include "services/network/network.h"
#include "keira/service.h"

// Uncomment this line to get debuging info
#define MDNS_DEBUG

#ifdef MDNS_DEBUG
#    define MDNS_DBG if (1)
#else
#    define MDNS_DBG if (0)
#endif

class MDNSService : public Service {
private:
    String hostname;
    NetworkService* networkService = NULL;
    bool mdnsStarted = false;

public:
    ~MDNSService();

private:
    void run() override;
    void startMDNS();
    void stopMDNS();
};
