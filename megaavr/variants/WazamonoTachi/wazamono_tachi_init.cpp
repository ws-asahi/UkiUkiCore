/* wazamono_tachi_init.cpp - board-specific init + activity LEDs for Wazamono Tachi
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * (C) Workshop Asahi 2026.  DxCore is (C) Spence Konde, LGPL 2.1 (see LICENSE.md).
 *
 * Rev.4 (AVR64DU32) reproduces the Arduino Pro Micro / Leonardo (ATmega32U4)
 * RX/TX activity LEDs, both active-LOW, on two different ports:
 *
 *   - RX LED on PF3 (= D17 = LED_BUILTIN = LED_BUILTIN_RX): lit when CDC data
 *     is RECEIVED (host -> device).
 *   - TX LED on PC3 (= D30 = LED_BUILTIN_TX): lit when CDC data is SENT
 *     (device -> host).
 *   - Each data event lights its LED and (re)arms a 100-tick one-shot; the USB
 *     Start-of-Frame (~1 ms) decrements the one-shot and turns the LED off when
 *     it expires. Continuous traffic keeps re-arming it (LED stays lit); an
 *     idle link turns the LED off ~100 ms after the last byte.
 *
 *   There is no separate dedicated user LED: like the real Pro Micro, the RX
 *   LED doubles as LED_BUILTIN. A sketch that drives D17/D30 as GPIO simply
 *   fights the short activity pulses; digitalWrite works normally between them.
 *
 * This mirrors the 32U4 core's behaviour (USBCore.cpp: RXLED1/TXLED1 +
 * RxLEDPulse/TxLEDPulse = TX_RX_LED_PULSE_MS(100), decremented in the SOF ISR),
 * reimplemented for WazamonoCore's USB stack. The clean-room USB stack only
 * calls three board-agnostic weak hooks (usb_cdc_on_rx_activity /
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

/* On-board activity LEDs (Pro Micro convention), both active-LOW:
 *   RX LED = PF3 (= D17 / LED_BUILTIN / LED_BUILTIN_RX)
 *   TX LED = PC3 (= D30 / LED_BUILTIN_TX)
 *   ON  = drive pin LOW  (OUTCLR);  OFF = drive pin HIGH (OUTSET). */
#define TACHI_RXLED_bm    PIN3_bm    /* in PORTF */
#define TACHI_TXLED_bm    PIN3_bm    /* in PORTC */
#define TACHI_LED_PULSE   100        /* one-shot length in SOF ticks (~ms), = 32U4 TX_RX_LED_PULSE_MS */

static volatile uint8_t s_rx_pulse = 0;   /* ticks remaining for RX LED one-shot */
static volatile uint8_t s_tx_pulse = 0;   /* ticks remaining for TX LED one-shot */

/* initVariant(): runs once after init(), before the USB stack starts. Park both
 * LED pins OFF (driven HIGH, since active-LOW) and as outputs. Setting OUTSET
 * before DIRSET avoids a momentary lit flash when the pins become outputs. */
void initVariant(void) {
    PORTF.OUTSET = TACHI_RXLED_bm;   /* OFF first */
    PORTF.DIRSET = TACHI_RXLED_bm;   /* then output */
    PORTC.OUTSET = TACHI_TXLED_bm;
    PORTC.DIRSET = TACHI_TXLED_bm;
}

extern "C" {

/* CDC OUT data received (host -> device): light RX LED, (re)arm its one-shot. */
void usb_cdc_on_rx_activity(void) {
    PORTF.OUTCLR = TACHI_RXLED_bm;        /* ON (active-LOW) */
    s_rx_pulse   = TACHI_LED_PULSE;
}

/* CDC IN data sent (device -> host): light TX LED, (re)arm its one-shot. */
void usb_cdc_on_tx_activity(void) {
    PORTC.OUTCLR = TACHI_TXLED_bm;        /* ON (active-LOW) */
    s_tx_pulse   = TACHI_LED_PULSE;
}

/* USB SOF (~1 ms): tick each one-shot; when it expires, turn that LED off. */
void usb_cdc_on_led_tick(void) {
    if (s_rx_pulse && !--s_rx_pulse) PORTF.OUTSET = TACHI_RXLED_bm;  /* OFF */
    if (s_tx_pulse && !--s_tx_pulse) PORTC.OUTSET = TACHI_TXLED_bm;  /* OFF */
}

} /* extern "C" */

#endif /* WAZAMONO_TACHI_PINOUT */
