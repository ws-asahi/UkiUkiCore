/* AnalogComp / EdgeInterrupt(しきい値横断で割り込み)
 *
 * +入力がしきい値を横切った瞬間に関数を実行します。デジタルピンの
 * attachInterrupt()の、アナログ電圧版だと考えてください。
 *
 * +入力ピン: D9
 * モード: RISING(上向きに横断)、FALLING(下向きに横断)、CHANGE。
 * ここでのしきい値は電源電圧の半分です: begin(VDD, 128) -> VDD × 128/256
 */
#include <AnalogComp.h>

volatile uint16_t crossings = 0;

void onCross() {
  crossings++;
}

void setup() {
  Serial.begin(115200);
  AnalogComp.begin(VDD, 128);                // +入力 vs VDDの半分
  AnalogComp.setHysteresis(AC_HYST_MEDIUM);  // 重要: ノイズの多い信号で
                                             // 割り込みが連発するのを防ぐ
  AnalogComp.attachInterrupt(onCross, RISING);
}

void loop() {
  Serial.print("upward crossings: ");
  Serial.println(crossings);
  delay(1000);
}
