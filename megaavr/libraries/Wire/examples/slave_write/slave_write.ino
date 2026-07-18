/* Wire Slave Write(スレーブ: データ応答)
 *
 * アドレス0x54宛のデータ要求に対して、最大4バイトのデータ
 * (現在のmillis()値)を返すスケッチです。
 *
 * millisが無効の場合は、代わりに常に1234567890
 * (16進で0x499602D2)を返します。
 *
 * 使い方: このボードのSCL(A5)/SDA(A4)を、Master Readサンプルを
 * 実行しているもう1台のボードのSCL/SDAへ接続します。
 *
 * SDA/SCLの両方に、Vccへのプルアップ抵抗が必要です。
 * 詳しくはWireライブラリのREADME.mdを参照してください。
 *
 * UkiUkiduino向けに日本語化
 */

#include <Wire.h>

void setup() {
  Wire.begin(0x54);                 // アドレス0x54のスレーブとして初期化する
  Wire.onRequest(transmitDataWire); // 要求時に呼ばれる関数を登録する
}

void loop() {

}

void transmitDataWire() {
  #if !defined(MILLIS_USE_TIMERNONE)
  uint32_t ms = millis();
  #else
  uint32_t ms = 123456789UL; // millis無効時の代わりの値
  #endif
  Wire.write((uint8_t) ms);
  Wire.write((uint8_t)(ms >> 8));
  Wire.write((uint8_t)(ms >> 16));
  Wire.write((uint8_t)(ms >> 24));
}
