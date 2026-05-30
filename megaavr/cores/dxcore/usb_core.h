/*
 * usb_core.h - USB stack core: public API and shared declarations
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
 * usb_core.h
 * Public USB stack API
 *
 * Uses official USB_EP_TABLE_t and bit definitions from ioavr64du32.h.
 * Architecture: polled (no ISRs) 窶・call usbPoll() from loop().
 */
#ifndef USB_CORE_H
#define USB_CORE_H

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include "usb_descriptors.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * USB SETUP packet structure (USB 2.0 spec 9.3, Table 9-2)
 * ============================================================ */
typedef struct __attribute__((packed)) {
    uint8_t  bmRequestType;
    uint8_t  bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} usb_setup_t;

/* ============================================================
 * Control transfer state machine
 * ============================================================ */
typedef enum {
    CTRL_IDLE,
    CTRL_DATA_IN_STAGE,
    CTRL_DATA_OUT_STAGE,
    CTRL_STATUS_IN_STAGE,
    CTRL_STATUS_OUT_STAGE,
    CTRL_STATUS_PENDING_ADDR
} ctrl_state_t;

/* ============================================================
 * Public API
 * ============================================================ */
void usbInit(void);
void usbAttach(void);
void usbDetach(void);
bool usbIsConfigured(void);

/* Stage `n` bytes from a PROGMEM source into the config-descriptor accumulator.
 * Used by usb_standard.c to ship device/string descriptors that live in flash. */
void usbcore_acc_reset(void);
void usbcore_acc_load_P(const uint8_t *src_pgm, uint16_t n);
const uint8_t *usbcore_acc_buf(void);
uint16_t       usbcore_acc_len(void);

/* Call frequently from loop() 窶・drives all USB activity */
void usbPoll(void);

/* ============================================================
 * Internal globals shared between USB modules
 *
 * g_ep_table is USB_EP_TABLE_t (from ioavr64du32.h):
 *   FIFO[32]    : transaction-complete FIFO area
 *   EP[16]      : endpoint descriptors (16 x 16 bytes)
 *   FRAMENUM    : frame number (2 bytes)
 *
 * Access endpoints as: g_ep_table.EP[n].OUT.STATUS, .CTRL, .CNT, etc.
 * EPPTR must be set to &g_ep_table.EP[0] (NOT the struct base).
 * ============================================================ */
extern USB_EP_TABLE_t g_ep_table;

extern uint8_t  g_ep0_setup_buf[8];
extern uint8_t  g_ep0_data_buf[USB_EP0_SIZE];
extern uint8_t  g_ep1_in_buf[USB_EP1_SIZE];   /* CDC notify           */
extern uint8_t  g_ep2_out_buf[USB_EP2_SIZE];  /* CDC data RX          */
extern uint8_t  g_ep3_in_buf[USB_EP3_SIZE];   /* CDC data TX          */

extern ctrl_state_t g_ctrl_state;
extern uint8_t  g_pending_address;
extern uint8_t  g_current_configuration;
extern uint8_t  g_remote_wakeup_enabled;

/* Diagnostic counters */

/* diagnostic snapshot globals (set by usbPoll) */

/* ============================================================
 * Internal helpers (shared between USB modules)
 * ============================================================ */
void ep0_start_data_in(const uint8_t *data, uint16_t len, uint16_t host_requested);
void ep0_start_data_out(uint8_t *buffer, uint16_t len);
void ep0_send_zlp(void);
void ep0_stall(void);

/* RMWBUSY wait - call before any STATUS[n].xxxCLR/xxxSET write */
static inline void usb_rmw_wait(void) {
    while (USB0.INTFLAGSB & USB_RMWBUSY_bm) {}
}

#ifdef __cplusplus
}
#endif

#endif /* USB_CORE_H */
