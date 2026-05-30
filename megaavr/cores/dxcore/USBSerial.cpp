/* AVR DU native-USB stack. Compiled only on parts with the USB peripheral. */
#include <avr/io.h>   /* defines USB0 on parts that have USB; must precede the guard */
#if defined(USB0)
/**
 * USBSerial.cpp
 * Global instance of USBSerial_ plus the USB bring-up.
 *
 * The USB stack is INTERRUPT-DRIVEN: usbInit() enables USB0_BUSEVENT
 * and USB0_TRNCOMPL, whose ISRs (in usb_core.c) service bus reset, SETUP,
 * and all endpoint transactions. The sketch needs NO usbPoll() calls and
 * no yield() hook - enumeration and data transfer happen in the background,
 * even while the sketch sits in delay().
 *
 * On startup, main() calls usb_auto_init() (declared extern "C" below) so the
 * device enumerates at boot regardless of whether the sketch ever calls
 * Serial.begin() - matching Arduino Leonardo / Micro / UNO R4, where the
 * native USB CDC stays present (and the COM port stays visible) on every
 * sketch including Blink. usb_auto_init() is idempotent, so Serial.begin()
 * remains safe.
 */
#include "USBSerial.h"

extern "C" {
  #include "usb_core.h"
  #include "usb_cdc.h"
}

USBSerial_ USBSerial;

/* ============================================================
 *  usb_auto_init  -  bring up native USB CDC (idempotent)
 * ============================================================
 *  Called automatically from main() after init()/initVariant() but
 *  before sei() on USB-capable parts (USB0 defined). Also called by
 *  USBSerial_::begin() so explicit init from the sketch is harmless.
 *  usbInit() configures OSCHF SOF autotune + the VUSB regulator, sets
 *  up the endpoint table, and enables the USB peripheral; usbAttach()
 *  connects the D+ pull-up so the host starts enumeration.
 */
extern "C" void usb_auto_init(void) {
    static bool s_usb_started = false;
    if (s_usb_started) return;
    s_usb_started = true;
    usbInit();
    usbAttach();
}

void USBSerial_::begin(unsigned long baud) {
    (void)baud;
    usb_auto_init();
}

void USBSerial_::begin(unsigned long baud, uint8_t config) {
    (void)baud;
    (void)config;
    usb_auto_init();
}

void USBSerial_::end(void) {
    /* Leave the peripheral attached; a full detach (usbDetach) would drop
     * the COM port mid-run, which is rarely what a sketch wants. */
}

#endif /* USB0 */
