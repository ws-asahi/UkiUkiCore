/* ClockOut.h - main clock output (CLKOUT) on Wazamono boards.
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * (C) Workshop Asahi 2026.  DxCore is (C) Spence Konde, LGPL 2.1 (see License.md).
 *
 * Puts the peripheral clock (CLK_PER) on the CLKOUT pin, so an external
 * device can share the board's time base instead of carrying its own crystal:
 *
 *     ClockOut.begin();                    // start driving CLK_PER on the pin
 *     Serial.println(ClockOut.frequency()); // 24000000 on a 24 MHz board
 *     ClockOut.end();                      // release the pin again
 *
 * Typical uses: clocking a codec/ADC/logic IC, feeding another MCU's external
 * clock input for synchronous operation, and checking the real system clock
 * with a scope or frequency counter (useful on the crystal-less Kunai, whose
 * OSCHF is tuned against the USB SOF).
 *
 * Hardware (DS40002548A chapter 12, CLKCTRL):
 *   - The output is CLK_PER (12.2.2 signal description), enabled by the
 *     CLKOUT bit in CLKCTRL.MCLKCTRLA, which is CCP-protected.
 *   - The pin is PA7 and has no alternate position. On the Wazamono boards
 *     that is Tachi D8, Tsurugi D2, Kunai D1.
 *   - A Clock Failure Detection (CFD) event that overrides CLKSEL clears the
 *     CLKOUT bit in hardware, so the output stops by itself if the clock
 *     source fails. isRunning() reports the bit's real state.
 *
 * There is deliberately no divider argument: CLKOUT emits CLK_PER, and the
 * only divider in that path is the CLK_MAIN prescaler, which sets the CPU
 * clock as well - changing it would move millis(), the USB clocking and every
 * UART baud rate with it. For an arbitrary frequency on a pin, use a TCA/TCB
 * PWM output (analogWrite / tone) or the CCL instead.
 *
 * PA7 conflicts: AC0's comparator output, the EVSYS event output EVOUTA, the
 * USART0 XDIR signal in the ALT1 position, and - on the Kunai - the hardware
 * SPI SS pin all land on PA7. begin() refuses (returns false) when it can see
 * AC0's output, EVOUTA or (Kunai) an enabled SPI0 already claiming the pin;
 * plain GPIO use by the sketch cannot be detected, so mind it yourself.
 *
 * EMI note: this is a continuously running 24 MHz square wave with fast edges.
 * Keep the trace short, and prefer enabling it only while it is needed rather
 * than leaving it on for the life of the product.
 */

#ifndef CLOCKOUT_H
#define CLOCKOUT_H

#include <Arduino.h>

#if !defined(PIN_PA7)
  #error "ClockOut requires a variant that maps PA7 (the CLKOUT pin)."
#endif

class ClockOutClass {
public:
  /* Start driving CLK_PER on the CLKOUT pin (PA7).
   * Returns false, without changing anything, if another peripheral is
   * already driving that pin (see the header comment). */
  bool begin(void);

  /* Stop the output and return the pin to a plain input. */
  void end(void);

  /* True while the hardware is actually driving the clock out. This reads
   * the CLKOUT bit rather than a cached flag, so it also reports the output
   * having been switched off by a Clock Failure Detection event. */
  bool isRunning(void);

  /* Frequency appearing on the pin, in Hz: CLK_PER, i.e. the clock the core
   * was built for. (A sketch that reprograms CLKCTRL.MCLKCTRLB changes the
   * whole system clock, F_CPU included, and this value with it.) */
  uint32_t frequency(void) { return F_CPU; }

  /* The Arduino pin number the clock comes out on (Tachi D8 / Tsurugi D2 /
   * Kunai D1) - handy for printing and for pin-conflict checks in sketches. */
  uint8_t pin(void) { return PIN_PA7; }
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_CLOCKOUT)
extern ClockOutClass ClockOut;
#endif

#endif /* CLOCKOUT_H */
