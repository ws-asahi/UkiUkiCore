/* EEPROM Clear(EEPROM全消去)
 *
 * EEPROMの全バイトを0xFF(消去状態)にします。
 * EEPROMを順に処理する方法の詳細はeeprom_iterationも参照してください。
 *
 * 原作はパブリックドメイン。UkiUkiduino向けに日本語化。
 */

#include <EEPROM.h>

void setup() {
  // 完了表示用にオンボードLED(D13)を出力に設定する
  pinMode(LED_BUILTIN, OUTPUT);

  /*
   * EEPROMの全バイトを順に処理する。
   *
   * EEPROMの容量はマイコンによって異なります。
   * UkiUkiduino(AVR64DU32)の容量は256バイトです。
   * (参考: Arduino Uno R3のATmega328Pは1024バイト)
   *
   * 容量を数値で直書きせず、EEPROM.length()を使うことで、
   * どのAVRでもそのまま動くコードになります。
   */

  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0xFF);
  }

  // 消去が終わったらLEDを点灯する
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  /* 何もしない */
}
