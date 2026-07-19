/*
  Copyright (c) 2014-2015 NicoHood
  他の貢献者はライブラリのreadmeを参照。

  RawHID応用サンプル

  RawHIDでバイト列を送る方法を示します。
  ボタンを押すとサンプル値を送信します。
  ホストから受信したデータはすべてシリアルへ表示します。

  UkiUkiduinoでは基板上のボタン(BTN_BUILTIN、押下=HIGH)を使うので
  配線なしで試せます。ホスト側の送受信にはhidapiベースのツール等が
  使えます。

  https://github.com/NicoHood/HID/wiki/RawHID-API

  UkiUkiduino向けに日本語化
*/

#include "HID-Project.h"

const int pinLed = LED_BUILTIN;
const int pinButton = BTN_BUILTIN;  // 基板上のボタン(押下=HIGH)

// RawHIDデータを受けるバッファ。
// ホストがこれより大きいデータを送ろうとするとエラー応答になります。
// また、次のデータが届くまでに読み出さなかった場合もエラー応答となり
// データは失われます。
uint8_t rawhidData[255];

void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(pinButton, INPUT);  // 基板にプルダウン実装済み

  Serial.begin(115200);

  // RawHIDのOUTレポート受信バッファを設定する。
  // フィーチャーレポートも(並行して)使えます。別サンプル参照。
  RawHID.begin(rawhidData, sizeof(rawhidData));
}

void loop() {
  // ホストへデータを送る
  if (digitalRead(pinButton)) {   // 押下=HIGH
    digitalWrite(pinLed, HIGH);

    // 連番入りのバッファを作って送信する
    uint8_t megabuff[100];
    for (uint8_t i = 0; i < sizeof(megabuff); i++) {
      megabuff[i] = i;
    }
    RawHID.write(megabuff, sizeof(megabuff));

    // 簡易チャタリング対策
    delay(300);
    digitalWrite(pinLed, LOW);
  }


  // RawHIDデバイスへの新着データを確認する
  auto bytesAvailable = RawHID.available();
  if (bytesAvailable)
  {
    digitalWrite(pinLed, HIGH);

    // データをシリアルへ転送する
    while (bytesAvailable--) {
      Serial.println(RawHID.read());
    }

    digitalWrite(pinLed, LOW);
  }
}
