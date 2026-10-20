#pragma once

#include <stdint.h>
#include <stddef.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

//////////////////////////////////////////////////////////////////////////////
// USB composite device: CDC-ACM (serial monitor) + MSC (mass storage)
//////////////////////////////////////////////////////////////////////////////
// ESP32-S3 has a single USB PHY shared between USB-Serial/JTAG and USB-OTG.
// At power-on it belongs to USB-Serial/JTAG (ARDUINO_USB_MODE=1), which
// carries `Serial` and therefore lilka::serial. Keira keeps it that way and
// starts this device only while the USB Drive app is open.
//
// begin() moves the PHY to USB-OTG and enumerates one composite device:
//  - CDC-ACM port: everything written to `Serial` is forwarded here
//    (lilka::serial, ESP logs and stdout all end up in HWCDC::write,
//    which is wrapped at link time, see src/CMakeLists.txt). Output written
//    while no host has the port open is kept in a backlog and sent on connect.
//    esptool's DTR/RTS reset sequence and a 1200 baud touch reboot to download
//    mode (on USB-Serial/JTAG), so flashing works without buttons. No USB serial
//    number, so macOS names the port by location, the same as USB-Serial/JTAG.
//  - MSC LUN that behaves like a card reader: no medium until attachDisk()
// end() disconnects it and hands the PHY back to USB-Serial/JTAG.
// A restart while active hands the PHY back to USB-Serial/JTAG as well.
//
// Only one instance can be active at a time (TinyUSB callbacks are global).
//////////////////////////////////////////////////////////////////////////////

/// Block device exposed through MSC. Methods are called from the USB task.
class USBBlockDevice {
public:
    virtual ~USBBlockDevice() = default;
    virtual uint32_t blockCount() = 0;
    virtual uint16_t blockSize() = 0;
    virtual bool readBlocks(uint32_t lba, uint8_t* dst, uint32_t count) = 0;
    virtual bool writeBlocks(uint32_t lba, const uint8_t* src, uint32_t count) = 0;
};

class USBComposite {
public:
    USBComposite() = default;
    ~USBComposite();

    bool begin();
    void end();

    bool isActive() const {
        return active;
    }
    // Host has configured the device
    bool isMounted() const;
    // Host has the CDC port open (DTR set)
    bool isSerialConnected() const;
    // Ticks since begin()
    TickType_t uptime() const {
        return xTaskGetTickCount() - startTick;
    }
    // Stack state and registers, for logs
    static void diagnose(char* buf, size_t size);

    // MSC medium. Without one the drive reports "no medium", like an empty card reader
    bool attachDisk(USBBlockDevice* blockDevice);
    void detachDisk();
    bool isDiskAttached() const {
        return disk != nullptr;
    }
    // Host has "safely removed" the attached disk
    bool isEjected() const {
        return ejected;
    }

    // Called from the HWCDC::write wrapper
    size_t writeSerial(const uint8_t* data, size_t size);

    // Called from TinyUSB callbacks (USB task)
    bool mscReady(uint8_t lun);
    void mscCapacity(uint32_t* blockCount, uint16_t* blockSize);
    int32_t mscRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t size);
    int32_t mscWrite(uint32_t lba, uint32_t offset, const uint8_t* buffer, uint32_t size);
    void mscStartStop(bool start, bool loadEject);
    void cdcLineState(bool dtr, bool rts);
    void cdcLineCoding(uint32_t baudRate);
    void cdcHostGone();

private:
    static void usbTask(void* arg);
    static void rebootToDownloadMode();

    bool serialReady() const;
    void cdcHostOpened();
    static void readAheadTask(void* arg);
    void readAheadWait();

    void backlogAppend(const uint8_t* data, size_t size);
    void backlogDrain();

    TaskHandle_t taskHandle = nullptr;
    TickType_t startTick = 0;
    volatile bool active = false;
    volatile bool stopRequested = false;

    // MSC
    SemaphoreHandle_t diskMutex = xSemaphoreCreateMutex();
    USBBlockDevice* volatile disk = nullptr;
    volatile bool ejected = false;
    bool mediumChanged = false;

    // MSC read-ahead, see mscRead()
    static constexpr size_t READ_AHEAD_SIZE = 4096; // >= CFG_TUD_MSC_BUFSIZE, checked in the .cpp
    enum ReadAheadState : uint8_t { READ_AHEAD_IDLE, READ_AHEAD_BUSY, READ_AHEAD_READY };
    TaskHandle_t readAheadTaskHandle = nullptr;
    SemaphoreHandle_t readAheadDone = xSemaphoreCreateBinary();
    uint8_t* readAhead = nullptr;
    volatile ReadAheadState readAheadState = READ_AHEAD_IDLE;
    uint32_t readAheadLba = 0;
    uint32_t readAheadCount = 0;
    bool readAheadPending = false;

    // CDC
    SemaphoreHandle_t serialMutex = xSemaphoreCreateMutex();
    uint8_t* backlog = nullptr;
    size_t backlogHead = 0;
    size_t backlogLen = 0;
    uint8_t resetStep = 0;
    bool lineDtr = false;
    bool lineRts = false;
    volatile TickType_t openedSince = 0;
    volatile bool hostOpened = false;
    volatile bool serialStalled = false;
};
