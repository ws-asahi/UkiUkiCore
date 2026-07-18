/* AnalogComp / Threshold
 *
 * Compares the plus input pin against an internal reference voltage -
 * no external divider needed. Here the threshold is 2.5 V: the built-in
 * LED turns on while the plus pin is above it.
 *
 * Plus input pin: Tachi A1 / Tsurugi D9 / Kunai D6
 * Pass the same constants used with analogReference(): INTERNAL1V024,
 * INTERNAL2V048, INTERNAL2V5, INTERNAL4V096, VDD or EXTERNAL. An optional
 * second argument scales the threshold: Vth = Vref * level / 256.
 */
#include <AnalogComp.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  AnalogComp.begin(INTERNAL2V5);            // plus pin vs internal 2.5 V
  AnalogComp.setHysteresis(AC_HYST_MEDIUM);
}

void loop() {
  // Note: LED_BUILTIN is active-LOW on Tachi, so "on" and "off" are swapped there.
  digitalWrite(LED_BUILTIN, AnalogComp.read() ? HIGH : LOW);
  delay(10);
}
