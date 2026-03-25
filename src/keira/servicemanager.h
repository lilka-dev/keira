#pragma once
#include "keira/bits/servicemanager.h"

#include "keira/threadmanager.h"

// Uncomment this line to get debugging information
//#define KEIRA_SERVICEMANAGER_DEBUG

class ServiceManager : public ThreadManager {
public:
    ~ServiceManager();
    ServiceManager();

    // Start service — reads enabled from registry config, spawns if true
    void up(const char* name);

    // Stop service
    void down(const char* name);

    // Returns true if service is currently running
    bool status(const char* name);

    // Run loop — iterates registry, spawns autostart services
    void run() override;
};