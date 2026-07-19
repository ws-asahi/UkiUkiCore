/*
  Copyright (c) 2014-2015 NicoHood
  他の貢献者はライブラリのreadmeを参照。

  Consumer(メディアキー)サンプル
  ボタンを押すと音楽プレーヤーが再生/一時停止します。

  UkiUkiduinoでは基板上のボタン(BTN_BUILTIN)を使うので、配線なしで
  そのまま試せます。※基板のボタンはプルダウン式で「押すとHIGH」です。

  単一レポートで使いたい場合はSingleConsumerも利用できます。
  他のConsumerキーはHID Projectのドキュメントを参照:
  https://github.com/NicoHood/HID/wiki/Consumer-API

  UkiUkiduino向けに日本語化
*/

#include "HID-Project.h"

const int pinLed = LED_BUILTIN;
const int pinButton = BTN_BUILTIN;  // 基板上のボタン(押下=HIGH)

void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(pinButton, INPUT);  // 基板に1kΩプルダウン実装済みのためINPUTでよい

  // ホストへクリーンなレポートを送る。どのArduinoでも重要な儀式です。
  Consumer.begin();
}

void loop() {
  if (digitalRead(pinButton)) {   // 押下=HIGH
    digitalWrite(pinLed, HIGH);

    // 他のConsumerキーはHID Projectのドキュメント参照
    Consumer.write(MEDIA_PLAY_PAUSE);

    // 簡易チャタリング対策
    delay(300);
    digitalWrite(pinLed, LOW);
  }
}
