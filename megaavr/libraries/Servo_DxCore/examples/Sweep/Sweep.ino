/* Sweep(スイープ)
   サーボモータを0度から180度まで往復させる基本サンプルです。

   接続: サーボの信号線をD9へ、電源(赤)を5Vへ、GND(茶/黒)をGNDへ。
   ※サーボは電流を多く消費します。複数のサーボや大型サーボを動かす
     場合は、USB給電ではなく外部電源の使用を推奨します。

   原作: BARRAGAN <http://barraganstudio.com> (パブリックドメイン)
   UkiUkiduino向けに移植・日本語化
*/

#include <Servo_DxCore.h>

Servo myservo;  // サーボを制御するためのServoオブジェクトを作成
                // (このライブラリは1つのタイマで最大12個まで制御できます)

int pos = 0;    // サーボの現在角度を保持する変数

void setup() {
  myservo.attach(9);  // D9に接続したサーボをオブジェクトに割り当てる
}

void loop() {
  for (pos = 0; pos <= 180; pos += 1) { // 0度から180度まで1度ずつ
    myservo.write(pos);              // 'pos'の角度へサーボを動かす
    delay(15);                       // サーボが追従するまで15ms待つ
  }
  for (pos = 180; pos >= 0; pos -= 1) { // 180度から0度まで1度ずつ
    myservo.write(pos);              // 'pos'の角度へサーボを動かす
    delay(15);                       // サーボが追従するまで15ms待つ
  }
}
