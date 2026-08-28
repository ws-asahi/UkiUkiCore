/*
  Yes, this library *really* can drive 12 servos from a single type B timer!
*/

#include <Servo.h>

/* Twelve pins that exist on each Wazamono board. Tachi has no D11-D13 and
 * Kunai has no D13/D14, so the last slots differ per board. */
#if defined(WAZAMONO_AVR_TACHI)
const byte servopins[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 14};
#elif defined(WAZAMONO_AVR_TSURUGI)
const byte servopins[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
#elif defined(WAZAMONO_AVR_KUNAI)
const byte servopins[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
#else
  #error "This example supports Wazamono boards only."
#endif

Servo myservos[12];
byte pos[12];
char dir[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

void setup() {
  for (byte i = 0; i < 12; i++) {
    myservos[i].attach(servopins[i]);
    pos[i] = i * 15;
  }
}

void loop() {
  for (byte i = 0; i < 12; i++) {
    myservos[i].write(pos[i]);
    pos[i] += dir[i];
    if (pos[i] == 180) {
      dir[i] = -1;
    }
    if (pos[i] == 0) {
      dir[i] = 1;
    }
  }
  delay(30);                       // waits 30ms for the servo to reach the position
}
