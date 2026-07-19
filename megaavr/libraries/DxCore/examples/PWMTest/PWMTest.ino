/* ============================================================
 * PWMTest - コア開発者向けのPWMセルフテスト
 *
 * initTCx()/analogWrite()/turnOffPWM()が正しいピンへ正しい周波数で
 * PWMを出しているかを実機で検証する、コア保守用のテストスケッチです。
 * 一般の利用者向けサンプルではないため、以下の本文コメントは
 * 原文(英語)のまま保持しています。
 * ============================================================ */
/******************************************************************************
 * PWM self-test for the Wazamono boards (AVR DU-series).
 *
 * This sketch verifies that the core is correctly outputting PWM on the pins
 * that the muxes are set to. It also measures the frequency of the PWM
 * (somewhat crudely, but easily good enough to see if you're on target).
 *
 * This is designed specifically to test initTCx(), analogWrite() and
 * turnOffPWM() (via digitalWrite). While the syntactical correctness is
 * tested every time you compile, whether the behavior is correct is another
 * matter.
 *
 * First, it prints the table of pins within each pinset of TCA0. 255 is
 * NOT_A_PIN (either the pin does not exist on this board, or it is not
 * available - e.g. the crystal pins on Tachi/Tsurugi, or PC3 which is the
 * VBUS divider on Tachi/Kunai; on Tsurugi PC3 is the plain GPIO D7).
 *
 * Then, starting with TCA0, it looks at the PORTMUX register to see which
 * pinset it is on, looks up the pins in the table, and analogWrite()s each
 * pin to start PWM for 1 second, polling the port input register and
 * counting transitions. It then calls takeOverTCA0() and restores CTRLA so
 * the timer keeps serving as a clock source for the TCBs, and tests the two
 * TCBs (skipping the one used by millis).
 *
 * At the end, it reports the total number of tests that passed, failed, or
 * were skipped. Pins not present are not recorded as tests or skips, but a
 * pin blocked off by millis is counted as an attempt.
 *****************************************************************************/

/* Key Parameters - set these to match your wiring: */

// Adjust as needed
#define MYSERIAL Serial

/* End of parameters/options section */
#include "FullSizeAVR_Timer.h"
#include <EEPROM.h>

void printTCA0Status() {
  uint16_t test = (uint16_t)&TCA0;
  volatile uint8_t *bp;
  bp = (volatile uint8_t *)test;
  for (uint8_t i = 4; i; i--) {
    if (i != 4) {
      MYSERIAL.println();
    }
    bp = MYSERIAL.printHex(bp, (uint8_t) 16, ':');
  }
  MYSERIAL.println();
}

uint8_t SuccessCount = 0;
uint8_t AttemptCount = 0;
uint8_t SkipCount = 0;
uint8_t tcactrla = 0;

uint8_t CurrentPortmux = 255;
uint8_t CurrentTimer = _tca0; // TCA0
static uint8_t CurrentChannel = 0;
uint8_t CurrentPin = NOT_A_PIN;
uint8_t CurrentTimerIndex = 0;
void setup() {
  // put your setup code here, to run once:
  VPORTA.DIR |= 0x10;
  MYSERIAL.begin(115200);
  delay(100);
  MYSERIAL.println("PWM selftest");
  CurrentTimer = MyTimers[0];
  delay(1000);
  for (byte x = 0; x < 7; x++) {
    for (byte y = 0; y < 6; y++) {
      MYSERIAL.print(TCA0pinsets[6 * x + y]);
      MYSERIAL.print(", ");
    }
    MYSERIAL.println();
  }
  MYSERIAL.println();
}


