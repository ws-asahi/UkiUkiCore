/* AnalogComp / Threshold(しきい値判定)
 *
 * +入力ピンの電圧を内蔵基準電圧と比較します - 外付けの分圧抵抗は
 * 不要です。ここではしきい値2.5Vとし、+入力が上回っている間
 * オンボードLEDを点灯させます。
 *
 * +入力ピン: D9
 * begin()にはanalogReference()と同じ定数を渡せます: INTERNAL1V024,
 * INTERNAL2V048, INTERNAL2V5, INTERNAL4V096, VDD, EXTERNAL。
 * 省略可能な第2引数でしきい値を調整できます: Vth = Vref × level / 256
 */
#include <AnalogComp.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  AnalogComp.begin(INTERNAL2V5);            // +入力 vs 内蔵2.5V基準
  AnalogComp.setHysteresis(AC_HYST_MEDIUM);
}

void loop() {
  digitalWrite(LED_BUILTIN, AnalogComp.read() ? HIGH : LOW);
  delay(10);
}
