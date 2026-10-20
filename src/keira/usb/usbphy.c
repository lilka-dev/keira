#include "usbphy.h"

#include <stdio.h>
#include <driver/gpio.h>
#include <hal/clk_gate_ll.h>
#include <esp_rom_gpio.h>
#include <esp_rom_sys.h>
#include <esp32s3/rom/gpio.h>
#include <hal/gpio_ll.h>
#include <hal/usb_hal.h>
#include <hal/usb_phy_ll.h>
#include <soc/rtc_cntl_struct.h>
#include <soc/usb_periph.h>
#include <soc/usb_pins.h>
#include <soc/usb_reg.h>
#include <soc/usb_serial_jtag_struct.h>
#include <soc/usb_struct.h>
#include <soc/usb_wrap_struct.h>

// Both controllers can override the pad pull-ups. Take D+ pull-up away on both,
// so the host sees a clean detach whichever one owns the PHY right now
static void force_detach(void) {
    USB_SERIAL_JTAG.conf0.pad_pull_override = 1;
    USB_SERIAL_JTAG.conf0.dp_pullup = 0;
    USB_WRAP.otg_conf.pad_pull_override = 1;
    USB_WRAP.otg_conf.dp_pullup = 0;
}

static void release_pulls(void) {
    USB_WRAP.otg_conf.pad_pull_override = 0;
    USB_SERIAL_JTAG.conf0.dp_pullup = 1;
    USB_SERIAL_JTAG.conf0.pad_pull_override = 0;
}

// USB_WRAP and USB0 registers are only accessible while the USB-OTG clock runs,
// touching them otherwise can stall the CPU. Clock gating here is not reference counted
// (unlike periph_module_enable), so "off" really means off
static void otg_clock_on(void) {
    if (!periph_ll_periph_enabled(PERIPH_USB_MODULE)) periph_ll_enable_clk_clear_rst(PERIPH_USB_MODULE);
}

void usb_phy_switch_to_otg(void) {
    // Reset the controller only when it isn't running: TinyUSB is initialized once and keeps its state,
    // a controller reset under it on a later start leaves the device invisible to the host
    if (!periph_ll_periph_enabled(PERIPH_USB_MODULE)) {
        periph_ll_disable_clk_set_rst(PERIPH_USB_MODULE);
        periph_ll_enable_clk_clear_rst(PERIPH_USB_MODULE);
    }

    force_detach();
    // Long enough for any host to register the detach
    esp_rom_delay_us(100 * 1000);

    // USB_WRAP.date doubles as the ROM's "USB persist" flag register and survives resets.
    // With bit 31 left set, dcd_init restores a "persisted" session instead of connecting,
    // so the device never pulls D+ up and the host never sees it
    USB_WRAP.date.val = 0;

    // Connect USB-OTG to the internal PHY
    usb_hal_context_t hal = {.use_external_phy = false};
    usb_hal_init(&hal);

    // Route the internal OTG signals (VBUS valid, ID, etc.)
    for (const usb_iopin_dsc_t* iopin = usb_periph_iopins; iopin->pin != -1; ++iopin) {
        if (iopin->ext_phy_only) continue;
        esp_rom_gpio_pad_select_gpio(iopin->pin);
        if (iopin->is_output) {
            esp_rom_gpio_connect_out_signal(iopin->pin, iopin->func, false, false);
        } else {
            esp_rom_gpio_connect_in_signal(iopin->pin, iopin->func, false);
            if (iopin->pin != GPIO_FUNC_IN_LOW && iopin->pin != GPIO_FUNC_IN_HIGH) {
                gpio_ll_input_enable(&GPIO, iopin->pin);
            }
        }
        esp_rom_gpio_pad_unhold(iopin->pin);
    }
    gpio_set_drive_capability(USBPHY_DM_NUM, GPIO_DRIVE_CAP_3);
    gpio_set_drive_capability(USBPHY_DP_NUM, GPIO_DRIVE_CAP_3);

    // The core starts in whatever mode the (debounced) ID signal says, which is still "host" here.
    // Device registers (DCFG, DCTL) written in host mode are dropped and reset once the core flips,
    // leaving DCTL.SftDiscon set: the device never connects. Force device mode and wait for the switch
    USB0.gusbcfg |= USB_FORCEDEVMODE_M;
    for (int i = 0; i < 100 && (USB0.gintsts & USB_CURMOD_INT_M); i++) {
        esp_rom_delay_us(1000);
    }

    // The OTG controller drives its own pull-up from here (soft connect in dcd_init)
    release_pulls();
}

void usb_phy_switch_to_serial_jtag(void) {
    bool clockWasOff = !periph_ll_periph_enabled(PERIPH_USB_MODULE);
    otg_clock_on();
    force_detach();
    esp_rom_delay_us(100 * 1000);
    release_pulls();
    usb_phy_ll_int_jtag_enable(&USB_SERIAL_JTAG);
    if (clockWasOff) periph_ll_disable_clk_set_rst(PERIPH_USB_MODULE);
}

void usb_phy_otg_detach(void) {
    if (!periph_ll_periph_enabled(PERIPH_USB_MODULE)) return;
    // Soft disconnect (DCTL.SftDiscon), the host sees the device leave right away
    USB0.dctl |= USB_SFTDISCON_M;
}

void usb_phy_dump(char* buf, size_t size) {
    if (!periph_ll_periph_enabled(PERIPH_USB_MODULE)) {
        snprintf(buf, size, "USB-OTG clock off");
        return;
    }
    snprintf(
        buf,
        size,
        "rtc.usb_conf=%08x wrap.otg_conf=%08x wrap.date=%08x usj.conf0=%08x "
        "gotgctl=%08x gusbcfg=%08x gintsts=%08x gintmsk=%08x dcfg=%08x dctl=%08x dsts=%08x",
        (unsigned)RTCCNTL.usb_conf.val,
        (unsigned)USB_WRAP.otg_conf.val,
        (unsigned)USB_WRAP.date.val,
        (unsigned)USB_SERIAL_JTAG.conf0.val,
        (unsigned)USB0.gotgctl,
        (unsigned)USB0.gusbcfg,
        (unsigned)USB0.gintsts,
        (unsigned)USB0.gintmsk,
        (unsigned)USB0.dcfg,
        (unsigned)USB0.dctl,
        (unsigned)USB0.dsts
    );
}

bool usb_phy_is_otg(void) {
    return RTCCNTL.usb_conf.sw_hw_usb_phy_sel && RTCCNTL.usb_conf.sw_usb_phy_sel;
}
