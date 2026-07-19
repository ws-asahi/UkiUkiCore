/*
  Copyright (c) 2014-2015 NicoHood
  他の貢献者はライブラリのreadmeを参照。

  Mouseサンプル
  ボタンでマウスのクリック・移動・スクロールを行います。

  UkiUkiduino:
    クリック   = 基板上のボタン(BTN_BUILTIN、押下=HIGH)
    移動       = D2に外付けボタン(GNDへ。内部プルアップ、押下=LOW)
    スクロール = D3に外付けボタン(同上)

  BIOS互換(単一レポート)のBootMouseもありますが、非常に特殊で
  非推奨です。BIOSマウスはホイール非対応のため、再起動後に問題が
  出ることがあります。
  https://github.com/NicoHood/HID/wiki/Mouse-API

  UkiUkiduino向けに日本語化
*/

#include "HID-Project.h"

const int pinLed = LED_BUILTIN;
const int pinButtonClick = BTN_BUILTIN;  // 押下=HIGH
const int pinButtonMove = 2;             // 押下=LOW
const int pinButtonScroll = 3;           // 押下=LOW

void setup() {
  // LEDとボタンを準備する
  pinMode(pinLed, OUTPUT);
  pinMode(pinButtonClick, INPUT);        // 基板にプルダウン実装済み
  pinMode(pinButtonMove, INPUT_PULLUP);
  pinMode(pinButtonScroll, INPUT_PULLUP);

  // ホストへクリーンなレポートを送る。どのArduinoでも重要な儀式です。
  Mouse.begin();
}

void loop() {
  if (digitalRead(pinButtonClick)) {     // 押下=HIGH
    digitalWrite(pinLed, HIGH);

    // 公式ライブラリと同じ使い方。ほぼ自明です
    Mouse.click();
    //Mouse.click(MOUSE_RIGHT);

    // 簡易チャタリング対策
    delay(300);
    digitalWrite(pinLed, LOW);
  }

  if (!digitalRead(pinButtonMove)) {     // 押下=LOW
    digitalWrite(pinLed, HIGH);

    // 公式ライブラリと同じ使い方。ほぼ自明です
    Mouse.move(100, 0);

    // 簡易チャタリング対策
    delay(300);
    digitalWrite(pinLed, LOW);
  }

  if (!digitalRead(pinButtonScroll)) {   // 押下=LOW
    digitalWrite(pinLed, HIGH);

    // 少し下へスクロールする。値は十分大きくすること
    Mouse.move(0, 0, 160);

    // 簡易チャタリング対策
    delay(300);
    digitalWrite(pinLed, LOW);
  }
}
