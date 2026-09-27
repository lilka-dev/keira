#include "usbcomposite.h"

#include <string.h>

#include <esp_heap_caps.h>
#include <esp_system.h>
#include <soc/rtc_cntl_reg.h>

#include "tusb.h"
#include "device/dcd.h"
#include "usbphy.h"

#include <lilka/serial.h>
#include "keira/utils/acquire.h"

// TinyUSB callbacks are global C functions, they reach the device through this pointer
static USBComposite* activeDevice = nullptr;

//////////////////////////////////////////////////////////////////////////////
// Descriptors
//////////////////////////////////////////////////////////////////////////////

static constexpr uint16_t DEVICE_VID = 0x303A; // Espressif
static constexpr uint16_t DEVICE_PID = 0x0002; // Same as arduino-esp32 TinyUSB default

enum {
    ITF_CDC = 0,
    ITF_CDC_DATA,
    ITF_MSC,
    ITF_TOTAL,
};

enum {
    STR_LANGUAGE = 0,
    STR_MANUFACTURER,
    STR_PRODUCT,
    STR_CDC,
    STR_MSC,
};

static constexpr uint8_t EP_CDC_NOTIF = 0x81;
static constexpr uint8_t EP_CDC_OUT = 0x02;
static constexpr uint8_t EP_CDC_IN = 0x82;
static constexpr uint8_t EP_MSC_OUT = 0x03;
static constexpr uint8_t EP_MSC_IN = 0x83;
static constexpr uint16_t EP_SIZE = 64; // Full speed bulk

static constexpr uint16_t CONFIG_TOTAL_LEN = TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_MSC_DESC_LEN;

static const tusb_desc_device_t deviceDescriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    // IAD is required for a composite device with CDC
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = DEVICE_VID,
    .idProduct = DEVICE_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = STR_MANUFACTURER,
    .iProduct = STR_PRODUCT,
    // No serial number: macOS then names the port after its physical location, same as USB-Serial/JTAG
    // (/dev/cu.usbmodem<location>), so one port path works for monitor, reset and flashing
    .iSerialNumber = 0,
    .bNumConfigurations = 1,
};

static const uint8_t configDescriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_TOTAL, 0, CONFIG_TOTAL_LEN, 0, 500),
    TUD_CDC_DESCRIPTOR(ITF_CDC, STR_CDC, EP_CDC_NOTIF, 8, EP_CDC_OUT, EP_CDC_IN, EP_SIZE),
    TUD_MSC_DESCRIPTOR(ITF_MSC, STR_MSC, EP_MSC_OUT, EP_MSC_IN, EP_SIZE),
};

extern "C" uint8_t const* tud_descriptor_device_cb(void) {
    return reinterpret_cast<uint8_t const*>(&deviceDescriptor);
}

extern "C" uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    return configDescriptor;
}

extern "C" uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    // Must stay valid after return, TinyUSB sends it asynchronously
    static uint16_t descriptor[32];
    const char* str;
    size_t len;

    if (index == STR_LANGUAGE) {
        descriptor[1] = 0x0409; // English
        len = 1;
    } else {
        switch (index) {
            case STR_MANUFACTURER:
                str = "Lilka";
                break;
            case STR_PRODUCT:
                str = "Lilka";
                break;
            case STR_CDC:
                str = "Lilka Serial";
                break;
            case STR_MSC:
                str = "Lilka SD Card";
                break;
            default:
                return nullptr;
        }
        len = strnlen(str, sizeof(descriptor) / sizeof(descriptor[0]) - 1);
        for (size_t i = 0; i < len; i++) {
            descriptor[1 + i] = str[i];
        }
    }

    descriptor[0] = (TUSB_DESC_STRING << 8) | (2 * len + 2);
    return descriptor;
}

//////////////////////////////////////////////////////////////////////////////
// TinyUSB callbacks
//////////////////////////////////////////////////////////////////////////////

