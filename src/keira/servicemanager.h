#pragma once
#include "keira/service.h"
#include "keira/thread.h"
#include "keira/threadmanager.h"

// TODO: Fully unloadable/loadable/reloadable services

#define REG_SERVICE(NAME, CLASS, ENABLED)

class ServiceManager : public ThreadManager {
public:
    bool getEnabled(const char* name);
    void setEnabled(bool enabled);
    void registerService(const char* name, KeiraCallback serviceFunc, bool enabled);
    //    void update()override;

private:
};
