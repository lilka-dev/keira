#include "usbdrive.h"
#include "keira/keira.h"
#include "keira/keira_lang.h"

#include <lilka.h>
#include <SD.h>
#include <SPI.h>

// TinyUSB includes for composite device (CDC + MSC)
#include "USB.h"
#include "USBCDC.h"
#include "USBMSC.h"

// Global pointers for MSC callbacks
static USBMSC* mscDevice = nullptr;
static bool sdCardReady = false;
static bool driveEjected = false; // Track if host safely ejected the drive
static uint32_t sdCardSectors = 0;
static uint16_t sdCardSectorSize = 512;

// MSC Callbacks - these are called by TinyUSB
static int32_t onMSCRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    if (!sdCardReady) return -1;

    uint32_t bytesToRead = bufsize;
    uint8_t* buf = static_cast<uint8_t*>(buffer);

    // Read sectors
    uint32_t sectorsToRead = (bytesToRead + sdCardSectorSize - 1) / sdCardSectorSize;
    for (uint32_t i = 0; i < sectorsToRead; i++) {
        if (!SD.readRAW(buf + i * sdCardSectorSize, lba + i)) {
            return -1;
        }
    }

    return bytesToRead;
}

static int32_t onMSCWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    if (!sdCardReady) return -1;

    uint32_t bytesToWrite = bufsize;

    // Write sectors
    uint32_t sectorsToWrite = (bytesToWrite + sdCardSectorSize - 1) / sdCardSectorSize;
    for (uint32_t i = 0; i < sectorsToWrite; i++) {
        if (!SD.writeRAW(buffer + i * sdCardSectorSize, lba + i)) {
            return -1;
        }
    }

    return bytesToWrite;
}

// Called when host wants to eject the drive
static bool onMSCStartStop(uint8_t power_condition, bool start, bool load_eject) {
    lilka::serial.log("USB MSC: StartStop power=%d start=%d eject=%d", power_condition, start, load_eject);

    if (load_eject) {
        if (!start) {
            // Host is ejecting the drive - mark as safely ejected
            sdCardReady = false;
            driveEjected = true;
            lilka::serial.log("USB MSC: Drive safely ejected by host");
        } else {
            // Host is loading/mounting the drive
            sdCardReady = true;
            driveEjected = false;
            lilka::serial.log("USB MSC: Drive mounted by host");
        }
    }

    return true;
}

// Called to check if device is ready
static bool onMSCReady() {
    return sdCardReady;
}

// Check if drive was safely ejected
bool USBDriveApp::isDriveEjected() {
    return driveEjected;
}

USBDriveApp::USBDriveApp() : App("USB Drive"), mscInitialized(false) {
    setFlags(APP_FLAG_FULLSCREEN);
}

bool USBDriveApp::initUSBMSC() {
    // Check if SD card is available
    if (!lilka::fileutils.isSDAvailable()) {
        lilka::serial.err("USB MSC: SD card not available");
        return false;
    }

    // Get SD card info
    uint64_t cardSize = SD.cardSize();
    if (cardSize == 0) {
        lilka::serial.err("USB MSC: Failed to get card size");
        return false;
    }

    // Get sector size from SD card (typically 512 bytes, but can vary)
    // SD cards use 512-byte sectors at the physical level
    sdCardSectorSize = SD.sectorSize();
    if (sdCardSectorSize == 0) {
        // Fallback to standard sector size if not available
        sdCardSectorSize = 512;
        lilka::serial.log("USB MSC: Using default sector size: %d", sdCardSectorSize);
    }

    sdCardSectors = cardSize / sdCardSectorSize;
    sdCardReady = true;

    lilka::serial.log(
        "USB MSC: Card size: %llu bytes, sector size: %d, sectors: %lu", cardSize, sdCardSectorSize, sdCardSectors
    );

    // Create MSC device
    mscDevice = new USBMSC();
    if (!mscDevice) {
        lilka::serial.err("USB MSC: Failed to create MSC device");
        sdCardReady = false;
        return false;
    }

    // Configure MSC
    mscDevice->vendorID("Lilka");
    mscDevice->productID("SD Card");
    mscDevice->productRevision("1.0");
    mscDevice->onRead(onMSCRead);
    mscDevice->onWrite(onMSCWrite);
    mscDevice->onStartStop(onMSCStartStop);
    mscDevice->mediaPresent(true);

    // Begin MSC with sector count and size
    if (!mscDevice->begin(sdCardSectors, sdCardSectorSize)) {
        lilka::serial.err("USB MSC: Failed to begin MSC");
        delete mscDevice;
        mscDevice = nullptr;
        sdCardReady = false;
        return false;
    }

    // Configure composite USB device (CDC + MSC)
    // CDC is already configured at boot (ARDUINO_USB_CDC_ON_BOOT=1)
    // We just need to ensure USB is started - it will enumerate as composite device
    USB.productName("Lilka");
    USB.manufacturerName("Lilka Team");

    if (!USB) {
        USB.begin();
    }

    lilka::serial.log("USB MSC: Composite device (CDC+MSC) initialized");
    return true;
}

