#pragma once

#include "keira/app.h"

/// USB Mass Storage Class app that exposes SD card to PC via USB.
/// When active, the SD card is unmounted from Keira and mounted as USB drive on PC.
class USBDriveApp : public App {
public:
    USBDriveApp();

private:
    void run() override;
    void onStop() override;

    bool initUSBMSC();
    void deinitUSBMSC();

    bool mscInitialized;
};
