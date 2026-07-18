/* CustomLogic / ThreeInputOR(3入力OR)
 *
 * 3入力ORゲート: どれか1つでも入力がHIGHの間、OUTがHIGHになります。
 * 3入力のロジックは2つの演算を左から右へ適用する形で書きます:
 * begin(logic1, logic2) は OUT = (IN0 logic1 IN1) logic2 IN2 の意味です。
 *
 *   begin(OR, OR)    -> IN0 OR IN1 OR IN2       (このサンプル)
 *   begin(AND, AND)  -> IN0 AND IN1 AND IN2
 *   begin(AND, OR)   -> (IN0 AND IN1) OR IN2
 *   begin(XOR, XOR)  -> 3入力パリティ
 *   begin(NOP, OR)   -> IN1 OR IN2  (IN0は不使用)
 *
 * CustomLogicユニットのピン(UkiUkiduino):
 *   IN0 = D5    IN1 = D6    IN2 = D9    OUT = D10
 *
 * UkiUkiduino向けに日本語化
 */
#include <CustomLogic.h>

void setup() {
  Serial.begin(115200);
  CustomLogic.begin(OR, OR);  // OUT = (IN0 OR IN1) OR IN2
}

void loop() {
  // read()はOUTピンの現在の状態を返す
  Serial.println(CustomLogic.read() ? "OUT = HIGH" : "OUT = LOW");
  delay(500);
}
