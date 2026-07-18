/* Wire Slave Secondary Address(スレーブ: 第2アドレス)
 * 原作: MX682X
 *
 * Wireライブラリの使用例です。
 * I2C/TWIスレーブとして「2つのアドレス」で受信します。
 * 対向側は「Wire Master Multi Address Write」サンプルを
 * 使用してください。
 *
 * どちらのアドレス宛で受信したかと、受信データをシリアルモニタへ
 * 表示します。
 *
 * 注意: このサンプルは簡潔さを優先して、受信ハンドラ(割り込み内)で
 * シリアル出力を行っています。実際の製品コードでは絶対に避けて
 * ください(シリアルバッファが埋まったときの影響が予測不能です)。
 * 本来はフラグを立ててloop()側で表示します。より「正しい」書き方は
 * 「Slave Address Mask」サンプルを参照してください。
 *
 * 使い方: このボードのSCL(A5)/SDA(A4)を、対向ボードのSCL/SDAへ
 * 接続します。SDA/SCLの両方に、Vccへのプルアップ抵抗が必要です。
 * 詳しくはWireライブラリのREADME.mdを参照してください。
 *
 * UkiUkiduino向けに日本語化
 */
#include <Wire.h>

char input[32];
int8_t len = 0;

#define MySerial Serial       // PCと接続しているシリアルポート


void setup() {
  // 第2アドレス付きでスレーブを初期化する
  // 第1引数: 待ち受ける第1アドレス
  // 第2引数: 一斉呼び出し(ゼネラルコール、アドレス0x00)を受けるか
  // 第3引数: ビット0が1なら、ビット7~1は第2アドレス
  //          ビット0が0なら、アドレスマスクとして扱われる
  Wire.begin(0x54, false, WIRE_ALT_ADDRESS(0x64));
  Wire.onReceive(receiveDataWire);

  // シリアルポートを初期化する
  MySerial.begin(115200);
}

void loop() {
  delay(100);
}



// マスタからデータを受信するたびに実行される関数
// (setup()でイベントとして登録済み)
void receiveDataWire(int16_t numBytes) {
  uint8_t addr = Wire.getIncomingAddress();   // どのアドレス宛だったかを取得する
  //                                          // 取得値は1ビット左シフトされている
  if (addr == (0x54 << 1)) {                  // 0x54宛ならこちら
    MySerial.print("Addr 0x54: ");
    for (uint8_t i = 0; i < numBytes; i++) {
      char c = Wire.read();
      MySerial.write(c);
    }
  } else if (addr == (0x64 << 1)) {           // 0x64宛ならこちら
    MySerial.print("Addr 0x64: ");
    for (uint8_t i = 0; i < numBytes; i++) {
      char c = Wire.read();
      MySerial.write(c);
    }
  }
}