static void copyPadded(uint8_t* dst, size_t size, const char* src) {
    memset(dst, ' ', size);
    memcpy(dst, src, strnlen(src, size));
}

extern "C" void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {
    copyPadded(vendor_id, 8, "Lilka");
    copyPadded(product_id, 16, "SD Card");
    copyPadded(product_rev, 4, "1.0");
}

extern "C" bool tud_msc_test_unit_ready_cb(uint8_t lun) {
    USBComposite* usb = activeDevice;
    if (!usb) {
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3A, 0x00);
        return false;
    }
    return usb->mscReady(lun);
}

extern "C" void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size) {
    USBComposite* usb = activeDevice;
    if (!usb) {
        *block_count = 0;
        *block_size = 512;
        return;
    }
    usb->mscCapacity(block_count, block_size);
}

extern "C" bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) {
    USBComposite* usb = activeDevice;
    if (usb) usb->mscStartStop(start, load_eject);
    return true;
}

extern "C" int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    USBComposite* usb = activeDevice;
    return usb ? usb->mscRead(lba, offset, buffer, bufsize) : -1;
}

// Signature is fixed by TinyUSB
// cppcheck-suppress constParameterPointer
extern "C" int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    USBComposite* usb = activeDevice;
    return usb ? usb->mscWrite(lba, offset, buffer, bufsize) : -1;
}

extern "C" int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize) {
    switch (scsi_cmd[0]) {
        case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
            // Nothing to lock, accept it
            return 0;
        default:
            // Invalid command operation code
            tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
            return -1;
    }
}

// Bus event counters, for diagnostics
static volatile uint32_t mountCount = 0;
static volatile uint32_t umountCount = 0;
static volatile uint32_t suspendCount = 0;
static volatile uint32_t resumeCount = 0;

extern "C" void tud_mount_cb(void) {
    mountCount = mountCount + 1;
}

extern "C" void tud_umount_cb(void) {
    umountCount = umountCount + 1;
    USBComposite* usb = activeDevice;
    if (usb) usb->cdcHostGone();
}

extern "C" void tud_suspend_cb(bool remote_wakeup_en) {
    suspendCount = suspendCount + 1;
}

extern "C" void tud_resume_cb(void) {
    resumeCount = resumeCount + 1;
}

extern "C" void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    USBComposite* usb = activeDevice;
    if (usb) usb->cdcLineState(dtr, rts);
}

extern "C" void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding) {
    USBComposite* usb = activeDevice;
    if (usb) usb->cdcLineCoding(p_line_coding->bit_rate);
}

//////////////////////////////////////////////////////////////////////////////
// Serial forwarding
//////////////////////////////////////////////////////////////////////////////
// `Serial` is HWCDC (USB-Serial/JTAG). While the composite device owns the PHY,
// HWCDC::write(const uint8_t*, size_t) is redirected to the CDC-ACM port, and
// availableForWrite()/flush() stop looking at the USB-Serial/JTAG ring buffer: it's stuck
// once its host is gone, and lilka::serial spins forever when availableForWrite() is 0.
// Enabled with -Wl,--wrap=... in src/CMakeLists.txt.

extern "C" size_t __real__ZN5HWCDC5writeEPKhj(void* self, const uint8_t* buffer, size_t size);

extern "C" size_t __wrap__ZN5HWCDC5writeEPKhj(void* self, const uint8_t* buffer, size_t size) {
    USBComposite* usb = activeDevice;
    if (usb && usb->isActive()) {
        return usb->writeSerial(buffer, size);
    }
    return __real__ZN5HWCDC5writeEPKhj(self, buffer, size);
}

extern "C" int __real__ZN5HWCDC17availableForWriteEv(void* self);

extern "C" int __wrap__ZN5HWCDC17availableForWriteEv(void* self) {
    const USBComposite* usb = activeDevice;
    if (usb && usb->isActive()) {
        // writeSerial() never blocks for long and takes any size
        return 4096;
    }
    return __real__ZN5HWCDC17availableForWriteEv(self);
}

