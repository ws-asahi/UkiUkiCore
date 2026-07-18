/* Wire Register Model(レジスタ方式のスレーブ実装)
 * 原作: Spence Konde
 *
 * Wireライブラリの使用例です。
 * 市販のI2Cデバイスの多くが採用している「レジスタマシン」方式の
 * インターフェースを実装します。マイコン内蔵ペリフェラルの
 * レジスタと同様に、自動インクリメント機能を持ちます。
 *
 * 書き込み: 複数バイトを書くと、先頭バイトが開始アドレス、
 * 2バイト目以降がそのアドレスから順に書かれる値になります。
 * 読み出し: 1バイト(読み始めたいレジスタのアドレス)を書いて
 * 「ポインタ」を設定し、その後好きなバイト数を読み出します。
 * マイコンのレジスタと同じく、読み出し専用のものもあります。
 *
 * ここでは32バイトの配列DeviceRegisters[32]を用意し、
 * 0~31の値で(安直に)初期化しています。あわせてWriteMask[32]で
 * 各「レジスタ」の書き込み可能ビットを定義しています。
 * マスクが1のビットだけ書き換えられ、それ以外は変化しません。
 * 0~4     全ビット書き込み可
 * 5       下位5ビットのみ書き込み可
 * 6~7     全ビット書き込み可
 * 8~11    各ニブルの下位2ビットのみ書き込み可
 * 12~15   読み出し専用
 * 16~17   下位ニブルのみ書き込み可
 * 18~19   上位ニブルのみ書き込み可
 *
 * アドレスポインタは自動的に進み、末尾で先頭へ折り返します。
 *
 * 特別なレジスタが2つあります(4と5)。loop()の動作をレジスタで
 * 制御する例として適当に選んだもので、4と5はLED点滅間隔の
 * 下位/上位バイトです。初期値は0x0504ms = 1284msになります。
 *
 * UkiUkiduino向けに日本語化
 */
#include <Wire.h>
volatile uint8_t DeviceRegisters[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                                        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
                                       };
const uint8_t WriteMask[32]          = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF,
                                        0x33, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00,
                                        0x0F, 0x0F, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                                       };
volatile uint8_t WirePointer = 0;

void setup() {
  Wire.begin(0x69); // スレーブアドレス0x69で開始する
  Wire.onReceive(receiveHandler);
  Wire.onRequest(requestHandler);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(230400);

}

void loop() {
  static uint32_t lastBlinkAt = 0;
  uint16_t delaytime = DeviceRegisters[4] + ((uint16_t)DeviceRegisters[5] << 8);
  if (millis() - lastBlinkAt > delaytime) {
    lastBlinkAt = millis();
    digitalWrite(LED_BUILTIN, CHANGE);
  }
}
/* 受信ハンドラは比較的素直です。読み出し済みバイト数の精算は要求ハンドラ側で行うため、
 * ここではgetBytesRead()を呼んで(戻り値は使わず)カウントをリセットするだけです。
 * アドレスポインタ以外も書かれた場合に配列を更新する行は読みにくいですが、ごく標準的な
 * マスク処理です。マスクが1のビットは新しい値になるので、新値とマスクのANDを取る。
 * マスクが0のビットは元のままなので、マスクを反転して旧値とANDを取る。両者のORが結果です。
 *
 * ポインタと0x1FのビットANDは折り返しのためです。剰余(%)演算と同じ働きですが、
 * 2のべき乗にしか使えない代わりに桁違いに効率的です。剰余は除算を意味し、AVRの除算は
 * ソフトウェア実装(libgcc内のアセンブラルーチン)で遅くて大きい。つまり剰余=除算=悪。
 * AVRでは必要にならない限り除算は避けましょう。micros()の計算からも除算を排除するために
 * かなりの労力が費やされています。
 */
void receiveHandler(int numbytes) {
  Wire.getBytesRead(); // 読み出し済みバイト数をリセットする。値自体は使わない
  WirePointer = Wire.read() & 0x1F; // 配列の外に書き込めないようにする!
  numbytes--; // 1バイト読んだので残数を減らす
  while (numbytes > 0) { // アドレス以外のバイトもあれば、以下のループで「レジスタ」へ書く

    uint8_t unchangedbits = (DeviceRegisters[WirePointer] & ~WriteMask[WirePointer]);
    DeviceRegisters[WirePointer] = (Wire.read() & WriteMask[WirePointer]) | unchangedbits;
    WirePointer++;          // ポインタを進める
    WirePointer &= 0x1F;    // 32を超えたら折り返す
    numbytes--;             // 残りバイト数を減らす
  }
}
/* こちらは少し奇妙です。onRequestの動作と、そこでやるべきことはかなり直感に反します
 * (Arduino製I2Cスレーブの多くが、まっとうなデバイスではなく「クロック付きシリアル」の
 * ようにWireを使ってしまう一因でしょう)。
 *
 * onRequestに登録したハンドラは、自分のアドレス宛の読み出しパケットを受けた時に
 * 「一度だけ」呼ばれ、マスタが読むかもしれない全データを書き出しておきます。次の
 * スタートコンディション+アドレス一致まで再度呼ばれることはありません。マスタは途中で
 * NACKして転送を打ち切るかもしれませんが、Arduino APIにはスレーブが実際に何バイト
 * 読まれたかを知る手段がありません。それを解決するのがgetBytesRead()拡張です。
 *
 * getBytesRead()が無いと、Arduino APIで書かれたスレーブは「マスタが何かを読んだか」に
 * 反応できません。市販のI2Cデバイスではごく一般的な動作なのにです。
 *
 * もう1つの注意点として、バッファアンダーフローが起きるとTWIはSDAを解放したままに
 * するため、マスタからは0xFFが送られてきたように見えます。
 */
void requestHandler() {
  // ポインタの位置から読み出しを始める。
  // ただし直前に読み出しがあり、マスタが続けて2回目の読み出しを
  // 始めた場合は、前回の続きから読めるようにポインタを進める。
  uint8_t bytes_read = Wire.getBytesRead();
  WirePointer       += bytes_read;
  WirePointer       &= 0x1F;
  // 次のように書いても同じです(効率も同じ):
  // WirePointer = (WirePointer + Wire.getBytesRead()) & 0x1F;

  // 「読まれたことに反応する」処理を入れるなら、bytes_readの取得と
  // ポインタ調整の間に入れます。センサを実装するなら、結果レジスタや
  // ステータスレジスタの読み出しに反応させることになるでしょう。
  // スレーブのloop()側でWire.slaveTransactionOpen()がfalseになったのを
  // 確認してからgetBytesRead()を調べる方法もあります。__ただし__
  // このハンドラの実行中、スレーブは「クロックストレッチ」をしている
  // 点に注意してください。実行時間を短く保つことは最優先事項です。
  // スレーブ自身の時間だけでなく、マスタや、バスを待っている他の全ての
  // デバイスの時間も浪費するからです。センサ管理装置を作るなら、値は
  // 測定のたびに「レジスタ」配列へ格納しておくべきで、このハンドラの
  // 中で何十回も測定しに行ってはいけません。これがISRであること、
  // それに伴う全ての制約も忘れずに。

  for (byte i = 0; i < 32; i++) {
    Wire.write(DeviceRegisters[(WirePointer + i) & 0x1F]);
    // 配列全体を「書いて」おく。マスタは1バイトしか読まないかも
    // しれないが、何バイト読むつもりかは、読み終えてストップ
    // コンディションを出すまでスレーブには分からない。
  }

}
