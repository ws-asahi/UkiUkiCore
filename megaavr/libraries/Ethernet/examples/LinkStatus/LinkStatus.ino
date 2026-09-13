/*
  リンク状態の表示

  イーサネットのリンク状態を表示します。LAN ケーブルを接続すると
  リンク状態が "ON" になります。
  注意: リンク状態を報告できるのは WIZnet W5200 と W5500 だけです。
  W5100 は常に "Unknown" を返します。

 回路(UkiUkiduino):
 * イーサネットシールド(WIZnet W5100/W5200/W5500)を Uno ヘッダに直挿し
   CS=D10 / MOSI=D11 / MISO=D12 / SCK=D13 (シールド上の SD カードは CS=D4)
 * UkiUkimicro の場合: シールドは直挿しできないので配線する
   MOSI=D16 / MISO=D14 / SCK=D15、CS は任意のピン(Ethernet.init(pin) で指定)
 * D13(SCK) は Serial2 の TX と共用のため、Ethernet 使用中は Serial2 を開かないこと

  原作: Cristian Maglie (パブリックドメイン)
  UkiUkiduino向けに日本語化
*/

#include <SPI.h>
#include <Ethernet.h>

void setup() {
  // CS ピンは Ethernet.init(pin) で変更できる(既定は D10 = Uno 用シールドの配線)
  //Ethernet.init(10);  // UkiUkiduino + Uno 用イーサネットシールド(既定値なので省略可)
  //Ethernet.init(10);  // UkiUkimicro: 配線した CS ピンの番号を指定する

  Serial.begin(9600);
}

void loop() {
  auto link = Ethernet.linkStatus();
  Serial.print("Link status: ");
  switch (link) {
    case Unknown:
      Serial.println("Unknown");
      break;
    case LinkON:
      Serial.println("ON");
      break;
    case LinkOFF:
      Serial.println("OFF");
      break;
  }
  delay(1000);
}
