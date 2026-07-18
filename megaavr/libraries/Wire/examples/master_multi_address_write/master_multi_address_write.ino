/* Wire Multi Address Write(マスタ: 複数アドレスへの書き込み)
 * 原作: Spence Konde (MX682Xの成果を元に作成)
 *
 * 第2アドレス/アドレスマスクを使うスレーブサンプルの対向用
 * マスタです。スレーブ側は「Slave Secondary Address」または
 * 「Slave Address Mask」サンプルを使用してください。
 *
 * シリアルモニタからの入力を送信します。入力はCRかLF、または
 * 30文字で区切られます。宛先アドレスの決まり方はDUAL_ADDRESS_ONLYの
 * 定義有無で変わります。
 *
 * 定義したまま(既定) → Slave Secondary Address用:
 *   先頭文字が数字なら、アドレス0x54のスレーブへ送ります。
 *   それ以外なら、アドレス0x64のスレーブへ送ります。
 * コメントアウトする → Slave Address Mask用:
 *   先頭文字が0~7なら、それをアドレスの上位桁(16進)として使います。
 *   例: '5test'は0x54へ、'2test'は0x24へ送られます。
 *   それ以外なら、アドレス0(ゼネラルコール)へ送ります。I2C仕様
 *   (Rev 7.0)ではゼネラルコールには最低1バイトのペイロードが必要で、
 *   先頭バイトは次のどちらかです:
 *     0x06 - リセットし、事前に設定された新アドレスを使う
 *     0x04 - 事前に設定された新アドレスを使う(リセットなし)
 *   その他のコードは通常無視されます。
 *
 * 使い方: このボードのSCL(A5)/SDA(A4)を、上記スレーブサンプルを
 * 実行しているもう1台のボードのSCL/SDAへ接続します。
 * SDA/SCLの両方に、Vccへのプルアップ抵抗が必要です。
 * 詳しくはWireライブラリのREADME.mdを参照してください。
 *
 * メモ: 原本はSEC_ADDRESS_ONLYを定義しつつDUAL_ADDRESS_ONLYで判定して
 * おり切替が機能していなかったため、マクロ名を統一して修正済みです。
 *
 * UkiUkiduino向けに移植・日本語化
 */
#include <Wire.h>

char input[32];
int8_t len = 0;

#define DUAL_ADDRESS_ONLY
// 第2アドレス方式のスレーブ(Slave Secondary Address)と組む場合は
// このまま。アドレスマスク方式(Slave Address Mask)と組む場合は
// コメントアウトしてください。

#define MySerial Serial             // PCと接続しているシリアルポート

void setup() {
  Wire.begin();                     // マスタとして初期化する
  MySerial.begin(115200);           // 115200ボーを使用する
}

void loop() {
  if (MySerial.available() > 0) {   // シリアルに1バイト届いたら
    readFromSerial();               // シリアルからデータを読み出す
    if (len > 0) {                  // 有効なデータがあれば
      sendDataWire();               // I2Cへ送信する
    }
    len = 0;                        // 送信済みなので位置を0に戻す
  }
}

void readFromSerial() {
  while (true) {                    // 無限ループの中で
    char c = MySerial.read();       // 次の文字を読む
    while (c == -1) {               // バッファが空だとread()は-1を返すので
      c = MySerial.read();          // 有効な文字が来るまで読み直す
    }
    if (c == '\n' || c == '\r') {   // 改行(LF)か復帰(CR)が来たら
      break;                        // ループを抜ける
    }                               // それ以外なら
    input[len] = c;                 // 文字を保存して
    len++;                          // 位置を進める
    if (len > 30) {                 // データが多すぎる場合は
      break;                        // バッファあふれ防止のため抜ける
    }
  }
}

void sendDataWire() {
  uint8_t firstElement = input[0];
  uint8_t address = 0;
  #if defined(DUAL_ADDRESS_ONLY)
  if (firstElement >= '0' && firstElement <= '9') {   // 先頭文字が数字かどうか調べる
    address = 0x54;
  } else {
    address = 0x64;
  }
  #else
  if (firstElement >= '0' && firstElement <= '7') {   // 先頭文字が0~7かどうか調べる
    address = firstElement - '0';  // charは8ビットの符号付き整数なので引き算でよい
    address <<= 4; // 得られた0~7を4ビット左シフトして上位桁にする
    address |= 4;  // 下位桁は取り決めどおり4にする(0x_4)
  } else {
    address = 0;
  }
  #endif
  MySerial.print("Sending to: ");
  MySerial.printHex(address);
  MySerial.println();
  Wire.beginTransmission(address);
  Wire.write(input, len);

  #if defined(DUAL_ADDRESS_ONLY)
  Wire.write("\r\n");              // シリアルモニタ用に改行を付ける
  #endif
  Wire.endTransmission();          // 送信を完了する
}
