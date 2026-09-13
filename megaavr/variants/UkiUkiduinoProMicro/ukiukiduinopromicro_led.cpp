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
 * The bit loop is cycle-counted per F_CPU. Each bit is P cycles; the line is
 * high for H0 cycles (bit 0) or H1 cycles (bit 1); the low time is P-H0 / P-H1.
 * The values are chosen so that EVERY figure in the table above is met -
 * including the tight T1L maximum of 0.35 us:
 *
 *   F_CPU  ns/cy   P     H0 (T0H)      H1 (T1H)      T0L=P-H0      T1L=P-H1
 *   24 MHz 41.67  24   7 = 292 ns   16 = 667 ns   17 = 708 ns    8 = 333 ns
 *   20 MHz 50.00  20   6 = 300 ns   14 = 700 ns   14 = 700 ns    6 = 300 ns
 *   16 MHz 62.50  16   5 = 312 ns   11 = 687 ns   11 = 687 ns    5 = 312 ns
 *   12 MHz 83.33  12   3 = 250 ns    8 = 667 ns    9 = 750 ns    4 = 333 ns
 *
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

/* Per-F_CPU loop constants (see the timing table in the header):
 *   WS_NA = H0 - 2       nops between SBI and SBRS
 *   WS_NB = H1 - H0 - 3  nops before LSL/DEC/CBI(1)
 *   WS_NC = P  - H1 - 3  nops before BRNE                                   */
#if   F_CPU == 24000000UL   /* P=24 H0=7  H1=16 */
  #define WS_NA 5
  #define WS_NB 6
  #define WS_NC 5
#elif F_CPU == 20000000UL   /* P=20 H0=6  H1=14 */
  #define WS_NA 4
  #define WS_NB 5
  #define WS_NC 3
#elif F_CPU == 16000000UL   /* P=16 H0=5  H1=11 */
  #define WS_NA 3
  #define WS_NB 3
  #define WS_NC 2
#elif F_CPU == 12000000UL   /* P=12 H0=3  H1=8  */
  #define WS_NA 1
  #define WS_NB 2
  #define WS_NC 1
#else
  #error "ukiukiduinopromicro_led.cpp: no WS2812B bit timing for this F_CPU (supported: 12/16/20/24 MHz)."
#endif

/* ---- NOP string builders (n = 0..12) ----------------------------------- */
#define WS_NOP0  ""
#define WS_NOP1  "nop\n\t"
#define WS_NOP2  WS_NOP1  WS_NOP1
#define WS_NOP3  WS_NOP2  WS_NOP1
#define WS_NOP4  WS_NOP3  WS_NOP1
#define WS_NOP5  WS_NOP4  WS_NOP1
#define WS_NOP6  WS_NOP5  WS_NOP1
#define WS_NOP7  WS_NOP6  WS_NOP1
#define WS_NOP8  WS_NOP7  WS_NOP1
#define WS_NOP9  WS_NOP8  WS_NOP1
#define WS_NOP10 WS_NOP9  WS_NOP1
#define WS_NOP11 WS_NOP10 WS_NOP1
#define WS_NOP12 WS_NOP11 WS_NOP1
#define WS_NOPS_(n) WS_NOP##n
#define WS_NOPS(n)  WS_NOPS_(n)

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

/* One byte, MSB first, P cycles/bit. Cycle numbers count from the SBI that
 * raises the line. Both bit values re-align after the SBRS because SBRS takes
 * 2 cycles when it skips the (1-word) CBI. LSL and DEC are placed in the
 * "line high" stretch so that only BRNE follows the bit-1 CBI, which keeps
 * the loop short enough for 12 MHz. CBI does not touch SREG, so the Z flag
 * set by DEC survives until BRNE. */
static void ws2812_byte(uint8_t b) {
  uint8_t cnt = 8;
  __asm__ __volatile__(
    "1:                          \n\t"
    "sbi  %[port], %[bit]        \n\t" /* c1          line HIGH (VPORTF.OUT.4) */
    WS_NOPS(WS_NA)                      /* c2..c(H0-1)                        */
    "sbrs %[b], 7                \n\t" /* c(H0)       (2 cycles when bit=1)  */
    "cbi  %[port], %[bit]        \n\t" /* c(H0+1)     bit=0: LOW, T0H=H0 cy  */
    WS_NOPS(WS_NB)                      /* c(H0+2)..                          */
    "lsl  %[b]                   \n\t"
    "dec  %[c]                   \n\t"
    "cbi  %[port], %[bit]        \n\t" /* c(H1+1)     bit=1: LOW, T1H=H1 cy  */
    WS_NOPS(WS_NC)
    "brne 1b                     \n\t" /* 2 cycles    -> P cycles/bit        */
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
