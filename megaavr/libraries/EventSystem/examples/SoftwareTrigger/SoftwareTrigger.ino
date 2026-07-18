/* EventSystem / SoftwareTrigger(ソフトウェアからのパルス)
 *
 * trigger()は、接続に1クロック分(24MHzで約42ナノ秒)のパルスを1発だけ
 * 送ります。LEDで見るにはあまりに短いですが、エッジで動くハードウェア
 * にはこれで十分です。ここではCustomLogicで作ったラッチをこのパルスで
 * セットし、ボタンでリセットします。「いつ実行するか」だけをコードが
 * 決め、それ以降はすべてハードウェアが処理します。
 *
 *   IN0 = ソフトウェアパルス (EVENT_TO_LOGIC_A経由)
 *   IN1 = リセットボタン      D6 (GNDへ。プルアップされ未押下=HIGH)
 *   IN2 = ラッチ自身の出力
 *   OUT = LED(+抵抗)          D10
 *
 * 真理値表(ビットiが「入力の並びがiのときの出力」。IN2=ビット2):
 *   リセット押下(IN1=0)            -> 0        (インデックス0,1,4,5)
 *   パルス到着(IN0=1かつIN1=1)     -> 1        (インデックス3,7)
 *   それ以外                       -> IN2を保持 (2->0, 6->1)
 *   = 0b11001000
 */
#include <CustomLogic.h>
#include <EventSystem.h>

void setup() {
  Serial.begin(115200);

  EventSystem.connect(EVENT_SOFTWARE, EVENT_TO_LOGIC_A);

  CustomLogic.setInputIN0(LOGIC_EVENT_A);      // IN0 = ソフトウェアパルス
  CustomLogic.setInputIN2(LOGIC_OWN_OUTPUT);   // IN2 = 自分の出力(=ラッチ化)
  CustomLogic.beginTruthTable(0b11001000, 3);
}

void loop() {
  Serial.println(F("trigger() - the latch goes HIGH (press the button to reset it)"));
  EventSystem.trigger();                        // 42nsのパルス1発: セット
  for (uint8_t i = 0; i < 6; i++) {
    Serial.print(F("  latch: "));
    Serial.println(CustomLogic.read() ? F("SET") : F("RESET"));
    delay(500);
  }
}
