#pragma once

#include <lilka.h>
#include "keira/keira.h"
#include "keira/appmanager.h"
#include "keira/thread.h"
#include "Preferences.h"

class Service : public KeiraThread {
public:
    Service(const char* name);
    bool getEnabled();
    void setEnabled(bool enabled);

private:
    bool enabled = false;
};
