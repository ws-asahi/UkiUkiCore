/*
  Copyright (c) 2014-2015 NicoHood
  他の貢献者はライブラリのreadmeを参照。

  NKROKeyboardサンプル

  ボタンを押すと大量のキーを同時押しします。
  NKROは最大113キーの同時押しが可能です
  (通常のキーボードは6キー+修飾8キーまで)。

  UkiUkiduinoでは基板上のボタン(BTN_BUILTIN、押下=HIGH)を使うので
  配線なしで試せます。

  単一レポート版のSingleNKROKeyboardもあります。
  https://github.com/NicoHood/HID/wiki/Keyboard-API#nkro-keyboard

  UkiUkiduino向けに日本語化
*/

#include "HID-Project.h"

const int pinLed = LED_BUILTIN;
const int pinButton = BTN_BUILTIN;  // 基板上のボタン(押下=HIGH)

void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(pinButton, INPUT);  // 基板にプルダウン実装済み

  // ホストへクリーンなレポートを送る。どのArduinoでも重要な儀式です。
  NKROKeyboard.begin();
}

void loop() {
  // 大量のキーを同時押しする
  if (digitalRead(pinButton)) {   // 押下=HIGH
    digitalWrite(pinLed, HIGH);

    // あまり多くを一度に押すとOSによっては問題が出ます。
    // 全キーが同時に押されるため、入力される順序は
    // 表示上入れ替わることがあります。
    NKROKeyboard.add('0');
    NKROKeyboard.add('1');
    NKROKeyboard.add('2');
    NKROKeyboard.add('3');
    NKROKeyboard.add('4');
    NKROKeyboard.add('5');
    NKROKeyboard.add('6');
    NKROKeyboard.add('7');
    NKROKeyboard.add('8');
    NKROKeyboard.add('9');
    NKROKeyboard.send();

    // 全キーを離してEnterを打つ
    NKROKeyboard.releaseAll();
    NKROKeyboard.println();

    // 簡易チャタリング対策
    delay(300);
  }
}
