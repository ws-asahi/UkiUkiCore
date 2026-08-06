/* wazamono_tachi_init.cpp - board-specific init + activity LED for Wazamono Tachi
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * (C) Workshop Asahi 2026.  DxCore is (C) Spence Konde, LGPL 2.1 (see LICENSE.md).
 *
 * Rev.3 (AVR64DU28) has a SINGLE on-board user LED:
 *
 *   - PA0 (= D17 = LED_BUILTIN = LED_BUILTIN_RX), active-LOW.
 *     It plays both Pro Micro roles at once: the sketch-controllable LED and
 *     the USB-CDC RX activity LED (lit when CDC data is RECEIVED, host ->
 *     device). Each RX event lights the LED and (re)arms a 100-tick one-shot;
 *     the USB Start-of-Frame (~1 ms) decrements the one-shot and turns the
 *     LED off when it expires. Continuous traffic keeps re-arming it (LED
 *     stays lit); an idle link turns it off ~100 ms after the last byte.
 *     A sketch that drives D17 as GPIO simply fights the short activity
 *     pulses; digitalWrite works normally between them (same arrangement as
 *     the Kunai's D11/D12).
 *
 *   - There is NO TX LED on this revision. The usb_cdc_on_tx_activity hook
 *     stays at its weak no-op default, and pins_arduino.h intentionally does
 *     not define LED_BUILTIN_TX.
 *
 * This mirrors the 32U4 core's RX LED behaviour (USBCore.cpp: RXLED1 +
 * RxLEDPulse = TX_RX_LED_PULSE_MS(100), decremented in the SOF ISR),
 * reimplemented for WazamonoCore's USB stack. The clean-room USB stack only
 * calls board-agnostic weak hooks (usb_cdc_on_rx_activity /
 * usb_cdc_on_tx_activity / usb_cdc_on_led_tick); all LED/pin knowledge lives
 * here.
 */

#include <Arduino.h>

#if defined(WAZAMONO_TACHI_PINOUT)

/* ---- Force-link marker ----------------------------------------------------
 * Arduino archives variant-folder objects into core.a, and the core's main.cpp
 * supplies a *weak* initVariant(). An archived strong symbol does NOT override
 * a weak one already provided by a linked object, so without help this entire
 * translation unit (initVariant + the activity-LED hooks below) is silently
 * dropped at link time. boards.txt passes
 *     -Wl,-u,wazamono_tachi_variant_keep
 * which forces the linker to pull this member; once pulled, the strong defs
 * here override the weak defaults. (Confirmed necessary, including under -flto.) */
extern "C" { __attribute__((used)) char wazamono_tachi_variant_keep = 0; }

/* On-board LED (single): PA0 = D17 = LED_BUILTIN = LED_BUILTIN_RX, active-LOW.
 *   ON  = drive pin LOW  (PORTA.OUTCLR);  OFF = drive pin HIGH (PORTA.OUTSET). */
#define TACHI_LED_bm      PIN0_bm
#define TACHI_LED_PULSE   100        /* one-shot length in SOF ticks (~ms), = 32U4 TX_RX_LED_PULSE_MS */

static volatile uint8_t s_rx_pulse = 0;   /* ticks remaining for the RX one-shot */

/* initVariant(): runs once after init(), before the USB stack starts. Park the
 * LED pin OFF (driven HIGH, since active-LOW) and as an output. Setting OUTSET
 * before DIRSET avoids a momentary lit flash when the pin becomes an output. */
void initVariant(void) {
    PORTA.OUTSET = TACHI_LED_bm;   /* OFF first */
    PORTA.DIRSET = TACHI_LED_bm;   /* then output */
}

extern "C" {

/* CDC OUT data received (host -> device): light the LED, (re)arm its one-shot. */
void usb_cdc_on_rx_activity(void) {
    PORTA.OUTCLR = TACHI_LED_bm;          /* ON (active-LOW) */
    s_rx_pulse   = TACHI_LED_PULSE;
}

/* usb_cdc_on_tx_activity: intentionally NOT overridden (no TX LED on rev.3). */

/* USB SOF (~1 ms): tick the one-shot; when it expires, turn the LED off. */
void usb_cdc_on_led_tick(void) {
    if (s_rx_pulse && !--s_rx_pulse) PORTA.OUTSET = TACHI_LED_bm;  /* OFF */
}

} /* extern "C" */

#endif /* WAZAMONO_TACHI_PINOUT */
