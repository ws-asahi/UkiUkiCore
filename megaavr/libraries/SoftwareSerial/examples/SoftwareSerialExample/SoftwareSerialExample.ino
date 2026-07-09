/* SoftwareSerialExample(ソフトウェアシリアルの基本)
   ソフトウェアシリアル(任意のピンで行うシリアル通信)のテストです。
   USBシリアル(Serial)で受けた文字をソフトウェアシリアルへ、
   ソフトウェアシリアルで受けた文字をUSBシリアルへ転送します。

   接続: D10(RX)を相手機器のTXへ、D11(TX)を相手機器のRXへ、
         GND同士を接続します。

   メモ: UkiUkiduinoではどのピンでも使えます。受信(RX)には
   ポート内でビット番号の小さいピンを使うと安定します。
   なおD0/D1にはハードウェアUART(Serial1)があるので、
   確実な通信が必要な場合はそちらを優先してください。

   原作: Tom Igoe / Mikal Hart (パブリックドメイン)
   UkiUkiduino向けに移植・日本語化
*/
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX = D10, TX = D11

void setup() {
  // USBシリアルを開き、ポートが開くまで待つ
  Serial.begin(57600);
  while (!Serial) {
    ; // USB接続のシリアルポートが開くのを待つ
  }

  Serial.println("Goodnight moon!");

  // ソフトウェアシリアルの通信速度を設定する
  mySerial.begin(4800);
  mySerial.println("Hello, world?");
}

void loop() { // 双方向に転送し続ける
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
  if (Serial.available()) {
    mySerial.write(Serial.read());
  }
}
