#include <ESPTelnet.h>
#include "Preferences.h"
#include "keira/servicemanager.h"
#include "services/network/network.h"
#include "keira/service.h"

class TelnetService : public Service {
public:
    TelnetService();
    ~TelnetService();

private:
    void setupEventHandlers();
    void run() override;
};
