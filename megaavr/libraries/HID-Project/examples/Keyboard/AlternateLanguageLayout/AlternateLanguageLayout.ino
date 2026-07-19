/*
  Copyright (c) 2019-2020 Blahlicus
  他の貢献者はライブラリのreadmeを参照。

  キーボードレイアウト切り替えサンプル(日本語JIS配列)

  _asciimapの割り当てを別の配列に切り替える方法のデモです。
  ここでは日本語(JIS)配列を選択します。OS側の入力配列も
  ファームウェアと同じもの(日本語キーボード)に設定されている
  必要があります。

  UkiUkiduino向けに日本語化・JIS配列化
*/
#define HID_CUSTOM_LAYOUT // カスタム配列を選ぶことを示すフラグ
// このフラグが未定義の場合は既定のUS配列が使われます

#define LAYOUT_JAPANESE // 上のフラグに続けて日本語(JIS)配列を指定する
// 他の配列は /src/KeyboardLayouts/ImprovedKeylayouts.h を参照
// (LAYOUT_GERMAN、LAYOUT_FRENCHなど多数あります)

// 上の2つのフラグは必ずHID-Projectのincludeより前に置くこと
#include "HID-Project.h"


void setup() {
  BootKeyboard.begin();
  delay(2000);
  // JIS配列とUS配列で位置が違う記号を打ってみる。
  // OSが日本語キーボード設定なら、そのまま正しく表示されます。
  BootKeyboard.print("JIS: @ [ ] : ^");
}


void loop() {

}
