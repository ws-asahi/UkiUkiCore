/* CustomLogic / AnalogCompInput(コンパレータを入力にする)
 *
 * 入力はピンからでなくても構いません。ここではIN0をチップ内蔵の
 * アナログコンパレータから直接供給します。ゲートはこうなります:
 *
 *     OUT = (電圧が2.5Vを超えている) AND (IN1ピンがHIGH)
 *
 * コンパレータとロジックの間に配線はありません - 接続はチップの
 * 内部で行われ、ピンを消費せず、CPUが何もしなくても動き続けます。
 *
 * 配線(UkiUkiduino):
 *   監視したい電圧      -> AnalogCompの+入力ピン = D9
 *   押しボタン          -> CustomLogicのIN1ピン = D6 からGNDへ
 *   LED(+抵抗)          -> CustomLogicのOUTピン = D10 からGNDへ
 *
 * 入力はプルアップ済みなので、電圧が2.5Vを超えていて、かつボタンを
 * 押していない間LEDが点灯します。ボタンを押すと消えます。
 *
 * メモ: コンパレータは内蔵2.5V基準と比較するため、-入力ピンは
 * 空いたままです - このピン(PD3)はロジックブロックがOUT(D10)に
 * 使うのと同じピンなので、ちょうど好都合です。
 *
 * UkiUkiduino向けに日本語化
 */
#include <AnalogComp.h>
#include <CustomLogic.h>

void setup() {
  Serial.begin(115200);

  AnalogComp.begin(INTERNAL2V5);              // +入力 vs 内蔵2.5V
  AnalogComp.setHysteresis(AC_HYST_MEDIUM);   // 境界付近のノイズを無視する
  // メモ: AnalogComp.enableOutput()は不要です - ロジックブロックは
  // ピンを介さずコンパレータの結果を直接読みます。

  CustomLogic.setInputIN0(LOGIC_ANALOG_COMP); // IN0 = コンパレータの結果
  CustomLogic.begin(AND);                     // OUT = コンパレータ AND IN1ピン
}

void loop() {
  Serial.print(F("voltage above 2.5V: "));
  Serial.print(AnalogComp.read() ? F("yes") : F("no "));
  Serial.print(F("   gate output: "));
  Serial.println(CustomLogic.read() ? F("HIGH") : F("LOW"));
  delay(500);
}
