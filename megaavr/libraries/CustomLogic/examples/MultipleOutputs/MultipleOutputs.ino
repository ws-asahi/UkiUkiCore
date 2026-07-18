/* CustomLogic / MultipleOutputs
 *
 * The result of a logic block does not have to stay on its own OUT pin.
 * addOutput() sends the same result to another pin as well - the library
 * wires it through the event system for you, so it appears on both pins at
 * the same instant, still with no CPU involvement at all.
 *
 * Output pins of the CustomLogic unit (event-output pins are FIXED per
 * board by its pin-configuration table):
 *                   OUT           OUT (alt)     event outputs
 *   Tachi           A0  (PD3)     D1  (PD6)     D2, D0, D7
 *   Tsurugi         D10 (PD3)     D13 (PD6)     D8, D9, A2
 *   Kunai           D2  (PA3)     D8  (PA6)     D0, D7
 *
 * setOutput(pin)  - the result appears on that pin only
 * addOutput(pin)  - ...and on this one too (dedicated pin + one pin per port)
 * disableOutput() - nowhere; the block still works for interrupts and for
 *                   feeding the other unit
 *
 * Here the AND gate drives its own OUT pin AND the event output on PA2, so
 * two LEDs (or an LED and a downstream circuit) follow the same result.
 */
#include <CustomLogic.h>

void setup() {
  Serial.begin(115200);

  CustomLogic.begin(AND);          // OUT pin, as usual

  #if defined(WAZAMONO_BOARD_TACHI)
    CustomLogic.addOutput(2);      // ...and also on D2 (PA2, EVOUTA)
  #elif defined(WAZAMONO_BOARD_TSURUGI)
    CustomLogic.addOutput(8);      // ...and also on D8 (PA7, EVOUTA)
  #elif defined(WAZAMONO_BOARD_KUNAI)
    CustomLogic.addOutput(0);      // ...and also on D0 (PA7, EVOUTA)
  #else
    #error "This example supports Wazamono boards only."
  #endif
}

void loop() {
  Serial.println(CustomLogic.read() ? F("both output pins HIGH")
                                    : F("both output pins LOW"));
  delay(500);
}
