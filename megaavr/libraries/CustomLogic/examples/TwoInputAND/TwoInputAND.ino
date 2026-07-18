/* CustomLogic / TwoInputAND
 *
 * A hardware AND gate: the OUT pin goes HIGH only while both inputs are
 * HIGH. Once begin() has run, the gate works entirely in hardware -
 * loop() can be empty, or even asleep, and the gate keeps working.
 *
 * Pins of the CustomLogic unit:
 *            IN0        IN1        OUT
 *   Tachi    A3         A2         A0
 *   Tsurugi  D5         D6         D10
 *   Kunai    D4         D5         D2
 *
 * The inputs have pull-ups, so for a quick test just wire two push
 * buttons from IN0/IN1 to GND and an LED (with a resistor) from OUT to
 * GND: the LED lights only while NEITHER button is pressed - both
 * inputs are HIGH thanks to the pull-ups. Press either button and it
 * goes out.
 */
#include <CustomLogic.h>

void setup() {
  CustomLogic.begin(AND);   // OUT = IN0 AND IN1
}

void loop() {
  // Nothing to do - the gate runs by itself.
}
