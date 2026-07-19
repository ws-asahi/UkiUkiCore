/*
  SDカードのファイルダンプ

  SDライブラリでSDカードからファイルを読み、シリアルポートへ
  送り出す方法を示します。

  回路(UkiUkiduino):
   SDカードをSPIバスに次のように接続:
 ** MOSI - D11
 ** MISO - D12
 ** SCK  - D13
 ** CS   - PIN_SPI_SS (D10)

  原作: Limor Fried (2010) / Tom Igoe改変 (2012)
  この例はパブリックドメインです。
  UkiUkiduino向けに日本語化
*/

#include <SPI.h>
#include <SD.h>

const int chipSelect = PIN_SPI_SS; // D10

void setup() {
  // シリアル通信を開き、ポートが開くのを待つ:
  Serial.begin(115200);
  while (!Serial) {
    ; // シリアルポートの接続を待つ(ネイティブUSBポートでのみ必要)
  }


  Serial.print("Initializing SD card...");

  // カードが挿さっていて初期化できるか確認する:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // これ以上は何もしない:
    while (1);
  }
  Serial.println("card initialized.");

  // ファイルを開く。同時に開けるファイルは1つだけなので、
  // 別のファイルを開く前にこれを閉じる必要がある。
  File dataFile = SD.open("datalog.txt");

  // ファイルが使えるなら読み出す:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  }
  // 開けなかったらエラーを表示する:
  else {
    Serial.println("error opening datalog.txt");
  }
}

void loop() {
}
