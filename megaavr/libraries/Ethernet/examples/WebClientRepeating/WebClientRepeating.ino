/*
 繰り返し接続する Web クライアント

 WIZnet イーサネットシールドを使って Web サーバーに接続し、定期的にリクエストを送ります。
 Arduino イーサネットシールドでも Adafruit のイーサネットシールドでも、
 WIZnet のイーサネットモジュールが載っていれば動作します。

 この例はイーサネットクライアントに MAC アドレス・IP アドレス・DNS アドレスを
 割り当てて DNS を使います。

 回路(UkiUkiduino):
 * イーサネットシールド(WIZnet W5100/W5200/W5500)を Uno ヘッダに直挿し
   CS=D10 / MOSI=D11 / MISO=D12 / SCK=D13 (シールド上の SD カードは CS=D4)
 * UkiUkiduino ProMicro の場合: シールドは直挿しできないので配線する
   MOSI=D16 / MISO=D14 / SCK=D15、CS は任意のピン(Ethernet.init(pin) で指定)
 * D13(SCK) は Serial2 の TX と共用のため、Ethernet 使用中は Serial2 を開かないこと

 原作: Tom Igoe (2012)、Federico Vanzati 改変 (2014)
 https://www.arduino.cc/en/Tutorial/WebClientRepeating
 このコードはパブリックドメインです。
 UkiUkiduino向けに日本語化
 */

#include <SPI.h>
#include <Ethernet.h>

// イーサネットコントローラの MAC アドレスを割り当てる。
// ここに自分のアドレスを入れる:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
// DHCP で取得できなかったときに使う固定 IP アドレス
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

// ライブラリのインスタンスを初期化する:
EthernetClient client;

char server[] = "www.arduino.cc";  // 変更するときは httpRequest() の Host 行も変える
//IPAddress server(64,131,82,241);

unsigned long lastConnectionTime = 0;           // 最後にサーバーへ接続した時刻(ミリ秒)
const unsigned long postingInterval = 10*1000;  // 更新の間隔(ミリ秒)

void setup() {
  // CS ピンは Ethernet.init(pin) で変更できる(既定は D10 = Uno 用シールドの配線)
  //Ethernet.init(10);  // UkiUkiduino + Uno 用イーサネットシールド(既定値なので省略可)
  //Ethernet.init(10);  // UkiUkiduino ProMicro: 配線した CS ピンの番号を指定する

  // シリアルポートを開く:
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
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // イーサネットシールドの初期化に 1 秒ほど猶予を与える:
  delay(1000);
}

void loop() {
  // ネットワーク接続から受信データがあればシリアルポートへ出力する。
  // デバッグ用途のみ:
  if (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // 前回の接続から 10 秒経過していたら
  // 再接続してリクエストを送る:
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }

}

// サーバーへ HTTP 接続する関数:
void httpRequest() {
  // 新しいリクエストを送る前に既存の接続を閉じる。
  // これでイーサネットシールド上のソケットが解放される
  client.stop();

  // 接続に成功したら:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // HTTP GET リクエストを送る:
    client.println("GET /latest.txt HTTP/1.1");
    client.println("Host: www.arduino.cc");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();

    // 接続した時刻を記録する:
    lastConnectionTime = millis();
  } else {
    // 接続できなかった場合:
    Serial.println("connection failed");
  }
}
