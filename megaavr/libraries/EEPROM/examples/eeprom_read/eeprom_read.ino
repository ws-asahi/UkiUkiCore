/* EEPROM Read(全バイト読み出し)
 *
 * EEPROMの各バイトの値を読み出し、シリアルモニタへ表示します。
 * 原作はパブリックドメイン。UkiUkiduino向けに日本語化。
 */

#include <EEPROM.h>

// EEPROMの先頭(アドレス0)から読み始める
int address = 0;
byte value;

void setup() {
  Serial.begin(115200);
}

void loop() {
  // 現在のアドレスから1バイト読む
  value = EEPROM.read(address);

  Serial.print(address);
  Serial.print("\t");
  Serial.print(value, DEC);
  Serial.println();

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

  delay(500);
}
