#include "usbdrive.h"
#include "keira/keira.h"
#include "keira/keira_lang.h"

#include <lilka.h>
#include <SD.h>
#include <ff.h>
#include <diskio.h>
#include <diskio_impl.h>

bool SDBlockDevice::begin() {
    uint32_t count = SD.numSectors();
    uint16_t size = SD.sectorSize();
    if (count == 0) {
        lilka::serial.err("USB MSC: failed to get SD card size");
        return false;
    }
    // SD cards use 512-byte sectors at the physical level
    sectorSize = size ? size : 512;
    sectors = count;
    pdrv = findDrive();
    lilka::serial.log(
        "USB MSC: SD card %llu bytes, sector size %u, sectors %lu, %s",
        static_cast<uint64_t>(sectors) * sectorSize,
        static_cast<unsigned>(sectorSize),
        static_cast<unsigned long>(sectors),
        pdrv == NO_DRIVE ? "single-sector access" : "multi-sector access"
    );
    return true;
}

// SD.readRAW()/writeRAW() move one sector per SD command, which makes USB MSC slow (the host reads
// megabytes of FAT on mount). The same driver does multi-sector transfers (CMD18/CMD25) through the
// FatFs diskio layer, but the SD object keeps its drive number private: find it here. Every slot below
// the first free one is registered, and the SD card is the one returning the same sector 0
uint8_t SDBlockDevice::findDrive() {
    BYTE firstFree = FF_VOLUMES;
    if (ff_diskio_get_drive(&firstFree) != ESP_OK) firstFree = FF_VOLUMES;
    if (sectorSize > sizeof(probe) / 2) return NO_DRIVE;
    if (!SD.readRAW(probe, 0)) return NO_DRIVE;
    for (BYTE drive = 0; drive < firstFree; drive++) {
        if (disk_read(drive, probe + sectorSize, 0, 1) != RES_OK) continue;
        if (memcmp(probe, probe + sectorSize, sectorSize) == 0) return drive;
    }
    return NO_DRIVE;
}

bool SDBlockDevice::readBlocks(uint32_t lba, uint8_t* dst, uint32_t count) {
    if (pdrv != NO_DRIVE) return disk_read(pdrv, dst, lba, count) == RES_OK;
    for (uint32_t i = 0; i < count; i++) {
        if (!SD.readRAW(dst + i * sectorSize, lba + i)) return false;
    }
    return true;
}

bool SDBlockDevice::writeBlocks(uint32_t lba, const uint8_t* src, uint32_t count) {
    if (pdrv != NO_DRIVE) return disk_write(pdrv, src, lba, count) == RES_OK;
    for (uint32_t i = 0; i < count; i++) {
        // writeRAW doesn't modify the buffer, its signature just lacks const
        if (!SD.writeRAW(const_cast<uint8_t*>(src + i * sectorSize), lba + i)) return false;
    }
    return true;
}

USBDriveApp::USBDriveApp() : App("USB Drive") {
    setFlags(APP_FLAG_FULLSCREEN);
}

