/* EventSystem / CompToPin
 *
 * The analog comparator's verdict, straight to a pin: the output pin is HIGH
 * while the voltage on the AnalogComp + input is above the internal 2.5 V
 * reference. No loop() code, no analogRead(), no CPU.
 *
 * Wiring:
 *   the voltage to watch -> AnalogComp + input
 *                           Tachi: A1 / Tsurugi: D9* / Kunai: D6
 *   an LED (+ resistor)  -> the event-output pin, to GND
 *                           Tachi: D2 / Tsurugi: D8 / Kunai: D0
 *
 * (*Tsurugi: D9 doubles as the EVOUTD pin; it is free while EVOUTD is unused.)
 */
#include <AnalogComp.h>
#include <EventSystem.h>

void setup() {
  Serial.begin(115200);

  AnalogComp.begin(INTERNAL2V5);              // + input vs internal 2.5 V
  AnalogComp.setHysteresis(AC_HYST_MEDIUM);

  #if defined(WAZAMONO_BOARD_TACHI)
  EventSystem.connect(EVENT_ANALOG_COMP, 2);  // -> D2 (EVOUTA)
  #elif defined(WAZAMONO_BOARD_TSURUGI)
  EventSystem.connect(EVENT_ANALOG_COMP, 8);  // -> D8 (EVOUTA)
  #elif defined(WAZAMONO_BOARD_KUNAI)
  EventSystem.connect(EVENT_ANALOG_COMP, 0);  // -> D0 (EVOUTA)
  #else
  #error "This example supports Wazamono boards only."
  #endif
}

void loop() {
  Serial.print(F("above 2.5V: "));
  Serial.println(AnalogComp.read() ? F("yes") : F("no"));
  delay(500);
}
