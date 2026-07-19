/*
  SDカードの基本ファイル操作

  SDカード上のファイルの作成と削除の方法を示します。

  回路(UkiUkiduino):
   SDカードをSPIバスに次のように接続:
 ** MOSI - D11
 ** MISO - D12
 ** SCK  - D13
 ** CS   - PIN_SPI_SS (D10)

  原作: David A. Mellis (2010) / Tom Igoe改変 (2012)
  この例はパブリックドメインです。
  UkiUkiduino向けに日本語化
*/
#include <SPI.h>
#include <SD.h>

File myFile;

void setup() {
  // シリアル通信を開き、ポートが開くのを待つ:
  Serial.begin(115200);
  while (!Serial) {
    ; // シリアルポートの接続を待つ(ネイティブUSBポートでのみ必要)
  }


  Serial.print("Initializing SD card...");

  if (!SD.begin(PIN_SPI_SS)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }

  // 新しいファイルを開いてすぐ閉じる:
  Serial.println("Creating example.txt...");
  myFile = SD.open("example.txt", FILE_WRITE);
  myFile.close();

  // ファイルができたか確認する:
  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }

  // ファイルを削除する:
  Serial.println("Removing example.txt...");
  SD.remove("example.txt");

  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }
}

void loop() {
  // setupの後は何もしない
}
