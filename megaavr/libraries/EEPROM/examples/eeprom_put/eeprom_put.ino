/* eeprom_put(構造体などの書き込み)
 *
 * EEPROM.put()の使い方を示すサンプルです。
 * eeprom_getサンプル用のデータを書き込む役割も兼ねています。
 *
 * 1バイト版のEEPROM.write()と違い、put()は「更新」方式で
 * 動作します。つまり値が実際に変わるバイトだけを書き込むため、
 * 不要な書き込み/消去によるEEPROMの寿命消費を避けられます。
 *
 * 原作: Christopher Andrews 2015 (MITライセンス)
 * UkiUkiduino向けに日本語化。
 */

#include <EEPROM.h>

struct MyObject {
  float field1;
  byte field2;
  char name[10];
};

void setup() {

  Serial.begin(115200);

  float f = 123.456f;  // EEPROMへ保存する値
  int eeAddress = 0;   // 保存先アドレス


  // アドレス、値の順で渡すだけで書き込める
  EEPROM.put(eeAddress, f);

  Serial.println("Written float data type!");

  /* put()は自作の構造体にも使えます。 */

  // 保存するデータ
  MyObject customVar = {
    3.14f,
    65,
    "Working!"
  };

  eeAddress += sizeof(float); // float 'f'の次のアドレスへ進める

  EEPROM.put(eeAddress, customVar);
  Serial.print("Written custom data type! \n\nView the example sketch eeprom_get to see how you can retrieve the values!");
}

void loop() {
  /* 何もしない */
}
