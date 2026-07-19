/*
  ファイル一覧の表示

  SDカードのディレクトリ内のファイルを一覧表示する方法を示します。

  回路(UkiUkiduino):
   SDカードをSPIバスに次のように接続:
 ** MOSI - D11
 ** MISO - D12
 ** SCK  - D13
 ** CS   - PIN_SPI_SS (D10)

  原作: David A. Mellis (2010) / Tom Igoe改変 (2012) /
        Scott Fitzgerald改変 (2014)
  この例はパブリックドメインです。
  UkiUkiduino向けに日本語化
*/
#include <SPI.h>
#include <SD.h>

File root;

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

  root = SD.open("/");

  printDirectory(root, 0);

  Serial.println("done!");
}

void loop() {
  // setupの後は何もしない
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // ファイルはもうない
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // ファイルにはサイズがあり、ディレクトリにはない
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
