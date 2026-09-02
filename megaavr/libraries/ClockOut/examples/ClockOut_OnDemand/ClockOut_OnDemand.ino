/* ClockOut_OnDemand - 必要なときだけCLKOUTを動かす
 *
 * エッジの速い24MHzの矩形波は強いEMI源であり、ピンドライバの消費
 * 電流も増えます。製品ではクロック出力を常時ONにせず、必要な
 * トランザクションの前後だけ有効にする方が一般に得策です。
 *
 * このスケッチは「変換中だけクロックが欲しい外部デバイス」と
 * 通信する体で、クロックを開始→処理→クロック停止、を繰り返します。
 *
 * UkiUkiduinoのCLKOUTピン: D2 (PA7)
 *
 * UkiUkiduino向けに日本語化
 */

#include <ClockOut.h>

void readExternalDevice() {
  if (!ClockOut.begin()) {
    Serial.println(F("skipped: PA7 in use"));
    return;
  }

  delayMicroseconds(50);        // 外部デバイスに安定したクロックを見せる
  // ... ここでデバイスと通信する(SPI/I2C/GPIO) ...
  delay(5);                     // デバイスの変換時間の代わり

  ClockOut.end();               // PA7を解放し、放射を止める
  Serial.println(F("done (clock off)"));
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { }
  Serial.print(F("CLKOUT would run at "));
  Serial.print(ClockOut.frequency());
  Serial.println(F(" Hz when enabled"));
}

void loop() {
  readExternalDevice();
  delay(1000);
}
