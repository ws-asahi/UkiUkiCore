/* CustomLogic / SetResetLatch
 *
 * A logic block can look at its own output. That is what turns a gate into
 * a memory: press SET and the output stays HIGH after you let go; press
 * RESET and it stays LOW. A latch made of nothing but hardware - no code
 * runs in loop(), and it keeps working while the CPU sleeps.
 *
 *   IN0 = SET button   (pin)
 *   IN1 = RESET button (pin)
 *   IN2 = this unit's own output   <- LOGIC_OWN_OUTPUT, no pin used
 *
 * Wiring (buttons to GND - the inputs are pulled up, so pressed = LOW):
 *   SET   -> IN0: Tachi A3 / Tsurugi D5  / Kunai D4
 *   RESET -> IN1: Tachi A2 / Tsurugi D6  / Kunai D5
 *   LED   -> OUT: Tachi A0 / Tsurugi D10 / Kunai D2
 *
 * The truth table below says: if SET is pressed the output is HIGH; else if
 * RESET is pressed it is LOW; else it keeps whatever it had (its own output).
 * Bit i is the output when the inputs spell i, with IN2 = bit 2:
 *
 *   IN2(self) IN1(RESET) IN0(SET) | i | OUT
 *       0          0        0     | 0 |  1   SET pressed
 *       0          0        1     | 1 |  0   RESET pressed
 *       0          1        0     | 2 |  1   SET pressed
 *       0          1        1     | 3 |  0   hold (was LOW)
 *       1          0        0     | 4 |  1   SET pressed
 *       1          0        1     | 5 |  0   RESET pressed
 *       1          1        0     | 6 |  1   SET pressed
 *       1          1        1     | 7 |  1   hold (was HIGH)
 *
 * -> 0b11010101 (SET wins if both are pressed).
 */
#include <CustomLogic.h>

void setup() {
  Serial.begin(115200);
  CustomLogic.setInputIN2(LOGIC_OWN_OUTPUT);      // IN2 = our own output
  CustomLogic.beginTruthTable(0b11010101, 3);
}

void loop() {
  Serial.println(CustomLogic.read() ? F("latched: SET") : F("latched: RESET"));
  delay(500);
}
