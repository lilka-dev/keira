#pragma once

#include "keira/app.h"
#include "keira/usb/usbcomposite.h"

/// Raw sector access to the SD card for USB MSC.
class SDBlockDevice : public USBBlockDevice {
public:
    bool begin();

    uint32_t blockCount() override {
        return sectors;
    }
    uint16_t blockSize() override {
        return sectorSize;
    }
    bool readBlocks(uint32_t lba, uint8_t* dst, uint32_t count) override;
    bool writeBlocks(uint32_t lba, const uint8_t* src, uint32_t count) override;

private:
    static constexpr uint8_t NO_DRIVE = 0xFF;

    uint8_t findDrive();

    uint32_t sectors = 0;
    uint16_t sectorSize = 512;
    // FatFs physical drive of the SD card, NO_DRIVE: fall back to SD.readRAW()/writeRAW()
    uint8_t pdrv = NO_DRIVE;
    uint8_t probe[1024] = {};
};

/// Exposes the SD card to PC as a removable drive.
/// While open, USB runs as a composite device (CDC + MSC), so the serial monitor keeps working.
/// On exit the port goes back to USB-Serial/JTAG and the SD card is remounted, no reboot needed.
class USBDriveApp : public App {
public:
    USBDriveApp();

private:
    void run() override;
    void onExit() override;

    void waitForExit();
    void drawStatus(int status);
    void stop();

    SDBlockDevice sd;
    USBComposite usb;
};
