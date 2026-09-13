/*
 高機能チャットサーバー

 受信したメッセージを、送信元を除く接続中の全クライアントに配信する
 少し高度なサーバーです。
 使い方: このボードの IP アドレスへ telnet で接続して文字を入力してください。
 クライアントの入力はシリアルモニタでも確認できます。
 WIZnet イーサネットシールドを使用します。

 回路(UkiUkiduino):
 * イーサネットシールド(WIZnet W5100/W5200/W5500)を Uno ヘッダに直挿し
   CS=D10 / MOSI=D11 / MISO=D12 / SCK=D13 (シールド上の SD カードは CS=D4)
 * UkiUkiduino ProMicro の場合: シールドは直挿しできないので配線する
   MOSI=D16 / MISO=D14 / SCK=D15、CS は任意のピン(Ethernet.init(pin) で指定)
 * D13(SCK) は Serial2 の TX と共用のため、Ethernet 使用中は Serial2 を開かないこと

 原作: David A. Mellis (2009)、Tom Igoe 改変 (2012)、
       Norbert Truchsess が operator== を使う形に再設計 (2013)
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

EthernetClient clients[8];

void setup() {
  // CS ピンは Ethernet.init(pin) で変更できる(既定は D10 = Uno 用シールドの配線)
  //Ethernet.init(10);  // UkiUkiduino + Uno 用イーサネットシールド(既定値なので省略可)
  //Ethernet.init(10);  // UkiUkiduino ProMicro: 配線した CS ピンの番号を指定する

  // イーサネットデバイスを初期化する
  Ethernet.begin(mac, ip, myDns, gateway, subnet);

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
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // クライアントの接続待ちを開始する
  server.begin();

  Serial.print("Chat server address:");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // 新しく接続してきたクライアントがあれば(データ受信前に)挨拶する
  EthernetClient newClient = server.accept();
  if (newClient) {
    for (byte i=0; i < 8; i++) {
      if (!clients[i]) {
        Serial.print("We have a new client #");
        Serial.println(i);
        newClient.print("Hello, client number: ");
        newClient.println(i);
        // accept() したクライアントは EthernetServer の管理から外れるので、
        // 自分のクライアント一覧に保存しておく必要がある
        clients[i] = newClient;
        break;
      }
    }
  }

  // 全クライアントからの受信データを確認する
  for (byte i=0; i < 8; i++) {
    if (clients[i] && clients[i].available() > 0) {
      // クライアントからバイト列を読む
      byte buffer[80];
      int count = clients[i].read(buffer, 80);
      // 送信元以外の接続中クライアント全員に書き出す
      for (byte j=0; j < 8; j++) {
        if (j != i && clients[j].connected()) {
          clients[j].write(buffer, count);
        }
      }
    }
  }

  // 切断されたクライアントを停止する
  for (byte i=0; i < 8; i++) {
    if (clients[i] && !clients[i].connected()) {
      Serial.print("disconnect client #");
      Serial.println(i);
      clients[i].stop();
    }
  }
}
