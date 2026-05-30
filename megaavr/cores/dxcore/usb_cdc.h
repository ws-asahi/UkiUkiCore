/*
 * usb_cdc.h - CDC-ACM implementation: public API
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
 * usb_cdc.h
 * CDC-ACM (Virtual COM Port) implementation
 *
 *   EP1 IN   notification (interrupt 16B) 窶・initialised but unused
 *   EP2 OUT  data RX from host            (bulk 64B)
 *   EP3 IN   data TX to   host            (bulk 64B)
 */
#ifndef USB_CDC_H
#define USB_CDC_H

#include <stdint.h>
#include <stdbool.h>
#include "usb_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Public API
 * ============================================================ */

/* True when host has SET_CONFIGURATION and asserted DTR */
bool usbCdcReady(void);
bool usbCdcTxIdle(void);   /* TX ring empty AND no packet in flight */

/* diagnostic: total raw bytes received on the CDC bulk-OUT endpoint */
extern volatile uint16_t g_cdc_rx_total;
extern volatile uint16_t g_cdc_tx_starts;
extern volatile uint16_t g_cdc_tx_pkts;
uint8_t usbCdcLineState(void);
bool    usbCdcTxInFlight(void);

/* Diagnostic: current host-requested line coding baud (dwDTERate). Useful
 * for confirming the 1200 bps touch path from a user sketch via Serial1. */
uint32_t usbCdcLineCodingBaud(void);

/* Reset-surviving breadcrumb (diagnostic). Call usbCdcDiagBreadcrumbInit()
 * once from setup(); then read the counters to see how far the last 1200bps
 * touch got even though the chip has since bounced back into the app:
 *   Saw1200   - SET_LINE_CODING(1200) reached the native CDC at least once
 *   TrigCount - trigger_1200bps_reset() was entered (the reset was requested)
 *   TrigBaud  - g_line_coding.dwDTERate captured at the last trigger
 *   TrigSite  - 1 = SET_CONTROL_LINE_STATE path, 2 = SET_LINE_CODING path */
void     usbCdcDiagBreadcrumbInit(void);
uint16_t usbCdcDiagSaw1200(void);
uint16_t usbCdcDiagTrigCount(void);
uint32_t usbCdcDiagTrigBaud(void);
uint8_t  usbCdcDiagTrigSite(void);

/* True when EP3 IN is ready to accept a fresh buffer */
bool usbCdcTxReady(void);

/* Returns number of bytes available to read from RX buffer */
uint16_t usbCdcAvailable(void);

/* Read one byte from RX buffer, returns -1 if empty */
int usbCdcRead(void);

/* Read up to maxlen bytes into dst, returns bytes actually read */
uint16_t usbCdcReadBytes(uint8_t *dst, uint16_t maxlen);

/* Write a single byte (queues; returns false on overflow) */
bool usbCdcWriteByte(uint8_t b);

/* Write a buffer; returns number actually queued */
uint16_t usbCdcWrite(const uint8_t *src, uint16_t len);

/* Convenience helpers */
uint16_t usbCdcPrint(const char *s);
uint16_t usbCdcPrintln(const char *s);

/* ============================================================
 * Internal hooks called by USB stack
 * ============================================================ */
void usb_cdc_handle_class_request(const usb_setup_t *s);
void usb_cdc_data_out_complete(void);     /* SET_LINE_CODING data done */
void usb_cdc_on_configured(void);          /* SET_CONFIGURATION(1)     */
void usb_cdc_on_reset(void);               /* Bus reset                */
void usb_cdc_on_ep2_out(uint16_t cnt);     /* EP2 OUT TRNCOMPL         */
void usb_cdc_on_ep3_in_done(void);         /* EP3 IN TRNCOMPL          */

/* Call from main loop to drive the TX pump */
void usbCdcPoll(void);

#ifdef __cplusplus
}
#endif

#endif /* USB_CDC_H */
