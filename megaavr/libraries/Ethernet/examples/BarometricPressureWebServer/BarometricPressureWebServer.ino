/*
 SCP1000 気圧センサの Web 表示

 気圧センサの測定値を Web ページとして配信します。
 SPI ライブラリを使用します。センサの詳細:
 http://www.sparkfun.com/commerce/product_info.php?products_id=8161

 Nathan Seidle の PIC 用 SCP1000 サンプルを元にしています:
 http://www.sparkfun.com/datasheets/Sensors/SCP1000-Testing.zip

 ※SCP1000 は既に入手困難ですが、「Ethernet と同じ SPI バスに別のデバイスを
   同居させ、その値を Web で配信する方法」の教材としてこのサンプルを残しています。
   他の SPI センサにも応用できます。

 回路(UkiUkiduino):
 * イーサネットシールド(WIZnet W5100/W5200/W5500)を Uno ヘッダに直挿し
   CS=D10 / MOSI=D11 / MISO=D12 / SCK=D13 (シールド上の SD カードは CS=D4)
 * UkiUkimicro の場合: シールドは直挿しできないので配線する
   MOSI=D16 / MISO=D14 / SCK=D15、CS は任意のピン(Ethernet.init(pin) で指定)
 * D13(SCK) は Serial2 の TX と共用のため、Ethernet 使用中は Serial2 を開かないこと
 * SCP1000 センサ: DRDY=D6 / CSB=D7 / MOSI=D11 / MISO=D12 / SCK=D13
   (UkiUkimicro の場合: DRDY=D6 / CSB=D7、MOSI=D16 / MISO=D14 / SCK=D15)

 原作: Tom Igoe (2010)
 UkiUkiduino向けに日本語化
 */

#include <Ethernet.h>
// センサは SPI で通信するのでライブラリを読み込む:
#include <SPI.h>


// イーサネットコントローラの MAC アドレスを割り当てる。
// ここに自分のアドレスを入れる:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
// コントローラの IP アドレスを割り当てる:
IPAddress ip(192, 168, 1, 20);


// 使用する IP アドレスとポートで
// イーサネットサーバーライブラリを初期化する
// (HTTP の既定ポートは 80):
EthernetServer server(80);


// センサのレジスタアドレス:
const int PRESSURE = 0x1F;      // 気圧の上位 3 ビット
const int PRESSURE_LSB = 0x20;  // 気圧の下位 16 ビット
const int TEMPERATURE = 0x21;   // 16 ビットの温度

// センサとの接続に使うピン
// (これ以外のSPIピンは SPI ライブラリが管理する):
const int dataReadyPin = 6;
const int chipSelectPin = 7;

float temperature = 0.0;
long pressure = 0;
long lastReadingTime = 0;

void setup() {
  // CS ピンは Ethernet.init(pin) で変更できる(既定は D10 = Uno 用シールドの配線)
  //Ethernet.init(10);  // UkiUkiduino + Uno 用イーサネットシールド(既定値なので省略可)
  //Ethernet.init(10);  // UkiUkimicro: 配線した CS ピンの番号を指定する

  // SPI ライブラリを開始する:
  SPI.begin();

  // イーサネット接続を開始する
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

  // クライアントの接続待ちを開始する
  server.begin();

  // DRDY と CS のピンを初期化する:
  pinMode(dataReadyPin, INPUT);
  pinMode(chipSelectPin, OUTPUT);

  // SCP1000 を低ノイズ構成に設定する:
  writeRegister(0x02, 0x2D);
  writeRegister(0x01, 0x03);
  writeRegister(0x03, 0x02);

  // センサとイーサネットシールドの立ち上がりを待つ:
  delay(1000);

  // センサを高分解能モードにして測定を開始する:
  writeRegister(0x03, 0x0A);

}

void loop() {
  // 測定は 1 秒に 1 回まで。
  if (millis() - lastReadingTime > 1000) {
    // 測定値が準備できていれば読む:
    // DRDY ピンが HIGH になるまで何もしない:
    if (digitalRead(dataReadyPin) == HIGH) {
      getData();
      // 最後に測定した時刻を記録する:
      lastReadingTime = millis();
    }
  }

  // イーサネットの接続を待ち受ける:
  listenForEthernetClients();
}


void getData() {
  Serial.println("Getting reading");
  // 温度データを読む
  int tempData = readRegister(0x21, 2);

  // 温度を摂氏に変換して表示する:
  temperature = (float)tempData / 20.0;

  // 気圧データの上位 3 ビットを読む:
  byte  pressureDataHigh = readRegister(0x1F, 1);
  pressureDataHigh &= 0b00000111; // 必要なのはビット 2〜0 だけ

  // 気圧データの下位 16 ビットを読む:
  unsigned int pressureDataLow = readRegister(0x20, 2);
  // 2 つを結合して 19 ビットの値にする:
  pressure = ((pressureDataHigh << 16) | pressureDataLow) / 4;

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" degrees C");
  Serial.print("Pressure: " + String(pressure));
  Serial.println(" Pa");
}

void listenForEthernetClients() {
  // 接続してくるクライアントを待ち受ける
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Got a client");
    // HTTP リクエストは空行で終わる
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // 行末(改行文字)に達し、かつその行が空行なら
        // HTTP リクエストは終わっているので応答を返してよい
        if (c == '\n' && currentLineIsBlank) {
          // 標準的な HTTP 応答ヘッダを送る
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          // 現在の測定値を HTML 形式で出力する:
          client.print("Temperature: ");
          client.print(temperature);
          client.print(" degrees C");
          client.println("<br />");
          client.print("Pressure: " + String(pressure));
          client.print(" Pa");
          client.println("<br />");
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
  }
}


// SCP1000 に書き込みコマンドを送る
void writeRegister(byte registerName, byte registerValue) {
  // SCP1000 はレジスタ名をバイトの上位 6 ビットに期待する:
  registerName <<= 2;
  // コマンド(読み/書き)は下位 2 ビット:
  registerName |= 0b00000010; // 書き込みコマンド

  // チップセレクトを LOW にしてデバイスを選択する:
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(registerName); // レジスタ位置を送る
  SPI.transfer(registerValue); // レジスタに記録する値を送る

  // チップセレクトを HIGH にして選択を解除する:
  digitalWrite(chipSelectPin, HIGH);
}


// SCP1000 のレジスタを読む:
unsigned int readRegister(byte registerName, int numBytes) {
  byte inByte = 0;           // SPI で読んだ受信バイト
  unsigned int result = 0;   // 返す結果

  // SCP1000 はレジスタ名をバイトの上位 6 ビットに期待する:
  registerName <<=  2;
  // コマンド(読み/書き)は下位 2 ビット:
  registerName &= 0b11111100; // 読み出しコマンド

  // チップセレクトを LOW にしてデバイスを選択する:
  digitalWrite(chipSelectPin, LOW);
  // 読みたいレジスタをデバイスに送る:
  SPI.transfer(registerName);
  // 0 を送って最初の返信バイトを読む:
  inByte = SPI.transfer(0x00);

  result = inByte;
  // 返信が 2 バイト以上なら、
  // 最初のバイトをシフトしてから 2 バイト目を読む:
  if (numBytes > 1) {
    result = inByte << 8;
    inByte = SPI.transfer(0x00);
    result = result | inByte;
  }
  // チップセレクトを HIGH にして選択を解除する:
  digitalWrite(chipSelectPin, HIGH);
  // 結果を返す:
  return (result);
}
