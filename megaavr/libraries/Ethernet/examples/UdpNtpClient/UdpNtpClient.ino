/*
 UDP NTP クライアント

 NTP(Network Time Protocol)サーバーから時刻を取得します。
 UDP の sendPacket / ReceivePacket の使い方を示します。
 NTP サーバーとやり取りするメッセージの詳細は
 https://en.wikipedia.org/wiki/Network_Time_Protocol を参照してください。

 回路(UkiUkiduino):
 * イーサネットシールド(WIZnet W5100/W5200/W5500)を Uno ヘッダに直挿し
   CS=D10 / MOSI=D11 / MISO=D12 / SCK=D13 (シールド上の SD カードは CS=D4)
 * UkiUkiduino ProMicro の場合: シールドは直挿しできないので配線する
   MOSI=D16 / MISO=D14 / SCK=D15、CS は任意のピン(Ethernet.init(pin) で指定)
 * D13(SCK) は Serial2 の TX と共用のため、Ethernet 使用中は Serial2 を開かないこと

 原作: Michael Margolis (2010)、Tom Igoe 改変 (2012)、Arturo Guadalupi 改変 (2015)
 このコードはパブリックドメインです。
 UkiUkiduino向けに日本語化
 */

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// コントローラの MAC アドレスを入力する。
// 新しいイーサネットシールドにはシールド上のシールに MAC アドレスが印刷されている
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned int localPort = 8888;       // UDP パケットを待ち受けるローカルポート

const char timeServer[] = "time.nist.gov"; // time.nist.gov の NTP サーバー

const int NTP_PACKET_SIZE = 48; // NTP のタイムスタンプはメッセージ先頭の 48 バイトにある

byte packetBuffer[NTP_PACKET_SIZE]; // 送受信パケットを入れるバッファ

// UDP でパケットを送受信するための UDP インスタンス
EthernetUDP Udp;

void setup() {
  // CS ピンは Ethernet.init(pin) で変更できる(既定は D10 = Uno 用シールドの配線)
  //Ethernet.init(10);  // UkiUkiduino + Uno 用イーサネットシールド(既定値なので省略可)
  //Ethernet.init(10);  // UkiUkiduino ProMicro: 配線した CS ピンの番号を指定する

  // シリアル通信を開き、ポートが開くのを待つ:
  Serial.begin(9600);
  while (!Serial) {
    ; // シリアルポートの接続を待つ(ネイティブUSBポートでのみ必要)
  }

  // イーサネットと UDP を開始する
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // イーサネットのハードウェアがあるか確認する
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // 続けても意味がないので、以後何もしない:
    while (true) {
      delay(1);
    }
  }
  Udp.begin(localPort);
}

void loop() {
  sendNTPpacket(timeServer); // タイムサーバーへ NTP パケットを送る

  // 応答が届くか待つ
  delay(1000);
  if (Udp.parsePacket()) {
    // パケットを受信したのでデータを読む
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // パケットをバッファに読み込む

    // タイムスタンプは受信パケットの 40 バイト目から 4 バイト(2 ワード)。
    // まず 2 つのワードを取り出す:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // 4 バイト(2 ワード)を long 整数に結合する。
    // これが NTP 時刻(1900 年 1 月 1 日からの秒数):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // NTP 時刻を普段使う時刻に変換する:
    Serial.print("Unix time = ");
    // Unix 時刻は 1970 年 1 月 1 日が起点。秒に直すと 2208988800 の差:
    const unsigned long seventyYears = 2208988800UL;
    // 70 年分を引く:
    unsigned long epoch = secsSince1900 - seventyYears;
    // Unix 時刻を表示する:
    Serial.println(epoch);


    // 時・分・秒を表示する:
    Serial.print("The UTC time is ");       // UTC はグリニッジ標準時(GMT)
    Serial.print((epoch  % 86400L) / 3600); // 時を表示(86400 は 1 日の秒数)
    Serial.print(':');
    if (((epoch % 3600) / 60) < 10) {
      // 毎時の最初の 10 分は先頭に '0' を付ける
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // 分を表示(3600 は 1 時間の秒数)
    Serial.print(':');
    if ((epoch % 60) < 10) {
      // 毎分の最初の 10 秒は先頭に '0' を付ける
      Serial.print('0');
    }
    Serial.println(epoch % 60); // 秒を表示
  }
  // 次に時刻を問い合わせるまで 10 秒待つ
  delay(10000);
  Ethernet.maintain();
}

// 指定アドレスのタイムサーバーへ NTP リクエストを送る
void sendNTPpacket(const char * address) {
  // バッファを全て 0 にする
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // NTP リクエストに必要な値を設定する
  // (パケットの詳細は上記 URL を参照)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum(時計の種別)
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // Root Delay と Root Dispersion は 8 バイトの 0
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // NTP の全フィールドに値を入れたので、
  // タイムスタンプを要求するパケットを送る:
  Udp.beginPacket(address, 123); // NTP リクエストはポート 123 へ
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
