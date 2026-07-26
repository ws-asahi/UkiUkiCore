/* ClockOut_OnDemand - run CLKOUT only while it is actually needed.
 *
 * A 24 MHz square wave with fast edges is a strong EMI source and costs
 * current in the pin driver, so a product is usually better off enabling the
 * clock output around the transaction that needs it instead of leaving it on
 * permanently.
 *
 * This sketch pretends to talk to an external device that wants a clock only
 * while it converts: the clock is started, the work happens, the clock is
 * stopped again.
 *
 * CLKOUT pin: Tachi D8 / Tsurugi D2 / Kunai D1.
 */

#include <ClockOut.h>

void readExternalDevice() {
  if (!ClockOut.begin()) {
    Serial.println(F("skipped: PA7 in use"));
    return;
  }

  delayMicroseconds(50);        // let the external device see a stable clock
  // ... talk to the device here (SPI/I2C/GPIO) ...
  delay(5);                     // stand-in for the device's conversion time

  ClockOut.end();               // release PA7 and stop radiating
  Serial.println(F("done (clock off)"));
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { }
  Serial.print(F("CLKOUT would run at "));
  Serial.print(ClockOut.frequency());
  Serial.println(F(" Hz when enabled"));
}

void loop() {
  readExternalDevice();
  delay(1000);
}
