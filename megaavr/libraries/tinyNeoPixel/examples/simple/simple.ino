/* simple - NeoPixelを1個ずつ順番に点灯する最小サンプル
 * 原作: Shae Erisson (c) 2013, GPLv3
 * UkiUkiduino向けに日本語化
 */
#include <tinyNeoPixel.h>

// NeoPixelをつないだピン
#define PIN            9   // D9

// つながっているNeoPixelの個数
#define NUMPIXELS      16

// ピクセル数と信号送出ピンを指定してライブラリを初期化する。
// 古いストリップでは第3引数の変更が必要なことがある(詳しくは
// strandtestサンプル参照)。
tinyNeoPixel pixels = tinyNeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 500; // 0.5秒待つ

void setup() {

  pixels.begin(); // NeoPixelライブラリを初期化する
}

void loop() {

  // NeoPixelの番号は先頭が0、2個目が1、…、末尾が(個数-1)。

  for (int i = 0; i < NUMPIXELS; i++) {

    // pixels.ColorはRGB値(0,0,0 ~ 255,255,255)から色を作る
    pixels.setPixelColor(i, pixels.Color(0, 150, 0)); // ほどほどの明るさの緑

    pixels.show(); // 更新した色を実際のハードウェアへ送る

    delay(delayval); // 少し待つ(ミリ秒)

  }
}
