/*
 DHCP チャットサーバー

 受信したメッセージを接続中の全クライアントに配信する簡単なサーバーです。
 使い方: このボードの IP アドレスへ telnet で接続して文字を入力してください。
 クライアントの入力はシリアルモニタでも確認できます。
 WIZnet イーサネットシールドを使用します。

 この版は DHCP で IP アドレスの取得を試みます。

 回路(UkiUkiduino):
 * イーサネットシールド(WIZnet W5100/W5200/W5500)を Uno ヘッダに直挿し
   CS=D10 / MOSI=D11 / MISO=D12 / SCK=D13 (シールド上の SD カードは CS=D4)
 * UkiUkiduino ProMicro の場合: シールドは直挿しできないので配線する
   MOSI=D16 / MISO=D14 / SCK=D15、CS は任意のピン(Ethernet.init(pin) で指定)
 * D13(SCK) は Serial2 の TX と共用のため、Ethernet 使用中は Serial2 を開かないこと

 原作: Tom Igoe (2011/2012)、Arturo Guadalupi 改変 (2015)
       David A. Mellis の ChatServer サンプルを基にしています
 UkiUkiduino向けに日本語化
 */

#include <SPI.h>
#include <Ethernet.h>

// コントローラの MAC アドレスと IP アドレスを入力する。
// IP アドレスは使用するネットワークに合わせる。
// ゲートウェイとサブネットは省略可:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);
IPAddress myDns(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

// telnet の既定ポートは 23
EthernetServer server(23);
bool gotAMessage = false; // クライアントからメッセージを受け取ったかどうか

void setup() {
  // CS ピンは Ethernet.init(pin) で変更できる(既定は D10 = Uno 用シールドの配線)
  //Ethernet.init(10);  // UkiUkiduino + Uno 用イーサネットシールド(既定値なので省略可)
  //Ethernet.init(10);  // UkiUkiduino ProMicro: 配線した CS ピンの番号を指定する

  // シリアル通信を開き、ポートが開くのを待つ:
  Serial.begin(9600);
  while (!Serial) {
    ; // シリアルポートの接続を待つ(ネイティブUSBポートでのみ必要)
  }

  // イーサネット接続を開始する:
  Serial.println("Trying to get an IP address using DHCP");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // イーサネットのハードウェアがあるか確認する
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // ハードウェアが無ければ動かしようがないので何もしない
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // DHCP を使わずに固定アドレスで初期化する:
    Ethernet.begin(mac, ip, myDns, gateway, subnet);
  }
  // 自分の IP アドレスを表示する:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());

  // クライアントの接続待ちを開始する
  server.begin();
}

void loop() {
  // 新しいクライアントを待つ:
  EthernetClient client = server.available();

  // クライアントが最初の 1 バイトを送ってきたら挨拶する:
  if (client) {
    if (!gotAMessage) {
      Serial.println("We have a new client");
      client.println("Hello, client!");
      gotAMessage = true;
    }

    // クライアントから届いたバイトを読む:
    char thisChar = client.read();
    // 全クライアントへエコーバックする:
    server.write(thisChar);
    // シリアルモニタにもエコーする:
    Serial.print(thisChar);
    Ethernet.maintain();
  }
}
