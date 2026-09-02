/* ClockOut_Basic - システムクロックをCLKOUTピンに出力する
 *
 * CLK_PER(周辺/CPUクロック)をPA7に出力します。外部デバイスとこの
 * ボードの時間基準を共有したり、オシロや周波数カウンタで実際の
 * システムクロックを測ったりできます。
 *
 * UkiUkiduinoのCLKOUTピン: D2 (PA7)
 *
 * UkiUkiduinoはクリスタルレスで、内蔵発振器をUSBのフレーム信号に
 * 合わせて自動調整しています。この出力にカウンタをつなぎ、読みを
 * frequency()の値と突き合わせれば、調整が効いていることを手軽に
 * 確認できます。
 *
 * UkiUkiduino向けに日本語化
 */

#include <ClockOut.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) { }            // ネイティブUSB CDC: モニタの接続を待つ

  if (ClockOut.begin()) {
    Serial.print(F("CLKOUT running on D"));
    Serial.print(ClockOut.pin());
    Serial.print(F(" at "));
    Serial.print(ClockOut.frequency());
    Serial.println(F(" Hz"));
  } else {
    // PA7がAC0の出力かイベント出力EVOUTAに既に使われている
    Serial.println(F("CLKOUT unavailable: PA7 is in use by another peripheral"));
  }
}

void loop() {
  // クロック故障検出(CFD)イベントが起きると出力はハードウェアで
  // 止まるので、動いている前提にせずisRunning()で確認する価値がある。
  if (!ClockOut.isRunning()) {
    Serial.println(F("CLKOUT has stopped (clock failure?)"));
    delay(1000);
  }
}
