/* ClockOut.cpp - main clock output (CLKOUT) on Wazamono boards.
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * (C) Workshop Asahi 2026.  DxCore is (C) Spence Konde, LGPL 2.1 (see License.md).
 *
 * Clean-room implementation from DS40002548A chapter 12 (CLKCTRL) only.
 * See ClockOut.h for the API description.
 */

#include "ClockOut.h"

/* CLKCTRL.MCLKCTRLA is CCP-protected. Two rules are observed here:
 *   - The new value is computed into a register BEFORE the CCP key is
 *     written: a read-modify-write of a protected register inside the CCP
 *     window does not complete in time on this core.
 *   - Interrupts are held off across the key/write pair, so an ISR cannot
 *     consume the protection window. */
static void _clockout_write_mclkctrla(uint8_t value) {
  uint8_t oldSREG = SREG;
  cli();
  _PROTECTED_WRITE(CLKCTRL.MCLKCTRLA, value);
  SREG = oldSREG;
}

/* Is some other peripheral already driving PA7? */
static bool _clockout_pin_taken(void) {
  /* AC0's comparator output is on PA7 whenever OUTEN is set. */
  if (AC0.CTRLA & AC_OUTEN_bm) {
    return true;
  }
  /* EVSYS event output EVOUTA: it drives the pin only when a generator is
   * assigned to the user AND the PORTMUX position is the one that lands on
   * PA7 for this board. */
  #if defined(WAZAMONO_EVOUTA_PIN) && (WAZAMONO_EVOUTA_PIN == PIN_PA7)
    if (EVSYS.USEREVSYSEVOUTA != 0) {
      uint8_t alt = (PORTMUX.EVSYSROUTEA & PORTMUX_EVOUTA_bm) ? 1 : 0;
      if (alt == (WAZAMONO_EVOUTA_ALT)) {
        return true;
      }
    }
  #endif
  /* Boards whose hardware SPI SS sits on PA7 (Kunai): an enabled SPI0 needs
   * that pin - as a client select input in client mode, and as the mode-flip
   * input in host mode unless SSD is set. */
  #if defined(PIN_SPI_SS) && (PIN_SPI_SS == PIN_PA7)
    if (SPI0.CTRLA & SPI_ENABLE_bm) {
      return true;
    }
  #endif
  return false;
}

bool ClockOutClass::begin(void) {
  if (_clockout_pin_taken()) {
    return false;
  }
  /* The signal description types CLKOUT as a digital output (12.2.2), but the
   * data sheet does not spell out whether the override also sets the pin
   * direction, so set it explicitly - harmless either way. */
  pinMode(PIN_PA7, OUTPUT);

  uint8_t v = CLKCTRL.MCLKCTRLA | CLKCTRL_CLKOUT_bm;  /* pre-loaded, see above */
  _clockout_write_mclkctrla(v);
  return true;
}

void ClockOutClass::end(void) {
  uint8_t v = CLKCTRL.MCLKCTRLA & (uint8_t)~CLKCTRL_CLKOUT_bm;
  _clockout_write_mclkctrla(v);
  pinMode(PIN_PA7, INPUT);
}

bool ClockOutClass::isRunning(void) {
  /* Read the bit itself: a CFD event with the main clock as its source clears
   * CLKOUT in hardware, so a cached flag would lie about the pin's state. */
  return (CLKCTRL.MCLKCTRLA & CLKCTRL_CLKOUT_bm) != 0;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_CLOCKOUT)
ClockOutClass ClockOut;
#endif
