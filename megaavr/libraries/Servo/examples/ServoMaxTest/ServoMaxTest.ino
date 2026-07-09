/* ServoMaxTest(最大数テスト)
   このライブラリは1つのタイマ(TCB)だけで本当に12個のサーボを
   同時に駆動できます。その実演サンプルです。

   接続: D2~D13の12ピンにそれぞれサーボの信号線を接続します。
   ※12個のサーボをUSB給電で動かすことはできません。必ずサーボ用の
     外部電源を用意し、GNDをボードと共通にしてください。

   UkiUkiduino向けに移植・日本語化
*/

#include <Servo.h>

Servo myservos[12];       // 12個分のServoオブジェクト
byte pos[12];             // 各サーボの現在角度
char dir[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; // 各サーボの動く向き

void setup() {
  for (byte i = 0; i < 12; i++) {
    myservos[i].attach(i + 2);  // D2~D13へ順に割り当てる
    pos[i] = i * 15;            // 開始角度をずらしてウェーブ状にする
  }
}

void loop() {
  for (byte i = 0; i < 12; i++) {
    myservos[i].write(pos[i]);  // 現在角度を出力
    pos[i] += dir[i];           // 向きに応じて1度進める
    if (pos[i] == 180) {
      dir[i] = -1;              // 端まで来たら折り返す
    }
    if (pos[i] == 0) {
      dir[i] = 1;
    }
  }
  delay(30);                    // サーボが追従するまで30ms待つ
}
