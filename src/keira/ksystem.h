#pragma once

// System Managers:
#include "keira/servicemanager.h"
#include "keira/appmanager.h"

// Libraries
#include <lilka.h>
#include <vector>

// VFS(Virtual File Systems):
#include "kvfs.h"
#include "keira/vfs/rootfs/rootfs.h"

#define KEIRA_VERSION_TYPE_ACSTR lilka::SDK_VERSION_TYPE_ACSTR

typedef enum KEIRA_VERSION_TYPE : uint8_t {
    KEIRA_VERSION_TYPE_DEV = 0,
    KEIRA_VERSION_TYPE_PRE_RELEASE = 1,
    KEIRA_VERSION_TYPE_RELEASE = 2
} version_type_t;

class KeiraSystem {
public:
    KeiraSystem();

    //===== VFS (Virtual File Systems)
    // This thing is inteded to be used with mountable/remountable in runtime
    // devices. Not used yet, but this would change on adding USB-OTG support
    // Though, not guaranteed we would use it at all
    // TODO: handle rootVFS reg/unreg here
    // void registerVFS(KeiraVFS* fs, const char* path);

    const String& getVersionStr() {
        return version;
    }

    //===== Arduino-like entry points
    void setup();
    void loop();

private:
    //==== Boot Stages
    void showStartupScreen();
    void handleCMDParams();
    void verifyOTA();
    void showWelcomeMessage();
    void launchServices();

    //===== Apps/Services
    AppManager* appManager;
    ServiceManager* serviceManager;

    //===== Version
    String version;
    version_type_t versionType;

    // VFS (Virtual File Systems)
    // std::vector<KeiraVFS*> vfs; // normally to store all here

    // special case, might require reg/unreg after each VFS reg as well as
    // might work normally, we don't know it yet, and this need to check
    RootVFS* rootVFS = NULL;
};

extern KeiraSystem ksystem;
