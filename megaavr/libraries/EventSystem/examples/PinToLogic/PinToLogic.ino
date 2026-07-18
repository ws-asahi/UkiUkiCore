/* EventSystem / PinToLogic(任意のピンをロジックへ)
 *
 * 「どのピンでも」CustomLogicの入力に接続できます。ロジックブロック
 * 自身の入力ピンはハードウェアで固定ですが、イベント接続を使えば他の
 * 任意のピンを届けられます。そのとき固定側の入力ピンには一切触れない
 * ため、そのピンは別の用途に使えるままです。
 *
 * このスケッチ: OUT = (任意ピンのボタン) AND (ユニット自身のIN1ボタン)
 *
 *   任意ピンのボタン = D2 (イベント経由でIN0へ)
 *   IN1のボタン      = D6 (ユニット固有の入力ピン)
 *   OUT              = D10 (LED+抵抗をGNDへ)
 *
 * ボタンはGNDへ接続します(入力はプルアップされるため、未押下=HIGH。
 * つまり両方とも押していない間LEDが点灯し、どちらかを押すと消えます)。
 *
 * メモ: 任意ピン側にはオンボードボタン(BTN_BUILTIN = D20)も使えます。
 * その場合D20はプルダウンのため論理が逆(押下=HIGH)になる点に注意。
 */
#include <CustomLogic.h>
#include <EventSystem.h>

void setup() {
  Serial.begin(115200);

  EventSystem.connect(2, EVENT_TO_LOGIC_A);   // D2 -> CustomLogicのEVENT_A

  CustomLogic.setInputIN0(LOGIC_EVENT_A);     // IN0はイベントから受け取る
  CustomLogic.begin(AND);                     // OUT = イベント AND IN1ピン
}

void loop() {
  Serial.println(CustomLogic.read() ? F("OUT = HIGH (no button pressed)")
                                    : F("OUT = LOW  (a button is pressed)"));
  delay(500);
}
