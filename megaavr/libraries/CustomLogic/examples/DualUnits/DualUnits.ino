/* CustomLogic / DualUnits
 *
 * Tachi and Tsurugi have a second unit, CustomLogic1, that works
 * independently of the first - two hardware gates at the same time.
 * (Kunai has a single unit, so this example uses just CustomLogic there.)
 *
 *                 IN0   IN1   OUT
 *   CustomLogic
 *     Tachi       A3    A2    A0
 *     Tsurugi     D5    D6    D10
 *     Kunai       D4    D5    D2
 *   CustomLogic1
 *     Tachi       D5    D6    D8
 *     Tsurugi     A0    A1    A3
 */
#include <CustomLogic.h>

void setup() {
  CustomLogic.begin(AND);     // unit 0: OUT high only while IN0 and IN1 are high
  #if !defined(WAZAMONO_BOARD_KUNAI)
  CustomLogic1.begin(OR);     // unit 1: OUT high while IN0 or IN1 is high
  #endif
}

void loop() {
  // Both gates run in hardware at the same time - nothing to do.
}
