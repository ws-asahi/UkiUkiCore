/* EventSystem / CompToPin(コンパレータからピンへ)
 *
 * アナログコンパレータの判定結果を、そのままピンへ届けます。
 * AnalogCompの+入力の電圧が内蔵2.5V基準を上回っている間、出力ピンが
 * HIGHになります。loop()のコードも、analogRead()も、CPUも使いません。
 *
 * 接続:
 *   監視したい電圧      -> AnalogComp +入力 = D9
 *   LED(+抵抗、GNDへ)   -> イベント出力ピン = D8
 *
 * (D9はEVOUTDピンとしても使われるピンですが、EVOUTDを使っていない
 *  限り+入力として自由に使えます。)
 */
#include <AnalogComp.h>
#include <EventSystem.h>

void setup() {
  Serial.begin(115200);

  AnalogComp.begin(INTERNAL2V5);              // +入力 vs 内蔵2.5V基準
  AnalogComp.setHysteresis(AC_HYST_MEDIUM);

  EventSystem.connect(EVENT_ANALOG_COMP, 8);  // 判定結果 -> D8(EVOUTA)
}

void loop() {
  Serial.print(F("above 2.5V: "));
  Serial.println(AnalogComp.read() ? F("yes") : F("no"));
  delay(500);
}
