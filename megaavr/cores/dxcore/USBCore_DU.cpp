/*
 * USBCore_DU.cpp - PluggableUSB bridge for the AVR-DU native USB stack
 *
 * Part of the AVR-DU native USB stack for DxCore.
 *
 * Copyright (c) 2026 Yusuke Shimizu (Workshop Asahi)
 *
 * Clean-room implementation written from public specifications only:
 *   - USB 2.0 Specification
 *   - USB CDC 1.2 and HID 1.11 class specifications
 *   - AVR64DU32 data sheet (Microchip)
 * No Microchip USB stack, ASF/START, TinyUSB, LUFA, or any other existing
 * USB implementation source was consulted or copied.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
 *  EP-type storage queried by PluggableUSB::plug()
 *
 *  The framework writes `node->endpointType[i]` as `unsigned int`
 *  to *(unsigned int *)epBuffer(lastEp), so we hand back a pointer
 *  to a uint16_t per slot. CDC slots (1..3) are pre-filled with
 *  EP_TYPE_INTERRUPT_IN / EP_TYPE_BULK_OUT / EP_TYPE_BULK_IN so a
 *  scan over the whole table can apply CDC and dynamic EPs uniformly
 *  if ever needed.
 * ============================================================ */
static unsigned int s_ep_types[USBCORE_NUM_EP] = {
    EP_TYPE_CONTROL,         /* EP0 control            */
    EP_TYPE_INTERRUPT_IN,    /* EP1 CDC notify         */
    EP_TYPE_BULK_OUT,        /* EP2 CDC data RX        */
    EP_TYPE_BULK_IN,         /* EP3 CDC data TX        */
    0, 0, 0, 0               /* EP4..EP7 dynamic       */
};

void *epBuffer(unsigned int n) {
    if (n >= USBCORE_NUM_EP) return NULL;
    return &s_ep_types[n];
}

/* ============================================================
 *  PluggableUSB_ constructor  (the API only declares it)
 *
 *  CDC occupies IF0..1 and EP1..3, so plugged modules start at
 *  lastIf = 2, lastEp = 4. totalEP is the per-class member that
 *  plug() compares against (lastEp + numEndpoints > totalEP rejects).
 * ============================================================ */
PluggableUSB_::PluggableUSB_() :
    lastIf(USBCORE_CDC_NUM_IF),
    lastEp(USBCORE_CDC_LAST_EP + 1),
    rootNode(NULL),
    totalEP(USBCORE_NUM_EP - 1)   /* highest EP number usable = 7 */
{
}

/* ============================================================
 *  USBSetup conversion (our packed usb_setup_t -> framework type)
 * ============================================================ */
static inline USBSetup as_usbsetup(const usb_setup_t *s) {
    USBSetup u;
    u.bmRequestType = s->bmRequestType;
    u.bRequest      = s->bRequest;
    u.wValueL       = (uint8_t)(s->wValue & 0xFF);
    u.wValueH       = (uint8_t)((s->wValue >> 8) & 0xFF);
    u.wIndex        = s->wIndex;
    u.wLength       = s->wLength;
    return u;
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

/* SET_REPORT / etc.: would receive payload during EP0 OUT data stage.
 * Not used by Keyboard/Mouse; leave as no-op stub returning 0 for now. */
int USB_RecvControl(void* /*d*/, int /*len*/)     { return 0; }
int USB_RecvControlLong(void* /*d*/, int /*len*/) { return 0; }

/* ============================================================
 *  Dynamic-EP staging buffers and USBAPI
 * ============================================================ */
/* Staging buffers only allocated for plugged EPs (EP_BASE..EP_BASE+COUNT-1).
 * Indexed by (ep - USBCORE_DYN_EP_BASE).  EP0..EP3 are owned by CDC and use
 * usb_core's g_ep0..g_ep3 buffers directly, so no staging is needed here. */
static uint8_t  s_dyn_ep_buf[USBCORE_DYN_EP_COUNT][USBCORE_DYN_EP_BUF];
static uint8_t  s_dyn_ep_pos[USBCORE_DYN_EP_COUNT];

uint8_t USB_Available(uint8_t /*ep*/)  { return 0; }   /* TBD when an OUT EP is plugged */
uint8_t USB_SendSpace(uint8_t ep)      {
    ep &= 0x07;
    if (ep < USBCORE_DYN_EP_BASE || ep >= USBCORE_NUM_EP) return 0;
    return USBCORE_DYN_EP_BUF - s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE];
}
int     USB_Recv(uint8_t /*ep*/, void* /*data*/, int /*len*/) { return 0; }
int     USB_Recv(uint8_t /*ep*/)                              { return -1; }
void    USB_Flush(uint8_t /*ep*/) { /* sent on RELEASE; nothing buffered after */ }

/* Issue an IN packet on a dynamic EP from its staging buffer.
 * Blocking - waits for HW to be idle, sets CNT/clears BUSNAK, then waits
 * for TRNCOMPL. Times out (returns) if the host never IN-polls; HID will
 * just retry next report. */
