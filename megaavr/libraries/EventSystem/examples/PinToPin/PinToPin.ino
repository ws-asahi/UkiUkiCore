/* EventSystem / PinToPin
 *
 * The smallest possible use of the event system: one pin's level appears on
 * another pin, carried entirely inside the chip. No wire between them, no
 * code in loop() - and it keeps working while the CPU sleeps.
 *
 * Which pins can be used?
 *
 * OUT - only the board's fixed event-output pins (one per event output):
 *   Tachi      D8, D9, A3
 *   Tsurugi    D2, D9, A2
 *   Kunai      D1, D3
 *
 * IN - any pin, but AT MOST TWO PER PORT at the same time (the hardware has
 * two event generators per port, shared by all EventSystem connections).
 * The ports group the pins like this:
 *   Tachi      PORTA: D0  D1  D2  D3  D7  D8
 *              PORTC: D4
 *              PORTD: D5  D6  D9  D10 D14 D15 D16 A0
 *              PORTF: A1  A2  A3  A4  A5  D17
 *   Tsurugi    PORTA: D0  D1  D2  D3  A4  A5
 *              PORTC: D4
 *              PORTD: D5  D6  D9  D10 D11 D12 D13 AREF
 *              PORTF: D7  D8  A0  A1  A2  A3
 *   Kunai      PORTA: D1  D4  D5  D6  D7  D8  D9  D10
 *              PORTC: D0
 *              PORTD: D2  D3  D11 D12
 * Example: on Kunai, D8 and D9 as sources works; adding D10 (a third PORTA
 * pin) makes that connect() return false.
 *
 * This sketch:
 *              from (button)   to (LED + resistor, to GND)
 *   Tachi      D10             D8
 *   Tsurugi    D10             D2
 *   Kunai      D8              D1
 *
 * The source pin is pulled up, so with a button to GND the output is HIGH
 * until you press it.
 */
#include <EventSystem.h>

void setup() {
  Serial.begin(115200);

  #if defined(ARDUINO_AVR_TACHI)
  bool ok = EventSystem.connect(10, 8);   // D10 -> D8 (EVOUTA)
  #elif defined(ARDUINO_AVR_TSURUGI)
  bool ok = EventSystem.connect(10, 2);   // D10 -> D2 (EVOUTA)
  #elif defined(ARDUINO_AVR_KUNAI)
  bool ok = EventSystem.connect(8, 1);    // D8 -> D1 (EVOUTA)
  #else
  #error "This example supports Wazamono boards only."
  #endif

  Serial.println(ok ? F("connected - the output pin now follows the button")
                    : F("connect() failed"));
}

void loop() {
  /* Nothing to do - the connection is pure hardware. */
}
