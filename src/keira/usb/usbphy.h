#pragma once

// ESP32-S3 USB PHY ownership switching. Kept in C: IDF's usb_phy_ll.h doesn't compile as C++.

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Detach USB-Serial/JTAG from the bus, reset and clock USB-OTG, and connect it to the internal PHY
void usb_phy_switch_to_otg(void);

/// Give the internal PHY back to USB-Serial/JTAG. Safe in a shutdown handler.
/// PHY selection lives in the RTC domain and survives a software reset.
void usb_phy_switch_to_serial_jtag(void);

/// The internal PHY is selected for USB-OTG. The selection lives in the RTC domain and survives resets
bool usb_phy_is_otg(void);

/// Soft-disconnect USB-OTG from the bus, PHY selection unchanged. Safe in a shutdown handler
void usb_phy_otg_detach(void);

/// Human-readable dump of the PHY/OTG registers, for diagnostics
void usb_phy_dump(char* buf, size_t size);

#ifdef __cplusplus
}
#endif
