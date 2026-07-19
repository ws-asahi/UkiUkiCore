/*
  ノンブロッキング書き込み

  SDカード上のファイルへのノンブロッキングな書き込みの方法を
  示します。ファイルには10msごとに現在のmillis()値が記録されます。
  SDカードがビジーの間はデータをバッファに溜めることで、スケッチを
  ブロックしません。

  注意: myFile.availableForWrite()は必要に応じてファイル内容を
        自動的にsyncします。それでもmyFile.sync()かmyFile.close()を
        呼ばないと、未syncのデータが失われることがあります。

  回路(UkiUkiduino):
  - SPIバスにmicroSDカードモジュールを接続
    (MOSI=D11 / MISO=D12 / SCK=D13、CSはPIN_SPI_SS=D10)

  この例はパブリックドメインです。
  UkiUkiduino向けに日本語化
*/

#include <SD.h>

// 書き込みに使うファイル名
const char filename[] = "demo.txt";

// ファイルを表すFileオブジェクト
File txtFile;

// 出力をバッファする文字列
String buffer;

unsigned long lastMillis = 0;

void setup() {
  Serial.begin(115200);

  // バッファ用Stringに1kBを確保する
  buffer.reserve(1024);

  // 書き込み時に点滅させるLEDピンを出力にする
  pinMode(LED_BUILTIN, OUTPUT);

  // SDカードを初期化する
  if (!SD.begin()) {
    Serial.println("Card failed, or not present");
    // これ以上は何もしない:
    while (1);
  }

  // 空のファイルから始めたい場合は
  // 次の行のコメントを外す:
  // SD.remove(filename);

  // 書き込み用にファイルを開いてみる
  txtFile = SD.open(filename, FILE_WRITE);
  if (!txtFile) {
    Serial.print("error opening ");
    Serial.println(filename);
    while (1);
  }

  // はじめに数行書いておく
  txtFile.println();
  txtFile.println("Hello World!");
}

void loop() {
  // 最後に行を追加してから10ms経過したか確認する
  unsigned long now = millis();
  if ((now - lastMillis) >= 10) {
    // バッファに新しい行を追加する
    buffer += "Hello ";
    buffer += now;
    buffer += "\r\n";

    lastMillis = now;
  }

  // SDカードがブロックせずに書き込める状態か、そして
  // バッファ済みデータが書き込み単位ぶん溜まったかを確認する
  unsigned int chunkSize = txtFile.availableForWrite();
  if (chunkSize && buffer.length() >= chunkSize) {
    // ファイルへ書き込み、LEDを点滅させる
    digitalWrite(LED_BUILTIN, HIGH);
    txtFile.write(buffer.c_str(), chunkSize);
    digitalWrite(LED_BUILTIN, LOW);

    // 書き込んだぶんをバッファから取り除く
    buffer.remove(0, chunkSize);
  }
}
