/* Wire Slave Address Mask(スレーブ: アドレスマスク)
 * 原作: Spence Konde (MX682Xの成果を元に作成)
 *
 * Wireライブラリの使用例です。
 * I2C/TWIスレーブとして「複数のアドレス」で受信します。
 * 対向側は「Wire Master Multi-Address Write」サンプルを
 * 使用してください。
 *
 * どのアドレス宛で受信したかと、受信データをシリアルモニタへ
 * テキストと16進の両方で表示します(実際のI2C通信ではASCII文字
 * 以外が流れることが多いため)。
 *
 * 全てのゼネラルコールに加え、次のアドレスに反応します:
 * 0x04*, 0x14, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74*
 * *印の0x04と0x74は正規のI2Cアドレスではありません。
 *     0x04~0x07は「高速(High-speed)」モード用の予約
 *     (「Fastモード」や「Fast-mode Plus」とは別物です)。
 *     0x74は「将来のために予約」されたアドレスです。
 *
 * ISR内で直接シリアル出力する代わりに、フラグを立てておいて
 * loop()側で表示しています(ISR内でのシリアル出力は避けること。
 * デバッグでやむを得ない場合も最小限の文字数に)。
 *
 * コマンド本体はinput配列に入っているため、表示中は割り込みを
 * 禁止しています。表示の途中で次の受信に書き換えられるのを防ぐ
 * ためです。ここでは壊れても実害はありませんが、データを別の
 * 用途に使う場合は重大な問題になり得ます。
 *
 * このサンプルは(第2アドレス版よりも)実用の出発点になるよう、
 * 網羅的に書いてあります。
 *
 * 使い方: このボードのSCL(A5)/SDA(A4)を、Multi Address Writeサンプルを
 * 実行しているもう1台のボードのSCL/SDAへ接続します。
 * SDA/SCLの両方に、Vccへのプルアップ抵抗が必要です。
 * 詳しくはWireライブラリのREADME.mdを参照してください。
 *
 * UkiUkiduino向けに日本語化
 */
#include <Wire.h>

volatile uint8_t input[32];       // 受信データは割り込みハンドラでここへ格納する(receiveDataWire()参照)
volatile uint8_t gotMessage = 0;  // 受信したアドレスをここへ入れる。1ビット左シフト済みなので、
//                                   空いた最下位ビットを「受信あり」フラグに使える(ゼネラルコールでも)
volatile uint8_t lenMessage = 0;  // 受信長はここへ入れる
#define MySerial Serial           // PCと接続しているシリアルポート


void setup() {
  // ゼネラルコールを受け、アドレス上位ビットを無視するスレーブとして初期化する
  // 第1引数: 待ち受ける第1アドレス
  // 第2引数: 一斉呼び出し(ゼネラルコール、アドレス0x00)を受けるか
  // 第3引数: ビット0が1なら、ビット7~1は第2アドレス
  //          ビット0が0なら、アドレスマスクとして扱われる
  //          WIRE_ALT_ADDRESS(7ビットアドレス)または
  //          WIRE_ADDRESS_MASK(7ビットマスク)マクロが使える
  // マクロはシフトと最下位ビットの設定をまとめて行うもので、意図が明確になります。
  // 7ビットのアドレス/マスクは0x01~0x7Fです。それより大きい値を渡すと暗黙の切り捨て
  // 警告が出ます(先にシフトした値をマクロへ渡した場合に起きがちです。手動でシフト
  // 済みの値を直接渡すか、シフト前の値にマクロを使ってください)。
  Wire.begin(0x54, true, WIRE_ADDRESS_MASK(0x78));
  // これで「アドレス0x54」のTWIスレーブが始まりますが、上位4ビットが無視されるため
  // 実際にはアドレスの下位3ビットだけが照合されます。0x74でも0x04でも0x4Cでも同じで、
  // 0bxxxx100(xは任意)にマッチします。
  Wire.onReceive(receiveDataWire);

  // シリアルポートを初期化する
  MySerial.begin(115200);
}

void loop() {
  delay(100);
  if (gotMessage & 1) {
    printMessage();
  }
}

void printMessage() {
  uint8_t addr = gotMessage >> 1;
  cli(); // 割り込みを禁止する。表示中に次の受信で上書きされるのを防ぐため。
  //        noInterrupts()と同じ意味です。
  uint8_t len = lenMessage; // lenMessageはvolatileだが、割り込み禁止中は
  // printMessage()の実行中に変化しない。このように一時変数へ退避すると、
  // 参照箇所ごとに2~4バイトのフラッシュと、アクセスごとに2~3クロックを
  // 節約できる(ループがあるため厳密には一致しない)。UkiUkiduinoでは
  // 気にならなくても、フラッシュ2~4KBの小容量マイコンでは効いてくるテクニックです。
  if (addr == 0) {
    MySerial.print("General Call");
    if (len > 1) {
      MySerial.print(" (malformed? Should only be a 1-byte command)");
    } else {
      MySerial.print(": ");
      MySerial.printHex(input[0]);
    }
  } else {
    MySerial.print("Addressed to ");
    MySerial.printHex(addr);
    if ((addr & 0x78) == 0 || addr > 0x6F) {
      MySerial.print(" (not a legal I2C address)");
    }
  }
  MySerial.println();
  if (len > 32) {
    MySerial.print("Oversized message showing 32/");
    MySerial.println(len);
    len = 32;
  }
  if (len > 1 || addr != 0) {
    MySerial.println();
    MySerial.print("Text: ");
    /* ここが冒頭で触れた「データが書き換わると危険」な箇所です。
     * テキストを期待している場合を想像してください。既知の長さ分だけ
     * ループして出力する代わりに、char配列に入れて終端文字を付けて
     * print()へ渡す実装にしていたら、運悪く割り込みが入ると、最初の
     * メッセージの一部+次のメッセージの一部+その後ろのRAMの中身が
     * ゼロに当たるまで延々と表示される、といったことが起こり得ます。
     */
    for (uint8_t i = 0; i < len; i++) {
      MySerial.write(input[i]);
    }
    MySerial.println();
    MySerial.print("Hex: ");
    for (uint8_t i = 0; i < len; i++) {
      MySerial.printHex(input[i]);
    }
    MySerial.println();
  }
  gotMessage = 0;
  sei(); // 割り込みを再び許可する。interrupts()と同じ意味です。
}

// マスタからデータを受信するたびに実行される関数
// (setup()でイベントとして登録済み)
void receiveDataWire(int16_t numBytes) {
  uint8_t addr = Wire.getIncomingAddress();
  // どのアドレス宛だったかを取得する
  // 取得値は1ビット左シフトされているので、
  // 空いた最下位ビットを「受信あり」フラグとして利用する。
  // ISRですべきことは、メッセージを読み、アプリにとって
  // 「タイミングが重要」な処理だけを行い、受信内容を記録することです。
  for (uint8_t i = 0; i < numBytes; i++) {
    if (i < 32) {               // 異常に大きいペイロードが来ても
      input[i] = Wire.read();   // input配列からあふれないようにする
    }
  }
  gotMessage = addr | 1; // アドレスを、最下位ビットに1を立てて保存する
  lenMessage = numBytes;
}
