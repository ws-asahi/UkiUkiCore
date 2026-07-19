/*
  Copyright (c) 2014-2015 NicoHood
  他の貢献者はライブラリのreadmeを参照。

  Improved Keyboardサンプル

  新しいKeyboard APIの使い方を示します。
  UkiUkiduinoでは基板上のボタン(BTN_BUILTIN、押下=HIGH)を使うので
  配線なしで試せます。

  ※押すたびにPCへ"Hello World!"が打ち込まれます。テキストエディタ等に
    フォーカスを合わせてから試してください。

  https://github.com/NicoHood/HID/wiki/Keyboard-API#improved-keyboard

  UkiUkiduino向けに日本語化
*/

#include "HID-Project.h"

const int pinLed = LED_BUILTIN;
const int pinButton = BTN_BUILTIN;  // 基板上のボタン(押下=HIGH)

void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(pinButton, INPUT);  // 基板にプルダウン実装済み

  // ホストへクリーンなレポートを送る。どのArduinoでも重要な儀式です。
  Keyboard.begin();
}


void loop() {
  if (digitalRead(pinButton)) {   // 押下=HIGH
    digitalWrite(pinLed, HIGH);

    // ふつうのprint系関数が使える
    Keyboard.println("Hello World!");

    // 1文字だけ打つ(ASCII外の特殊文字は不可)
    //Keyboard.write('a');

    // キーを直接打つ(数値を渡さないこと!)
    //Keyboard.write(KEY_ENTER);


    // どうしても生のキーコードを打ちたい場合はこう書く:
    //Keyboard.write(KeyboardKeycode(40));

    // (一部の)Consumerキーも使える。
    // 下位255キーのみ、かつLinux限定です。
    //Keyboard.write(MEDIA_PLAY_PAUSE);

    // LinuxではConsumerレポート経由のシステム機能もいくつか使える。
    //Keyboard.write(CONSUMER_POWER);
    //Keyboard.write(CONSUMER_SLEEP);

    // Linuxでは特殊なキーボードキーも使える。
    //Keyboard.write(KEY_POWER);
    //Keyboard.write(KEY_F13);

    // PCをスリープから起こすこともできる。
    // ハードウェアによっては非対応だが、OSの種類は問わない。
    //Keyboard.wakeupHost();

    // 簡易チャタリング対策
    delay(300);
    digitalWrite(pinLed, LOW);
  }
}
