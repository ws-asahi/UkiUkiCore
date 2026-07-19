/*
  SDカードの読み書き

  SDカード上のファイルへのデータの書き込みと読み出しの方法を示します。

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
  // シリアル通信を開く
  Serial.begin(115200);


  Serial.print("Initializing SD card...");

  if (!SD.begin(PIN_SPI_SS)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  // ファイルを開く。同時に開けるファイルは1つだけなので、
  // 別のファイルを開く前にこれを閉じる必要がある。
  myFile = SD.open("test.txt", FILE_WRITE);

  // 開けたら書き込む:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // ファイルを閉じる:
    myFile.close();
    Serial.println("done.");
  } else {
    // 開けなかったらエラーを表示する:
    Serial.println("error opening test.txt");
  }

  // 今度は読み出し用に開き直す:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // 中身がなくなるまでファイルから読む:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // ファイルを閉じる:
    myFile.close();
  } else {
    // 開けなかったらエラーを表示する:
    Serial.println("error opening test.txt");
  }
}

void loop() {
  // setupの後は何もしない
}
