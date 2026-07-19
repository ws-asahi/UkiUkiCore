/*
  Copyright (c) 2014-2015 NicoHood
  他の貢献者はライブラリのreadmeを参照。

  Gamepadサンプル
  ボタンを押すたびにゲームパッドの各操作をデモします。

  UkiUkiduinoでは基板上のボタン(BTN_BUILTIN、押下=HIGH)を使うので
  配線なしで試せます。動作確認はOSのゲームコントローラ設定画面や
  ブラウザのGamepad APIテストページが便利です。

  Gamepad1~4を使うと個別レポート(ゲームパッドごとに1エンドポイント)
  にもできます。
  https://github.com/NicoHood/HID/wiki/Gamepad-API

  UkiUkiduino向けに日本語化
*/

#include "HID-Project.h"

const int pinLed = LED_BUILTIN;
const int pinButton = BTN_BUILTIN;  // 基板上のボタン(押下=HIGH)

void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(pinButton, INPUT);  // 基板にプルダウン実装済み

  // ホストへクリーンなレポートを送る。どのArduinoでも重要な儀式です。
  Gamepad.begin();
}

void loop() {
  if (digitalRead(pinButton)) {   // 押下=HIGH
    digitalWrite(pinLed, HIGH);

    // ボタン1~32を順に押す
    static uint8_t count = 0;
    count++;
    if (count == 33) {
      Gamepad.releaseAll();
      count = 0;
    }
    else
      Gamepad.press(count);

    // X/Y軸を新しい位置へ動かす(16ビット)
    Gamepad.xAxis(random(0xFFFF));
    Gamepad.yAxis(random(0xFFFF));

    // 十字キー(D-Pad)の全方向を順に巡る
    // 値: 0~8(0=中立)
    static uint8_t dpad1 = GAMEPAD_DPAD_CENTERED;
    Gamepad.dPad1(dpad1++);
    if (dpad1 > GAMEPAD_DPAD_UP_LEFT)
      dpad1 = GAMEPAD_DPAD_CENTERED;

    static int8_t dpad2 = GAMEPAD_DPAD_CENTERED;
    Gamepad.dPad2(dpad2--);
    if (dpad2 < GAMEPAD_DPAD_CENTERED)
      dpad2 = GAMEPAD_DPAD_UP_LEFT;

    // ここまでの関数は値を設定するだけ。
    // この呼び出しでレポートがホストへ送られる。
    Gamepad.write();

    // 簡易チャタリング対策
    delay(300);
    digitalWrite(pinLed, LOW);
  }
}
