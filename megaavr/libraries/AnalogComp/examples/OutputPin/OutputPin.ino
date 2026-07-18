/* AnalogComp / OutputPin(結果をピンへ直接出力)
 *
 * コンパレータの判定結果は、ソフトウェアを介さずにピンを直接駆動
 * できます。スケッチが忙しくても、スリープ中でも動き続けます。
 *
 * 出力ピン(ハードウェア固定): D8 (PA7)
 *
 * D8にLED(+抵抗)を接続し、+入力ピン(D9)の電圧を1.02V前後で
 * 変化させると、切り替わる様子が見られます。
 */
#include <AnalogComp.h>

void setup() {
  AnalogComp.begin(INTERNAL1V024);   // +入力 vs 内蔵1.024V基準
  AnalogComp.setHysteresis(AC_HYST_LARGE);
  AnalogComp.enableOutput();      // 結果がD8(PA7)に現れる
}

void loop() {
  // 何もすることはありません - コンパレータが自力でピンを駆動します。
}
