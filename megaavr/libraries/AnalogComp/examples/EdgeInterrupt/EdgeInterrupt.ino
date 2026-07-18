/* AnalogComp / EdgeInterrupt
 *
 * Runs a function the instant the plus input crosses the threshold,
 * just like attachInterrupt() on a digital pin - but for an analog level.
 *
 * Plus input pin: Tachi A1 / Tsurugi D9 / Kunai D6
 * Modes: RISING (crosses upward), FALLING (crosses downward), CHANGE.
 * The threshold here is half the supply: begin(VDD, 128) -> VDD * 128/256.
 */
#include <AnalogComp.h>

volatile uint16_t crossings = 0;

void onCross() {
  crossings++;
}

void setup() {
  Serial.begin(115200);
  AnalogComp.begin(VDD, 128);                // plus pin vs half of VDD
  AnalogComp.setHysteresis(AC_HYST_MEDIUM);  // important: avoids bursts of
                                             // interrupts from a noisy signal
  AnalogComp.attachInterrupt(onCross, RISING);
}

void loop() {
  Serial.print("upward crossings: ");
  Serial.println(crossings);
  delay(1000);
}
