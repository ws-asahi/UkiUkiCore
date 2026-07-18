/* Wire Master Write(マスタ: 書き込み)
 * 原作: MX682X
 *
 * Wireライブラリの使用例です。
 * I2C/TWIスレーブデバイスへデータを書き込みます。
 * 対向側は「Wire Slave Read」サンプルを使用してください。
 *
 * シリアルモニタから任意の文字列を(改行付きで)送ると、その内容が
 * スレーブへ送信され、スレーブ側のシリアルモニタに表示されます。
 *
 * 使い方: このボードのSCL(A5)/SDA(A4)を、Wire Slave Readサンプルを
 * 実行しているもう1台のボードのSCL/SDAへ接続します。
 *
 * SDA/SCLの両方に、Vccへのプルアップ抵抗が必要です。
 * 詳しくはWireライブラリのREADME.mdを参照してください。
 *
 * UkiUkiduino向けに日本語化
 */

#include <Wire.h>

char input[32];
int8_t len = 0;

#define MySerial Serial               // PCと接続しているシリアルポート

void setup() {
  Wire.begin();                       // マスタとして初期化する
  MySerial.begin(115200);             // 115200ボーを使用(現代のAVRなら余裕です)
}

void loop() {
  if (MySerial.available() > 0) {     // シリアルに1バイト届いたら
    readFromSerial();                 // シリアルからデータを読み出す
    if (len > 0) {                    // 有効なデータがあれば
      sendDataWire();                 // I2Cへ送信する
    }
    len = 0;                          // 送信済みなので位置を0に戻す
  }
}

void readFromSerial() {
  while (true) {                      // 無限ループの中で
    while (MySerial.available() == 0);// 次の1文字が届くまで待つ
    char c = MySerial.read();         // 届いた文字を読む
    if (c == '\n' || c == '\r') {     // 改行(LF)か復帰(CR)が来たら
      break;                          // ループを抜ける
    }                                 // それ以外なら
    input[len] = c;                   // 文字を保存して
    len++;                            // 位置を進める
    if (len > 30) {                   // データが多すぎる場合は
      break;                          // バッファあふれ防止のため抜ける
    }
  }
}

void sendDataWire() {
  Wire.beginTransmission(0x54);     // アドレス0x54のスレーブへの送信を準備する
  for (uint8_t i = 0; i < len; i++) {
    Wire.write(input[i]);           // 受け取ったデータをバスのバッファへ書く
  }
  Wire.write("\r\n");               // シリアルモニタ用に改行を付ける
  Wire.endTransmission();           // 送信を完了する
}
