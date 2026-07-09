/* Knob(ノブ)
   可変抵抗(ポテンショメータ)のつまみでサーボの角度を操作するサンプルです。

   接続: サーボの信号線をD9へ。可変抵抗は両端を5VとGNDへ、
         中央の端子をA0へ接続します。

   原作: Michal Rinott (パブリックドメイン)
   UkiUkiduino向けに移植・日本語化
*/

#include <Servo_DxCore.h>

Servo myservo;  // サーボを制御するためのServoオブジェクトを作成

int potpin = A0;  // 可変抵抗を接続するアナログ入力ピン
int val;          // アナログ入力の読み取り値を入れる変数

void setup() {
  myservo.attach(9);  // D9に接続したサーボをオブジェクトに割り当てる
}

void loop() {
  val = analogRead(potpin);            // 可変抵抗の値を読む(0~1023)
  val = map(val, 0, 1023, 0, 180);     // サーボの角度(0~180)へ変換する
  myservo.write(val);                  // 変換した角度へサーボを動かす
  delay(15);                           // サーボが追従するまで待つ
}
