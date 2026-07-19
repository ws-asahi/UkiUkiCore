/*
  Copyright (c) 2014-2021 NicoHood
  他の貢献者はライブラリのreadmeを参照。

  KeymapTest(キー配列の検証)サンプル
  選んだキー配列で、すべてのキーが意図した文字を打つかテストします。

  https://github.com/NicoHood/HID/wiki/Keyboard-API

  UkiUkiduino向けに日本語化
*/

/* 既定ではUS英語配列が選ばれています。別の配列をテストするには
 * 下の2行のコメントを外し、2行目を試したい配列に変更してください。
 * 使える配列はライブラリ内 ImprovedKeylayouts.h の末尾で確認できます。
 * (日本語JIS配列はLAYOUT_JAPANESEです)
 */
//#define HID_CUSTOM_LAYOUT
//#define LAYOUT_JAPANESE

#include <HID-Project.h>

/* キーの押下/解放のたびに待つ時間[ms]。ホスト側の取りこぼし防止。
 */
#define KEYDELAY 5

void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }

  BootKeyboard.begin();
  BootKeyboard.releaseAll();

  Serial.println("--- HID Project キー配列テスト ---");
  Serial.println("2列の文字が表示されます。選んだ配列が正しければ、全行で左右の文字が");
  Serial.println("一致します。一致しない場合は別の配列で試してください。");
  Serial.println("テストが終わるまでウィンドウを切り替えないでください!!!");
  Serial.println("上の入力欄をクリックし、空であることを確認してからEnterを押してください");
  while (!Serial.available()) {
  }
  while (Serial.available()) {
    Serial.read ();
  }
  delay (100);

  for (char c = ' '; c <= '~'; ++c) {
    Serial.print(c);
    Serial.print(' ');
    BootKeyboard.press(c);
    delay (KEYDELAY);
    BootKeyboard.release(c);
    delay (KEYDELAY);
    BootKeyboard.press(KEY_ENTER);
    delay (KEYDELAY);
    BootKeyboard.release(KEY_ENTER);
    delay (KEYDELAY);
    while (!Serial.available()) {
    }
    char r = Serial.read();
    while (Serial.available()) {
      Serial.read();
    }
    Serial.print (r);
    if (r == c) {
      Serial.print(" OK");
    } else {
      Serial.print(" FAIL");
    }
    Serial.println();
  }

  Serial.println("--- 完了 ---");
}

void loop() {
  // 何もすることなし!
}
