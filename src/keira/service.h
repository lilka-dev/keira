#pragma once

#include <lilka.h>
#include "keira/keira.h"
#include "keira/appmanager.h"
#include "keira/thread.h"
#include "Preferences.h"

class Service : public KeiraThread {
    friend ServiceManager; // tmp

public:
    Service(const char* name);
    bool getEnabled();
    void setEnabled(bool enabled);

protected:
    const char* name;

private:
    bool enabled;
};
