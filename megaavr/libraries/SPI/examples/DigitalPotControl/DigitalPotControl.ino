/* Digital Pot Control(デジタルポテンショメータ制御)
 * SPIライブラリの使用例です。
 *
 * アナログ・デバイセズのデジタルポテンショメータAD5206を制御します。
 * AD5206は6チャネルの可変抵抗を内蔵しており、各チャネルの端子は
 * A - 電圧側に接続
 * W - ワイパー(設定値に応じて動く中点)
 * B - GND側に接続
 * となっています。
 *
 * AD5206はSPI互換で、チャネル番号(0~5)と抵抗値(0~255)の
 * 2バイトを送ると設定できます。
 *
 * 回路:
 *   - AD5206の全AピンをUkiUkiduinoの5Vへ
 *   - AD5206の全BピンをGNDへ
 *   - 各Wピンから、LED+220Ω抵抗を直列にしてGNDへ
 *   - CS  → D10 (任意のピンでよいがUno慣例のD10を使用)
 *   - SDI → D11 (MOSI)
 *   - CLK → D13 (SCK)
 *
 * ※本来デジタルポテンショは基準電圧用の部品で、LEDの電源用途は
 *   適しませんが、動作が目で見えるデモとして採用しています。
 *
 * 原作: Tom Igoe (2010) / 原案: Heather Dewey-Hagborg (2005)
 * UkiUkiduino向けに移植・日本語化
 */

// SPIライブラリを読み込む
#include <SPI.h>

// デジタルポテンショのチップセレクト(CS)ピン。任意のピンが使える
const int slaveSelectPin = 10; // D10(Uno慣例のSSピン)

void setup() {
  // CSピンを出力に設定する
  pinMode(slaveSelectPin, OUTPUT);
  // SPIを初期化する
  SPI.begin();
}

void loop() {
  // デジタルポテンショの6チャネルを順に処理する
  for (int channel = 0; channel < 6; channel++) {
    // このチャネルの抵抗値を最小から最大まで変化させる
    for (int level = 0; level < 255; level++) {
      digitalPotWrite(channel, level);
      delay(10);
    }
    // 最大値で1秒待つ
    delay(1000);
    // このチャネルの抵抗値を最大から最小まで変化させる
    for (int level = 0; level < 255; level++) {
      digitalPotWrite(channel, 255 - level);
      delay(10);
    }
  }

}

void digitalPotWrite(int address, int value) {
  // CSピンをLOWにしてチップを選択する
  digitalWrite(slaveSelectPin, LOW);
  // アドレスと値をSPIで送る
  SPI.transfer(address);
  SPI.transfer(value);
  // CSピンをHIGHに戻して選択を解除する
  digitalWrite(slaveSelectPin, HIGH);
  /* 上級テクニック: ピン番号がコンパイル時に確定している場合は
   * digitalWriteFast()/pinModeFast()が使えます。書き方は同じですが
   * ピン(できれば値も)が定数である必要があります。digitalWrite()や
   * pinMode()を全て置き換えられれば、数百バイトのフラッシュ節約と
   * 1クロックでの実行(通常は数百クロック)が得られます。
   */

}
