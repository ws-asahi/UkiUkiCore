/*
  Copyright (c) 2014-2015 NicoHood
  他の貢献者はライブラリのreadmeを参照。

  KeyboardLedサンプル

  ボタンでCaps Lockを切り替えます。
  Caps Lockの状態は基板上のLEDに表示されます。
  (キーボードLEDの取得は単一レポートのHIDデバイスのみ対応)

  UkiUkiduinoでは基板上のボタン(BTN_BUILTIN、押下=HIGH)を使うので
  配線なしで試せます。PCのキーボードでCaps Lockを押しても
  LEDが追従するのが見どころです。

  https://github.com/NicoHood/HID/wiki/Keyboard-API

  UkiUkiduino向けに日本語化
*/

#include "HID-Project.h"

const int pinLed = LED_BUILTIN;
const int pinButton = BTN_BUILTIN;  // 基板上のボタン(押下=HIGH)

void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(pinButton, INPUT);  // 基板にプルダウン実装済み

  // ホストへクリーンなレポートを送る。どのArduinoでも重要な儀式です。
  BootKeyboard.begin();
}


void loop() {
  // LEDをCaps Lock状態と同じにする
  if (BootKeyboard.getLeds() & LED_CAPS_LOCK)
    digitalWrite(pinLed, HIGH);
  else
    digitalWrite(pinLed, LOW);

  // ボタンでCaps Lockを手動トグルする
  if (digitalRead(pinButton)) {   // 押下=HIGH
    BootKeyboard.write(KEY_CAPS_LOCK);

    // 簡易チャタリング対策
    delay(300);
  }
}
