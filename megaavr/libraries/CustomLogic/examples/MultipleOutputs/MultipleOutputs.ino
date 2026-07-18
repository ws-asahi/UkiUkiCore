/* CustomLogic / MultipleOutputs(複数ピンへの同時出力)
 *
 * ロジックブロックの結果は自分のOUTピンだけに留まりません。
 * addOutput()で同じ結果を別のピンにも出せます - ライブラリが
 * イベントシステム経由の配線を代行するので、両方のピンに同じ瞬間に
 * 現れ、CPUはやはり一切関与しません。
 *
 * CustomLogicユニットの出力ピン(UkiUkiduino。イベント出力ピンは
 * ボードのピン構成表で固定されています):
 *   OUT = D10 (PD3)    OUT(代替) = D13 (PD6)
 *   イベント出力に使えるピン = D8, D9, A2
 *
 * setOutput(pin)  - 結果をそのピンだけに出す
 * addOutput(pin)  - ...に加えてこのピンにも出す(専用ピン+各ポート1本)
 * disableOutput() - どこにも出さない。割り込みや、もう一方のユニットへの
 *                   供給には引き続き使える
 *
 * ここではANDゲートが自分のOUTピン(D10)とイベント出力D8の両方を
 * 駆動します。2つのLED(またはLEDと後段回路)が同じ結果に追従します。
 *
 * UkiUkiduino向けに日本語化
 */
#include <CustomLogic.h>

void setup() {
  Serial.begin(115200);

  CustomLogic.begin(AND);          // OUTピンへはいつも通り出力
  CustomLogic.addOutput(8);        // ...加えてD8(PA7, EVOUTA)にも出力
}

void loop() {
  Serial.println(CustomLogic.read() ? F("both output pins HIGH")
                                    : F("both output pins LOW"));
  delay(500);
}
