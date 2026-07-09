/*
 * SCP1000 気圧センサの表示
 *
 * SPI接続の気圧センサSCP1000の測定値をシリアルモニタへ表示します。
 * SPIライブラリの使用例です。
 *
 * ※SCP1000は既に入手困難ですが、「専用ライブラリを使わずSPIで
 *   デバイスのレジスタを直接読み書きする方法」の教材として
 *   このサンプルを残しています。他のSPIセンサにも応用できます。
 *
 * 回路:
 *   - DRDY(データレディ) → D7 (任意のピンでよい)
 *   - CSB(チップセレクト) → D10 (任意のピンでよいがUno慣例のD10を使用)
 *   - MOSI → D11 / MISO → D12 / SCK → D13
 *
 * 原作: Nathan SeidleのPIC用SCP1000サンプルを元にTom Igoeが作成
 * UkiUkiduino向けに移植・日本語化
 */

// センサはSPIで通信するのでライブラリを読み込む
#include <SPI.h>

// センサのレジスタアドレス:
const int PRESSURE      = 0x1F;         // 気圧の上位3ビット
const int PRESSURE_LSB  = 0x20;         // 気圧の下位16ビット
const int TEMPERATURE   = 0x21;         // 16ビットの温度
const byte READ         = 0b11111100;   // SCP1000の読み出しコマンド
const byte WRITE        = 0b00000010;   // SCP1000の書き込みコマンド

// センサとの接続に使うピン
// (これ以外のSPIピンはSPIライブラリが管理する):
const int dataReadyPin  = 7;   // DRDY = D7
const int chipSelectPin = 10;  // CSB  = D10

void setup() {
  Serial.begin(9600);
  // SPIライブラリを開始する:
  SPI.begin();
  // DRDYとCSのピンを初期化する:
  pinMode(dataReadyPin, INPUT);
  pinMode(chipSelectPin, OUTPUT);
  // SCP1000を低ノイズ設定にする:
  writeRegister(0x02, 0x2D);
  writeRegister(0x01, 0x03);
  writeRegister(0x03, 0x02);
  // センサの準備が整うまで待つ:
  delay(100);
}

void loop() {
  // 高分解能モードを選択する
  writeRegister(0x03, 0x0A);
  // DRDYピンがHIGHになる(=データ準備完了)まで何もしない:
  if (digitalRead(dataReadyPin) == HIGH) {
    // 温度データを読む
    int tempData = readRegister(0x21, 2);
    // 摂氏に変換して表示する:
    float realTemp = (float)tempData / 20.0;
    Serial.print("Temp[C]=");
    Serial.print(realTemp);
    // 気圧データの上位3ビットを読む:
    byte  pressure_data_high = readRegister(0x1F, 1);
    pressure_data_high &= 0b00000111; // 必要なのはビット2~0だけ

    // 気圧データの下位16ビットを読む:
    unsigned int pressure_data_low = readRegister(0x20, 2);
    // 2つを結合して19ビットの値にする:
    /* 補足: pressure_data_highをそのまま16ビット左シフトすると
     * 16ビット型のため結果が0になるバグが長年ありました。
     * (long)へのキャストが必須です。コンパイラの警告を有効に
     * しておくべき好例です。
     */
    long pressure = (((long)pressure_data_high << 16) | pressure_data_low) / 4;
    // 気圧を表示する:
    Serial.println("\tPressure [Pa]=" + String(pressure));
  }
}
// SCP1000のレジスタを読み出す:
unsigned int readRegister(byte thisRegister, int bytesToRead) {
  byte inByte = 0;           // SPIから届いたバイト
  unsigned int result = 0;   // 返す結果
  Serial.print(thisRegister, BIN);
  Serial.print("\t");
  // SCP1000はレジスタ名をバイトの上位6ビットで受け取るため
  // 2ビット左へシフトする:
  thisRegister = thisRegister << 2;
  // アドレスとコマンドを1バイトに合成する
  byte dataToSend = thisRegister & READ;
  Serial.println(thisRegister, BIN);
  // CSをLOWにしてデバイスを選択する:
  digitalWrite(chipSelectPin, LOW);
  // 読みたいレジスタをデバイスへ送る:
  SPI.transfer(dataToSend);
  // 0x00を送って最初の応答バイトを受け取る:
  result = SPI.transfer(0x00);
  // 残り読み出しバイト数を減らす:
  bytesToRead--;
  // まだ読むバイトが残っている場合:
  if (bytesToRead > 0) {
    // 先に受けたバイトを左へシフトし、次のバイトを受け取る:
    result = result << 8;
    inByte = SPI.transfer(0x00);
    // 受け取ったバイトを結合する:
    result = result | inByte;
    // 残り読み出しバイト数を減らす:
    bytesToRead--;
  }
  // CSをHIGHに戻して選択を解除する:
  digitalWrite(chipSelectPin, HIGH);
  // 結果を返す:
  return (result);
}
// SCP1000へ書き込みコマンドを送る
void writeRegister(byte thisRegister, byte thisValue) {
  // SCP1000はレジスタアドレスをバイトの上位6ビットで受け取るため
  // 2ビット左へシフトする:
  thisRegister = thisRegister << 2;
  // アドレスとコマンドを1バイトに合成する:
  byte dataToSend = thisRegister | WRITE;
  // CSをLOWにしてデバイスを選択する:
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer(dataToSend); // レジスタ位置を送る
  SPI.transfer(thisValue);  // レジスタへ書く値を送る
  // CSをHIGHに戻して選択を解除する:
  digitalWrite(chipSelectPin, HIGH);
  /* 上級テクニック: ピン番号がコンパイル時に確定している場合は
   * digitalWriteFast()/pinModeFast()が使えます(DigitalPotControlの
   * 末尾コメント参照)。
   */
}
