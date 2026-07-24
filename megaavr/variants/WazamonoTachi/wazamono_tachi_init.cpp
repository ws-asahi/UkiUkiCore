/* wazamono_tachi_init.cpp - board-specific init + activity LEDs for Wazamono Tachi
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * (C) Workshop Asahi 2026.  DxCore is (C) Spence Konde, LGPL 2.1 (see LICENSE.md).
 *
 * Reproduces the Arduino Pro Micro / Leonardo (ATmega32U4) RX/TX activity LEDs:
 *
 *   - RX LED on PF5 (active-LOW, = D17/LED_BUILTIN_RX): lit when CDC data is
 *     RECEIVED (host -> device).
 *   - TX LED on PF4 (active-LOW, = D30/LED_BUILTIN_TX): lit when CDC data is
 *     SENT (device -> host).
 *   - Each data event lights the LED and (re)arms a 100-tick one-shot; the USB
 *     Start-of-Frame (~1 ms) decrements the one-shot and turns the LED off when
 *     it expires. Continuous traffic keeps re-arming it (LED stays lit); an idle
 *     link turns the LED off ~100 ms after the last byte.
 *
 *   LED_BUILTIN itself is a separate, dedicated LED on PC3 (D13, active-HIGH)
 *   and is NOT touched here: like the Uno/Leonardo D13 LED it is left as an
 *   input until the sketch drives it.
 *
 * This mirrors the 32U4 core's behaviour (USBCore.cpp: RXLED1/TXLED1 +
 * RxLEDPulse/TxLEDPulse = TX_RX_LED_PULSE_MS(100), decremented in the SOF ISR),
 * reimplemented for WazamonoCore's interrupt-driven USB stack.
 *
 * The clean-room USB stack only calls three board-agnostic hooks
 * (usb_cdc_on_rx_activity / usb_cdc_on_tx_activity / usb_cdc_on_led_tick, weak
 * no-ops by default). All LED/pin knowledge lives here, so Tsurugi - which has
 * no RX/TX LEDs - keeps the no-ops and is untouched.
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

/* On-board activity LEDs (Pro Micro convention), both active-LOW, both on PORTF:
 *   RX LED = PF4 (= D17 / LED_BUILTIN_RX),  TX LED = PF5 (= D30 / LED_BUILTIN_TX).
 *   ON  = drive pin LOW  (PORTF.OUTCLR);  OFF = drive pin HIGH (PORTF.OUTSET).   */
#define TACHI_RXLED_bm    PIN4_bm
#define TACHI_TXLED_bm    PIN5_bm
#define TACHI_LED_PULSE   100        /* one-shot length in SOF ticks (~ms), = 32U4 TX_RX_LED_PULSE_MS */

static volatile uint8_t s_rx_pulse = 0;   /* ticks remaining for RX LED one-shot */
static volatile uint8_t s_tx_pulse = 0;   /* ticks remaining for TX LED one-shot */

/* initVariant(): runs once after init(), before the USB stack starts. Park both
 * LED pins OFF (driven HIGH, since active-LOW) and as outputs. Setting OUTSET
 * before DIRSET avoids a momentary lit flash when the pins become outputs. */
void initVariant(void) {
    PORTF.OUTSET = TACHI_RXLED_bm | TACHI_TXLED_bm;   /* OFF first */
    PORTF.DIRSET = TACHI_RXLED_bm | TACHI_TXLED_bm;   /* then output */
}

extern "C" {

/* CDC OUT data received (host -> device): light RX LED, (re)arm its one-shot. */
void usb_cdc_on_rx_activity(void) {
    PORTF.OUTCLR = TACHI_RXLED_bm;        /* ON (active-LOW) */
    s_rx_pulse   = TACHI_LED_PULSE;
}

/* CDC IN data sent (device -> host): light TX LED, (re)arm its one-shot. */
void usb_cdc_on_tx_activity(void) {
    PORTF.OUTCLR = TACHI_TXLED_bm;        /* ON (active-LOW) */
    s_tx_pulse   = TACHI_LED_PULSE;
}

/* USB SOF (~1 ms): tick each one-shot; when it expires, turn that LED off. */
void usb_cdc_on_led_tick(void) {
    if (s_rx_pulse && !--s_rx_pulse) PORTF.OUTSET = TACHI_RXLED_bm;  /* OFF */
    if (s_tx_pulse && !--s_tx_pulse) PORTF.OUTSET = TACHI_TXLED_bm;  /* OFF */
}

} /* extern "C" */

#endif /* WAZAMONO_TACHI_PINOUT */
