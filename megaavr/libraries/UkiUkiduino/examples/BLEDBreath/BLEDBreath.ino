/* BLEDBreath - オンボードLEDのブレス(呼吸)点灯
 *
 * オンボードのフルカラーLEDを、ブートローダのDFUモードと同じように
 * ゆっくり明滅(ブレス)させます。配線は不要です。
 * ボタン(BTN_BUILTIN)を押すと色が切り替わります。
 *
 * setBLEDColor(色名, 明るさ)の第2引数(0~255)を三角波で動かすだけで
 * 実現できます。LEDは点灯状態(D13=HIGH)のままにしておき、
 * 「点灯中の色変更は即時反映」される性質を利用します。
 *
 * ※明るさ変更1回ごとに約0.34msのLED通信が入るため、更新間隔は
 *   数ms以上にするのがおすすめです(このサンプルは8ms間隔)。
 */

const LEDColorName colors[] = { Yellow, Cyan, Magenta, Green };
const uint8_t numColors = sizeof(colors) / sizeof(colors[0]);
uint8_t colorIndex = 0;

int16_t level = 0;      // 現在の明るさ 0~255
int8_t  dir = 1;        // +1=明るく / -1=暗く

void setup() {
  pinMode(BTN_BUILTIN, INPUT);          // 基板に1kΩプルダウン実装済み
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);      // 点灯状態を維持したまま明るさを動かす
}

void loop() {
  // 三角波: 0 -> 255 -> 0 -> ... と明るさを往復させる
  level += dir;
  if (level >= 255) { level = 255; dir = -1; }
  if (level <= 0)   { level = 0;   dir = +1; }

  setBLEDColor(colors[colorIndex], (uint8_t)level);
  delay(8);   // 全体で約4秒周期のゆったりしたブレスになる

  // ボタンで色変更(押している間に一度だけ進める)
  if (digitalRead(BTN_BUILTIN)) {       // 押下=HIGH
    colorIndex = (colorIndex + 1) % numColors;
    while (digitalRead(BTN_BUILTIN)) { }   // 離すまで待つ
  }
}
