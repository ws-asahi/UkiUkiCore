/*
  Copyright (c) 2014-2015 NicoHood
  他の貢献者はライブラリのreadmeを参照。

  BootKeyboardサンプル

  キーボードがBIOS画面でも動くことを示します。
  LEDは「今BIOS(ブートプロトコル)かどうか」を表示します。

  UkiUkiduinoでは基板上のボタン(BTN_BUILTIN、押下=HIGH)を使うので
  配線なしで試せます。

  https://github.com/NicoHood/HID/wiki/Keyboard-API#boot-keyboard

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
  // キーボードがブートプロトコルで動いている間(通常はBIOS画面)
  // LEDを点灯する
  if (BootKeyboard.getProtocol() == HID_BOOT_PROTOCOL)
    digitalWrite(pinLed, HIGH);
  else
    digitalWrite(pinLed, LOW);

  // ボタンでEnterキーを送る
  if (digitalRead(pinButton)) {   // 押下=HIGH
    BootKeyboard.write(KEY_ENTER);

    // 簡易チャタリング対策
    delay(300);
  }
}
