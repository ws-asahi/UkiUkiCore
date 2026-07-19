/* RGBWstrandtest - RGBW(白チップ入り)NeoPixelストリップの動作テスト
 * 原作: Adafruit NeoPixelライブラリ付属サンプル
 * UkiUkiduino向けに日本語化
 *
 * SK6812などRGBWタイプのストリップ用です。第4の値が専用の
 * 白色LEDの明るさになります(WS2812系のRGBストリップには
 * strandtestを使ってください)。
 */
#include <tinyNeoPixel.h>


#define PIN 9 // D9: NeoPixelのデータピン
#define NUM_LEDS 60

#define BRIGHTNESS 50

tinyNeoPixel strip = tinyNeoPixel(NUM_LEDS, PIN, NEO_GRBW + NEO_KHZ800);


void setup() {
  Serial.begin(115200);
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // 全ピクセルを「消灯」で初期化する
}

void loop() {
  // ピクセルへの表示方法をいくつかのパターンで実演する:
  colorWipe(strip.Color(255, 0, 0), 50); // 赤
  colorWipe(strip.Color(0, 255, 0), 50); // 緑
  colorWipe(strip.Color(0, 0, 255), 50); // 青
  colorWipe(strip.Color(0, 0, 0, 255), 50); // 白(専用白チップ)

  whiteOverRainbow(20, 75, 5);

  pulseWhite(5);

  // fullWhite();
  // delay(2000);

  rainbowFade2White(3, 3, 1);


}

// 1つずつ順番に同じ色で塗りつぶす
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// 白チップだけをゆっくり明滅させる(ガンマ補正付き)
void pulseWhite(uint8_t wait) {
  for (int j = 0; j < 256; j++) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0, tinyNeoPixel::gamma8(j)));
    }
    delay(wait);
    strip.show();
  }

  for (int j = 255; j >= 0; j--) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0, tinyNeoPixel::gamma8(j)));
    }
    delay(wait);
    strip.show();
  }
}

// 虹をフェードインさせて回し、最後は白の点滅へつなぐ
void rainbowFade2White(uint8_t wait, int rainbowLoops, int whiteLoops) {
  float fadeMax = 100.0;
  int fadeVal = 0;
  uint32_t wheelVal;
  int redVal, greenVal, blueVal;

  for (int k = 0; k < rainbowLoops; k ++) {
    for (int j = 0; j < 256; j++) { // 色相環を一巡する

      for (uint16_t i = 0; i < strip.numPixels(); i++) {

        wheelVal = Wheel(((i * 256 / strip.numPixels()) + j) & 255);

        redVal = red(wheelVal) * float(fadeVal / fadeMax);
        greenVal = green(wheelVal) * float(fadeVal / fadeMax);
        blueVal = blue(wheelVal) * float(fadeVal / fadeMax);

        strip.setPixelColor(i, strip.Color(redVal, greenVal, blueVal));

      }

      // 最初の周ではフェードイン!
      if (k == 0 && fadeVal < fadeMax - 1) {
        fadeVal++;
      }

      // 最後の周ではフェードアウト!
      else if (k == rainbowLoops - 1 && j > 255 - fadeMax) {
        fadeVal--;
      }

      strip.show();
      delay(wait);
    }
  }



  delay(500);


  for (int k = 0; k < whiteLoops; k ++) {

    for (int j = 0; j < 256; j++) {

      for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0, tinyNeoPixel::gamma8(j)));
      }
      strip.show();
    }

    delay(2000);
    for (int j = 255; j >= 0; j--) {

      for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0, tinyNeoPixel::gamma8(j)));
      }
      strip.show();
    }
  }

  delay(500);


}

// 虹色の上を白い光の帯が走り抜ける
void whiteOverRainbow(uint8_t wait, uint8_t whiteSpeed, uint8_t whiteLength) {
  if (whiteLength >= strip.numPixels()) {
    whiteLength = strip.numPixels() - 1;
  }

  uint16_t head = whiteLength - 1;
  uint16_t tail = 0;

  int loops = 3;
  int loopNum = 0;

  static unsigned long lastTime = 0;


  while (true) {
    for (int j = 0; j < 256; j++) {
      for (uint16_t i = 0; i < strip.numPixels(); i++) {
        if ((i >= tail && i <= head) || (tail > head && i >= tail) || (tail > head && i <= head)) {
          strip.setPixelColor(i, strip.Color(0, 0, 0, 255));
        } else {
          strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
      }
      // *INDENT-OFF*
      #if !defined(MILLIS_USE_TIMERNONE)
      if (millis() - lastTime > whiteSpeed) {
        head++;
        tail++;
        if (head == strip.numPixels()) {
          loopNum++;
        }
        lastTime = millis();
      }
      #else
        #pragma message("whiteOverRainbow()はmillisを必要としますが、現在無効化されています。この関数は正しく動作しません。")
      #endif
      // *INDENT-ON*

      if (loopNum == loops) {
        return;
      }
      head %= strip.numPixels();
      tail %= strip.numPixels();
      strip.show();
      delay(wait);
    }
  }
}

// 全ピクセルを白チップで全点灯する
void fullWhite() {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0, 255));
  }
  strip.show();
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

// 0~255の値から色を作る。
// 色は 赤 - 緑 - 青 - 赤 と遷移する(白チップは使わない)。
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3, 0);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0, 0);
}

uint8_t red(uint32_t c) {
  return (c >> 8);
}
uint8_t green(uint32_t c) {
  return (c >> 16);
}
uint8_t blue(uint32_t c) {
  return (c);
}