void USBDriveApp::deinitUSBMSC() {
    if (mscInitialized) {
        sdCardReady = false;

        if (mscDevice) {
            mscDevice->end();
            delete mscDevice;
            mscDevice = nullptr;
        }

        // Note: USB.end() might cause issues, so we just disable the MSC
        // The USB stack will be reset on reboot

        mscInitialized = false;
        lilka::serial.log("USB MSC: Deinitialized");
    }
}

void USBDriveApp::run() {
    // Show experimental feature disclaimer
    lilka::Alert confirm(K_S_USB_DRIVE_EXPERIMENTAL_TITLE, K_S_USB_DRIVE_EXPERIMENTAL_WARNING);
    confirm.addActivationButton(K_BTN_CONFIRM);
    confirm.draw(canvas);
    queueDraw();
    while (!confirm.isFinished()) {
        confirm.update();
        taskYIELD();
    }
    if (confirm.getButton() != K_BTN_CONFIRM) {
        return; // User cancelled
    }

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

        // Wait for button press
        while (true) {
            lilka::State state = lilka::controller.getState();
            if (state.a.justPressed || state.b.justPressed) {
                return;
            }
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
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

    // Try to initialize USB MSC
    mscInitialized = initUSBMSC();

    if (!mscInitialized) {
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

        // Wait for button press
        while (true) {
            lilka::State state = lilka::controller.getState();
            if (state.a.justPressed || state.b.justPressed) {
                return;
            }
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }

    // Show success screen with instructions
    while (true) {
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

        // Status - show different message if ejected
        if (isDriveEjected()) {
            canvas->setTextColor(lilka::colors::Arylide_yellow);
            canvas->setCursor(16, 140);
            canvas->print(K_S_USB_DRIVE_EJECTED);
        } else {
            canvas->setTextColor(lilka::colors::Mint);
            canvas->setCursor(16, 140);
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

        // Check for exit button with delay to prevent flickering
        vTaskDelay(50 / portTICK_PERIOD_MS);
        lilka::State state = lilka::controller.getState();
        if (state.a.justPressed) {
            // Warn if not safely ejected
            if (!isDriveEjected()) {
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
                    continue; // Cancel - go back to main screen
                }
            }

            // Show reboot message - USB stack changes require reboot
            canvas->fillScreen(lilka::colors::Black);
            canvas->setTextColor(lilka::colors::Arylide_yellow);
            canvas->setFont(FONT_9x15);
            canvas->setCursor(16, 80);
            canvas->print(K_S_USB_DRIVE_DISCONNECTING);
            canvas->setTextColor(lilka::colors::White);
            canvas->setFont(FONT_6x13);
            canvas->setCursor(16, 120);
            canvas->print(K_S_USB_DRIVE_REBOOT_REQUIRED);
            queueDraw();

            deinitUSBMSC();
            vTaskDelay(2000 / portTICK_PERIOD_MS);

            // Reboot the device
            ESP.restart();
            return;
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void USBDriveApp::onStop() {
    deinitUSBMSC();
}
