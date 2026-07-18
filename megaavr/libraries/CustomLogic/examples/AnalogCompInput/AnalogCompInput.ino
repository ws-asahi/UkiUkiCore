/* CustomLogic / AnalogCompInput
 *
 * An input does not have to come from a pin. Here IN0 is fed straight from
 * the analog comparator inside the chip, so the gate becomes:
 *
 *     OUT = (voltage is above 2.5 V) AND (IN1 pin is HIGH)
 *
 * No wire runs between the comparator and the logic - the connection is
 * inside the chip, costs no pin, and the whole thing keeps working with the
 * CPU doing nothing at all.
 *
 * Wiring:
 *   the voltage to watch -> the AnalogComp + pin
 *                           Tachi: A1 / Tsurugi: D9 / Kunai: D6
 *   a push button        -> the CustomLogic IN1 pin, to GND
 *                           Tachi: A2 / Tsurugi: D6 / Kunai: D5
 *   an LED (+ resistor)  -> the CustomLogic OUT pin, to GND
 *                           Tachi: A0 / Tsurugi: D10 / Kunai: D2
 *
 * The inputs are pulled up, so the LED lights while the voltage is above
 * 2.5 V and the button is NOT pressed; pressing the button puts it out.
 *
 * Note: the comparator is compared against its internal 2.5 V reference, so
 * its minus pin stays free - on Tachi and Tsurugi that pin is the same one
 * the logic block would use for OUT.
 */
#include <AnalogComp.h>
#include <CustomLogic.h>

void setup() {
  Serial.begin(115200);

  AnalogComp.begin(INTERNAL2V5);              // + pin vs internal 2.5 V
  AnalogComp.setHysteresis(AC_HYST_MEDIUM);   // ignore noise around the edge
  // Note: AnalogComp.enableOutput() is NOT needed - the logic block reads the
  // comparator directly, without going through a pin.

  CustomLogic.setInputIN0(LOGIC_ANALOG_COMP); // IN0 = the comparator result
  CustomLogic.begin(AND);                     // OUT = comparator AND IN1 pin
}

void loop() {
  Serial.print(F("voltage above 2.5V: "));
  Serial.print(AnalogComp.read() ? F("yes") : F("no "));
  Serial.print(F("   gate output: "));
  Serial.println(CustomLogic.read() ? F("HIGH") : F("LOW"));
  delay(500);
}