void USBDriveApp::waitForExit() {
    while (true) {
        lilka::State state = lilka::controller.getState();
        if (state.a.justPressed || state.b.justPressed) {
            return;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void USBDriveApp::run() {
    // Show experimental feature disclaimer
    if (!confirm(K_S_USB_DRIVE_EXPERIMENTAL_TITLE, K_S_USB_DRIVE_EXPERIMENTAL_WARNING)) return; // User cancelled

    // Check SD card first
    if (!lilka::fileutils.isSDAvailable()) {
        canvas->fillScreen(lilka::colors::Black);
        canvas->setTextColor(lilka::colors::Red);
        canvas->setFont(FONT_9x15);
        canvas->setCursor(16, 40);
        canvas->print(K_S_USB_DRIVE_NO_SD);
        canvas->setTextColor(lilka::colors::White);
        canvas->setCursor(16, 120);
        canvas->print(K_S_USB_DRIVE_PRESS_A_TO_EXIT);
        queueDraw();
        waitForExit();
        return;
    }

    canvas->fillScreen(lilka::colors::Black);
    canvas->setTextColor(lilka::colors::White);
    canvas->setFont(FONT_9x15);
    canvas->setCursor(16, 40);
    canvas->print(K_S_USB_DRIVE_INITIALIZING);
    canvas->setTextColor(lilka::colors::Light_gray);
    canvas->setFont(FONT_6x13);
    canvas->setCursor(16, 70);
    canvas->print(K_S_USB_DRIVE_CONNECT_USB);
    queueDraw();

    // Switch the port to the CDC + MSC device, the serial monitor keeps working on it
    if (!sd.begin() || !usb.begin() || !usb.attachDisk(&sd)) {
        stop();
        // Show error - SD card is OK but USB init failed
        canvas->fillScreen(lilka::colors::Black);
        canvas->setTextColor(lilka::colors::Red);
        canvas->setFont(FONT_9x15);
        canvas->setCursor(16, 40);
        canvas->print(K_S_USB_DRIVE_INIT_ERROR);
        canvas->setTextColor(lilka::colors::White);
        canvas->setCursor(16, 120);
        canvas->print(K_S_USB_DRIVE_PRESS_A_TO_EXIT);
        queueDraw();
        waitForExit();
        return;
    }

    // Redraw only when the status changes: the display shares the SPI bus with the SD card
    int shownStatus = -1;
    while (true) {
        int status = usb.isEjected() ? 0 : (!usb.isMounted() ? 1 : 2);
        if (status != shownStatus) {
            shownStatus = status;
            drawStatus(status);
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
        lilka::State state = lilka::controller.getState();
        if (!state.a.justPressed) continue;

        // Warn if not safely ejected
        if (!usb.isEjected()) {
            canvas->fillScreen(lilka::colors::Black);
            canvas->setTextColor(lilka::colors::Red);
            canvas->setFont(FONT_9x15);
            canvas->setCursor(16, 60);
            canvas->print(K_S_USB_DRIVE_NOT_EJECTED);
            canvas->setTextColor(lilka::colors::White);
            canvas->setFont(FONT_6x13);
            canvas->setCursor(16, 100);
            canvas->print(K_S_USB_DRIVE_EJECT_WARNING);
            canvas->setTextColor(lilka::colors::Arylide_yellow);
            canvas->setCursor(16, 140);
            canvas->print(K_S_USB_DRIVE_PRESS_START_CONTINUE);
            canvas->setCursor(16, 160);
            canvas->print(K_S_USB_DRIVE_PRESS_B_CANCEL);
            queueDraw();

            // Wait for confirmation
            bool cancelled = false;
            while (true) {
                lilka::State confirmState = lilka::controller.getState();
                if (confirmState.start.justPressed) {
                    break; // Continue with exit
                }
                if (confirmState.b.justPressed) {
                    cancelled = true;
                    break; // Cancel - go back to main loop
                }
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }

            if (cancelled) {
                shownStatus = -1; // Back to the main screen
                continue;
            }
        }

        canvas->fillScreen(lilka::colors::Black);
        canvas->setTextColor(lilka::colors::Arylide_yellow);
        canvas->setFont(FONT_9x15);
        canvas->setCursor(16, 80);
        canvas->print(K_S_USB_DRIVE_DISCONNECTING);
        queueDraw();

        stop();
        return;
    }
}

void USBDriveApp::drawStatus(int status) {
    canvas->fillScreen(lilka::colors::Black);

    // Title
    canvas->setTextColor(lilka::colors::Jasmine);
    canvas->setFont(FONT_9x15);
    canvas->setCursor(16, 30);
    canvas->print(K_S_USB_DRIVE_TITLE);

    // USB icon (simple representation)
    int centerX = canvas->width() / 2;
    int iconY = 60;
    canvas->fillRoundRect(centerX - 20, iconY, 40, 50, 5, lilka::colors::Dim_gray);
    canvas->fillRect(centerX - 15, iconY + 5, 30, 20, lilka::colors::White);
    canvas->fillRect(centerX - 5, iconY + 50, 10, 15, lilka::colors::Dim_gray);

    // Status
    canvas->setCursor(16, 140);
    if (status == 0) {
        canvas->setTextColor(lilka::colors::Arylide_yellow);
        canvas->print(K_S_USB_DRIVE_EJECTED);
    } else if (status == 1) {
        canvas->setTextColor(lilka::colors::Light_gray);
        canvas->print(K_S_USB_DRIVE_CONNECT_USB);
    } else {
        canvas->setTextColor(lilka::colors::Mint);
        canvas->print(K_S_USB_DRIVE_CONNECTED);
    }

    // Instructions
    canvas->setTextColor(lilka::colors::Light_gray);
    canvas->setFont(FONT_6x13);
    canvas->setCursor(16, 170);
    canvas->print(K_S_USB_DRIVE_PC_INSTRUCTION);

    canvas->setCursor(16, 185);
    canvas->print(K_S_USB_DRIVE_SAFE_EJECT);

    // Exit instruction
    canvas->setTextColor(lilka::colors::Arylide_yellow);
    canvas->setCursor(16, 205);
    canvas->print(K_S_USB_DRIVE_PRESS_A_TO_EXIT);

    queueDraw();
}

// Give the port back to USB-Serial/JTAG and let Keira see what the host changed
void USBDriveApp::stop() {
    if (!usb.isActive()) return;
    usb.end();
    // The host wrote to the card behind FATFS's back, its cached state is stale: remount
    SD.end();
    lilka::fileutils.initSD();
}

void USBDriveApp::onExit() {
    stop();
}