extern "C" void __real__ZN5HWCDC5flushEv(void* self);

extern "C" void __wrap__ZN5HWCDC5flushEv(void* self) {
    const USBComposite* usb = activeDevice;
    if (usb && usb->isActive()) return;
    __real__ZN5HWCDC5flushEv(self);
}

// Keeps the boot log and anything written while no monitor is attached
static constexpr size_t BACKLOG_SIZE = 16 * 1024;
// Terminals (pyserial, idf_monitor) raise DTR while opening the port and flush their input right after,
// so anything sent in that moment is lost. Hold output back for a bit after DTR goes up
static constexpr TickType_t SERIAL_OPEN_GRACE = pdMS_TO_TICKS(200);
// Don't stall the writer for long if the host stops reading
static constexpr TickType_t SERIAL_WRITE_TIMEOUT = pdMS_TO_TICKS(50);

void USBComposite::backlogAppend(const uint8_t* data, size_t size) {
    if (!backlog) return;
    // Ring buffer, the oldest output is overwritten
    for (size_t i = 0; i < size; i++) {
        backlog[backlogHead] = data[i];
        backlogHead = (backlogHead + 1) % BACKLOG_SIZE;
        if (backlogLen < BACKLOG_SIZE) backlogLen++;
    }
}

// Non-blocking: sends as much of the backlog as the CDC FIFO takes right now
void USBComposite::backlogDrain() {
    while (backlogLen > 0) {
        uint32_t available = tud_cdc_write_available();
        if (available == 0) break;
        size_t tail = (backlogHead + BACKLOG_SIZE - backlogLen) % BACKLOG_SIZE;
        size_t chunk = backlogLen;
        if (chunk > BACKLOG_SIZE - tail) chunk = BACKLOG_SIZE - tail;
        if (chunk > available) chunk = available;
        uint32_t written = tud_cdc_write(backlog + tail, chunk);
        if (written == 0) break;
        backlogLen -= written;
    }
    tud_cdc_write_flush();
}

// A host "listens" once it has opened the port: DTR up, or at least line coding/state set since the device
// was mounted. idf_monitor keeps DTR low on purpose (it would reset UART boards), so DTR alone isn't enough.
// There's no close notification without DTR, so after such a tool exits output just goes nowhere, as on a UART
bool USBComposite::serialReady() const {
    return tud_mounted() && (tud_cdc_connected() || hostOpened) &&
           xTaskGetTickCount() - openedSince > SERIAL_OPEN_GRACE;
}

void USBComposite::cdcHostOpened() {
    if (!hostOpened) openedSince = xTaskGetTickCount();
    hostOpened = true;
}

void USBComposite::cdcHostGone() {
    hostOpened = false;
    serialStalled = false;
}

size_t USBComposite::writeSerial(const uint8_t* data, size_t size) {
    Acquire lock(serialMutex);

    // Everything goes through the backlog, so older output is always sent first
    backlogAppend(data, size);
    if (!serialReady() || serialStalled) return size;

    TickType_t lastProgress = xTaskGetTickCount();
    size_t lastLen = backlogLen;
    while (backlogLen > 0) {
        backlogDrain();
        if (backlogLen == 0) break;
        if (backlogLen != lastLen) {
            lastLen = backlogLen;
            lastProgress = xTaskGetTickCount();
        } else if (xTaskGetTickCount() - lastProgress > SERIAL_WRITE_TIMEOUT) {
            // Host stopped reading: stop waiting on every write, the USB task picks it up once it moves again
            serialStalled = true;
            break;
        }
        vTaskDelay(1);
    }
    return size;
}

//////////////////////////////////////////////////////////////////////////////
// Download mode reset
//////////////////////////////////////////////////////////////////////////////

// Set right before a restart into download mode, read by the shutdown handler
static volatile bool downloadRestart = false;

