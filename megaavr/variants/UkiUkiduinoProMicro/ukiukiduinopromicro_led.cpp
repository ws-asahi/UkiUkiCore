/* ukiukiduinopromicro_led.cpp - on-board WS2812B LED driver for the UkiUkiduino ProMicro
 * ---------------------------------------------------------------------------
 * Part of UkiUkiCore (a product-specific fork of WazamonoCore / SpenceKonde's
 * DxCore). (C) Workshop Asahi 2026. DxCore is (C) Spence Konde, LGPL 2.1
 * (see LICENSE.md).
 *
 * The on-board LED is an XL-5050RGBC-WS2812B (XINGLIGHT, 5050 SMD, LCSC
 * C2843785) addressable RGB LED whose DIN is wired to PF4 through a 330 Ohm
 * series resistor. LED_BUILTIN is D17 = PF5, used as the LED's STATE pin: the
 * core's LED_BUILTIN_MIRROR hook (wiring_digital.c) calls
 * __led_builtin_mirror_hook() after every digitalWrite()/digitalWriteFast()
 * that lands on D17. The hook reads the RESULTING PF5 OUT bit and sends the
 * matching one-pixel frame on PF4:
 *
 *      PF5 OUT = 1  ->  LED lit in the current color (default: yellow)
 *      PF5 OUT = 0  ->  LED off (0,0,0 frame)
 *
 * so the stock Blink sketch works completely unmodified, and blinks yellow.
 * setBLEDColor() (declared in pins_arduino.h) changes what "lit" looks like;
 * when the LED is currently lit it re-sends the frame immediately. The API is
 * identical to the Uno-form UkiUkiduino (ukiukiduino_led.cpp).
 *
 * This driver is fully self-contained (the bit-banged protocol below is
 * written from the XL-5050RGBC-WS2812B datasheet) - it does NOT use or depend
 * on the tinyNeoPixel libraries in any way.
 *
 * --- XL-5050RGBC-WS2812B protocol timing (datasheet "Signal transmission
 *     definition", p.7) ------------------------------------------------------
 *      T0H  0.20 - 0.35 us (typ 0.295)    T1H  0.55 - 1.2 us (typ 0.595)
 *      T0L  0.55 - 1.2  us (typ 0.595)    T1L  0.20 - 0.35 us (typ 0.295)
 *      bit period T0/T1 >= 0.89 us;  RESET (latch) low >= 80 us
 *      Data order: G7..G0 R7..R0 B7..B0 (GRB, MSB first) - the usual WS2812B
 *      order. NOTE: the Uno UkiUkiduino's WS2812D-F5-12mA-C1 is RGB-ordered
 *      and has different limits, which is why the two variants have separate
 *      drivers.
 *
 * At F_CPU = 24 MHz (41.67 ns/cycle) the bit loop below runs 24 cycles/bit
 * (= 1.00 us), chosen so that EVERY figure in the table above is met -
 * including the tight T1L maximum of 0.35 us:
 *      T0H =  7 cycles = 292 ns   (0.20 - 0.35)
 *      T1H = 16 cycles = 667 ns   (0.55 - 1.2)
 *      T0L = 17 cycles = 708 ns   (0.55 - 1.2)
 *      T1L =  8 cycles = 333 ns   (0.20 - 0.35)
 * The frame (24 bits + inter-byte call overhead) takes ~25 us with interrupts
 * disabled. Before every frame we busy-wait 300 us so the PREVIOUS frame has
 * latched (RESET >= 80 us; the same guard the Uno variant uses). A mirrored
 * digitalWrite(17) therefore costs ~330 us.
 *
 * PF4 is toggled with SBI/CBI on VPORTF.OUT (I/O address 0x15, 1 cycle each
 * on AVRxt), so the other PORTF pins (PF0 button, PF1..PF3 = A1..A3, PF5 =
 * the LED state pin) are never disturbed.
 */

#include <Arduino.h>

#if defined(UKIUKIDUINO_PROMICRO_PINOUT)

#if F_CPU != 24000000UL
  #error "ukiukiduinopromicro_led.cpp: the WS2812B bit timing below is cycle-counted for 24 MHz."
#endif

/* VPORTF.OUT I/O address: VPORTn base = 4*n (A=0x00, C=0x08, D=0x0C, F=0x14),
 * OUT = base + 1. Kept as a literal because SBI/CBI need an immediate. */
#define UUPM_WS_VPORT_OUT  0x15
#define UUPM_WS_BIT        4        /* PF4 */

/* Current "lit" appearance, stored as the FINAL RGB values to display
 * (brightness is applied when a named color is set, not at send time).
 * Defaults: yellow at brightness 40 - the classic Uno "L" LED look.
 * (255,255,0) scaled by 40 -> (40,40,0). */
