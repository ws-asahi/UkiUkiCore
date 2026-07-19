/* strandtest - NeoPixelストリップの動作テスト(Static版)
 * 原作: Adafruit NeoPixelライブラリ付属サンプル
 * UkiUkiduino向けに日本語化
 */
#include <tinyNeoPixel_Static.h>

#define PIN 9 // D9: NeoPixelのデータピン

// 引数1 = ストリップのピクセル数
// 引数2 = Arduinoピン番号(ほとんどのピンが使える)
// 引数3 = ピクセルの種別。必要に応じて足し合わせる:
//   NEO_GRB     GRB順のビットストリーム(大半のNeoPixel製品)
//   NEO_RGB     RGB順のビットストリーム(v1 FLORAピクセル等)
// 引数4 = ピクセルデータを格納する配列

#define NUMPIXELS 60

// ライブラリのStatic版なので、ピクセル配列を自分で用意します。
// malloc()/free()を使わないぶん省フラッシュで、フレームバッファの
// RAM使用量がコンパイル時に見えるようになる利点があります。

byte pixels[NUMPIXELS * 3];

// ピクセル数・ピン・種別に加え、第4引数として上で定義した配列を渡します。

tinyNeoPixel strip = tinyNeoPixel(NUMPIXELS, PIN, NEO_GRB, pixels);

// 重要: NeoPixelの焼損リスクを減らすため、ピクセルの電源両端に
// 1000uFのコンデンサを追加し、最初のピクセルのデータ入力には
// 300~500Ωの抵抗を直列に入れ、ボードと最初のピクセルの距離を
// できるだけ短くしてください。通電中の回路への接続は避けること。
// やむを得ない場合はGNDを最初に接続してください。

void setup() {
  pinMode(PIN, OUTPUT); // Static版はライブラリがピン設定をしないため自分で行う
  // strip.begin(); // Static版では使わない
  strip.show(); // 全ピクセルを「消灯」で初期化する
}

void loop() {
  // ピクセルへの表示方法をいくつかのパターンで実演する:
  colorWipe(strip.Color(255, 0, 0), 50); // 赤
  colorWipe(strip.Color(0, 255, 0), 50); // 緑
  colorWipe(strip.Color(0, 0, 255), 50); // 青
  // 劇場マーキー風の流れる点灯を送る...
  theaterChase(strip.Color(127, 127, 127), 50); // 白
  theaterChase(strip.Color(127, 0, 0), 50); // 赤
  theaterChase(strip.Color(0, 0, 127), 50); // 青

  rainbow(20);
  rainbowCycle(20);
  theaterChaseRainbow(50);
}

// 1つずつ順番に同じ色で塗りつぶす
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// 少し違うバージョン。虹をストリップ全体に均等に分布させる
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 色相環を5周する
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// 劇場マーキー風の流れる点灯
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { // 10周ぶん流す
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, c);  // 3個おきに点灯する
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      // 3個おきに消灯する
      }
    }
  }
}

// 劇場マーキー風の流れる点灯(虹バージョン)
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // 色相環の全256色を一巡する
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, Wheel((i + j) % 255)); // 3個おきに点灯する
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      // 3個おきに消灯する
      }
    }
  }
}

// 0~255の値から色を作る。
// 色は 赤 - 緑 - 青 - 赤 と遷移する。
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
