/* wazamono_tsurugi_init.cpp - board-specific init + activity LEDs for Wazamono Tsurugi
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * (C) Workshop Asahi 2026.  DxCore is (C) Spence Konde, LGPL 2.1 (see LICENSE.md).
 *
 * Board rev.C drives two USB-CDC activity LEDs (Pro Micro / Leonardo
 * convention), both active-LOW on the 3.3 V rail, on the pads freed by the
 * removed 24 MHz crystal:
 *
 *   - TX LED on PA0 (= PIN_LED_TX): lit while CDC data is SENT (device -> host).
 *   - RX LED on PA1 (= PIN_LED_RX): lit while CDC data is RECEIVED (host -> device).
 *   - Each data event lights its LED and (re)arms a 100-tick one-shot; the USB
 *     Start-of-Frame (~1 ms) decrements the one-shot and turns the LED off when
 *     it expires. Continuous traffic keeps re-arming it (LED stays lit); an
 *     idle link turns the LED off ~100 ms after the last byte.
 *
 * This mirrors the 32U4 core's behaviour (TX_RX_LED_PULSE_MS = 100,
 * decremented in the SOF handler), reimplemented for WazamonoCore's clean-room
 * USB stack, which only calls three board-agnostic weak hooks
 * (usb_cdc_on_rx_activity / usb_cdc_on_tx_activity / usb_cdc_on_led_tick);
 * all LED/pin knowledge lives here. Same structure as the Tachi variant.
 *
 * History
 *   Until board rev. B this file configured a CCL/EVSYS hardware mirror that
 *   reproduced D13 (PD6) on PC3 to drive the on-board LED; measurement showed
 *   PC3's driver is VDD-powered, the LED moved to a buffered D13, and PC3
 *   became a GPIO (now D4, the LUT1 PWM outlet). Rev. C then removed the
 *   crystal, freeing PA0/PA1 for these activity LEDs.
 */

#include <Arduino.h>

#if defined(WAZAMONO_AVR_TSURUGI)

/* ---- Force-link marker ----------------------------------------------------
 * Arduino archives variant-folder objects into core.a, and the core's main.cpp
 * supplies a *weak* initVariant(). An archived strong symbol does NOT override
 * a weak one already provided by a linked object, so without help this entire
 * translation unit (initVariant + the activity-LED hooks below) is silently
 * dropped at link time. boards.txt passes
 *     -Wl,-u,wazamono_tsurugi_variant_keep
 * which forces the linker to pull this member; once pulled, the strong defs
 * here override the weak defaults. (Confirmed necessary, including under -flto.) */
extern "C" { __attribute__((used)) char wazamono_tsurugi_variant_keep = 0; }

/* On-board activity LEDs, both active-LOW in PORTA:
 *   TX LED = PA0 (= PIN_LED_TX);  RX LED = PA1 (= PIN_LED_RX)
 *   ON  = drive pin LOW  (OUTCLR);  OFF = drive pin HIGH (OUTSET). */
#define TSURUGI_TXLED_bm  PIN0_bm    /* in PORTA */
#define TSURUGI_RXLED_bm  PIN1_bm    /* in PORTA */
#define TSURUGI_LED_PULSE 100        /* one-shot length in SOF ticks (~ms) */

static volatile uint8_t s_tx_pulse = 0;   /* ticks remaining for TX LED one-shot */
static volatile uint8_t s_rx_pulse = 0;   /* ticks remaining for RX LED one-shot */

/* initVariant(): runs once after init(), before the USB stack starts. Park both
 * LED pins OFF (driven HIGH, since active-LOW) and as outputs. Setting OUTSET
 * before DIRSET avoids a momentary lit flash when the pins become outputs. */
void initVariant(void) {
    PORTA.OUTSET = TSURUGI_TXLED_bm | TSURUGI_RXLED_bm;   /* OFF first */
    PORTA.DIRSET = TSURUGI_TXLED_bm | TSURUGI_RXLED_bm;   /* then outputs */
}

extern "C" {

/* CDC OUT data received (host -> device): light RX LED, (re)arm its one-shot. */
void usb_cdc_on_rx_activity(void) {
    PORTA.OUTCLR = TSURUGI_RXLED_bm;      /* ON (active-LOW) */
    s_rx_pulse   = TSURUGI_LED_PULSE;
}

/* CDC IN data sent (device -> host): light TX LED, (re)arm its one-shot. */
void usb_cdc_on_tx_activity(void) {
    PORTA.OUTCLR = TSURUGI_TXLED_bm;      /* ON (active-LOW) */
    s_tx_pulse   = TSURUGI_LED_PULSE;
}

/* USB SOF (~1 ms): tick each one-shot; when it expires, turn that LED off. */
void usb_cdc_on_led_tick(void) {
    if (s_tx_pulse && !--s_tx_pulse) PORTA.OUTSET = TSURUGI_TXLED_bm;  /* OFF */
    if (s_rx_pulse && !--s_rx_pulse) PORTA.OUTSET = TSURUGI_RXLED_bm;  /* OFF */
}

} /* extern "C" */

#endif /* WAZAMONO_AVR_TSURUGI */
