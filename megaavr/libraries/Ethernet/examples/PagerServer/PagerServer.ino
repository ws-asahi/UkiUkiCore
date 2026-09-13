/*
 ページャーサーバー

 受信したメッセージを接続中の全クライアントにエコーする簡単なサーバーです。
 telnet セッションを 2 つ以上つないで、server.available() と server.print()
 の動きを確認してください。

 回路(UkiUkiduino):
 * イーサネットシールド(WIZnet W5100/W5200/W5500)を Uno ヘッダに直挿し
   CS=D10 / MOSI=D11 / MISO=D12 / SCK=D13 (シールド上の SD カードは CS=D4)
 * UkiUkiduino ProMicro の場合: シールドは直挿しできないので配線する
   MOSI=D16 / MISO=D14 / SCK=D15、CS は任意のピン(Ethernet.init(pin) で指定)
 * D13(SCK) は Serial2 の TX と共用のため、Ethernet 使用中は Serial2 を開かないこと

 原作: Juraj Andrassy https://github.com/jandrassy (2020、Ethernet ライブラリ用)
 UkiUkiduino向けに日本語化
*/
#include <Ethernet.h>

// コントローラの MAC アドレスを入力する。
// 新しいイーサネットシールドにはシールド上のシールに MAC アドレスが印刷されている
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// DHCP で取得できなかったときに使う固定 IP アドレス
IPAddress ip(192, 168, 0, 177);

EthernetServer server(2323);

void setup() {

  Serial.begin(9600);
  while (!Serial);

  // CS ピンは Ethernet.init(pin) で変更できる(既定は D10 = Uno 用シールドの配線)
  //Ethernet.init(10);  // UkiUkiduino + Uno 用イーサネットシールド(既定値なので省略可)
  //Ethernet.init(10);  // UkiUkiduino ProMicro: 配線した CS ピンの番号を指定する

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
    Ethernet.begin(mac, ip);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }

  server.begin();

  IPAddress ip = Ethernet.localIP();
  Serial.println();
  Serial.print("To access the server, connect with Telnet client to ");
  Serial.print(ip);
  Serial.println(" 2323");
}

void loop() {

  EthernetClient client = server.available(); // 読めるデータを持つ最初のクライアントか、'false' なクライアントを返す
  if (client) { // 接続中かつ読めるデータがあるときだけ client は true
    String s = client.readStringUntil('\n'); // いずれかのクライアントからのメッセージを読む
    s.trim(); // 末尾に \r があれば取り除く
    Serial.println(s); // メッセージをシリアルモニタに表示する
    client.print("echo: "); // これは送信元クライアントだけに送る
    server.println(s); // メッセージを接続中の全クライアントに送る
#ifndef ARDUINO_ARCH_SAM
    server.flush(); // バッファを吐き出す
#endif /* !defined(ARDUINO_ARCH_SAM) */
  }
}
