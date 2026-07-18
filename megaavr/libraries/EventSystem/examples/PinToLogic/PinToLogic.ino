/* EventSystem / PinToLogic
 *
 * Feed ANY pin into a CustomLogic input. The logic block's own input pins
 * are fixed - but an event connection can bring any other pin to it, and
 * the fixed input pin stays completely untouched, free for other work.
 *
 * On Kunai this matters most: CustomLogic's own pins overlap I2C, so
 * routing the inputs through events keeps the I2C bus usable.
 *
 * This sketch: OUT = (any-pin button) AND (the unit's own IN1 button)
 *
 *              any-pin button   IN1 button      OUT (LED to GND)
 *   Tachi      D9               A2              A0
 *   Tsurugi    D2               D6              D10
 *   Kunai      D8               D5              D2
 *
 * Buttons go to GND (inputs are pulled up, so unpressed = HIGH).
 */
#include <CustomLogic.h>
#include <EventSystem.h>

void setup() {
  Serial.begin(115200);

  #if defined(WAZAMONO_BOARD_TACHI)
  EventSystem.connect(9, EVENT_TO_LOGIC_A);   // D9 -> CustomLogic EVENT_A
  #elif defined(WAZAMONO_BOARD_TSURUGI)
  EventSystem.connect(2, EVENT_TO_LOGIC_A);   // D2 -> CustomLogic EVENT_A
  #elif defined(WAZAMONO_BOARD_KUNAI)
  EventSystem.connect(8, EVENT_TO_LOGIC_A);   // D8 -> CustomLogic EVENT_A
  #else
  #error "This example supports Wazamono boards only."
  #endif

  CustomLogic.setInputIN0(LOGIC_EVENT_A);     // IN0 comes from the event
  CustomLogic.begin(AND);                     // OUT = event AND IN1 pin
}

void loop() {
  Serial.println(CustomLogic.read() ? F("OUT = HIGH (no button pressed)")
                                    : F("OUT = LOW  (a button is pressed)"));
  delay(500);
}
