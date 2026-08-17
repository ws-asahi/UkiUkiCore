/**
 * USBCore_DU.cpp - PluggableUSB bridge for the AVR DU native-USB stack.
 *
 * Provides:
 *   - PluggableUSB_::PluggableUSB_()    (the API declares it, core must define it)
 *   - epBuffer()                        (per-EP type storage queried by plug())
 *   - USB_SendControl / USB_Send / ... (the USBAPI consumed by HID & friends)
 *   - usbcore_*() helpers called from usb_standard.c when handling SETUP
 *
 * Design notes:
 *   * Endpoint allocation:  EP0=control, EP1..3=CDC (notify/RX/TX),
 *     EP4..7=dynamic PluggableUSB. totalEP=8 slots, plug() refuses any
 *     module that would push lastEp past it.
 *   * USB_SendControl appends to a 256B static accumulator. The caller in
 *     usb_standard.c (after PluggableUSB().setup() / getInterface() returns)
 *     emits the accumulator via the existing ep0_start_data_in() multi-packet
 *     control-IN path. No change to the EP0 state machine itself.
 *   * USB_Send (HID reports) appends to a per-EP staging buffer; when the
 *     caller sets TRANSFER_RELEASE we move the bytes into the EP DATAPTR,
 *     arm the IN, and busy-wait for TRNCOMPL. Reports are small (HID = 8B
 *     typical) and only sent at host poll cadence, so the blocking cost is
 *     bounded and matches the standard Arduino HID semantics.
 */
#include <avr/io.h>
#if defined(USB0)

#include <Arduino.h>
#include <string.h>
#include <avr/pgmspace.h>

extern "C" {
  #include "usb_core.h"
  #include "usb_descriptors.h"
}
#include "USBCore_DU.h"
#include "usb_ep_types.h"

#include "api/USBAPI.h"
#include "api/PluggableUSB.h"

/* ============================================================
 *  Constants
 * ============================================================ */
#define USBCORE_NUM_EP        8                 /* EP0..EP7 (logical max)    */
#define USBCORE_DYN_EP_BASE   4                 /* first plugged EP          */
#define USBCORE_DYN_EP_COUNT  (USBCORE_NUM_EP - USBCORE_DYN_EP_BASE)
#define USBCORE_CDC_LAST_EP   3                 /* CDC owns EP1..EP3       */
#define USBCORE_CDC_NUM_IF    2                 /* CDC owns IF0..IF1       */
#define USBCORE_ACC_SIZE      192       /* CDC IAD+CDC*2+HID ~= 100 B, with headroom */
#define USBCORE_DYN_EP_BUF    USB_EP_SIZE       /* per-dynamic-EP staging  */

/* ============================================================
 *  Accumulator buffer for USB_SendControl
 * ============================================================ */
static uint8_t   s_acc[USBCORE_ACC_SIZE];
static uint16_t  s_acc_pos = 0;

void usbcore_acc_reset(void)      { s_acc_pos = 0; }
const uint8_t *usbcore_acc_buf(void) { return s_acc; }
uint16_t       usbcore_acc_len(void) { return s_acc_pos; }

/* Stage `n` bytes from a PROGMEM source into s_acc at the current write
 * position.  Used by usb_standard.c (which is C, hence cannot call the
 * C++ USB_SendControl directly) to ship device/string descriptors that
 * now live in flash.  Truncates silently if s_acc would overflow.       */
void usbcore_acc_load_P(const uint8_t *src_pgm, uint16_t n) {
    uint16_t room = USBCORE_ACC_SIZE - s_acc_pos;
    if (room == 0) return;
    if (n > room) n = room;
    memcpy_P(&s_acc[s_acc_pos], src_pgm, n);
    s_acc_pos += n;
}

/* ============================================================
 *  USBAPI - control transfers
 * ============================================================ */
int USB_SendControl(uint8_t flags, const void* d, int len) {
    if (len <= 0) return 0;
    int room = USBCORE_ACC_SIZE - s_acc_pos;
    if (room <= 0) return -1;
    int n = (len > room) ? room : len;
    if (flags & TRANSFER_PGM) {
        memcpy_P(&s_acc[s_acc_pos], d, n);
    } else {
        memcpy(&s_acc[s_acc_pos], d, n);
    }
    s_acc_pos += n;
    return n;
}

/* ============================================================
 *  Dynamic config descriptor builder  -  emits into accumulator
 * ============================================================ */
/* Emit the 66-byte CDC block (IAD + IF0 + IF1 + 3 EPs) by slicing it out
 * of the existing g_config_descriptor (bytes 9..74). Sharing the single
 * source of truth means we cannot drift between the static and dynamic
 * representations of CDC. */
static void emit_cdc_interfaces(uint8_t *ifCount) {
    USB_SendControl(TRANSFER_PGM, &g_config_descriptor[9], 66);
    *ifCount += 2;
}

