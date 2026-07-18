/* AnalogComp / ReadState
 *
 * Compares the plus input pin against the minus input pin and prints the
 * result. The output is true while the plus voltage is above the minus
 * voltage - a hardware voltage comparison with no ADC and no math.
 *
 * Default input pins:
 *            plus (+)         minus (-)
 *   Tachi    A1  (PD2)        A0  (PD3)
 *   Tsurugi  D9  (PD2)        D10 (PD3)
 *   Kunai    D6  (PD6)        D7  (PD7)
 *
 * Try: connect the minus pin to a potentiometer wiper (or a resistor
 * divider) and the plus pin to the voltage you want to watch.
 */
#include <AnalogComp.h>

void setup() {
  Serial.begin(115200);
  AnalogComp.begin();
  AnalogComp.setHysteresis(AC_HYST_SMALL); // suppress chatter near the crossing
}

void loop() {
  if (AnalogComp.read()) {
    Serial.println("plus > minus");
  } else {
    Serial.println("plus < minus");
  }
  delay(500);
}
