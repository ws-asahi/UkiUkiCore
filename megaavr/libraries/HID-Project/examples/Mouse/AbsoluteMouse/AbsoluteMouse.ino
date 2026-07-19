/*
  Copyright (c) 2014-2015 NicoHood
  他の貢献者はライブラリのreadmeを参照。

  AbsoluteMouse(絶対座標マウス)サンプル
  ボタンでクリック・座標指定の移動を行います。

  UkiUkiduino:
    クリック     = 基板上のボタン(BTN_BUILTIN、押下=HIGH)
    左上へ移動   = D2に外付けボタン(GNDへ。内部プルアップ、押下=LOW)
    相対移動     = D3に外付けボタン(同上)
  ※原本はD1(シリアルTX)をボタンに使っていたためD3へ変更しています。

  単一レポート版のSingleAbsoluteMouseもあります。
  https://github.com/NicoHood/HID/wiki/Mouse-API
  https://github.com/NicoHood/HID/wiki/AbsoluteMouse-API

  UkiUkiduino向けに日本語化
*/

#include "HID-Project.h"

const int pinLed = LED_BUILTIN;
const int pinButtonClick = BTN_BUILTIN;  // 押下=HIGH
const int pinButtonCenter = 2;           // 押下=LOW
const int pinButtonMove = 3;             // 押下=LOW

void setup() {
  // LEDとボタンを準備する
  pinMode(pinLed, OUTPUT);
  pinMode(pinButtonClick, INPUT);        // 基板にプルダウン実装済み
  pinMode(pinButtonCenter, INPUT_PULLUP);
  pinMode(pinButtonMove, INPUT_PULLUP);

  // ホストへクリーンなレポートを送る。どのArduinoでも重要な儀式です。
  AbsoluteMouse.begin();
}

void loop() {
  if (digitalRead(pinButtonClick)) {     // 押下=HIGH
    digitalWrite(pinLed, HIGH);

    // 公式ライブラリと同じ使い方。ほぼ自明です
    AbsoluteMouse.click();
    //AbsoluteMouse.click(MOUSE_RIGHT);

    // 簡易チャタリング対策
    delay(300);
    digitalWrite(pinLed, LOW);
  }

  if (!digitalRead(pinButtonCenter)) {   // 押下=LOW
    digitalWrite(pinLed, HIGH);

    // 指定座標へ移動する(16ビット符号付き、-32768~32767)
    // 同じ座標へ2回続けて移動することはできない!
    // X/Yは画面左上が原点。
    AbsoluteMouse.moveTo(0, 0);

    // 簡易チャタリング対策
    delay(300);
    digitalWrite(pinLed, LOW);
  }

  if (!digitalRead(pinButtonMove)) {     // 押下=LOW
    digitalWrite(pinLed, HIGH);

    // 直前の座標からの相対移動
    AbsoluteMouse.move(1000, 1000);

    // 簡易チャタリング対策
    delay(300);
    digitalWrite(pinLed, LOW);
  }
}
