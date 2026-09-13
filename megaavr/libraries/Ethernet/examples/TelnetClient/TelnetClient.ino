/*
 Telnet クライアント

 WIZnet イーサネットシールドを使って telnet サーバーに接続します。
 動作確認には telnet サーバーが必要です。
 Processing の Network ライブラリに含まれる ChatServer サンプル(ポート 10002)が
 相手として使えます。Processing は https://processing.org/ から入手できます。

 回路(UkiUkiduino):
 * イーサネットシールド(WIZnet W5100/W5200/W5500)を Uno ヘッダに直挿し
   CS=D10 / MOSI=D11 / MISO=D12 / SCK=D13 (シールド上の SD カードは CS=D4)
 * UkiUkimicro の場合: シールドは直挿しできないので配線する
   MOSI=D16 / MISO=D14 / SCK=D15、CS は任意のピン(Ethernet.init(pin) で指定)
 * D13(SCK) は Serial2 の TX と共用のため、Ethernet 使用中は Serial2 を開かないこと

 原作: Tom Igoe (2010/2012)
 UkiUkiduino向けに日本語化
 */

#include <SPI.h>
#include <Ethernet.h>

// コントローラの MAC アドレスと IP アドレスを入力する。
// IP アドレスは使用するネットワークに合わせる:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

// 接続先サーバーの IP アドレスを入力する:
IPAddress server(1, 1, 1, 1);

// 接続先サーバーの IP アドレスとポートで
// イーサネットクライアントライブラリを初期化する
// (telnet の既定ポートは 23、Processing の ChatServer なら 10002):
EthernetClient client;

void setup() {
  // CS ピンは Ethernet.init(pin) で変更できる(既定は D10 = Uno 用シールドの配線)
  //Ethernet.init(10);  // UkiUkiduino + Uno 用イーサネットシールド(既定値なので省略可)
  //Ethernet.init(10);  // UkiUkimicro: 配線した CS ピンの番号を指定する

  // イーサネット接続を開始する:
  Ethernet.begin(mac, ip);

  // シリアル通信を開き、ポートが開くのを待つ:
  Serial.begin(9600);
  while (!Serial) {
    ; // シリアルポートの接続を待つ(ネイティブUSBポートでのみ必要)
  }

  // イーサネットのハードウェアがあるか確認する
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // ハードウェアが無ければ動かしようがないので何もしない
    }
  }
  while (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
    delay(500);
  }

  // イーサネットシールドの初期化に 1 秒ほど猶予を与える:
  delay(1000);
  Serial.println("connecting...");

  // 接続できたらシリアルに報告する:
  if (client.connect(server, 10002)) {
    Serial.println("connected");
  } else {
    // サーバーに接続できなかった場合:
    Serial.println("connection failed");
  }
}

void loop() {
  // サーバーから届いたバイトがあれば
  // 読み出して表示する:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // シリアルの受信キューにバイトがある限り読み出し、
  // ソケットが開いていればそこへ送る:
  while (Serial.available() > 0) {
    char inChar = Serial.read();
    if (client.connected()) {
      client.print(inChar);
    }
  }

  // サーバーとの接続が切れたらクライアントを停止する:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    // 以後何もしない:
    while (true) {
      delay(1);
    }
  }
}
