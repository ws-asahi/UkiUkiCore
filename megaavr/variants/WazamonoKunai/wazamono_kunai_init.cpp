/* wazamono_kunai_init.cpp - board-level initialization for the Wazamono Kunai
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * (C) Workshop Asahi 2026.  DxCore is (C) Spence Konde, LGPL 2.1 (see License.md).
 *
 * The Kunai's rev.2 pin map needs NO PORTMUX changes at startup: I2C sits on
 * TWI0's DEFAULT position (PA2/PA3 = D4/D5), SPI on SPI0's DEFAULT position
 * (PA4..PA7), USART0 on its DEFAULT position (PA0/PA1 = D6/D7, crystal-less
 * chip) and USART1 on ALT2 (set by Serial2.begin() through the normal mux
 * machinery). TCB1's WO parking and TCA0's PORTA routing are handled by the
 * core's init_TCBs()/init_TCA0() from TCB1_PINS/TCA0_PINS in pins_arduino.h.
 *
 * What remains here:
 *   1. The force-link marker. Arduino archives variant sources into core.a;
 *      without a referenced symbol the archive member would be silently
 *      dropped at link time. boards.txt passes -Wl,-u,wazamono_kunai_variant_keep,
 *      which forces this translation unit in.
 *   2. The USB-CDC activity LEDs (XIAO numbering: TX = D11, RX = D12):
 *        PD4 (D11) = LED_BUILTIN, doubles as the CDC TX activity LED
 *        PD5 (D12) = CDC RX activity LED
 *      Both active-LOW (matching the bootloader's PD4 blink polarity).
 *      usb_cdc.c publishes weak no-op hooks (usb_cdc_on_rx_activity /
 *      usb_cdc_on_tx_activity / usb_cdc_on_led_tick, the last called from the
 *      USB SOF path roughly every 1 ms); the strong definitions below
 *      override them - the same arrangement as the Wazamono Tachi.
 *
 * The LEDs are initialized OFF (driven HIGH, active-LOW) and set as outputs in
 * initVariant(). A sketch that takes D11/D12 over as GPIO simply fights the
 * short activity pulses; digitalWrite works normally between them.
 */

#include <Arduino.h>

#if defined(WAZAMONO_AVR_KUNAI)

extern "C" { __attribute__((used)) char wazamono_kunai_variant_keep = 0; }

#define KUNAI_TXLED_bm  PIN4_bm   /* PD4 = D11 (also LED_BUILTIN) */
#define KUNAI_RXLED_bm  PIN5_bm   /* PD5 = D12 */

/* One-shot length in SOF ticks (~ms), = 32U4 TX_RX_LED_PULSE_MS (Tachi parity). */
#define KUNAI_LED_PULSE  100

static volatile uint8_t s_rx_pulse = 0;
static volatile uint8_t s_tx_pulse = 0;

void initVariant(void) {
  PORTD.OUTSET = KUNAI_RXLED_bm | KUNAI_TXLED_bm;   /* OFF first (active-LOW) */
  PORTD.DIRSET = KUNAI_RXLED_bm | KUNAI_TXLED_bm;   /* then output */
}

extern "C" {

void usb_cdc_on_rx_activity(void) {
  PORTD.OUTCLR = KUNAI_RXLED_bm;        /* ON (active-LOW) */
  s_rx_pulse = KUNAI_LED_PULSE;
}

void usb_cdc_on_tx_activity(void) {
  PORTD.OUTCLR = KUNAI_TXLED_bm;        /* ON (active-LOW) */
  s_tx_pulse = KUNAI_LED_PULSE;
}

void usb_cdc_on_led_tick(void) {
  if (s_rx_pulse && !--s_rx_pulse) PORTD.OUTSET = KUNAI_RXLED_bm;  /* OFF */
  if (s_tx_pulse && !--s_tx_pulse) PORTD.OUTSET = KUNAI_TXLED_bm;  /* OFF */
}

} /* extern "C" */

#endif /* WAZAMONO_AVR_KUNAI */
