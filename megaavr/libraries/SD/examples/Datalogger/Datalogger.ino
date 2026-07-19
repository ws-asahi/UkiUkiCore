/*
  SDカードデータロガー

  3つのアナログセンサの値をSDライブラリでSDカードに記録する
  方法を示します。

  回路(UkiUkiduino):
   アナログセンサをA0、A1、A2に接続
   SDカードをSPIバスに次のように接続:
 ** MOSI - D11
 ** MISO - D12
 ** SCK  - D13
 ** CS   - PIN_SPI_SS (D10)

  原作: Tom Igoe (2010/2012)
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
}

void loop() {
  // 記録するデータを組み立てるための文字列を作る:
  String dataString = "";

  // 3つのセンサを読んで文字列に追記する:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }

  // ファイルを開く。同時に開けるファイルは1つだけなので、
  // 別のファイルを開く前にこれを閉じる必要がある。
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // ファイルが使えるなら書き込む:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // シリアルポートにも表示する:
    Serial.println(dataString);
  }
  // 開けなかったらエラーを表示する:
  else {
    Serial.println("error opening datalog.txt");
  }
}
