/* BLEDColorCycle - ボタンでオンボードLEDの色を切り替える
 *
 * UkiUkiduinoのオンボードボタン(BTN_BUILTIN)を押すたびに、
 * オンボードのフルカラーLEDが10色を順番に切り替わります。
 * 配線は不要です - 書き込んだらそのままボタンを押してみてください。
 *
 * オンボードLEDのしくみ:
 *   - LEDは今まで通りD13(LED_BUILTIN)で点灯/消灯します。
 *     digitalWrite(LED_BUILTIN, HIGH) = 点灯、LOW = 消灯。
 *     (だからBlinkスケッチも無改変で動きます)
 *   - setBLEDColor()は「点灯したときの色」を変えます:
 *       setBLEDColor(Yellow);         色名で指定(既定の明るさ40)
 *       setBLEDColor(Yellow, 255);    色名+明るさ0~255
 *       setBLEDColor(r, g, b);        RGB値をそのまま表示
 *   - 点灯中に呼ぶと即時反映、消灯中に呼ぶと次の点灯から反映です。
 *
 * ※オンボードボタンはプルダウン式で「押すとHIGH」です
 *   (一般的なArduinoのプルアップ式とは逆なので注意)。
 */

// 順番に切り替える色のリスト
const LEDColorName colors[] = {
  Yellow, Red, Orange, Green, Cyan, Blue, Purple, Magenta, Pink, White
};
const char* colorNames[] = {
  "Yellow", "Red", "Orange", "Green", "Cyan", "Blue", "Purple", "Magenta", "Pink", "White"
};
const uint8_t numColors = sizeof(colors) / sizeof(colors[0]);

uint8_t colorIdx = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BTN_BUILTIN, INPUT);          // 基板に1kΩプルダウン実装済み
  pinMode(LED_BUILTIN, OUTPUT);

  setBLEDColor(colors[colorIdx]);          // 最初の色(黄色)にして…
  digitalWrite(LED_BUILTIN, HIGH);      // 点灯!
  Serial.println("Button: next color");
}

void loop() {
  if (digitalRead(BTN_BUILTIN)) {       // 押下=HIGH
    colorIdx = (colorIdx + 1) % numColors;    // 次の色へ(最後まで行ったら先頭へ)
    setBLEDColor(colors[colorIdx]);        // 点灯中なので即時に色が変わる
    Serial.println(colorNames[colorIdx]);

    delay(30);                                // チャタリング吸収
    while (digitalRead(BTN_BUILTIN)) { }      // ボタンが離されるまで待つ
    delay(30);
  }
}
