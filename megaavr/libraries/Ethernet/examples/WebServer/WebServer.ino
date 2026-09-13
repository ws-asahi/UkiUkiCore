/*
 Web サーバー

 アナログ入力ピンの値を表示する簡単な Web サーバーです。
 WIZnet イーサネットシールドを使用します。

 回路(UkiUkiduino):
 * イーサネットシールド(WIZnet W5100/W5200/W5500)を Uno ヘッダに直挿し
   CS=D10 / MOSI=D11 / MISO=D12 / SCK=D13 (シールド上の SD カードは CS=D4)
 * UkiUkimicro の場合: シールドは直挿しできないので配線する
   MOSI=D16 / MISO=D14 / SCK=D15、CS は任意のピン(Ethernet.init(pin) で指定)
 * D13(SCK) は Serial2 の TX と共用のため、Ethernet 使用中は Serial2 を開かないこと
 * アナログ入力を A0〜A5 に接続(任意)

 原作: David A. Mellis (2009)、Tom Igoe 改変 (2012)、Arturo Guadalupi 改変 (2015)
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

// 使用する IP アドレスとポートで
// イーサネットサーバーライブラリを初期化する
// (HTTP の既定ポートは 80):
EthernetServer server(80);

void setup() {
  // CS ピンは Ethernet.init(pin) で変更できる(既定は D10 = Uno 用シールドの配線)
  //Ethernet.init(10);  // UkiUkiduino + Uno 用イーサネットシールド(既定値なので省略可)
  //Ethernet.init(10);  // UkiUkimicro: 配線した CS ピンの番号を指定する

  // シリアル通信を開き、ポートが開くのを待つ:
  Serial.begin(9600);
  while (!Serial) {
    ; // シリアルポートの接続を待つ(ネイティブUSBポートでのみ必要)
  }
  Serial.println("Ethernet WebServer Example");

  // イーサネット接続とサーバーを開始する:
  Ethernet.begin(mac, ip);

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

  // サーバーを開始する
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {
  // 接続してくるクライアントを待ち受ける
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // HTTP リクエストは空行で終わる
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // 行末(改行文字)に達し、かつその行が空行なら
        // HTTP リクエストは終わっているので応答を返してよい
        if (c == '\n' && currentLineIsBlank) {
          // 標準的な HTTP 応答ヘッダを送る
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // 応答完了後に接続を閉じる
          client.println("Refresh: 5");  // 5 秒ごとにページを自動更新する
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // 各アナログ入力ピンの値を出力する
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // 新しい行の始まり
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // 現在の行に文字がある
          currentLineIsBlank = false;
        }
      }
    }
    // Web ブラウザがデータを受け取る時間を与える
    delay(1);
    // 接続を閉じる:
    client.stop();
    Serial.println("client disconnected");
  }
}
