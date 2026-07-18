/* CustomLogic / SetResetLatch(セット/リセットラッチ)
 *
 * ロジックブロックは自分自身の出力を見ることができます。これが
 * ゲートを「記憶素子」に変えます: SETを押すと、離した後も出力は
 * HIGHのまま。RESETを押すとLOWのままになります。ハードウェアだけで
 * できたラッチです - loop()では何のコードも動かず、CPUがスリープ
 * していても動き続けます。
 *
 *   IN0 = SETボタン   (ピン)
 *   IN1 = RESETボタン (ピン)
 *   IN2 = このユニット自身の出力   <- LOGIC_OWN_OUTPUT、ピン不使用
 *
 * 配線(ボタンはGNDへ - 入力はプルアップ済みなので押下=LOW):
 *   SET   -> IN0: D5
 *   RESET -> IN1: D6
 *   LED   -> OUT: D10 (抵抗を直列にしてGNDへ)
 *
 * 下の真理値表の意味: SETが押されていれば出力HIGH。そうでなく
 * RESETが押されていればLOW。どちらでもなければ今の値(自身の出力)を
 * 保持。ビットiは入力が数値iのときの出力で、IN2=ビット2です:
 *
 *   IN2(自身) IN1(RESET) IN0(SET) | i | OUT
 *       0          0        0     | 0 |  1   SET押下
 *       0          0        1     | 1 |  0   RESET押下
 *       0          1        0     | 2 |  1   SET押下
 *       0          1        1     | 3 |  0   保持(LOWだった)
 *       1          0        0     | 4 |  1   SET押下
 *       1          0        1     | 5 |  0   RESET押下
 *       1          1        0     | 6 |  1   SET押下
 *       1          1        1     | 7 |  1   保持(HIGHだった)
 *
 * -> 0b11010101 (両方押されたらSETが勝つ)
 *
 * ※押下=LOWなので、ビット位置の0が「押されている」ことに注意。
 *
 * UkiUkiduino向けに日本語化
 */
#include <CustomLogic.h>

void setup() {
  Serial.begin(115200);
  CustomLogic.setInputIN2(LOGIC_OWN_OUTPUT);      // IN2 = 自身の出力
  CustomLogic.beginTruthTable(0b11010101, 3);
}

void loop() {
  Serial.println(CustomLogic.read() ? F("latched: SET") : F("latched: RESET"));
  delay(500);
}
