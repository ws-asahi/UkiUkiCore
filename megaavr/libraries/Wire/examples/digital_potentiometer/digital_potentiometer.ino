// I2C Digital Potentiometer(I2Cデジタルポテンショメータ)
// 原作: Nicholas Zambetti / Shawn Bonkowski (パブリックドメイン)
//
// Wireライブラリの使用例です。
// I2C接続のデジタルポテンショメータAD5171を制御します。
//
// 接続: AD5171のSDAをA4へ、SCLをA5へ。SDA/SCLそれぞれと5Vの間に
//       プルアップ抵抗(4.7kΩ程度)を接続してください。
//
// UkiUkiduino向けに日本語化

#include <Wire.h>

void setup() {
  Wire.begin(); // I2Cバスに参加する(マスタはアドレス指定不要)
}

byte val = 0;

void loop() {
  Wire.beginTransmission(44); // デバイス#44(0x2C)への送信を開始する
  // デバイスアドレスはデータシートに記載されている
  Wire.write(byte(0x00));     // 命令バイトを送る
  Wire.write(val);            // ポテンショメータの設定値を送る
  Wire.endTransmission();     // 送信を完了する

  val++;           // 値を1増やす
  if (val == 64) { // 64段階目(最大)まで来たら
    val = 0;       // 最小値からやり直す
  }
  delay(500);
}
