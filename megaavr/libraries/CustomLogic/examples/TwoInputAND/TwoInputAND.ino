/* CustomLogic / TwoInputAND(2入力AND)
 *
 * ハードウェアANDゲート: 両方の入力がHIGHの間だけOUTピンがHIGHに
 * なります。begin()を実行した後はゲートが完全にハードウェアだけで
 * 動作します。loop()が空でも、CPUがスリープ中でも、ゲートは
 * 動き続けます。
 *
 * CustomLogicユニットのピン(UkiUkiduino):
 *   IN0 = D5    IN1 = D6    OUT = D10
 *
 * 入力にはプルアップが効いているので、手軽に試すならIN0/IN1から
 * GNDへ押しボタンを2つ、OUTからGNDへLED(+抵抗)を配線して
 * ください。どちらのボタンも押していない間だけLEDが点灯します
 * (プルアップにより両入力がHIGHのため)。どちらかを押すと消えます。
 *
 * UkiUkiduino向けに日本語化
 */
#include <CustomLogic.h>

void setup() {
  CustomLogic.begin(AND);   // OUT = IN0 AND IN1
}

void loop() {
  // 何もしない - ゲートは勝手に動き続ける
}
