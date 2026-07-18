/* CustomLogic / EdgeInterrupt(出力エッジで割り込み)
 *
 * ゲート出力が変化した瞬間に関数を実行します - 論理の評価は
 * ハードウェアが行い、コードはエッジのときだけ呼ばれます。
 *
 * ここでは2入力NANDを「警報」として使います: プルアップにより、
 * 両方の入力がHIGHになった瞬間(両方のボタンを離した瞬間)にだけ
 * OUTがLOWへ落ちます - onAlarm()はその回数を数えます。
 *
 * ピン: TwoInputANDと同じです(IN0=D5 / IN1=D6 / OUT=D10)。
 *
 * UkiUkiduino向けに日本語化
 */
#include <CustomLogic.h>

volatile uint16_t alarms = 0;

void onAlarm() {
  alarms++;
}

void setup() {
  Serial.begin(115200);
  CustomLogic.begin(NAND);                       // OUT = NOT (IN0 AND IN1)
  CustomLogic.attachInterrupt(onAlarm, FALLING); // 両入力がHIGHになった
}

void loop() {
  Serial.print("alarm count: ");
  Serial.println(alarms);
  delay(1000);
}
