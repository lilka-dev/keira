#pragma once

#include "services/script/script.h"

// Runs mJS script as a background service. Modules that need a canvas or buttons
// (display, controller, resources, transforms, audio) are not available. Errors are logged to serial.
// mJS can't be interrupted between instructions, so a stop request takes effect on the next util.sleep() call.
class MJSService : public ScriptService {
public:
    explicit MJSService(const String& path);

private:
    void run() override;
};