// esp_restart() resets only the CPUs, the PHY selection (RTC domain) survives: hand the port back
// to USB-Serial/JTAG, so the next boot has its serial port
static void onRestart() {
    // For download mode this is the second switch, right before the ROM starts: without it the ROM's
    // port never showed up
    if (downloadRestart || usb_phy_is_otg()) usb_phy_switch_to_serial_jtag();
}

void USBComposite::rebootToDownloadMode() {
    // ROM download mode talks over USB-Serial/JTAG, give it the PHY back first
    tud_disconnect();
    vTaskDelay(pdMS_TO_TICKS(100));
    usb_phy_switch_to_serial_jtag();
    downloadRestart = true;
    REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
    esp_restart();
}

// Reboot to download mode on esptool's reset sequence. Modeled on the auto-reset circuit of UART boards
// (RTS pulls EN, DTR pulls IO0), so both the classic (0,1)->(1,1)->(1,0)->(0,0) and the tight
// (0,1)->(1,0)->(0,0) (DTR,RTS) sequences match. A lone RTS pulse deliberately does nothing: macOS pulses
// RTS whenever a port is opened, and since the port re-enumerates on every reboot a monitor would
// reset the board on each reconnect, forever
void USBComposite::cdcLineState(bool dtr, bool rts) {
    cdcHostOpened();
    if (dtr == lineDtr && rts == lineRts) return;
    if (dtr && !lineDtr) openedSince = xTaskGetTickCount();
    lineDtr = dtr;
    lineRts = rts;

    if (rts && !dtr) {
        // EN low: "held in reset"
        resetStep = 1;
    } else if (resetStep == 0 || (dtr && rts)) {
        // Not in a reset sequence, or both asserted (no change on real boards)
    } else if (dtr) {
        // EN released while IO0 is low: boot to download mode once the host lets go
        resetStep = 2;
    } else {
        // Both released
        if (resetStep == 2) rebootToDownloadMode();
        resetStep = 0;
    }
}

void USBComposite::cdcLineCoding(uint32_t baudRate) {
    cdcHostOpened();
    // "1200 baud touch", used by Arduino IDE and friends
    if (baudRate == 1200) rebootToDownloadMode();
}

//////////////////////////////////////////////////////////////////////////////
// Mass storage
//////////////////////////////////////////////////////////////////////////////

bool USBComposite::attachDisk(USBBlockDevice* blockDevice) {
    Acquire lock(diskMutex);
    if (disk) return false;
    disk = blockDevice;
    readAheadState = READ_AHEAD_IDLE;
    ejected = false;
    // Report "medium may have changed" once, so the host rereads it
    mediumChanged = true;
    lilka::serial.log("USB MSC: disk attached");
    return true;
}

void USBComposite::detachDisk() {
    Acquire lock(diskMutex);
    if (!disk) return;
    disk = nullptr;
    readAheadState = READ_AHEAD_IDLE;
    ejected = false;
    lilka::serial.log("USB MSC: disk detached");
}

bool USBComposite::mscReady(uint8_t lun) {
    Acquire lock(diskMutex);
    if (!disk || ejected) {
        // Medium not present
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3A, 0x00);
        return false;
    }
    if (mediumChanged) {
        // Not ready to ready change, medium may have changed
        mediumChanged = false;
        tud_msc_set_sense(lun, SCSI_SENSE_UNIT_ATTENTION, 0x28, 0x00);
        return false;
    }
    return true;
}

void USBComposite::mscCapacity(uint32_t* blockCount, uint16_t* blockSize) {
    Acquire lock(diskMutex);
    *blockCount = disk ? disk->blockCount() : 0;
    *blockSize = disk ? disk->blockSize() : 512;
}

void USBComposite::mscStartStop(bool start, bool loadEject) {
    if (!loadEject) return;
    Acquire lock(diskMutex);
    if (!disk) return;
    ejected = !start;
    lilka::serial.log("USB MSC: disk %s by host", start ? "loaded" : "ejected");
}

// Read-ahead: MSC handles one chunk at a time (read from the disk, then send it), and hosts read
// sequentially. So right after a chunk is read, the next one is fetched by readAheadTask while USB
// sends this one. Doubles as a check of the SD card being the slow side (SPI + software CRC per block)

