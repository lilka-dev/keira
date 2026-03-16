#pragma once

#include "keira/app.h"
#include <lilka/dynloader.h>

/**
 * @brief Keira App that loads and runs a dynamically linked .so from SD card.
 *
 * The .so app must export an `app_main(int argc, char *argv[])` entry point.
 * It can call Keira API functions (display, controller, etc.) which are
 * exported to the loaded ELF via the symbol table.
 */
class DynApp : public App {
public:
    explicit DynApp(const String& path);

private:
    void run() override;
    void onExit() override;

    /// Register Keira API symbols for the loaded app
    void registerAppSymbols();
    /// Unregister Keira API symbols
    void unregisterAppSymbols();

    String filePath;
};
