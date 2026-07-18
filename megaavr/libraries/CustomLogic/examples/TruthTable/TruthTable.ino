/* CustomLogic / TruthTable
 *
 * Beyond the named gates, you can define the output for every input
 * combination yourself. Bit i of the table is the output when the
 * inputs spell the number i (IN2 = bit 2, IN1 = bit 1, IN0 = bit 0):
 *
 *   IN2 IN1 IN0 | i | example 0b10010110 (0x96)
 *    0   0   0  | 0 |  0
 *    0   0   1  | 1 |  1
 *    0   1   0  | 2 |  1
 *    0   1   1  | 3 |  0
 *    1   0   0  | 4 |  1
 *    1   0   1  | 5 |  0
 *    1   1   0  | 6 |  0
 *    1   1   1  | 7 |  1
 *
 * 0x96 makes a 3-input XOR ("odd parity"): OUT is HIGH while an odd
 * number of inputs are HIGH. Try designing your own function!
 *
 * Pins: same as the other examples (see ThreeInputOR).
 */
#include <CustomLogic.h>

void setup() {
  CustomLogic.beginTruthTable(0b10010110, 3);
}

void loop() {
  // The gate runs in hardware; nothing to do here.
}
