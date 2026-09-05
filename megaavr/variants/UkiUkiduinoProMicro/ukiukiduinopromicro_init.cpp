/* ukiukiduinopromicro_init.cpp - board-specific init for the UkiUkiduino ProMicro
 * ---------------------------------------------------------------------------
 * Part of UkiUkiCore (a product-specific fork of WazamonoCore / SpenceKonde's
 * DxCore). (C) Workshop Asahi 2026. DxCore is (C) Spence Konde, LGPL 2.1
 * (see LICENSE.md).
 *
 * Purpose
 *   1. Prepare PF4 as the data pin for the on-board XL-5050RGBC-WS2812B
 *      full-color LED and make sure the LED starts dark (a WS2812's power-on
 *      pixel state is undefined, and the bootloader may have left its
 *      breathing color lit).
 *      The LED follows digitalWrite()/digitalWriteFast() calls made to
 *      LED_BUILTIN = D17 (PF5, an otherwise unused pin used as the LED's
 *      state pin): the core's mirror hook reads the resulting PF5 OUT bit and
 *      sends the matching WS2812 frame on PF4 (see LED_BUILTIN_MIRROR in
 *      pins_arduino.h and ukiukiduinopromicro_led.cpp). This keeps the classic
 *      "LED_BUILTIN = LED" experience - the stock Blink sketch blinks the LED
 *      (yellow by default) unmodified - while digitalRead(LED_BUILTIN) still
 *      returns the LED state (the data line itself must idle LOW for the
 *      WS2812 latch, so it cannot carry the state).
 *   2. Reproduce the Arduino Pro Micro / Leonardo RX/TX activity LEDs, both
 *      active-LOW (anode to +5V via 1 kOhm), on PA0/PA1 (the XTALHF pins -
 *      the board is crystal-less, so they are plain GPIO):
 *        - TX LED on PA0 (= D30 = LED_BUILTIN_TX): lit when CDC data is SENT
 *        - RX LED on PA1 (= D31 = LED_BUILTIN_RX): lit when CDC data is RECEIVED
 *      Each data event lights its LED and (re)arms a 100-tick one-shot; the
 *      USB Start-of-Frame (~1 ms) decrements it and turns the LED off when it
 *      expires - the 32U4 core behaviour (USBCore.cpp: TX_RX_LED_PULSE_MS),
 *      reimplemented on the clean-room USB stack's three board-agnostic weak
 *      hooks (usb_cdc_on_rx_activity / usb_cdc_on_tx_activity /
 *      usb_cdc_on_led_tick). Taken over unchanged from the Wazamono Tachi.
 *
 * Resource use
 *   None. No EVSYS channel and no CCL LUT is consumed.
 *
 * Note: BTN_BUILTIN (D22 = PF0) needs no init here - it has an external
 *   5.1 kOhm pull-down on the board (pressed = HIGH) and is read as a plain
 *   input.
 */

#include <Arduino.h>

#if defined(UKIUKIDUINO_PROMICRO_PINOUT)

/* ---- Force-link marker ----------------------------------------------------
 * Arduino archives variant-folder objects into core.a, and the core's main.cpp
 * supplies a *weak* initVariant(). An archived strong symbol does NOT override
 * a weak one already provided by a linked object, so without help this entire
 * translation unit is silently dropped at link time. boards.txt passes
 *     -Wl,-u,ukiukiduinopromicro_variant_keep
 * which forces the linker to pull this member. */
extern "C" { __attribute__((used)) char ukiukiduinopromicro_variant_keep = 0; }

/* On-board activity LEDs (Pro Micro convention), both active-LOW, both in PORTA:
 *   TX LED = PA0 (= D30 / LED_BUILTIN_TX)
 *   RX LED = PA1 (= D31 / LED_BUILTIN_RX)
 *   ON  = drive pin LOW  (OUTCLR);  OFF = drive pin HIGH (OUTSET). */
#define UUPM_TXLED_bm    PIN0_bm    /* in PORTA */
#define UUPM_RXLED_bm    PIN1_bm    /* in PORTA */
#define UUPM_LED_PULSE   100        /* one-shot length in SOF ticks (~ms), = 32U4 TX_RX_LED_PULSE_MS */

static volatile uint8_t s_rx_pulse = 0;   /* ticks remaining for RX LED one-shot */
static volatile uint8_t s_tx_pulse = 0;   /* ticks remaining for TX LED one-shot */

/* initVariant() is a weak no-op in the core (cores/dxcore/main.cpp); this strong
 * definition overrides it and runs once, immediately after init(), before setup(). */
void initVariant(void) {
  /* TX/RX LEDs: park OFF (driven HIGH, active-LOW) BEFORE making them outputs
   * so they cannot flash on. */
  PORTA.OUTSET = UUPM_TXLED_bm | UUPM_RXLED_bm;
  PORTA.DIRSET = UUPM_TXLED_bm | UUPM_RXLED_bm;

  /* WS2812B data pin PF4: output, idle LOW. The pin stays an output
   * permanently; pinMode(LED_BUILTIN, ...) only affects PF5, so the LED simply
   * holds its last mirrored frame if D17 is made an input. */
  VPORTF.OUT &= ~(1 << 4);
  VPORTF.DIR |= (1 << 4);
  /* Blank the LED: PF5 OUT is 0 after reset, so the mirror hook sends the
   * (0,0,0) frame. This clears both the WS2812's undefined power-on state
   * and anything the bootloader's breathing animation left behind. */
  __led_builtin_mirror_hook();
}

extern "C" {

/* CDC OUT data received (host -> device): light RX LED, (re)arm its one-shot. */
void usb_cdc_on_rx_activity(void) {
  PORTA.OUTCLR = UUPM_RXLED_bm;        /* ON (active-LOW) */
  s_rx_pulse   = UUPM_LED_PULSE;
}

/* CDC IN data sent (device -> host): light TX LED, (re)arm its one-shot. */
void usb_cdc_on_tx_activity(void) {
  PORTA.OUTCLR = UUPM_TXLED_bm;        /* ON (active-LOW) */
  s_tx_pulse   = UUPM_LED_PULSE;
}

/* USB SOF (~1 ms): tick each one-shot; when it expires, turn that LED off. */
void usb_cdc_on_led_tick(void) {
  if (s_rx_pulse && !--s_rx_pulse) PORTA.OUTSET = UUPM_RXLED_bm;  /* OFF */
  if (s_tx_pulse && !--s_tx_pulse) PORTA.OUTSET = UUPM_TXLED_bm;  /* OFF */
}

} /* extern "C" */

#endif /* UKIUKIDUINO_PROMICRO_PINOUT */
