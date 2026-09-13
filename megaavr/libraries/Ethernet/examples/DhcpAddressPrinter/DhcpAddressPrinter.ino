/*
  DHCP による IP アドレス表示

  Ethernet ライブラリの DHCP 拡張を使って DHCP で IP アドレスを取得し、
  取得したアドレスを表示します。WIZnet イーサネットシールドを使用します。

 回路(UkiUkiduino):
 * イーサネットシールド(WIZnet W5100/W5200/W5500)を Uno ヘッダに直挿し
   CS=D10 / MOSI=D11 / MISO=D12 / SCK=D13 (シールド上の SD カードは CS=D4)
 * UkiUkiduino ProMicro の場合: シールドは直挿しできないので配線する
   MOSI=D16 / MISO=D14 / SCK=D15、CS は任意のピン(Ethernet.init(pin) で指定)
 * D13(SCK) は Serial2 の TX と共用のため、Ethernet 使用中は Serial2 を開かないこと

  原作: Tom Igoe (2011/2012)、Arturo Guadalupi 改変 (2015)
  UkiUkiduino向けに日本語化
*/

#include <SPI.h>
#include <Ethernet.h>

// コントローラの MAC アドレスを入力する。
// 新しいイーサネットシールドにはシールド上のシールに MAC アドレスが印刷されている
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

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
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
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
  // 自分の IP アドレスを表示する:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  switch (Ethernet.maintain()) {
    case 1:
      // 更新(renew)失敗
      Serial.println("Error: renewed fail");
      break;

    case 2:
      // 更新(renew)成功
      Serial.println("Renewed success");
      // 自分の IP アドレスを表示する:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    case 3:
      // 再取得(rebind)失敗
      Serial.println("Error: rebind fail");
      break;

    case 4:
      // 再取得(rebind)成功
      Serial.println("Rebind success");
      // 自分の IP アドレスを表示する:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    default:
      // 何も起きていない
      break;
  }
}
