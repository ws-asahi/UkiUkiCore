// I2C SRF10/SRF08 超音波距離センサの読み取り
// 原作: Nicholas Zambetti / James Tichenor (パブリックドメイン)
//
// Wireライブラリを使って、Devantech製の超音波距離センサ
// SRF08/SRF10からデータを読み出す例です。
//
// 接続: センサのSDAをA4へ、SCLをA5へ。SDA/SCLそれぞれと5Vの間に
//       プルアップ抵抗(4.7kΩ程度)を接続してください。
//
// UkiUkiduino向けに日本語化

#include <Wire.h>

void setup() {
  Wire.begin();          // I2Cバスに参加する(マスタはアドレス指定不要)
  Serial.begin(115200);  // シリアルを115200bpsで開始する
}

int reading = 0;

void loop() {
  // 手順1: センサへ計測開始を指示する
  Wire.beginTransmission(112); // デバイス#112(0x70)へ送信を開始する
  // データシート記載のアドレスは224(0xE0)だが、I2Cのアドレスは
  // 上位7ビットを使うため112になる
  Wire.write(byte(0x00));      // レジスタポインタをコマンドレジスタ(0x00)へ
  Wire.write(byte(0x50));      // 「インチ単位で計測」コマンド(0x50)
  // センチメートル単位なら0x51
  // 往復時間(マイクロ秒)なら0x52
  Wire.endTransmission();      // 送信を完了する

  // 手順2: 計測が終わるまで待つ
  delay(70);                   // データシート推奨は65ミリ秒以上

  // 手順3: 読み出したいエコー値をセンサへ指示する
  Wire.beginTransmission(112); // デバイス#112へ送信を開始する
  Wire.write(byte(0x02));      // レジスタポインタをエコー#1レジスタ(0x02)へ
  Wire.endTransmission();      // 送信を完了する

  // 手順4: センサへ読み出しを要求する
  Wire.requestFrom(112, 2);    // デバイス#112から2バイトを要求する

  // 手順5: センサから測定値を受け取る
  if (2 <= Wire.available()) { // 2バイト受信できていたら
    reading = Wire.read();     // 上位バイトを受け取り
    reading = reading << 8;    // 上位8ビットへシフトして
    reading |= Wire.read();    // 下位バイトを下位8ビットに合成する
    Serial.println(reading);   // 測定値を表示する
  }

  delay(250);                  // 表示を読めるように少し待つ :)
}


/*

// 以下はDevantech超音波センサ(SRF10/SRF08)のI2Cアドレスを
// 変更するコードです。
// 使い方: changeAddress(0x70, 0xE6);

void changeAddress(byte oldAddress, byte newAddress) {
  Wire.beginTransmission(oldAddress);
  Wire.write(byte(0x00));
  Wire.write(byte(0xA0));
  Wire.endTransmission();

  Wire.beginTransmission(oldAddress);
  Wire.write(byte(0x00));
  Wire.write(byte(0xAA));
  Wire.endTransmission();

  Wire.beginTransmission(oldAddress);
  Wire.write(byte(0x00));
  Wire.write(byte(0xA5));
  Wire.endTransmission();

  Wire.beginTransmission(oldAddress);
  Wire.write(byte(0x00));
  Wire.write(newAddress);
  Wire.endTransmission();
}

*/