static_assert(CFG_TUD_MSC_BUFSIZE <= 4096, "MSC chunks must fit the read-ahead buffer");

void USBComposite::readAheadTask(void* arg) {
    auto* self = static_cast<USBComposite*>(arg);
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        {
            Acquire lock(self->diskMutex);
            // Attach/detach reset the state under this lock: don't mark stale data as ready
            if (self->readAheadState == READ_AHEAD_BUSY) {
                bool ok =
                    self->disk && self->disk->readBlocks(self->readAheadLba, self->readAhead, self->readAheadCount);
                self->readAheadState = ok ? READ_AHEAD_READY : READ_AHEAD_IDLE;
            }
        }
        xSemaphoreGive(self->readAheadDone);
    }
}

// Only the USB task starts and waits for read-aheads
void USBComposite::readAheadWait() {
    if (!readAheadPending) return;
    xSemaphoreTake(readAheadDone, portMAX_DELAY);
    readAheadPending = false;
}

int32_t USBComposite::mscRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t size) {
    // A read-ahead in flight holds the disk, let it finish first
    readAheadWait();

    Acquire lock(diskMutex);
    if (!disk || ejected) return -1;

    // TinyUSB asks for whole blocks as long as CFG_TUD_MSC_BUFSIZE is a multiple of the block size
    uint16_t blockSize = disk->blockSize();
    if (offset != 0 || size % blockSize != 0) return -1;
    uint32_t count = size / blockSize;

    if (readAheadState == READ_AHEAD_READY && readAheadLba == lba && readAheadCount == count) {
        memcpy(buffer, readAhead, size);
    } else if (!disk->readBlocks(lba, static_cast<uint8_t*>(buffer), count)) {
        readAheadState = READ_AHEAD_IDLE;
        return -1;
    }
    readAheadState = READ_AHEAD_IDLE;

    // Fetch the next chunk while this one goes out; the task starts once the lock is released
    if (readAheadTaskHandle && size <= READ_AHEAD_SIZE && lba + 2 * count <= disk->blockCount()) {
        readAheadLba = lba + count;
        readAheadCount = count;
        readAheadState = READ_AHEAD_BUSY;
        readAheadPending = true;
        xTaskNotifyGive(readAheadTaskHandle);
    }
    return static_cast<int32_t>(size);
}

int32_t USBComposite::mscWrite(uint32_t lba, uint32_t offset, const uint8_t* buffer, uint32_t size) {
    readAheadWait();
    Acquire lock(diskMutex);
    // Writes go straight to the disk, drop read-ahead data that may be outdated now
    readAheadState = READ_AHEAD_IDLE;
    if (!disk || ejected) return -1;

    uint16_t blockSize = disk->blockSize();
    if (offset != 0 || size % blockSize != 0) return -1;

    if (!disk->writeBlocks(lba, buffer, size / blockSize)) return -1;
    return static_cast<int32_t>(size);
}

//////////////////////////////////////////////////////////////////////////////
// Lifecycle
//////////////////////////////////////////////////////////////////////////////

USBComposite::~USBComposite() {
    end();
    if (readAheadTaskHandle) vTaskDelete(readAheadTaskHandle);
    free(readAhead);
    vSemaphoreDelete(readAheadDone);
    free(backlog);
    vSemaphoreDelete(diskMutex);
    vSemaphoreDelete(serialMutex);
}

