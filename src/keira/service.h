#pragma once
#include "keira/thread.h"
class KeiraConfig;

class Service : public KeiraThread {
public:
    KeiraConfig& getConfig();
    Service();
};
