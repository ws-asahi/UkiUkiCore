/* Wire Slave Read(スレーブ: 受信)
 * 原作: MX682X
 *
 * Wireライブラリの使用例です。
 * I2C/TWIスレーブとしてデータを受信します。
 * 対向側は「Wire Master Write」サンプルを使用してください。
 *
 * I2Cバスで受信した内容をすべてシリアルモニタへ表示します。
 *
 * 使い方: このボードのSCL(A5)/SDA(A4)を、Master Writeサンプルを
 * 実行しているもう1台のボードのSCL/SDAへ接続します。
 *
 * SDA/SCLの両方に、Vccへのプルアップ抵抗が必要です。
 * 詳しくはWireライブラリのREADME.mdを参照してください。
 *
 * UkiUkiduino向けに日本語化
 */

#include <Wire.h>

#define MySerial Serial

void setup() {
  Wire.begin(0x54);                 // アドレス0x54でI2Cバスに参加する
  Wire.onReceive(receiveDataWire);  // マスタからの書き込み時に呼ばれる
  //                                   関数をWireライブラリへ登録する
  MySerial.begin(115200);
}

void loop() {
  delay(100);
}

// マスタからデータを受信するたびに実行される関数
// (setup()でイベントとして登録済み)
void receiveDataWire(int16_t numBytes) {      // 受信したバイト数はWireライブラリが
  for (uint8_t i = 0; i < numBytes; i++) {    // 教えてくれるので、その回数だけ
    char c = Wire.read();                     // 受信データを読み出して
    MySerial.write(c);                        // シリアルモニタへ表示する
  }
}
