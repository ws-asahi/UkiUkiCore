/* AnalogComp / ReadState(状態の読み取り)
 *
 * +入力ピンと-入力ピンの電圧を比較し、結果を表示します。
 * +側の電圧が-側より高い間trueになります。ADCも計算も使わない、
 * ハードウェアによる電圧比較です。
 *
 * 既定の入力ピン:
 *   +入力 = D9  (PD2)
 *   -入力 = D10 (PD3)
 *
 * 試してみよう: -入力を可変抵抗のワイパー(または分圧抵抗)に、
 * +入力を監視したい電圧につないでみてください。
 */
#include <AnalogComp.h>

void setup() {
  Serial.begin(115200);
  AnalogComp.begin();
  AnalogComp.setHysteresis(AC_HYST_SMALL); // 境界付近のばたつきを抑える
}

void loop() {
  if (AnalogComp.read()) {
    Serial.println("plus > minus");
  } else {
    Serial.println("plus < minus");
  }
  delay(500);
}
