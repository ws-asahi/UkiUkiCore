/* CustomLogic / DualUnits(2ユニット同時使用)
 *
 * UkiUkiduinoには第1ユニットと独立に動く第2ユニット
 * 「CustomLogic1」があります - ハードウェアゲートを同時に2つ
 * 使えるということです。
 *
 * ピン(UkiUkiduino):
 *                 IN0   IN1   OUT
 *   CustomLogic   D5    D6    D10
 *   CustomLogic1  A0    A1    A3
 *
 * UkiUkiduino向けに日本語化
 */
#include <CustomLogic.h>

void setup() {
  CustomLogic.begin(AND);     // ユニット0: IN0とIN1が両方HIGHの間だけOUTがHIGH
  CustomLogic1.begin(OR);     // ユニット1: IN0かIN1がHIGHの間OUTがHIGH
}

void loop() {
  // 2つのゲートが同時にハードウェアで動作中 - することはない
}
