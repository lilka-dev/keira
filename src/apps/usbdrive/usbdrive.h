#pragma once

#include "keira/app.h"

/// USB Mass Storage Class app that exposes SD card to PC via USB.
/// Creates a composite USB device (CDC + MSC) so serial still works.
/// When active, the SD card is accessible from PC as a removable drive.
class USBDriveApp : public App {
public:
    USBDriveApp();

private:
    void run() override;
    void onStop() override;

    bool initUSBMSC();
    void deinitUSBMSC();
    bool isDriveEjected();

    bool mscInitialized;
};
