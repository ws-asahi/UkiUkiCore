/*
  Copyright (c) 2017 wind-rider
  他の貢献者はライブラリのreadmeを参照。

  Surface Dialサンプル

  ロータリーエンコーダとボタンで、Surface Dial互換デバイスを作ります。

  UkiUkiduino:
    エンコーダA相 = D2、B相 = D3(エンコーダのCOMはGNDへ)
    押し込みボタン = 基板上のボタン(BTN_BUILTIN、押下=HIGH)
  ※原本は外付けボタン(プルアップ、押下=LOW)前提のままpress/releaseの
    論理が逆になっていたため、押下=HIGHのBTN_BUILTINに置き換えつつ
    論理を正しました。

  エンコーダ処理コードの出典:
  https://www.allwinedesigns.com/blog/pocketnc-jog-wheel

  UkiUkiduino向けに日本語化
*/

#include "HID-Project.h"

// エンコーダA相/B相の入力ピン
int pinA = 2;
int pinB = 3;

// 押し込みボタン(基板上ボタン、押下=HIGH)
int pinButton = BTN_BUILTIN;

volatile bool previousButtonValue = false;

volatile int previous = 0;
volatile int counter = 0;

void setup() {
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);

  pinMode(pinButton, INPUT);  // 基板にプルダウン実装済み

  attachInterrupt(digitalPinToInterrupt(pinA), changed, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB), changed, CHANGE);

  SurfaceDial.begin();
}

void changed() {
  int A = digitalRead(pinA);
  int B = digitalRead(pinB);

  int current = (A << 1) | B;
  int combined  = (previous << 2) | current;

  if(combined == 0b0010 ||
     combined == 0b1011 ||
     combined == 0b1101 ||
     combined == 0b0100) {
    counter++;
  }

  if(combined == 0b0001 ||
     combined == 0b0111 ||
     combined == 0b1110 ||
     combined == 0b1000) {
    counter--;
  }

  previous = current;
}

void loop(){
  bool buttonValue = digitalRead(pinButton);   // 押下=HIGH
  if(buttonValue != previousButtonValue){
    if(buttonValue) {
      SurfaceDial.press();      // 押した瞬間にpress
    } else {
      SurfaceDial.release();    // 離した瞬間にrelease
    }
    previousButtonValue = buttonValue;
  }

  if(counter >= 4) {
    SurfaceDial.rotate(10);
    counter -= 4;
  } else if(counter <= -4) {
    SurfaceDial.rotate(-10);
    counter += 4;
  }
}
