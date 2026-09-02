/* SPISlave_Test - SPIクライアント(スレーブ)側のデモ
 *
 * UkiUkiduinoをSPIクライアントとして動かし、SPIホストからの質問に
 * 答えます。もう1枚のボードでSPISlave_Hostサンプルを動かして組み
 * 合わせてください。流れはESP8266 Arduinoコアの同名サンプルに
 * 倣っています。
 *
 * 配線(クライアント <-> ホスト):
 *   MOSI <- ホストのMOSI     D11
 *   MISO -> ホストのMISO     D12
 *   SCK  <- ホストのSCK      D13
 *   SS   <- ホストのCSピン   AREF (D20)
 *   GND  -- GND
 * 注意: UkiUkiduinoではAREFヘッダピンがSS入力になります。
 *   SPISlave使用中はAREFに外部基準電圧を加えないでください。
 *   また同じピンを使うSerial2は同時に使えません。
 *
 * コールバックは割り込みコンテキストで動きます。このスケッチでは
 * そこではデータのコピーだけを行い、シリアル表示はloop()で行います。
 *
 * UkiUkiduino向けに日本語化
 */

#include <SPISlave.h>

volatile bool     got_message = false;
char              message[SPISLAVE_BUFFER_SIZE + 1];

void setup() {
  Serial.begin(115200);

  // ホストからのトランザクションを受信した(SSがHIGHに戻ると発火)
  SPISlave.onData([](uint8_t *data, size_t len) {
    memcpy(message, data, len);
    message[len] = '\0';
    got_message = true;
    // ホストが「次の」トランザクションで読み出す答えを用意する
    if (strcmp(message, "Are you alive?") == 0) {
      SPISlave.setData("Yes, alive!");
    } else {
      SPISlave.setData("Say what?");
    }
  });

  // 用意した答えをホストが最後まで読み出した
  SPISlave.onDataSent([]() {
    // 割り込みコンテキストなので短く。報告はloop()がgot_message経由で行う
  });

  SPISlave.begin();               // SPIモード0、MSBファースト
  SPISlave.setData("Hello Host!");  // いちばん最初の読み出しへの答え
}

void loop() {
  if (got_message) {
    got_message = false;
    Serial.print(F("Question: "));
    Serial.println(message);
  }
}
