/* Wire Register Model Master(レジスタ方式スレーブを操作するマスタ)
 * 原作: Spence Konde
 *
 * 「レジスタマシン」方式のWireスレーブ(register_modelサンプル)を
 * 操作するマスタのデモです。スレーブの公開仕様から、各バイトの
 * 書き込み保護は次のとおりと分かっています:
 *
 * 0~4     全ビット書き込み可
 * 5       下位5ビットのみ書き込み可
 * 6~7     全ビット書き込み可
 * 8~11    各ニブルの下位2ビットのみ書き込み可
 * 12~15   読み出し専用
 * 16~17   下位ニブルのみ書き込み可
 * 18~19   上位ニブルのみ書き込み可
 *
 * 初期状態では各レジスタにアドレスと同じ値が入っています。
 * アドレスポインタは自動的に進み、末尾で先頭へ折り返します。
 *
 * レジスタ4と5はLEDの点滅周期を制御します。I2C経由でスレーブを
 * 設定する方法の一例です。
 *
 * UkiUkiduino向けに日本語化
 */
#include <Wire.h>

#define MySerial Serial

void setup() {
  MySerial.begin(115200);
  MySerial.println("Hi, now to use a register model slave");
  Wire.begin();
}

void setAddressPointer(uint8_t address) {
  Wire.beginTransmission(0x69);   // アドレス0x69のスレーブへの送信を準備する
  Wire.write(address);            // アドレスだけを書き込む
  Wire.endTransmission();
}

void loop() {
  MySerial.println("going to write to 0, then make a series of reads");
  setAddressPointer(0);
  Wire.requestFrom(0x69, 8);
  while (Wire.available()) {
    MySerial.printHex((uint8_t)Wire.read());
    MySerial.print(' ');
  }
  MySerial.println("that was 8 bytes");
  Wire.requestFrom(0x69, 12);
  while (Wire.available()) {
    MySerial.printHex((uint8_t)Wire.read());
    MySerial.print(' ');
  }
  MySerial.println("that was 12 bytes more");
  MySerial.println("Now, let's request a whopping 32, the whole shebang");

  Wire.requestFrom(0x69, 32);
  while (Wire.available()) {
    MySerial.printHex((uint8_t) Wire.read());
    MySerial.print(' ');
  }
  MySerial.println("Now that was cool, no?");
  MySerial.println("Let's demo write protect in action");
  Wire.beginTransmission(0x69);     // アドレス0x69のスレーブへの送信を準備する
  Wire.write(0x16);                 // まずアドレスを書き込む
  Wire.write(0xEE);                 // 値を書き込む
  Wire.write(0xDD);                 // 値を書き込む
  Wire.write(0xCC);                 // 値を書き込む
  Wire.write(0xBB);                 // 値を書き込む
  Wire.write(0xFF);                 // 値を書き込む
  Wire.endTransmission();           // 送信するとスレーブ側のISRが動く
  MySerial.println("Read-em-back:");// ポインタを戻して読み返してみる
  setAddressPointer(16);            // ポインタを16に設定する
  Wire.requestFrom(0x69, 5);        // 5バイト読む
  while (Wire.available()) {        // このライブラリの拡張機能が無いと、スレーブは何バイト読まれたか分からない!
    MySerial.printHex((uint8_t)Wire.read());   // 読めた値を表示する
    MySerial.print(' ');                       // バイトの間に空白を入れる
  }
  MySerial.println("Change speed at which the LED blinks");
  Wire.beginTransmission(0x69);   // アドレス0x69のスレーブへの送信を準備する
  Wire.write(0x4);                // まずアドレスを書き込む
  Wire.write(0x80);               // 値を書き込む
  Wire.write(0x01);               // 値を書き込む: 約3/8秒の点滅になる
  Wire.endTransmission();
  delay(10000);
}
