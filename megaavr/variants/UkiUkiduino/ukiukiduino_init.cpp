/* ukiukiduino_init.cpp - board-specific hardware init for the UkiUkiduino
 * ---------------------------------------------------------------------------
 * Part of UkiUkiCore (a product-specific fork of WazamonoCore / SpenceKonde's
 * DxCore). (C) Workshop Asahi 2026. DxCore is (C) Spence Konde, LGPL 2.1
 * (see LICENSE.md).
 *
 * Purpose
 *   Prepare PA0 as the on-board LED driver pin.  The LED itself follows
 *   digitalWrite()/digitalWriteFast() calls made to D13 (PD6): the core copies
 *   the resulting PD6 OUT bit onto PA0 (software mirror; see LED_BUILTIN_MIRROR
 *   in pins_arduino.h and the hooks in wiring_digital.c).  This replaces the
 *   op-amp buffer that a classic Arduino Uno R3 puts between pin 13 (SCK) and
 *   its LED, without loading the D13/SCK line with the LED at all.
 *
 *   Design note: only software writes are mirrored.  SPI (SCK) traffic does
 *   NOT blink the LED - a deliberate difference from the Uno R3.  A hardware
 *   mirror onto PA0 is not possible on the AVR64DU32: PA0 carries only CCL
 *   LUT0-IN0 (an input), and has no CCL output or EVOUT function (datasheet
 *   DS40002548A, I/O multiplexing table).  PC3 - the only spare pin with a CCL
 *   output (LUT1-OUT) - is exposed on the header as D7 instead, where its ADC
 *   channel (AIN31 = A13), AC0 AINP4 and LUT1-OUT are available to the user.
 *   PA0 is a plain VDD-domain 5V GPIO, as is PC3 (per datasheet section 6.1,
 *   VDD powers all I/O lines; VUSB powers only the DM/DP transceiver).
 *
 * Resource use
 *   None. Unlike the earlier CCL-based design, no EVSYS channel and no CCL LUT
 *   is consumed - EVSYS CH0 and CCL LUT1 are fully available to user sketches.
 *
 * Note: BTN_BUILTIN (D20 = PA1) needs no init here - it has an external 1 kOhm
 *   pull-down on the board (pressed = HIGH) and is read as a plain input.
 */

#include <Arduino.h>

#if defined(UKIUKIDUINO_PINOUT)

/* ---- Force-link marker ----------------------------------------------------
 * Arduino archives variant-folder objects into core.a, and the core's main.cpp
 * supplies a *weak* initVariant(). An archived strong symbol does NOT override
 * a weak one already provided by a linked object, so without help this entire
 * translation unit is silently dropped at link time. boards.txt passes
 *     -Wl,-u,ukiukiduino_variant_keep
 * which forces the linker to pull this member. */
extern "C" { __attribute__((used)) char ukiukiduino_variant_keep = 0; }

/* initVariant() is a weak no-op in the core (cores/dxcore/main.cpp); this strong
 * definition overrides it and runs once, immediately after init(), before setup(). */
void initVariant(void) {
  /* LED driver pin PA0: output, LED off (LED is active-HIGH via R7).
   * The pin stays an output permanently; pinMode(13, ...) only affects PD6,
   * so the LED simply holds its last mirrored state if D13 is made an input. */
  VPORTA.OUT &= ~(1 << 0);
  VPORTA.DIR |= (1 << 0);
}

#endif /* UKIUKIDUINO_PINOUT */
