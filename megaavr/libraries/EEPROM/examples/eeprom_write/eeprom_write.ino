/* EEPROM Write(書き込み)
 *
 * アナログ入力A0の読み取り値をEEPROMへ保存します。
 * 保存した値は電源を切っても残り、別のスケッチから後で
 * 読み出せます。
 *
 * 注意: write()は値が同じでも毎回書き込みます。通常は寿命に
 * 優しいEEPROM.update()の使用を推奨します(eeprom_update参照)。
 *
 * 原作はパブリックドメイン。UkiUkiduino向けに日本語化。
 */

#include <EEPROM.h>

#define ANALOG_PIN A0   // 読み取るアナログ入力ピン

/* 現在のEEPROMアドレス(次に書き込む位置) */
int addr = 0;

void setup() {
  // 動作表示用にオンボードLED(D13)を出力に設定する
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  /* アナログ入力は0~1023の値を返しますが、EEPROMの1バイトには
   * 0~255しか入らないため、4で割って格納します。
   */

  int val = analogRead(ANALOG_PIN) / 4;

  /* EEPROMの該当バイトへ書き込む。
   * 保存した値は電源を切っても残ります。
   */

  EEPROM.write(addr, val);

  /*
   * EEPROMの容量はマイコンによって異なります。
   * UkiUkiduino(AVR64DU32)の容量は256バイトです。
   *
   * 容量を数値で直書きせず、EEPROM.length()を使うことで、
   * どのAVRでもそのまま動くコードになります。
   */

  addr = addr + 1;
  if (addr == EEPROM.length()) { // EEPROM全体に書き込み終えた
    while (1); // これ以上寿命を消費しないよう、ここで停止する
  }

  /*
   * EEPROM容量は2のべき乗なので、アドレスの折り返し(あふれ防止)は
   * 「容量-1」とのビットANDでも書けます。
   *
   * ++addr &= EEPROM.length() - 1;
   */

  digitalWrite(LED_BUILTIN, HIGH); // 動作表示としてLEDを点灯する
  delay(2000);
}
