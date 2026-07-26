/* ClockOut_Basic - put the system clock on the CLKOUT pin.
 *
 * Drives CLK_PER (the peripheral/CPU clock) out on PA7 so an external device
 * can share this board's time base, or so you can measure the real system
 * clock with a scope or frequency counter.
 *
 * CLKOUT pin by board:
 *   Wazamono Tachi   -> D8
 *   Wazamono Tsurugi -> D2
 *   Wazamono Kunai   -> D1
 *
 * On the crystal-less Kunai, whose internal oscillator is tuned against the
 * USB frame marker, this is a convenient way to confirm the tuning is
 * working: connect a counter and check the reading against frequency().
 */

#include <ClockOut.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) { }            // native USB CDC: wait for the monitor

  if (ClockOut.begin()) {
    Serial.print(F("CLKOUT running on D"));
    Serial.print(ClockOut.pin());
    Serial.print(F(" at "));
    Serial.print(ClockOut.frequency());
    Serial.println(F(" Hz"));
  } else {
    // PA7 is already driven by AC0's output, by the event output EVOUTA,
    // or (on the Kunai) by an enabled SPI0 that needs it as SS.
    Serial.println(F("CLKOUT unavailable: PA7 is in use by another peripheral"));
  }
}

void loop() {
  // A Clock Failure Detection event switches the output off in hardware,
  // so isRunning() is worth checking rather than assuming.
  if (!ClockOut.isRunning()) {
    Serial.println(F("CLKOUT has stopped (clock failure?)"));
    delay(1000);
  }
}
