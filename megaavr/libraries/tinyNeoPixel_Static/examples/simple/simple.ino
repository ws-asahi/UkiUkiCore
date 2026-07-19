/* simple - NeoPixelを1個ずつ順番に点灯する最小サンプル(Static版)
 * 原作: Shae Erisson (c) 2013, GPLv3
 * UkiUkiduino向けに日本語化。あわせてStatic版のAPI
 * (ピクセル配列を渡す4引数コンストラクタ+手動pinMode)に修正
 * (原本は非Static版の書き方のままでコンパイルできなかった)
 */
#include <tinyNeoPixel_Static.h>

// NeoPixelをつないだピン
#define PIN            9   // D9

// つながっているNeoPixelの個数
#define NUMPIXELS      16

// Static版はピクセルデータの配列を自分で用意して第4引数で渡す
byte pixelData[NUMPIXELS * 3];

tinyNeoPixel pixels = tinyNeoPixel(NUMPIXELS, PIN, NEO_GRB, pixelData);

int delayval = 500; // 0.5秒待つ

void setup() {
  pinMode(PIN, OUTPUT); // Static版はライブラリがピン設定をしないため自分で行う
  // pixels.begin()はStatic版では使わない
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
