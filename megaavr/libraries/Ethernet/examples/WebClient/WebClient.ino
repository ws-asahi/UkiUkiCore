/*
 Web クライアント

 WIZnet イーサネットシールドを使って Web サイト(http://www.google.com)に接続します。

 回路(UkiUkiduino):
 * イーサネットシールド(WIZnet W5100/W5200/W5500)を Uno ヘッダに直挿し
   CS=D10 / MOSI=D11 / MISO=D12 / SCK=D13 (シールド上の SD カードは CS=D4)
 * UkiUkimicro の場合: シールドは直挿しできないので配線する
   MOSI=D16 / MISO=D14 / SCK=D15、CS は任意のピン(Ethernet.init(pin) で指定)
 * D13(SCK) は Serial2 の TX と共用のため、Ethernet 使用中は Serial2 を開かないこと

 原作: David A. Mellis (2009)、Tom Igoe 改変 (2012、Adrian McEwen の成果を基にしています)
 UkiUkiduino向けに日本語化
 */

#include <SPI.h>
#include <Ethernet.h>

// コントローラの MAC アドレスを入力する。
// 新しいイーサネットシールドにはシールド上のシールに MAC アドレスが印刷されている
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// DNS を使いたくない(スケッチを小さくしたい)場合は
// サーバー名の代わりに数値の IP アドレスを使う:
//IPAddress server(74,125,232,128);  // Google の数値 IP(DNS 不使用)
char server[] = "www.google.com";    // Google のホスト名(DNS 使用)

// DHCP で取得できなかったときに使う固定 IP アドレス
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

// 接続先サーバーの IP アドレスとポートで
// イーサネットクライアントライブラリを初期化する
// (HTTP の既定ポートは 80):
EthernetClient client;

// 速度計測用の変数
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // 速度を正確に測りたいときは false にする

void setup() {
  // CS ピンは Ethernet.init(pin) で変更できる(既定は D10 = Uno 用シールドの配線)
  //Ethernet.init(10);  // UkiUkiduino + Uno 用イーサネットシールド(既定値なので省略可)
  //Ethernet.init(10);  // UkiUkimicro: 配線した CS ピンの番号を指定する

  // シリアル通信を開き、ポートが開くのを待つ:
  Serial.begin(9600);
  while (!Serial) {
    ; // シリアルポートの接続を待つ(ネイティブUSBポートでのみ必要)
  }

  // イーサネット接続を開始する:
  Serial.println("Initialize Ethernet with DHCP:");
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
    // DHCP の代わりに固定 IP アドレスで設定を試みる:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // イーサネットシールドの初期化に 1 秒ほど猶予を与える:
  delay(1000);
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.println("...");

  // 接続できたらシリアルに報告する:
  if (client.connect(server, 80)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    // HTTP リクエストを送る:
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
  } else {
    // サーバーに接続できなかった場合:
    Serial.println("connection failed");
  }
  beginMicros = micros();
}

void loop() {
  // サーバーから届いたバイトがあれば
  // 読み出して表示する:
  int len = client.available();
  if (len > 0) {
    byte buffer[80];
    if (len > 80) len = 80;
    client.read(buffer, len);
    if (printWebData) {
      Serial.write(buffer, len); // シリアルモニタに表示する(ボードによっては遅くなる)
    }
    byteCount = byteCount + len;
  }

  // サーバーとの接続が切れたらクライアントを停止する:
  if (!client.connected()) {
    endMicros = micros();
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    Serial.print("Received ");
    Serial.print(byteCount);
    Serial.print(" bytes in ");
    float seconds = (float)(endMicros - beginMicros) / 1000000.0;
    Serial.print(seconds, 4);
    float rate = (float)byteCount / seconds / 1000.0;
    Serial.print(", rate = ");
    Serial.print(rate);
    Serial.print(" kbytes/second");
    Serial.println();

    // 以後何もしない:
    while (true) {
      delay(1);
    }
  }
}
