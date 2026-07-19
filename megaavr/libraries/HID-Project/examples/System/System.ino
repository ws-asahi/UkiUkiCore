/*
  Copyright (c) 2014-2015 NicoHood
  他の貢献者はライブラリのreadmeを参照。

  System(電源操作)サンプル
  ボタンでPCをスリープ(または電源断)させ、もう1つのボタンで
  起こします。

  UkiUkiduino: スリープ=基板上のボタン(BTN_BUILTIN、押下=HIGH)、
  ウェイク=D2に外付けボタン(GNDへ接続。内部プルアップ、押下=LOW)。

  単一レポートで使いたい場合はSingleSystemも利用できます。
  https://github.com/NicoHood/HID/wiki/System-API

  UkiUkiduino向けに日本語化
*/

#include "HID-Project.h"

const int pinLed = LED_BUILTIN;
const int pinButtonS = BTN_BUILTIN;  // スリープ: 基板上ボタン(押下=HIGH)
const int pinButtonW = 2;            // ウェイク: 外付けボタン(押下=LOW)

void setup() {
  // LEDとボタンを準備する
  pinMode(pinLed, OUTPUT);
  pinMode(pinButtonS, INPUT);         // 基板にプルダウン実装済み
  pinMode(pinButtonW, INPUT_PULLUP);

  // ホストへクリーンなレポートを送る。どのArduinoでも重要な儀式です。
  System.begin();
}

void loop() {
  if (digitalRead(pinButtonS)) {      // 押下=HIGH
    digitalWrite(pinLed, HIGH);

    // PCをスリープ(または電源断)させる
    System.write(SYSTEM_SLEEP);
    //System.write(SYSTEM_POWER_DOWN);

    // 簡易チャタリング対策
    delay(300);
    digitalWrite(pinLed, LOW);
  }

  if (!digitalRead(pinButtonW)) {     // 押下=LOW
    digitalWrite(pinLed, HIGH);

    // PCのウェイクアップを試みる
    // USBウェイクアップ非対応のPC/ノートでは効かないことがあります
    System.write(SYSTEM_WAKE_UP);

    // 簡易チャタリング対策
    delay(300);
    digitalWrite(pinLed, LOW);
  }
}