void usbcore_build_config_descriptor(void) {
    s_acc_pos = 0;

    /* Skeleton CONFIGURATION header (wTotalLength + bNumInterfaces patched later) */
    static const uint8_t cfg_hdr[9] PROGMEM = {
        9,            /* bLength                                */
        0x02,         /* bDescriptorType = CONFIGURATION        */
        0, 0,         /* wTotalLength    (patched below)        */
        0,            /* bNumInterfaces  (patched below)        */
        1,            /* bConfigurationValue                    */
        0,            /* iConfiguration                         */
        0xA0,         /* bmAttributes: bus-powered, rem.wakeup  */
        50            /* bMaxPower = 100 mA                     */
    };
    USB_SendControl(TRANSFER_PGM, cfg_hdr, sizeof(cfg_hdr));

    uint8_t ifCount = 0;
    emit_cdc_interfaces(&ifCount);
    usbcore_plugged_get_interfaces(&ifCount);   /* weak no-op unless a module is plugged */

    /* Patch wTotalLength and bNumInterfaces. */
    s_acc[2] = (uint8_t)(s_acc_pos & 0xFF);
    s_acc[3] = (uint8_t)((s_acc_pos >> 8) & 0xFF);
    s_acc[4] = ifCount;
}


/* ============================================================
 *  Weak defaults for the PluggableUSB bridge
 *
 *  The dynamic-endpoint machinery lives in USBCore_DU_dyn.cpp, which is
 *  extracted from core.a only when a PluggableUSB module (HID, MIDIUSB,
 *  ...) references it. For CDC-only sketches these no-op weak defaults
 *  are used instead, and the ~330 B of dynamic-EP RAM never exists.
 *  When USBCore_DU_dyn.cpp IS extracted, its strong definitions override
 *  every one of these.
 * ============================================================ */
extern "C" {

__attribute__((weak)) void usbcore_plugged_get_interfaces(uint8_t *ifCount) {
    (void)ifCount;                      /* no plugged modules: CDC only */
}

__attribute__((weak)) bool usbcore_try_plugged_setup(const usb_setup_t *s) {
    (void)s; return false;              /* -> usb_standard.c STALLs */
}

__attribute__((weak)) bool usbcore_try_plugged_get_descriptor(const usb_setup_t *s) {
    (void)s; return false;              /* -> usb_standard.c STALLs */
}

__attribute__((weak)) void usbcore_init_plugged_endpoints(void) { }

__attribute__((weak)) void usbcore_service_dynamic_ep_trncompl(void) { }

/* No plugged interfaces exist, so no host request can legitimately target
 * one; rejecting (0) makes usb_standard.c STALL, same as an unknown IF. */
__attribute__((weak)) uint16_t usbcore_ctrl_out_begin(const usb_setup_t *s) {
    (void)s; return 0;
}
__attribute__((weak)) uint8_t *usbcore_ctrl_out_buf(void)     { return 0; }
__attribute__((weak)) bool     usbcore_ctrl_out_pending(void) { return false; }
__attribute__((weak)) void     usbcore_ctrl_out_dispatch(void) { }

} /* extern "C" */

/* ============================================================
 *  USBDevice control object  (Arduino Leonardo / UNO R4 compatible)
 *  Declared in api/USBAPI.h; implemented here for the AVR-DU USB peripheral
 *  so libraries referencing the global USBDevice (HID-Project System,
 *  MIDIUSB, ...) link and run unchanged.
 * ============================================================ */

/* USB0 bus-signalling bits for device-initiated remote wakeup.
 * Datasheet DS40002548A: USB.CTRLB and USB.BUSSTATE. Fallbacks match the
 * documented bit positions if the I/O header names them differently. */
#ifndef USB_URESUME_bm
#define USB_URESUME_bm    (1 << 3)   /* CTRLB / BUSSTATE bit 3: Upstream Resume */
#endif
#ifndef USB_SUSPENDED_bm
#define USB_SUSPENDED_bm  (1 << 1)   /* BUSSTATE bit 1: Bus Suspended           */
#endif
#ifndef USB_WTRSM_bm
#define USB_WTRSM_bm      (1 << 4)   /* BUSSTATE bit 4: Wait Time Resume elapsed */
#endif

USBDevice_::USBDevice_() { }

bool USBDevice_::configured() { return usbIsConfigured(); }
void USBDevice_::attach()     { usbAttach(); }
void USBDevice_::detach()     { usbDetach(); }
void USBDevice_::poll()       { usbPoll(); }

bool USBDevice_::isSuspended() {
    return (USB0.BUSSTATE & USB_SUSPENDED_bm) != 0;
}

/* Device-initiated remote wakeup (upstream resume).
 * Returns false unless the host enabled it (SET_FEATURE DEVICE_REMOTE_WAKEUP,
 * tracked in g_remote_wakeup_enabled) AND the bus is suspended. Per the
 * datasheet, an upstream resume must not start until the bus has been
 * suspended >= 5 ms (T_WTRSM); BUSSTATE.WTRSM signals that. We wait (bounded)
 * for WTRSM, then write CTRLB.URESUME (hardware self-clears it). */
bool USBDevice_::wakeupHost() {
    if (!g_remote_wakeup_enabled)            return false;
    if (!(USB0.BUSSTATE & USB_SUSPENDED_bm)) return false;

    for (uint16_t guard = 0; !(USB0.BUSSTATE & USB_WTRSM_bm); ++guard) {
        if (guard == 0xFFFF) return false;   /* WTRSM never set: bail out */
    }

    USB0.CTRLB |= USB_URESUME_bm;   /* initiate upstream resume (self-clearing) */
    return true;
}

USBDevice_ USBDevice;

#endif /* USB0 */
