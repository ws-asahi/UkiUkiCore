/*
 * USBSerial.h - USB CDC Serial class declaration
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
 * USBSerial.h  -  Arduino Stream wrapper for the AVR DU native-USB CDC stack.
 *
 * This is a DxCore *core* header. On parts that have the USB peripheral
 * (AVR DU family -> USB0 defined) it provides the `USBSerial` instance, which
 * the DU variant aliases to `Serial` (Arduino Leonardo convention: Serial =
 * native USB CDC, Serial1 = the first hardware UART).
 *
 * The stack is fully interrupt-driven (see usb_core.c): enumeration and data
 * transfer happen in the USB ISRs, so a sketch needs no usbPoll() calls:
 *
 *     Serial.begin(115200);
 *     if (Serial) {                 // host has opened the COM port (DTR)
 *       Serial.println("Hello, USB!");
 *     }
 *
 * Included near the end of HardwareSerial.h (guarded by USB0) so the
 * Serial/USBSerial symbol is declared wherever Serial is used. It depends only
 * on the ArduinoCore-API Stream/Print classes, which HardwareSerial.h has
 * already pulled in by that point - it must NOT include <Arduino.h> (that
 * would be a circular include during core compilation).
 */
#pragma once

#include <avr/io.h>   /* ensure the USB0 macro is defined before we test for it */
#if defined(USB0)

#include <stdint.h>
#include <stddef.h>
#include "api/Stream.h"

extern "C" {
  #include "usb_core.h"   /* usbInit / usbAttach / usbPoll / usbIsConfigured */
  #include "usb_cdc.h"    /* usbCdc* ring-buffer API                          */
}

class USBSerial_ : public Stream {
public:
    USBSerial_(void) {}

    /** Bring up the native USB CDC (usbInit + usbAttach). The baud argument is
     *  accepted for Arduino compatibility but ignored - USB CDC always runs at
     *  full speed. Safe to call repeatedly; heavy init runs once (guarded in
     *  the .cpp). */
    void begin(unsigned long baud = 115200);
    void begin(unsigned long baud, uint8_t config);
    void end(void);

    /** True once the host has opened the virtual COM port (DTR asserted). */
    operator bool(void) const { return usbCdcReady(); }

    /* ---- Stream / Print API ---- */
    int available(void) override { return (int)usbCdcAvailable(); }
    int read(void) override      { return usbCdcRead(); }
    int peek(void) override      { return -1; }   /* ring buffer has no peek */

    void flush(void) override {
        /* TX is drained by the USB ISR in the background; wait until the ring
         * is empty and the last packet has gone out (bounded spin). */
        for (uint32_t guard = 0; guard < 2000000UL; guard++) {
            if (!usbCdcReady()) return;   /* port closed: nothing to wait for */
            if (usbCdcTxIdle())  return;  /* all data sent                    */
        }
    }
    size_t write(uint8_t b) override {
        return usbCdcWriteByte(b) ? 1 : 0;            /* enqueue; ISR sends it */
    }
    size_t write(const uint8_t *buffer, size_t size) override {
        return usbCdcWrite(buffer, (uint16_t)size);   /* enqueue; ISR sends    */
    }

    using Print::write;   /* pull in write(const char*), write(str,len), etc. */
};

extern USBSerial_ USBSerial;

/* Bring up the native USB CDC at boot (Arduino Leonardo-style USBDevice.attach()).
 * Defined in USBSerial.cpp; idempotent. Called from main() so even sketches that
 * never touch Serial (e.g. Blink) still enumerate as the COM port. */
extern "C" void usb_auto_init(void);

#endif /* USB0 */
