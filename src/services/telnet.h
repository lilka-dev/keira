#include <ESPTelnet.h>
#include "Preferences.h"
#include "servicemanager.h"
#include "network.h"
#include "service.h"

class TelnetService : public Service {
public:
    TelnetService();
    ~TelnetService();

private:
    void setupEventHandlers();
    void run() override;
};
