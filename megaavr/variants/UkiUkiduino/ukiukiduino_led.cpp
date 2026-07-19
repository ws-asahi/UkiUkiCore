/* ukiukiduino_led.cpp - on-board WS2812D-F5 LED driver for the UkiUkiduino
 * ---------------------------------------------------------------------------
 * Part of UkiUkiCore (a product-specific fork of WazamonoCore / SpenceKonde's
 * DxCore). (C) Workshop Asahi 2026. DxCore is (C) Spence Konde, LGPL 2.1
 * (see LICENSE.md).
 *
 * The on-board LED is a WS2812D-F5 addressable RGB LED whose DIN is wired to
 * PA0. To preserve the classic Arduino Uno "D13 LED" experience, the core's
 * LED_BUILTIN_MIRROR hook (wiring_digital.c) calls __led_builtin_mirror_hook()
 * after every digitalWrite()/digitalWriteFast() that lands on D13 (PD6). The
 * hook reads the RESULTING PD6 OUT bit and sends the matching one-pixel frame:
 *
 *      PD6 OUT = 1  ->  LED lit in the current color (default: yellow)
 *      PD6 OUT = 0  ->  LED off (0,0,0 frame)
 *
 * so the stock Blink sketch works completely unmodified, and blinks yellow.
 * setLEDColor()/setLEDBrightness() (declared in pins_arduino.h) change what
 * "lit" looks like; when the LED is currently lit they re-send the frame
 * immediately.
 *
 * --- WS2812D-F5 protocol timing (datasheet "Data Transfer Time") -----------
 *      T0H  220 ns - 380 ns     T1H  580 ns - 1 us
 *      T0L  580 ns - 1 us       T1L  580 ns - 1 us
 *      RES  > 280 us (frame latch; the D-F5 explicitly supports slow/paused
 *                     transfer, so inter-bit lows longer than nominal are safe)
 *      Data order: G7..G0 R7..R0 B7..B0 (GRB, MSB first)
 *
 * At F_CPU = 24 MHz (41.67 ns/cycle) the bit loop below runs 31 cycles/bit:
 *      T0H =  7 cycles = 292 ns   (within 220-380)
 *      T1H = 17 cycles = 708 ns   (within 580-1000)
 *      T1L = 14 cycles = 583 ns   (within 580-1000)
 *      T0L = 24 cycles = 1000 ns  (at the nominal edge; longer lows are fine
 *                                  on this part - see RES note above)
 * The frame (24 bits + inter-byte call overhead) takes ~31 us with interrupts
 * disabled. Before every frame we busy-wait 300 us so the PREVIOUS frame has
 * latched (RES > 280 us). A mirrored digitalWrite(13) therefore costs ~340 us.
 *
 * PA0 is toggled with SBI/CBI on VPORTA.OUT (I/O address 0x01, 1 cycle each on
 * AVRxt), so the other PORTA pins (PA1 button, PA2/PA3 I2C, PA4/PA5 UART...)
 * are never disturbed.
 */

#include <Arduino.h>

#if defined(UKIUKIDUINO_PINOUT)

#if F_CPU != 24000000UL
  #error "ukiukiduino_led.cpp: the WS2812 bit timing below is cycle-counted for 24 MHz."
#endif

/* Current "lit" appearance. Defaults: yellow at a tasteful brightness -
 * roughly the classic Uno "L" LED look, not a retina-searing full white. */
static uint8_t s_r = 255, s_g = 255, s_b = 0;
static uint8_t s_brightness = 40;

/* (component * (brightness+1)) >> 8 : cheap 0..255 scaling; 255 -> identity. */
static inline uint8_t scale8(uint8_t c) {
  return (uint8_t)(((uint16_t)c * (uint16_t)(s_brightness + 1)) >> 8);
}

/* One byte, MSB first, 31 cycles/bit. Cycle numbers in comments count from
 * the SBI that raises the line. Both bit values re-align at cycle 9 because
 * SBRS takes 2 cycles when it skips the (1-word) CBI. */
static void ws2812_byte(uint8_t b) {
  uint8_t cnt = 8;
  __asm__ __volatile__(
    "1:                          \n\t"
    "sbi  0x01, 0                \n\t" /* c1        line HIGH (VPORTA.OUT.0) */
    "nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t" /* c2-c6              */
    "sbrs %[b], 7                \n\t" /* c7 (c7-c8 when bit=1: skips CBI)  */
    "cbi  0x01, 0                \n\t" /* c8        bit=0: LOW, T0H=7cy     */
    "nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t"
    "nop \n\t nop \n\t nop \n\t nop \n\t"          /* c9-c17             */
    "cbi  0x01, 0                \n\t" /* c18       bit=1: LOW, T1H=17cy    */
    "lsl  %[b]                   \n\t" /* c19                              */
    "nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t"
    "nop \n\t nop \n\t nop \n\t nop \n\t"          /* c20-c28            */
    "dec  %[c]                   \n\t" /* c29                              */
    "brne 1b                     \n\t" /* c30-c31   -> 31 cycles/bit       */
    : [b] "+r" (b), [c] "+r" (cnt)
    :
  );
}

/* Send one GRB frame. Waits out the previous frame's latch time first
 * (RES > 280 us), then streams 24 bits with interrupts disabled. */
static void ws2812_frame(uint8_t r, uint8_t g, uint8_t b) {
  __builtin_avr_delay_cycles(F_CPU / 1000000UL * 300UL); /* 300 us guard */
  uint8_t s = SREG;
  cli();
  ws2812_byte(g);   /* GRB order, per the WS2812D-F5 datasheet */
  ws2812_byte(r);
  ws2812_byte(b);
  SREG = s;
}

/* Core hook: called by wiring_digital.c after every digitalWrite()/
 * digitalWriteFast() on D13 (PD6). Reads the RESULTING OUT bit so
 * HIGH/LOW/CHANGE all behave. Also used at startup to blank the LED. */
extern "C" void __led_builtin_mirror_hook(void) {
  if (VPORTD.OUT & (1 << 6)) {
    ws2812_frame(scale8(s_r), scale8(s_g), scale8(s_b));
  } else {
    ws2812_frame(0, 0, 0);
  }
}

/* ---- sketch-facing API (declared in pins_arduino.h) ---------------------- */

void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
  s_r = r;
  s_g = g;
  s_b = b;
  if (VPORTD.OUT & (1 << 6)) {   /* lit right now -> apply immediately */
    __led_builtin_mirror_hook();
  }
}

void setLEDColor(LEDColorName color) {
  switch (color) {
    case Red:     setLEDColor(255,   0,   0); break;
    case Green:   setLEDColor(  0, 255,   0); break;
    case Blue:    setLEDColor(  0,   0, 255); break;
    default:
    case Yellow:  setLEDColor(255, 255,   0); break;
    case Orange:  setLEDColor(255,  80,   0); break;
    case Cyan:    setLEDColor(  0, 255, 255); break;
    case Magenta: setLEDColor(255,   0, 255); break;
    case Purple:  setLEDColor(128,   0, 255); break;
    case Pink:    setLEDColor(255,  40,  70); break;
    case White:   setLEDColor(255, 255, 255); break;
  }
}

void setLEDBrightness(uint8_t brightness) {
  s_brightness = brightness;
  if (VPORTD.OUT & (1 << 6)) {   /* lit right now -> apply immediately */
    __led_builtin_mirror_hook();
  }
}

#endif /* UKIUKIDUINO_PINOUT */