static uint8_t s_r = 40, s_g = 40, s_b = 0;

/* (component * (brightness+1)) >> 8 : cheap 0..255 scaling; 255 -> identity. */
static inline uint8_t scale8(uint8_t c, uint8_t brightness) {
  return (uint8_t)(((uint16_t)c * (uint16_t)(brightness + 1)) >> 8);
}

/* One byte, MSB first, 24 cycles/bit. Cycle numbers in comments count from
 * the SBI that raises the line. Both bit values re-align at cycle 9 because
 * SBRS takes 2 cycles when it skips the (1-word) CBI. */
static void ws2812_byte(uint8_t b) {
  uint8_t cnt = 8;
  __asm__ __volatile__(
    "1:                          \n\t"
    "sbi  %[port], %[bit]        \n\t" /* c1        line HIGH (VPORTF.OUT.4) */
    "nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t" /* c2-c6              */
    "sbrs %[b], 7                \n\t" /* c7 (c7-c8 when bit=1: skips CBI)  */
    "cbi  %[port], %[bit]        \n\t" /* c8        bit=0: LOW, T0H=7cy     */
    "nop \n\t nop \n\t nop \n\t nop \n\t"
    "nop \n\t nop \n\t nop \n\t nop \n\t" /* c9-c16                        */
    "cbi  %[port], %[bit]        \n\t" /* c17       bit=1: LOW, T1H=16cy    */
    "lsl  %[b]                   \n\t" /* c18                              */
    "nop \n\t nop \n\t nop \n\t"       /* c19-c21                          */
    "dec  %[c]                   \n\t" /* c22                              */
    "brne 1b                     \n\t" /* c23-c24   -> 24 cycles/bit       */
    : [b] "+r" (b), [c] "+r" (cnt)
    : [port] "I" (UUPM_WS_VPORT_OUT), [bit] "I" (UUPM_WS_BIT)
  );
}

/* Send one GRB frame. Waits out the previous frame's latch time first
 * (RESET >= 80 us; 300 us used), then streams 24 bits with interrupts disabled. */
static void ws2812_frame(uint8_t r, uint8_t g, uint8_t b) {
  __builtin_avr_delay_cycles(F_CPU / 1000000UL * 300UL); /* 300 us guard */
  uint8_t s = SREG;
  cli();
  ws2812_byte(g);   /* GRB order, per the XL-5050RGBC-WS2812B datasheet */
  ws2812_byte(r);
  ws2812_byte(b);
  SREG = s;
}

/* Core hook: called by wiring_digital.c after every digitalWrite()/
 * digitalWriteFast() on D17 (PF5). Reads the RESULTING OUT bit so
 * HIGH/LOW/CHANGE all behave. Also used at startup to blank the LED. */
extern "C" void __led_builtin_mirror_hook(void) {
  if (VPORTF.OUT & (1 << 5)) {
    ws2812_frame(s_r, s_g, s_b);
  } else {
    ws2812_frame(0, 0, 0);
  }
}

/* ---- sketch-facing API (declared in pins_arduino.h) ---------------------- */

/* Raw RGB: shown as-is, no brightness scaling. */
void setBLEDColor(uint8_t r, uint8_t g, uint8_t b) {
  s_r = r;
  s_g = g;
  s_b = b;
  if (VPORTF.OUT & (1 << 5)) {   /* lit right now -> apply immediately */
    __led_builtin_mirror_hook();
  }
}

/* Named color at a given brightness (0-255; the default argument is
 * BLED_DEFAULT_BRIGHTNESS = 40, see pins_arduino.h). Same table as the Uno
 * UkiUkiduino so sketches look the same on both boards. */
void setBLEDColor(LEDColorName color, uint8_t brightness) {
  uint8_t r, g, b;
  switch (color) {
    case Red:     r = 255; g =   0; b =   0; break;
    case Green:   r =   0; g = 255; b =   0; break;
    case Blue:    r =   0; g =   0; b = 255; break;
    default:
    case Yellow:  r = 255; g = 255; b =   0; break;
    case Orange:  r = 255; g =  80; b =   0; break;
    case Cyan:    r =   0; g = 255; b = 255; break;
    case Magenta: r = 255; g =   0; b = 255; break;
    case Purple:  r = 128; g =   0; b = 255; break;
    case Pink:    r = 255; g =  40; b =  70; break;
    case White:   r = 255; g = 255; b = 255; break;
  }
  setBLEDColor(scale8(r, brightness), scale8(g, brightness), scale8(b, brightness));
}

#endif /* UKIUKIDUINO_PROMICRO_PINOUT */
