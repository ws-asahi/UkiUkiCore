/* BLEDRainbow - オンボードLEDを虹色に変化させる
 *
 * オンボードのフルカラーLEDの色相を連続的に一周させ、
 * 虹のグラデーションを表示します。配線は不要です。
 *
 * こちらはsetBLEDColor(r, g, b)のRGB3引数版を使う例です。
 * RGB版は値がそのまま表示される(明るさスケーリングなし)ので、
 * 明るさはwheel()が返す値の大きさ自体で調整します。
 * MAX_BRIGHTを大きくすると明るくなります(255で最大)。
 */

#define MAX_BRIGHT 64   // 虹の明るさ(1~255)。まぶしければ下げる

uint8_t pos = 0;        // 色相位置 0~255で一周

// 0~255の位置から虹色(R,G,B)を作る。赤→緑→青→赤と遷移する。
void wheel(uint8_t p, uint8_t* r, uint8_t* g, uint8_t* b) {
  p = 255 - p;
  if (p < 85) {
    *r = 255 - p * 3;  *g = 0;            *b = p * 3;
  } else if (p < 170) {
    p -= 85;
    *r = 0;            *g = p * 3;        *b = 255 - p * 3;
  } else {
    p -= 170;
    *r = p * 3;        *g = 255 - p * 3;  *b = 0;
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);   // 点灯状態を維持したまま色を動かす
}

void loop() {
  uint8_t r, g, b;
  wheel(pos++, &r, &g, &b);          // posは255の次に0へ戻る(uint8_t)

  // MAX_BRIGHTでスケールしてから表示する
  setBLEDColor((uint16_t)r * MAX_BRIGHT / 255,
               (uint16_t)g * MAX_BRIGHT / 255,
               (uint16_t)b * MAX_BRIGHT / 255);
  delay(20);   // 一周 = 256ステップ × 20ms ≒ 約5秒
}
