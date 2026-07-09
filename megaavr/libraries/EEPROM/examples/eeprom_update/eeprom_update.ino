/* EEPROM Update(更新方式の書き込み)
 *
 * アナログ入力A0の読み取り値をEEPROMへ保存します。
 * 保存した値は電源を切っても残り、別のスケッチから後で
 * 読み出せます。
 *
 * update()は、EEPROM上の値と書き込む値が同じ場合には
 * 書き込みを行いません。これにより不要な書き込み/消去による
 * EEPROMの寿命消費を避けられます。
 *
 * 原作はMITライセンス。UkiUkiduino向けに日本語化。
 */

#include <EEPROM.h>

#define ANALOG_PIN A0   // 読み取るアナログ入力ピン

/* 現在のEEPROMアドレス(次に書き込む位置) */
int address = 0;

void setup() {
  /* 何もしない */
}

void loop() {
  /*
   * アナログ入力は0~1023の値を返しますが、EEPROMの1バイトには
   * 0~255しか入らないため、4で割って格納します。
   */
  int val = analogRead(ANALOG_PIN) / 4;

  /*
   * EEPROMの該当バイトを更新する。
   * 保存した値は電源を切っても残ります。
   */
  EEPROM.update(address, val);

  /*
   * EEPROM.update(address, val)は次のコードと等価です:
   *
   * if (EEPROM.read(address) != val) {
   *   EEPROM.write(address, val);
   * }
   */

  /*
   * EEPROMの容量はマイコンによって異なります。
   * UkiUkiduino(AVR64DU32)の容量は256バイトです。
   *
   * 容量を数値で直書きせず、EEPROM.length()を使うことで、
   * どのAVRでもそのまま動くコードになります。
   */

  address = address + 1;
  if (address == EEPROM.length()) {
    address = 0;
  }

  /*
   * EEPROM容量は2のべき乗なので、アドレスの折り返し(あふれ防止)は
   * 「容量-1」とのビットANDでも書けます。
   *
   * ++address &= EEPROM.length() - 1;
   */

  delay(100);
}