void showDone() {
  MYSERIAL.println("The test has completed");
  MYSERIAL.print("Pins tested: ");
  MYSERIAL.println(AttemptCount);
  MYSERIAL.print("Pass: ");
  MYSERIAL.println(SuccessCount);
  MYSERIAL.print("Fail: ");
  MYSERIAL.println(AttemptCount - SuccessCount - SkipCount);
  MYSERIAL.print("Skipped: ");
  MYSERIAL.println(SkipCount);
  if (SuccessCount + SkipCount < AttemptCount) {
    MYSERIAL.print("Test FAILED!");
    MYSERIAL.print(AttemptCount - SuccessCount - SkipCount);
    MYSERIAL.print('/');
    MYSERIAL.print(AttemptCount - SkipCount);
    MYSERIAL.println(" failed!");
  } else {
    MYSERIAL.println("Test PASSED!");
  }

  MYSERIAL.println();
}
void loop() {
  // put your main code here, to run repeatedly:
  uint8_t timertype = CurrentTimer & 0xF0;
  _swap(timertype);
  uint8_t timernum = CurrentTimer & 0x0F;
  uint8_t currentpin = NOT_A_PIN;
  switch (timertype) {
    case 0: {
        //TCA0
        CurrentPortmux = ((PORTMUX.TCAROUTEA & 0x07));
        MYSERIAL.print("Testing TCA");
        MYSERIAL.print(timernum);
        MYSERIAL.print(" mux number ");
        MYSERIAL.print(CurrentPortmux);
        MYSERIAL.print(" channel ");
        MYSERIAL.println(CurrentChannel);
        currentpin = TCA0pinsets[6 * CurrentPortmux + CurrentChannel];
        MYSERIAL.print("Pin is ");
        MYSERIAL.println(currentpin);
        break;
      }

    case 1: {
        //TCB
        if ((MILLIS_TIMER & 0xF0) == 0x10) {  // TCB family is encoded 0x10-0x1F (TIMERB0..B7); was incorrectly 0x20 (TCF range)
          if ((MILLIS_TIMER & 0x07) == timernum) {
            MYSERIAL.print("Millis is using TCB");
            SkipCount++;
            AttemptCount++;
            currentpin = 254;
            break;
          }
        }
        MYSERIAL.println();
        MYSERIAL.print("Testing TCB");
        MYSERIAL.print(timernum);
        CurrentPortmux = (PORTMUX.TCBROUTEA & (1 << timernum)) ? 1 : 0;
        currentpin = TCBpinsets[(timernum << 1) + CurrentPortmux];
        if (CurrentPortmux) {
          MYSERIAL.print(" AltPin ");
        } else {
          MYSERIAL.print(" DefaultPin ");
        }
        MYSERIAL.println(currentpin);
        break;
      }
    case 255: {
        MYSERIAL.println("Done!");
        while (1);
      }
    default: {
        MYSERIAL.println("Timer not present on this part");
      }
  }
  if (currentpin == NOT_A_PIN || currentpin == 254) {
    MYSERIAL.println("The current configuration does not permit PWM from this timer channel.");
  } else {
    uint16_t tglcount = 0;
    PORT_t *thisport = portToPortStruct(digitalPinToPort(currentpin));
    uint8_t bit_mask = digitalPinToBitMask(currentpin);
    if (bit_mask == NOT_A_PIN) {
      MYSERIAL.println("Pin does not exist on this part");
    } else {
      uint8_t laststate = ((thisport->IN) & bit_mask) ? 1 : 0;
      MYSERIAL.flush();
      AttemptCount++;
      analogWrite(currentpin, 129);
      uint32_t startmillis = millis();
      while (millis() - startmillis < 1000) {
        if ((((thisport->IN) & bit_mask) ? 1 : 0) != laststate) {
          tglcount++;
          laststate = !laststate;
        }
      }
      MYSERIAL.println(digitalPinToPort(currentpin));
      MYSERIAL.print("One second togglecount: ");
      MYSERIAL.println(tglcount);
      MYSERIAL.print("Freq = ");
      MYSERIAL.print(tglcount >> 1);
      MYSERIAL.println(" Hz.");
      digitalWrite(currentpin, LOW);
      if (tglcount > 900 && tglcount < 4000) {
        tglcount = 0;
        laststate = 0;
        while (millis() - startmillis < 1000) {
          if ((((thisport->IN) & bit_mask) ? 1 : 0) != laststate) {
            tglcount++;
            laststate = !laststate;
          }
        }
        if (tglcount < 2) {
          SuccessCount++;
        } else {
          MYSERIAL.print("Uhoh, saw additional pulses: ");
          MYSERIAL.println(tglcount);
        }
      }
    }
  }
  MYSERIAL.println("--------------------");
  switch (timertype) {
    case 0: {
        //TCA0
        if (CurrentChannel >= 5) {
          MYSERIAL.println();
          CurrentChannel = 0;
          CurrentTimerIndex++;
          if (CurrentTimerIndex == 1) {
            tcactrla = TCA0.SPLIT.CTRLA;
            takeOverTCA0();
            TCA0.SINGLE.CTRLA = tcactrla;
          }
        } else {
          CurrentChannel++;
        }
        break;
      }
    case 1: {
        //TCB
        // Just go to the next one.
        CurrentTimerIndex++;
        break;
      }
    case 255:
    default: {
        showDone();
        while (1);
      }
  }

  CurrentTimer = MyTimers[CurrentTimerIndex];
  if (CurrentTimer == 255) {
    showDone();
    while (1);
  }
}
