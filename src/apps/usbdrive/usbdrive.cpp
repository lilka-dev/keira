#include "usbdrive.h"
#include "keira/keira_lang.h"

#include <lilka.h>
#include <SD.h>
#include <SPI.h>

// TinyUSB includes
#include "USB.h"
#include "USBMSC.h"

// Global pointers for MSC callbacks
static USBMSC* mscDevice = nullptr;
static bool sdCardReady = false;
static uint32_t sdCardSectors = 0;
static uint16_t sdCardSectorSize = 512;

// MSC Callbacks - these are called by TinyUSB
static int32_t onMSCRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    if (!sdCardReady) return -1;
    
    uint32_t bytesToRead = bufsize;
    uint8_t* buf = (uint8_t*)buffer;
    
    // Read sectors
    uint32_t sectorsToRead = (bytesToRead + sdCardSectorSize - 1) / sdCardSectorSize;
    for (uint32_t i = 0; i < sectorsToRead; i++) {
        if (!SD.readRAW((uint8_t*)(buf + i * sdCardSectorSize), lba + i)) {
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
        if (!SD.writeRAW((uint8_t*)(buffer + i * sdCardSectorSize), lba + i)) {
            return -1;
        }
    }
    
    return bytesToWrite;
}

static bool onMSCStartStop(uint8_t power_condition, bool start, bool load_eject) {
    lilka::serial.log("USB MSC: StartStop power=%d start=%d eject=%d", power_condition, start, load_eject);
    return true;
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
    
    sdCardSectors = cardSize / sdCardSectorSize;
    sdCardReady = true;
    
    lilka::serial.log("USB MSC: Card size: %llu bytes, sectors: %lu", cardSize, sdCardSectors);

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

    // Start USB
    USB.begin();
    
    lilka::serial.log("USB MSC: Initialized successfully");
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
    canvas->fillScreen(lilka::colors::Black);
    canvas->setTextColor(lilka::colors::White);
    canvas->setFont(FONT_9x15);
    canvas->setCursor(16, 40);
    canvas->print(K_S_USB_DRIVE_INITIALIZING);
    queueDraw();

    // Try to initialize USB MSC
    mscInitialized = initUSBMSC();

    if (!mscInitialized) {
        // Show error
        canvas->fillScreen(lilka::colors::Black);
        canvas->setTextColor(lilka::colors::Red);
        canvas->setCursor(16, 40);
        canvas->print(K_S_USB_DRIVE_INIT_ERROR);
        canvas->setTextColor(lilka::colors::White);
        canvas->setCursor(16, 80);
        canvas->print(K_S_USB_DRIVE_CHECK_SD);
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
        int iconY = 70;
        canvas->fillRoundRect(centerX - 20, iconY, 40, 50, 5, lilka::colors::Dim_gray);
        canvas->fillRect(centerX - 15, iconY + 5, 30, 20, lilka::colors::White);
        canvas->fillRect(centerX - 5, iconY + 50, 10, 15, lilka::colors::Dim_gray);

        // Status
        canvas->setTextColor(lilka::colors::Mint);
        canvas->setCursor(16, 150);
        canvas->print(K_S_USB_DRIVE_CONNECTED);

        // Instructions
        canvas->setTextColor(lilka::colors::Light_gray);
        canvas->setFont(FONT_6x13);
        canvas->setCursor(16, 180);
        canvas->print(K_S_USB_DRIVE_PC_INSTRUCTION);

        canvas->setCursor(16, 200);
        canvas->print(K_S_USB_DRIVE_SAFE_EJECT);

        // Exit instruction
        canvas->setTextColor(lilka::colors::Arylide_yellow);
        canvas->setCursor(16, 230);
        canvas->print(K_S_USB_DRIVE_PRESS_A_TO_EXIT);

        queueDraw();

        // Check for exit button
        lilka::State state = lilka::controller.getState();
        if (state.a.justPressed) {
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
