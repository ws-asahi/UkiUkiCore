/*
 * USBCore_DU.h - PluggableUSB bridge: C-callable interface
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
 * USBCore_DU.h - C-callable bridge between the AVR DU low-level USB stack
 *                (usb_core.c / usb_standard.c / usb_descriptors.c, all C)
 *                and the C++ PluggableUSB framework (api/PluggableUSB.cpp).
 *
 * Implementation in USBCore_DU.cpp.
 */
#pragma once
#if defined(USB0)

#include <stdint.h>
#include <stdbool.h>
#include "usb_core.h"   /* for usb_setup_t */

#ifdef __cplusplus
extern "C" {
#endif

/* --- Accumulator buffer (control-IN descriptor assembly) ---------------- */
/* The accumulator backs USB_SendControl: each call appends bytes to a
 * static buffer; the caller (handle_get_descriptor in usb_standard.c)
 * then sends the buffer with the existing ep0_start_data_in().            */
void           usbcore_acc_reset(void);
const uint8_t *usbcore_acc_buf(void);
uint16_t       usbcore_acc_len(void);

/* --- Dynamic CONFIGURATION descriptor builder --------------------------- *
 * Resets the accumulator, emits the CDC interfaces (always present), then
 * iterates the PluggableUSB modules' getInterface() to append the rest.
 * Fixes up wTotalLength and bNumInterfaces in the accumulator header.     */
void usbcore_build_config_descriptor(void);

/* --- PluggableUSB dispatch hooks (called from usb_standard.c) ----------- *
 * `try_setup`        - non-CDC class / vendor requests
 * `try_get_descriptor` - non-standard descriptor types (HID report, etc.) *
 * Both return true if a registered PluggableUSB module handled it.        *
 * On true return, send accumulator buffer (which the module filled via    *
 * USB_SendControl) as the control-IN response; on false, STALL.           */
bool usbcore_try_plugged_setup(const usb_setup_t *s);
bool usbcore_try_plugged_get_descriptor(const usb_setup_t *s);

/* --- Endpoint configuration ---------------------------------------------- *
 * Iterate the epBuffer[] entries (filled by PluggableUSB.plug at static    *
 * init) and program the DU endpoint table accordingly. Call from           *
 * handle_set_configuration after CDC EPs have been configured.            */
void usbcore_init_plugged_endpoints(void);

/* --- TRNCOMPL servicing for dynamic EPs (called from USB ISR) ----------- *
 * For now this just clears flags; data movement for those EPs is handled  *
 * by USB_Send/Recv via polling (blocking writes).                         */
void usbcore_service_dynamic_ep_trncompl(void);

#ifdef __cplusplus
}
#endif

#endif /* USB0 */