bool USBComposite::begin() {
    if (active || activeDevice) {
        lilka::serial.err("USB: composite device is already active");
        return false;
    }

    if (!backlog) {
        backlog = static_cast<uint8_t*>(heap_caps_malloc(BACKLOG_SIZE, MALLOC_CAP_SPIRAM));
        if (!backlog) backlog = static_cast<uint8_t*>(malloc(BACKLOG_SIZE));
        if (!backlog) {
            lilka::serial.err("USB: no memory for the serial backlog");
            activeDevice = nullptr;
            return false;
        }
    }
    if (!readAheadTaskHandle) {
        readAhead = static_cast<uint8_t*>(malloc(READ_AHEAD_SIZE));
        // Without it reads just aren't overlapped
        if (readAhead) xTaskCreate(readAheadTask, "usb_msc_ra", 4096, this, 1, &readAheadTaskHandle);
    }
    backlogHead = 0;
    backlogLen = 0;
    resetStep = 0;
    hostOpened = false;
    serialStalled = false;
    stopRequested = false;
    activeDevice = this;

    bool firstStart = !tusb_inited();
    if (firstStart) {
        // esp_restart() keeps the PHY on USB-OTG (the selection lives in the RTC domain) but resets only
        // the CPUs: detach on the way down, so the host notices the reboot at once
        esp_register_shutdown_handler(onRestart);
    }

    // From here on USB-Serial/JTAG is gone, the log continues on the CDC port
    lilka::serial.log("USB: switching PHY to USB-OTG (CDC + MSC)");
    vTaskDelay(pdMS_TO_TICKS(50)); // Let lilka::serial flush the line above
    usb_phy_switch_to_otg();

    if (firstStart) {
        if (!tusb_init()) {
            usb_phy_switch_to_serial_jtag();
            activeDevice = nullptr;
            lilka::serial.err("USB: TinyUSB init failed");
            return false;
        }
    } else {
        // Restarted after end(): stack and controller are still set up, just reconnect
        dcd_int_enable(0);
        tud_connect();
    }

    if (xTaskCreate(usbTask, "usbd", 4096, this, 2, &taskHandle) != pdPASS) {
        taskHandle = nullptr;
        tud_disconnect();
        dcd_int_disable(0);
        usb_phy_switch_to_serial_jtag();
        activeDevice = nullptr;
        lilka::serial.err("USB: failed to create USB task");
        return false;
    }

    startTick = xTaskGetTickCount();
    active = true;
    lilka::serial.log("USB: composite device started");
    return true;
}

void USBComposite::end() {
    if (!active) return;

    detachDisk();

    {
        // Stop forwarding Serial first so nothing writes to CDC while the stack goes down
        Acquire lock(serialMutex);
        active = false;
    }
    tud_disconnect();
    vTaskDelay(pdMS_TO_TICKS(100)); // Give the host time to notice the removal

    stopRequested = true;
    for (int i = 0; i < 50 && taskHandle; i++) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    dcd_int_disable(0);

    usb_phy_switch_to_serial_jtag();
    activeDevice = nullptr;
    lilka::serial.log("USB: PHY returned to USB-Serial/JTAG");
}

void USBComposite::usbTask(void* arg) {
    auto* self = static_cast<USBComposite*>(arg);
    while (!self->stopRequested) {
        // Short timeout, so the backlog goes out soon after a monitor opens the port
        tud_task_ext(10, false);
        if (self->backlogLen > 0 && self->serialReady() && xSemaphoreTake(self->serialMutex, 0) == pdTRUE) {
            size_t before = self->backlogLen;
            self->backlogDrain();
            if (self->backlogLen < before) self->serialStalled = false;
            xSemaphoreGive(self->serialMutex);
        }
    }
    self->taskHandle = nullptr;
    vTaskDelete(nullptr);
}

bool USBComposite::isMounted() const {
    return active && tud_mounted();
}

void USBComposite::diagnose(char* buf, size_t size) {
    char regs[256];
    usb_phy_dump(regs, sizeof(regs));
    snprintf(
        buf,
        size,
        "connected=%d mounted=%d mount=%u umount=%u suspend=%u resume=%u %s",
        tud_connected(),
        tud_mounted(),
        static_cast<unsigned>(mountCount),
        static_cast<unsigned>(umountCount),
        static_cast<unsigned>(suspendCount),
        static_cast<unsigned>(resumeCount),
        regs
    );
}

bool USBComposite::isSerialConnected() const {
    return active && tud_cdc_connected();
}
