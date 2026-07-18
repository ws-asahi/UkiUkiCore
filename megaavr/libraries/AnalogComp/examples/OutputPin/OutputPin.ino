/* AnalogComp / OutputPin
 *
 * The comparator result can drive a pin directly, with no software in
 * between - it keeps working even while your sketch is busy or sleeping.
 *
 * Output pin (fixed by hardware): PA7
 *   Tachi: D4 / Tsurugi: D8 / Kunai: D0
 *
 * Wire an LED (with a resistor) to the output pin and vary the voltage
 * on the plus input pin around 1.02 V to see it switch.
 */
#include <AnalogComp.h>

void setup() {
  AnalogComp.begin(INTERNAL1V024);   // plus pin vs internal 1.024 V
  AnalogComp.setHysteresis(AC_HYST_LARGE);
  AnalogComp.enableOutput();      // result appears on PA7
}

void loop() {
  // Nothing to do - the comparator drives the pin all by itself.
}
