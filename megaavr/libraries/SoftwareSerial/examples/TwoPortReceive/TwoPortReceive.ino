/* TwoPortReceive(2ポート受信)
   2つのソフトウェアシリアルポートから受信し、USBシリアル(Serial)へ
   表示するサンプルです。

   接続: ポート1 = D10(RX)/D11(TX)、ポート2 = D8(RX)/D9(TX)。

   重要: ソフトウェアシリアルを複数使う場合、同時に受信できるのは
   1ポートだけです。受信したいポートをlisten()で切り替えながら
   使います。切り替えるタイミングは「相手の送信が一区切りついた時」や
   「バッファが空になった時」など、取りこぼしが起きにくい時を選びます。
   このサンプルでは読み尽くしたタイミングで切り替えています。

   メモ: SoftwareSerialは互換性のために収録されています。可能であれば
   ハードウェアUART(Serial1 = D0/D1)の使用を推奨します。

   原作: Tom Igoe / Mikal Hart (パブリックドメイン)
   UkiUkiduino向けに移植・日本語化
*/

#include <SoftwareSerial.h>
// ソフトウェアシリアル1: RX = D10, TX = D11
SoftwareSerial portOne(10, 11);

// ソフトウェアシリアル2: RX = D8, TX = D9
SoftwareSerial portTwo(8, 9);

void setup() {
  // USBシリアルを開き、ポートが開くまで待つ
  Serial.begin(115200);
  while (!Serial) {
    ; // USB接続のシリアルポートが開くのを待つ
  }

  // 各ソフトウェアシリアルポートを開始する
  portOne.begin(9600);
  portTwo.begin(9600);
}

void loop() {
  // 初期状態では最後にbegin()したポートが受信状態になっている。
  // 受信したいポートをlisten()で明示的に選択する:
  portOne.listen();
  Serial.println("Data from port one:");
  // データが届いている間、読み取ってUSBシリアルへ送る
  while (portOne.available() > 0) {
    char inByte = portOne.read();
    Serial.write(inByte);
  }

  // 2つのポートの出力を区切る空行
  Serial.println();

  // 次にポート2を受信状態にする
  portTwo.listen();
  Serial.println("Data from port two:");
  // データが届いている間、読み取ってUSBシリアルへ送る
  while (portTwo.available() > 0) {
    char inByte = portTwo.read();
    Serial.write(inByte);
  }

  // 2つのポートの出力を区切る空行
  Serial.println();
}
