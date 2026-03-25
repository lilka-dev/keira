#pragma once
#include "keira/ksystem.h"
#include "keira/thread.h"
#include "keira/config.h"

class Service : public KeiraThread {
public:
    KeiraConfig& getConfig();
    Service();
};
