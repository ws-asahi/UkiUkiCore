/*
  SDカードのテスト

  SDライブラリの土台になっているユーティリティライブラリを使って、
  SDカードの情報を取得する方法を示します。カードが正常に動くか
  自信がないときのテストにとても便利です。

  回路(UkiUkiduino):
    SDカードをSPIバスに次のように接続:
 ** MOSI - D11
 ** MISO - D12
 ** SCK  - D13
 ** CS   - PIN_SPI_SS (D10)


  原作: Limor Fried (2011) / Tom Igoe改変 (2012)
  UkiUkiduino向けに日本語化
*/
// SDライブラリを読み込む:
#include <SPI.h>
#include <SD.h>

// SDユーティリティライブラリの各オブジェクトを用意する:
Sd2Card card;
SdVolume volume;
SdFile root;

// SDモジュールの配線に合わせて変更する。PIN_SPI_SSは
// 基板のハードウェアSSピン(UkiUkiduinoではD10)
const int chipSelect = PIN_SPI_SS;

void setup() {
  // シリアル通信を開き、ポートが開くのを待つ:
  Serial.begin(115200);
  while (!Serial) {
    ; // シリアルポートの接続を待つ(ネイティブUSBポートでのみ必要)
  }


  Serial.print("\nInitializing SD card...");

  // カードが動くかを調べるだけなので、
  // ユーティリティライブラリの初期化コードを使う!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // カードの種別を表示する
  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // 次に「ボリューム」(パーティション)を開いてみる - FAT16かFAT32のはず
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    while (1);
  }

  Serial.print("Clusters:          ");
  Serial.println(volume.clusterCount());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(volume.blocksPerCluster());

  Serial.print("Total Blocks:      ");
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  // 最初のFAT系ボリュームの種別とサイズを表示する
  uint32_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster();    // クラスタはブロックの集まり
  volumesize *= volume.clusterCount();       // クラスタはたくさんある
  volumesize /= 2;                           // SDカードのブロックは常に512バイト(2ブロックで1KB)
  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // カード内の全ファイルを日付・サイズ付きで一覧する
  root.ls(LS_R | LS_DATE | LS_SIZE);
}

void loop(void) {
}
