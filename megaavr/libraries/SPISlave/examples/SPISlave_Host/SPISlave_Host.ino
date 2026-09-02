/* SPISlave_Host - SPISlave_Testサンプルのホスト側
 *
 * 標準のSPIライブラリが使えるArduino互換ボードなら何でも動きます
 * (もう1枚のUkiUkiduinoでも可)。質問を送り、2回目のトランザクションで
 * クライアントが用意した答えを読み出します。
 *
 * MOSI/MISO/SCKをクライアントのSPIピンへ、CS_PINをクライアントのSS
 * (UkiUkiduinoならAREF/D20)へ接続し、GNDを共通にしてください。
 *
 * クライアントは答えをonData()の中 - つまり質問のトランザクションが
 * 終わった後 - で用意するため、答えは別の読み出しトランザクションで
 * 取得します。2回の間の短い待ちは、クライアント側の割り込み処理が
 * 走る時間を確保するためのものです。
 *
 * UkiUkiduino向けに日本語化
 */

#include <SPI.h>

const uint8_t CS_PIN = 10;   // 空いているピンならどれでも。クライアントのSSへ配線する
const uint32_t SPI_HZ = 1000000;

void spiSend(const char *msg) {
  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_PIN, LOW);
  while (*msg) {
    SPI.transfer(*msg++);
  }
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
}

void spiReadAnswer(char *buf, uint8_t maxlen) {
  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_PIN, LOW);
  for (uint8_t i = 0; i < maxlen; i++) {
    buf[i] = (char)SPI.transfer(0x00);
  }
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  buf[maxlen] = '\0';
}

void setup() {
  Serial.begin(115200);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();
}

void loop() {
  char answer[33];

  spiSend("Are you alive?");
  delay(1);                       // クライアントが答えを用意する時間
  spiReadAnswer(answer, 32);
  Serial.print(F("Answer: "));
  Serial.println(answer);

  delay(1000);
}
