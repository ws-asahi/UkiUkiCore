/* EventSystem / CompToPin
 *
 * The analog comparator's verdict, straight to a pin: the output pin is HIGH
 * while the voltage on the AnalogComp + input is above the internal 2.5 V
 * reference. No loop() code, no analogRead(), no CPU.
 *
 * Wiring:
 *   the voltage to watch -> AnalogComp + input
 *                           Tachi: D9* / Tsurugi: D9* / Kunai: D2
 *   an LED (+ resistor)  -> the event-output pin, to GND
 *                           Tachi: D8 / Tsurugi: D2 / Kunai: D1
 *
 * (*Tachi/Tsurugi: D9 doubles as the EVOUTD pin; it is free while EVOUTD is unused.)
 */
#include <AnalogComp.h>
#include <EventSystem.h>

void setup() {
  Serial.begin(115200);

  AnalogComp.begin(INTERNAL2V5);              // + input vs internal 2.5 V
  AnalogComp.setHysteresis(AC_HYST_MEDIUM);

  #if defined(ARDUINO_AVR_TACHI)
  EventSystem.connect(EVENT_ANALOG_COMP, 8);  // -> D8 (EVOUTA)
  #elif defined(ARDUINO_AVR_TSURUGI)
  EventSystem.connect(EVENT_ANALOG_COMP, 2);  // -> D2 (EVOUTA)
  #elif defined(ARDUINO_AVR_KUNAI)
  EventSystem.connect(EVENT_ANALOG_COMP, 1);  // -> D1 (EVOUTA)
  #else
  #error "This example supports Wazamono boards only."
  #endif
}

void loop() {
  Serial.print(F("above 2.5V: "));
  Serial.println(AnalogComp.read() ? F("yes") : F("no"));
  delay(500);
}
