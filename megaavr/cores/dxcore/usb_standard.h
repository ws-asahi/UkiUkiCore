/*
 * usb_standard.h - USB standard / class request dispatcher declarations
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
 * usb_standard.h
 * USB Standard / Class request dispatcher declarations
 */
#ifndef USB_STANDARD_H
#define USB_STANDARD_H

#include "usb_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dispatcher entry points - called from SETUP handler */
void usb_handle_standard_request(const usb_setup_t *s);
void usb_handle_class_request(const usb_setup_t *s);

/* Called when EP0 DATA-OUT stage completes */
void usb_class_data_out_complete(void);

#ifdef __cplusplus
}
#endif

#endif /* USB_STANDARD_H */
