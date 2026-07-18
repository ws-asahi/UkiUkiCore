/* CustomLogic / ThreeInputOR
 *
 * A 3-input OR gate: OUT is HIGH while at least one input is HIGH.
 * Three-input logic is written as two operations, applied left to
 * right: begin(logic1, logic2) means OUT = (IN0 logic1 IN1) logic2 IN2.
 *
 *   begin(OR, OR)    -> IN0 OR IN1 OR IN2       (this example)
 *   begin(AND, AND)  -> IN0 AND IN1 AND IN2
 *   begin(AND, OR)   -> (IN0 AND IN1) OR IN2
 *   begin(XOR, XOR)  -> 3-input parity
 *   begin(NOP, OR)   -> IN1 OR IN2  (IN0 unused)
 *
 * Pins of the CustomLogic unit:
 *            IN0        IN1        IN2        OUT
 *   Tachi    A3         A2         A1         A0
 *   Tsurugi  D5         D6         D9         D10
 *   Kunai    D4         D5         D3         D2
 */
#include <CustomLogic.h>

void setup() {
  Serial.begin(115200);
  CustomLogic.begin(OR, OR);  // OUT = (IN0 OR IN1) OR IN2
}

void loop() {
  // read() reports the current state of the OUT pin.
  Serial.println(CustomLogic.read() ? "OUT = HIGH" : "OUT = LOW");
  delay(500);
}
