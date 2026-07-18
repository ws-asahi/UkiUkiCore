/* CustomLogic / EdgeInterrupt
 *
 * Runs a function the moment the gate output changes - the logic is
 * evaluated in hardware, and your code is only called on the edges.
 *
 * Here a 2-input NAND is used as an "alarm": with the pull-ups, OUT
 * falls to LOW only at the moment both inputs are HIGH (both buttons
 * released) - and onAlarm() counts how many times that happened.
 *
 * Pins: same as TwoInputAND (IN0/IN1/OUT).
 */
#include <CustomLogic.h>

volatile uint16_t alarms = 0;

void onAlarm() {
  alarms++;
}

void setup() {
  Serial.begin(115200);
  CustomLogic.begin(NAND);                       // OUT = NOT (IN0 AND IN1)
  CustomLogic.attachInterrupt(onAlarm, FALLING); // both inputs became HIGH
}

void loop() {
  Serial.print("alarm count: ");
  Serial.println(alarms);
  delay(1000);
}