static int dyn_ep_release(uint8_t ep) {
    if (ep < 4 || ep >= USBCORE_NUM_EP) return -1;

    /* Wait for HW idle (BUSNAK = 1 means no packet armed). */
    uint16_t tmo = 0;
    while (!(g_ep_table.EP[ep].IN.STATUS & USB_BUSNAK_bm)) {
        if (++tmo == 0) return -1;
    }

    /* Arm the packet. */
    g_ep_table.EP[ep].IN.DATAPTR = (uint16_t)s_dyn_ep_buf[ep - USBCORE_DYN_EP_BASE];
    g_ep_table.EP[ep].IN.CNT     = s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE];
    /* Clear BUSNAK (release) and any previous TRNCOMPL together. */
    g_ep_table.EP[ep].IN.STATUS  = 0;

    /* Wait for completion. */
    tmo = 0;
    while (!(g_ep_table.EP[ep].IN.STATUS & USB_TRNCOMPL_bm)) {
        if (++tmo == 0) { s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE] = 0; return -1; }
    }
    /* Re-arm BUSNAK so we own the buffer; clear TRNCOMPL. */
    g_ep_table.EP[ep].IN.STATUS = USB_BUSNAK_bm | USB_TRNCOMPL_bm;

    int sent = s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE];
    s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE] = 0;
    return sent;
}

int USB_Send(uint8_t ep_with_flags, const void* data, int len) {
    uint8_t ep    = ep_with_flags & 0x07;
    uint8_t flags = ep_with_flags & 0xF8;
    if (ep < 4 || ep >= USBCORE_NUM_EP) return -1;
    if (len < 0) return -1;

    /* Append payload to the staging buffer. */
    int total = 0;
    const uint8_t *src = (const uint8_t *)data;
    while (len > 0) {
        int room = USBCORE_DYN_EP_BUF - s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE];
        if (room == 0) {
            /* Flush a full packet, then keep going. */
            int r = dyn_ep_release(ep);
            if (r < 0) return -1;
            room = USBCORE_DYN_EP_BUF;
        }
        int n = (len > room) ? room : len;
        if (flags & TRANSFER_PGM) {
            memcpy_P(&s_dyn_ep_buf[ep - USBCORE_DYN_EP_BASE][s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE]], src, n);
        } else {
            memcpy(&s_dyn_ep_buf[ep - USBCORE_DYN_EP_BASE][s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE]], src, n);
        }
        s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE] += n;
        src   += n;
        len   -= n;
        total += n;
    }

    if (flags & TRANSFER_RELEASE) {
        int r = dyn_ep_release(ep);
        if (r < 0) return -1;
    }
    return total;
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
    PluggableUSB().getInterface(&ifCount);

    /* Patch wTotalLength and bNumInterfaces. */
    s_acc[2] = (uint8_t)(s_acc_pos & 0xFF);
    s_acc[3] = (uint8_t)((s_acc_pos >> 8) & 0xFF);
    s_acc[4] = ifCount;
}

/* ============================================================
 *  Dispatch helpers (called from usb_standard.c)
 * ============================================================ */
bool usbcore_try_plugged_setup(const usb_setup_t *s) {
    USBSetup u = as_usbsetup(s);
    return PluggableUSB().setup(u);
}

bool usbcore_try_plugged_get_descriptor(const usb_setup_t *s) {
    USBSetup u = as_usbsetup(s);
    return PluggableUSB().getDescriptor(u) > 0;
}

/* ============================================================
 *  SET_CONFIGURATION : program dynamic EPs from epBuffer types
 *  (CDC EPs are programmed inline in handle_set_configuration.)
 * ============================================================ */
void usbcore_init_plugged_endpoints(void) {
    for (uint8_t ep = 4; ep < USBCORE_NUM_EP; ep++) {
        unsigned int t = s_ep_types[ep];
        if (t == 0) continue;       /* slot unused */

        uint8_t  dir_in  = (t & 0x80) ? 1 : 0;
        /* DU TYPE: control / iso / bulkint (bulk and interrupt are the same
         * register value; the descriptor type bits 1..0 distinguish them). */
        uint8_t  type    = (t & 0x03);
        uint8_t  ctrl_tp = (type == 0) ? USB_TYPE_CONTROL_gc
                         : (type == 1) ? USB_TYPE_ISO_gc
                         :               USB_TYPE_BULKINT_gc;

        /* Per-EP buffer size selector. We use 64-byte buffers for all
         * dynamic EPs; HID/Keyboard/Mouse only send a handful per report. */
        uint8_t  bufsz   = USB_BUFSIZE_DEFAULT_BUF64_gc;

        s_dyn_ep_pos[ep - USBCORE_DYN_EP_BASE] = 0;
        if (dir_in) {
            g_ep_table.EP[ep].IN.CTRL    = ctrl_tp | bufsz;
            g_ep_table.EP[ep].IN.DATAPTR = (uint16_t)s_dyn_ep_buf[ep - USBCORE_DYN_EP_BASE];
            g_ep_table.EP[ep].IN.CNT     = 0;
            g_ep_table.EP[ep].IN.STATUS  = USB_BUSNAK_bm;    /* idle, owned by SW */
        } else {
            g_ep_table.EP[ep].OUT.CTRL    = ctrl_tp | bufsz;
            g_ep_table.EP[ep].OUT.DATAPTR = (uint16_t)s_dyn_ep_buf[ep - USBCORE_DYN_EP_BASE];
            g_ep_table.EP[ep].OUT.CNT     = 0;
            g_ep_table.EP[ep].OUT.STATUS  = 0;               /* armed for RX */
        }
    }
}

/* TRNCOMPL bookkeeping for dynamic IN EPs.
 * Currently a no-op: USB_Send polls the same flag synchronously.
 * Kept as a hook for when we add asynchronous OUT (USB_Recv) support. */
void usbcore_service_dynamic_ep_trncompl(void) { }

#endif /* USB0 */
