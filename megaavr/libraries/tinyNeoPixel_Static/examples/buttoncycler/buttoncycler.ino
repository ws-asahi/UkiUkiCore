/* buttoncycler(ボタンでアニメーション切替・Static版)
   ボタン入力でNeoPixel(WS2812系フルカラーLED)のアニメーションを
   切り替えるデモです。UkiUkiduinoに搭載のボタン(BTN_BUILTIN = D20)を
   押すたびに次のアニメーションへ進みます。最初のアニメーションを
   始めるにも1回ボタンを押してください。

   接続: NeoPixelのDIN(データ入力)をD6へ、5VとGNDを電源へ接続します。
   ※LEDの数が多い場合は外部電源を使用してください。

   メモ: BTN_BUILTINは基板上でプルダウンされており「押すとHIGH」に
   なります。pinMode(BTN_BUILTIN, INPUT)のままで使えます(プルアップ
   前提の一般的なサンプルとは論理が逆なので注意)。

   UkiUkiduino向けに移植・日本語化
*/

#include <tinyNeoPixel_Static.h>

#define PIXEL_PIN    6    // NeoPixelのDINを接続するピン(D6)

#define PIXEL_COUNT 16    // 接続するLEDの個数

// Static版ではピクセルバッファを自分で用意します。malloc()/free()を
// 使わないためRAM使用量がコンパイル時に確定し、節約にもなります。
byte pixels[PIXEL_COUNT * 3];
// LED個数は必ず1つの変数/#defineにまとめ、バッファ宣言ではそれに
// 3(RGB)または4(RGBW)を掛けて使うと、二重管理によるミスを防げます。

// 第1引数 = LEDの個数
// 第2引数 = 接続ピン
// 第3引数 = 色の並び順(NEO_RGB, NEO_GRBなど。多くの製品はNEO_GRB)
// 第4引数 = 上で宣言したピクセルバッファ
tinyNeoPixel strip = tinyNeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB, pixels);

bool oldState = LOW;   // 前回のボタン状態(押していない=LOW)
int showType = 0;      // 現在のアニメーション番号

void setup() {
  pinMode(BTN_BUILTIN, INPUT);   // 基板上でプルダウン済みなのでINPUTでよい
  pinMode(PIXEL_PIN, OUTPUT);
  strip.show(); // 全LEDを消灯状態で初期化
}

void loop() {
  // ボタンの現在の状態を読む(UkiUkiduinoのBTN_BUILTINは押すとHIGH)
  bool newState = digitalRead(BTN_BUILTIN);

  // LOW→HIGHの変化(=ボタンが押された瞬間)を検出する
  if (newState == HIGH && oldState == LOW) {
    // チャタリング防止のため少し待つ
    delay(20);
    // 待った後もまだ押されているか確認する
    newState = digitalRead(BTN_BUILTIN);
    if (newState == HIGH) {
      showType++;
      if (showType > 9) {
        showType = 0;
      }
      startShow(showType);
    }
  }

  // 今回の状態を記憶して次回の比較に使う
  oldState = newState;
}

void startShow(int i) {
  switch (i) {
    case 0: colorWipe(strip.Color(0, 0, 0), 50);    // 消灯
      break;
    case 1: colorWipe(strip.Color(255, 0, 0), 50);  // 赤
      break;
    case 2: colorWipe(strip.Color(0, 255, 0), 50);  // 緑
      break;
    case 3: colorWipe(strip.Color(0, 0, 255), 50);  // 青
      break;
    case 4: theaterChase(strip.Color(127, 127, 127), 50); // 白
      break;
    case 5: theaterChase(strip.Color(127,   0,   0), 50); // 赤
      break;
    case 6: theaterChase(strip.Color(0,   0, 127), 50);   // 青
      break;
    case 7: rainbow(20);
      break;
    case 8: rainbowCycle(20);
      break;
    case 9: theaterChaseRainbow(50);
      break;
  }
}

// 先頭から1個ずつ指定色で塗りつぶしていく
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// 全体の色相を少しずつ回して虹色に変化させる
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

// こちらは虹がストリップ全体に均等に分布するバージョン
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 色相環5周分
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// 劇場の電飾風に流れる点滅
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { // 10周流す
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, c);  // 3個おきに点灯
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      // 3個おきに消灯
      }
    }
  }
}

// 劇場電飾風+虹色
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // 色相環を一周
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, Wheel((i + j) % 255)); // 3個おきに点灯
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      // 3個おきに消灯
      }
    }
  }
}

// 0~255の値を色に変換する(赤→緑→青→赤と一周する色相環)
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
