/* 例4: スプリットモードでちょっと遊ぶ
 * 参考: https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/TakingOverTCA0.md
 *
 * スプリットモードの面白さを示す小ネタです - 1つのタイマから2つの
 * 異なるPWM周波数を取り出せます。スプリットモードにはモードが1つしか
 * なく、タイマの前半と後半がそれぞれ独立にダウンカウントします。
 * ここではさらに面白くするため、ほぼ同じ2つの周波数を使います…
 * 両者は1.43Hz(366Hz/256)の周期で「うなり」を生じます。2本のピンの
 * 間に2色LED(と適切な抵抗)をつなぐと観察できます。2色LEDは極性が
 * 逆の2つのLED(通常は赤と緑)が2端子間に入ったものです… さて、
 * どう見えるでしょうか? 単色LEDと何が違うでしょうか? 予想してから
 * 試してください。原作者のSpenceは予想を外したそうです。
 *
 * UkiUkiduino向けに日本語化・単板化
 */

#if defined(MILLIS_USE_TIMERA0)
  #error "このスケッチはTCA0を占有します。millisにTCA0を使う設定では使えません。"
#endif

/* スプリットモードのPWMはTCA0のWO2とWO3に出る。
 * UkiUkiduinoのTCA0はPORTD配置なのでD9(PD2)とD10(PD3)。 */
#define DEMO_WO2_PIN 9                            // D9 (PD2)
#define DEMO_WO3_PIN 10                           // D10 (PD3)
#define DEMO_TCA_MUX PORTMUX_TCA0_PORTD_gc


void setup() {
  // D9(PD2)とD10(PD3)へPWMを出力する
  // スプリットモードの有効化は不要 - コアが既にやってくれている。
  pinMode(DEMO_WO2_PIN, OUTPUT); // TCA0 WO2
  pinMode(DEMO_WO3_PIN, OUTPUT); // TCA0 WO3
  PORTMUX.TCAROUTEA = (PORTMUX.TCAROUTEA & ~(PORTMUX_TCA0_gm)) | DEMO_TCA_MUX;
  TCA0.SPLIT.CTRLB = TCA_SPLIT_LCMP2EN_bm | TCA_SPLIT_HCMP0EN_bm; // WO2とWO3にPWM
  TCA0.SPLIT.LPER = 0xFF; // WO0/WO1/WO2側は255からダウンカウント
  TCA0.SPLIT.HPER = 0xFE; // WO3/WO4/WO5側は254からダウンカウント
  TCA0.SPLIT.LCMP2 = 128; // デューティ比50%
  TCA0.SPLIT.HCMP0 = 127; // デューティ比50%
  TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV256_gc | TCA_SPLIT_ENABLE_bm; // 256分周でタイマを有効化 - 位相のずれをゆっくりに、ただしチラつかない程度に…
}

void loop() {
  // することはない。PWMを楽しんで。
}
