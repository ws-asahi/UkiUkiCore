/* eeprom_get(構造体などの読み出し)
 *
 * EEPROM.get()の使い方を示すサンプルです。
 *
 * 事前にeeprom_putを実行してEEPROMへデータを書き込んでおいて
 * ください。実行していなくても動きますが、その場合はEEPROMに
 * 元々入っていた値(不定値)が表示されます。文字列の途中に終端
 * 文字が無いと、シリアルに長いゴミが表示されることがあります。
 *
 * 原作: Christopher Andrews 2015 (MITライセンス)
 * UkiUkiduino向けに日本語化。
 */

#include <EEPROM.h>

void setup() {

  float f = 0.00f;   // EEPROMから読んだ値を入れる変数
  int eeAddress = 0; // 読み出しを開始するEEPROMアドレス

  Serial.begin(115200);

  Serial.print("Read float from EEPROM: ");

  // アドレス'eeAddress'からfloat値を読み出す
  EEPROM.get(eeAddress, f);
  Serial.println(f, 3);    // 有効なfloatでない場合'ovf'や'nan'と表示されることがある

  /*
   * get()は'f'への参照も返すので、次のように一行でも書けます。
   * 例: Serial.print(EEPROM.get(eeAddress, f));
   */

  /*
   * get()は自作の構造体にも使えます。
   * その例を別関数に分けています。
   */

  secondTest(); // 次のテストを実行する
}

struct MyObject {
  float field1;
  byte field2;
  char name[10];
};

void secondTest() {
  int eeAddress = sizeof(float); // float 'f'の次のアドレスへ進める

  MyObject customVar; // EEPROMから読んだ構造体を入れる変数
  EEPROM.get(eeAddress, customVar);

  Serial.println("Read custom object from EEPROM: ");
  Serial.println(customVar.field1);
  Serial.println(customVar.field2);
  Serial.println(customVar.name);
}

void loop() {
  /* 何もしない */
}
