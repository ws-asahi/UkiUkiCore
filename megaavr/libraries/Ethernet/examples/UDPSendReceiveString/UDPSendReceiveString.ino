/*
 UDPSendReceiveString

 UDP のメッセージ文字列を受信してシリアルポートに表示し、
 送信元へ "acknowledged" という文字列を返します。

 パソコンからテスト用にメッセージを送受信できる Processing のスケッチを
 ファイル末尾に付けています。

 回路(UkiUkiduino):
 * イーサネットシールド(WIZnet W5100/W5200/W5500)を Uno ヘッダに直挿し
   CS=D10 / MOSI=D11 / MISO=D12 / SCK=D13 (シールド上の SD カードは CS=D4)
 * UkiUkimicro の場合: シールドは直挿しできないので配線する
   MOSI=D16 / MISO=D14 / SCK=D15、CS は任意のピン(Ethernet.init(pin) で指定)
 * D13(SCK) は Serial2 の TX と共用のため、Ethernet 使用中は Serial2 を開かないこと

 原作: Michael Margolis (2010)
 このコードはパブリックドメインです。
 UkiUkiduino向けに日本語化
 */


#include <Ethernet.h>
#include <EthernetUdp.h>

// コントローラの MAC アドレスと IP アドレスを入力する。
// IP アドレスは使用するネットワークに合わせる:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

unsigned int localPort = 8888;      // 待ち受けるローカルポート

// 送受信データ用のバッファ
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // 受信パケットを入れるバッファ
char ReplyBuffer[] = "acknowledged";        // 返信する文字列

// UDP でパケットを送受信するための EthernetUDP インスタンス
EthernetUDP Udp;

void setup() {
  // CS ピンは Ethernet.init(pin) で変更できる(既定は D10 = Uno 用シールドの配線)
  //Ethernet.init(10);  // UkiUkiduino + Uno 用イーサネットシールド(既定値なので省略可)
  //Ethernet.init(10);  // UkiUkimicro: 配線した CS ピンの番号を指定する

  // イーサネットを開始する
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
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // UDP を開始する
  Udp.begin(localPort);
}

void loop() {
  // 受信データがあればパケットを読む
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i=0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // パケットを packetBuffer に読み込む
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // パケットを送ってきた IP アドレスとポートへ返信する
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  }
  delay(10);
}


/*
  この例と組み合わせて動かす Processing スケッチ
 =====================================================

 // Arduino と文字列データを送受信する Processing の UDP サンプル
 // 何かキーを押すと "Hello Arduino" メッセージを送る


 import hypermedia.net.*;

 UDP udp;  // UDP オブジェクトを定義する


 void setup() {
 udp = new UDP( this, 6000 );  // ポート 6000 で新しいデータグラム接続を作る
 //udp.log( true ); 		// <-- 接続の動作を表示する
 udp.listen( true );           // 受信メッセージを待つ
 }

 void draw()
 {
 }

 void keyPressed() {
 String ip       = "192.168.1.177";	// 送信先の IP アドレス
 int port        = 8888;		// 送信先のポート

 udp.send("Hello World", ip, port );   // 送るメッセージ

 }

 void receive( byte[] data ) { 			// <-- 既定のハンドラ
 //void receive( byte[] data, String ip, int port ) {	// <-- 拡張ハンドラ

 for(int i=0; i < data.length; i++)
 print(char(data[i]));
 println();
 }
 */
