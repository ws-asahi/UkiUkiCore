/* EventSystem / SoftwareTrigger
 *
 * trigger() sends a single one-clock pulse (about 42 ns at 24 MHz) down the
 * connection - far too short to see on an LED, but exactly what edge-driven
 * hardware wants. Here it SETs a CustomLogic latch; a button RESETs it.
 * The sketch decides WHEN (in code); everything after that is hardware.
 *
 *   IN0 = the software pulse   (via EVENT_TO_LOGIC_A)
 *   IN1 = RESET button          Tachi: A2 / Tsurugi: D6 / Kunai: D5
 *   IN2 = the latch's own output
 *   OUT = LED (+ resistor)      Tachi: A0 / Tsurugi: D10 / Kunai: D2
 *
 * Truth table (bit i = output for inputs spelling i, IN2 = bit 2):
 *   RESET pressed (IN1 = 0)          -> 0          (indices 0,1,4,5)
 *   pulse arrives (IN0 = 1, IN1 = 1) -> 1          (indices 3,7)
 *   otherwise                        -> keep IN2   (index 2 -> 0, 6 -> 1)
 *   = 0b11001000
 */
#include <CustomLogic.h>
#include <EventSystem.h>

void setup() {
  Serial.begin(115200);

  EventSystem.connect(EVENT_SOFTWARE, EVENT_TO_LOGIC_A);

  CustomLogic.setInputIN0(LOGIC_EVENT_A);      // IN0 = the software pulse
  CustomLogic.setInputIN2(LOGIC_OWN_OUTPUT);   // IN2 = our own output (latch)
  CustomLogic.beginTruthTable(0b11001000, 3);
}

void loop() {
  Serial.println(F("trigger() - the latch goes HIGH (press the button to reset it)"));
  EventSystem.trigger();                        // one 42 ns pulse: SET
  for (uint8_t i = 0; i < 6; i++) {
    Serial.print(F("  latch: "));
    Serial.println(CustomLogic.read() ? F("SET") : F("RESET"));
    delay(500);
  }
}
