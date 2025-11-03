#pragma once

#include <lilka.h>
#include "keira/keira.h"
#include "keira/appmanager.h"
#include "Preferences.h"

class Service {
    friend class AppManager;
    friend class ServiceManager;

public:
    Service(const char* name);
    virtual ~Service();
    bool getEnabled();
    void setEnabled(bool enabled);
    void start();

protected:
    void setStackSize(uint32_t stackSize);
    const char* name;

private:
    bool enabled;
    static void _run(void* arg);
    virtual void run() = 0;

    xTaskHandle taskHandle;
    uint32_t stackSize;
};
