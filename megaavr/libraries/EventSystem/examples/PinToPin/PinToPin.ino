/* EventSystem / PinToPin
 *
 * The smallest possible use of the event system: one pin's level appears on
 * another pin, carried entirely inside the chip. No wire between them, no
 * code in loop() - and it keeps working while the CPU sleeps.
 *
 * Which pins can be used?
 *
 * OUT - only the board's fixed event-output pins (one per event output):
 *   Tachi      D2, D0, D7
 *   Tsurugi    D8, D9, A2
 *   Kunai      D0, D7
 *
 * IN - any pin, but AT MOST TWO PER PORT at the same time (the hardware has
 * two event generators per port, shared by all EventSystem connections).
 * The ports group the pins like this:
 *   Tachi      PORTA: D2  D3  D4  D14 D15 D16
 *              PORTD: D0  D1  A0  A1  A2  A3
 *              PORTF: D5  D6  D7  D8  D9  D10
 *   Tsurugi    PORTA: D0  D1  D2  D8  A4  A5
 *              PORTC: D7
 *              PORTD: D5  D6  D9  D10 D11 D12 D13
 *              PORTF: D3  D4  A0  A1  A2  A3
 *   Kunai      PORTA: D0  D2  D3  D4  D5  D8  D9  D10
 *              PORTD: D1  D6  D7  D13
 * Example: on Kunai, D8 and D9 as sources works; adding D10 (a third PORTA
 * pin) makes that connect() return false.
 *
 * This sketch:
 *              from (button)   to (LED + resistor, to GND)
 *   Tachi      D8              D2
 *   Tsurugi    D10             D8
 *   Kunai      D8              D0
 *
 * The source pin is pulled up, so with a button to GND the output is HIGH
 * until you press it.
 */
#include <EventSystem.h>

void setup() {
  Serial.begin(115200);

  #if defined(WAZAMONO_BOARD_TACHI)
  bool ok = EventSystem.connect(8, 2);    // D8 -> D2 (EVOUTA)
  #elif defined(WAZAMONO_BOARD_TSURUGI)
  bool ok = EventSystem.connect(10, 8);   // D10 -> D8 (EVOUTA)
  #elif defined(WAZAMONO_BOARD_KUNAI)
  bool ok = EventSystem.connect(8, 0);    // D8 -> D0 (EVOUTA)
  #else
  #error "This example supports Wazamono boards only."
  #endif

  Serial.println(ok ? F("connected - the output pin now follows the button")
                    : F("connect() failed"));
}

void loop() {
  /* Nothing to do - the connection is pure hardware. */
}
