/* ukiukiduino_init.cpp - board-specific hardware init for the UkiUkiduino
 * ---------------------------------------------------------------------------
 * Part of UkiUkiCore (a product-specific fork of WazamonoCore / SpenceKonde's
 * DxCore). (C) Workshop Asahi 2026. DxCore is (C) Spence Konde, LGPL 2.1
 * (see LICENSE.md).
 *
 * Purpose
 *   Drive the on-board LED from PC3 while mirroring D13 (PD6, the SPI SCK pin) in
 *   hardware, so the LED never loads the SCK line.  This replaces the unity-gain
 *   op-amp buffer that a classic Arduino Uno R3 puts between pin 13 (SCK) and its
 *   LED.  The mirror is a pure hardware path - zero CPU, and it tracks PD6 whether
 *   PD6 is driven by digitalWrite(13, ...) or by the SPI peripheral (SCK):
 *
 *     PD6 pin level --(PORTD EVGEN0)--(EVSYS CH0)--> CCL LUT1 IN0 (EVENTA)
 *                   --> LUT1 truth-table = buffer --> LUT1-OUT = PC3 --> LED
 *
 *   LUT1's input pins (PC0..PC2) are not bonded on the AVR64DU32 TQFP32, but that
 *   does not matter: the input is taken from the event channel, and LUT1's OUTPUT
 *   pin (PC3) IS bonded.  Per the datasheet PORTMUX CCLROUTEA table, LUT1's default
 *   output position is PC3, so no CCLROUTEA change is needed.
 *
 * Datasheet references (DS40002548A)
 *   - PORTMUX 17.3.2 (CCLROUTEA): LUT1 default OUT = PC3, IN0..IN2 = none.
 *   - CCL 30.2.2.1 (INSEL): IN0 = EVENTA selects EVSYS event channel A.
 *   - EVSYS: PORTD EVGENn pin-level generator; channel generator code for
 *     PORTD EVGEN0 = 0x40 | (port<<1) = 0x40 | (3<<1) = 0x46.
 *   - PORT EVGENCTRLA: EVGEN0SEL picks which PORTD pin feeds EVGEN0 (6 = PD6).
 *
 * IMPORTANT (VUSB power domain)
 *   PC3 is in the VUSB power domain, so its output buffer is only powered while the
 *   USB voltage regulator is enabled (UkiUkiduino uses the internal VUSB regulator;
 *   boards.txt passes -DUSB_VREG_INTERNAL).  The LED therefore lights only while
 *   the USB stack has brought up the regulator - it will NOT light at power-on
 *   before that.  This is expected and is documented for the user.
 *
 * Note: BTN_BUILTIN (D20 = PA1) needs no init here - it has an external 1 kOhm
 *   pull-down on the board (pressed = HIGH) and is read as a plain input.
 *
 * Resource use
 *   Consumes EVSYS channel 0 and CCL LUT1 (and its OUT pin PC3).  Sketches that need
 *   the Event/Logic libraries should avoid EVSYS channel 0 and CCL LUT1.
 */

#include <Arduino.h>

#if defined(UKIUKIDUINO_PINOUT)

/* ---- Force-link marker ----------------------------------------------------
 * Arduino archives variant-folder objects into core.a, and the core's main.cpp
 * supplies a *weak* initVariant(). An archived strong symbol does NOT override
 * a weak one already provided by a linked object, so without help this entire
 * translation unit (the strong initVariant() that sets up the CCL/EVSYS LED
 * mirror) is silently dropped at link time. boards.txt passes
 *     -Wl,-u,ukiukiduino_variant_keep
 * which forces the linker to pull this member. (Confirmed necessary under -flto.) */
extern "C" { __attribute__((used)) char ukiukiduino_variant_keep = 0; }

/* initVariant() is a weak no-op in the core (cores/dxcore/main.cpp); this strong
 * definition overrides it and runs once, immediately after init(), before setup(). */
void initVariant(void) {
  /* Keep PD6's digital input buffer enabled (ISC = INTDISABLE = 0) so the PORT
   * event generator can sense its pin level even while PD6 is driven as an output
   * (SPI SCK or digitalWrite). Preserves PULLUPEN/INVEN bits. */
  PORTD.PIN6CTRL &= ~PORT_ISC_gm;

  /* PORTD EVGEN0 follows PD6.  EVGENCTRLA = (EVGEN1SEL<<4)|EVGEN0SEL; 6 -> PD6. */
  PORTD.EVGENCTRLA = (0x0 << PORT_EVGEN1SEL_gp) | (0x6 << PORT_EVGEN0SEL_gp);

  /* Route PORTD EVGEN0 onto EVSYS channel 0, then feed channel 0 to CCL LUT1 in A.
   *   CHANNEL0 generator   = 0x46  (PORTD EVGEN0; see header)
   *   USERCCLLUT1A         = 0x01  (channel index + 1; channel 0) */
  EVSYS.CHANNEL0     = 0x46;
  EVSYS.USERCCLLUT1A = 0x01;

  /* CCL LUT1 inputs: IN0 = EVENTA (0x3), IN1 = MASK (0x0), IN2 = MASK (0x0).
   *   LUT1CTRLB[3:0]=INSEL0, LUT1CTRLB[7:4]=INSEL1, LUT1CTRLC[3:0]=INSEL2. */
  CCL.LUT1CTRLB = (0x0 << 4) | 0x3;   /* INSEL1=MASK, INSEL0=EVENTA */
  CCL.LUT1CTRLC = 0x0;                /* INSEL2=MASK                */

  /* Truth table = buffer of IN0 (with IN1=IN2=0, index == IN0): TRUTH[1]=1. */
  CCL.TRUTH1 = 0x02;

  /* Enable LUT1 output to its OUT pin (PC3, CCLROUTEA default) and enable the LUT.
   * Asynchronous pass-through: FILTSEL/EDGEDET left at reset (disabled). */
  CCL.LUT1CTRLA = CCL_OUTEN_bm | CCL_ENABLE_bm;

  /* Enable the CCL peripheral. */
  CCL.CTRLA = CCL_ENABLE_bm;
}

#endif /* UKIUKIDUINO_PINOUT */
