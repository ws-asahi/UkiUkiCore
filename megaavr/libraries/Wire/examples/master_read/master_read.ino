/* Wire Master Read(マスタ: 読み出し)
 * 原作: MX682X
 *
 * Wireライブラリの使用例です。
 * I2C/TWIスレーブデバイスからデータを読み出します。
 * 対向側は「Wire Slave Write」サンプルを使用してください。
 *
 * シリアルモニタから'm'または'M'を入力すると、アドレス0x54の
 * スレーブへ4バイトを要求します。対のサンプルと組み合わせると、
 * スレーブのmillis()値が届き、シリアルモニタに表示されます。
 *
 * 使い方: このボードのSCL(A5)/SDA(A4)を、Wire Slave Writeサンプルを
 * 実行しているもう1台のボードのSCL/SDAへ接続します。
 *
 * SDA/SCLの両方に、Vccへのプルアップ抵抗が必要です。
 * 詳しくはWireライブラリのREADME.mdを参照してください。
 *
 * UkiUkiduino向けに日本語化
 */

#define MySerial Serial

#include <Wire.h>

int8_t rxLen = 0;
int8_t len = 0;

void setup() {
  Wire.begin();                                 // マスタとして初期化する
  MySerial.begin(115200);
}

void loop() {
  if (MySerial.available() > 0) {   // シリアルに1バイト届いたら
    char c = MySerial.read();       // 読み出して
    if (c == 'm' || c == 'M') {
      sendDataWire();               // I2Cへ要求を送る
    }
    len = 0;                        // 送信済みなので位置を0に戻す
  }
}

void sendDataWire() {
  uint32_t ms;
  if (4 == Wire.requestFrom(0x54, 4, 0x01)) {    // スレーブへ要求する
    while (Wire.available()) {
      ms  = (uint32_t)Wire.read();               // 32ビット値を組み立てながら読む
      ms |= (uint32_t)Wire.read() <<  8;
      ms |= (uint32_t)Wire.read() << 16;
      ms |= (uint32_t)Wire.read() << 24;
      MySerial.println(ms);              // スレーブのミリ秒値を表示する
    }
  } else {
    MySerial.println("Wire.requestFrom() timed out!");
  }
}
