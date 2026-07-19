/* 例2: 周波数もデューティ比も変えられるPWM
 * 参考: https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/TakingOverTCA0.md
 *
 * 例1と似たPWMを生成しますが(デューティを割り込みでいじる遊びは抜き)、
 * デューティ比と周波数を設定する2つの関数を備えた、より実用的な形に
 * 一歩進めています。PWMDemo()の代わりにその2関数を呼ぶだけで
 * 実用に使えます。
 *
 * UkiUkiduino向けに日本語化・単板化
 */


#if defined(MILLIS_USE_TIMERA0)
  #error "このスケッチはTCA0を占有します。millisにTCA0を使う設定では使えません。"
#endif

/* PWMはTCA0のWO1に出る。UkiUkiduinoのTCA0はPORTD配置なのでD6(PD1)。 */
uint8_t OutputPin = 6;                            // D6 (PD1, TCA0 WO1)
#define DEMO_TCA_MUX PORTMUX_TCA0_PORTD_gc

unsigned int Period = 0xFFFF;

void setup() {
  pinMode(OutputPin, OUTPUT);
  PORTMUX.TCAROUTEA = (PORTMUX.TCAROUTEA & ~(PORTMUX_TCA0_gm)) | DEMO_TCA_MUX;
  takeOverTCA0(); // 以前必要だった「タイマ停止+リセット」の代わりにこれを呼ぶ
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP1EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc); // シングルスロープPWM、WO1に出力
  TCA0.SINGLE.PER   = Period; // 0xFFFFまでカウント。24MHz・分周なしで約366HzのPWM
  TCA0.SINGLE.CMP1  = 0;
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; // 分周なしでタイマを有効化する
}

void loop() {
  PWMDemo(150000);  // 150kHz
  PWMDemo(70000);   // 70kHz
  PWMDemo(15000);   // 15kHz
  PWMDemo(3000);    // 3kHz
  PWMDemo(120);     // 120Hz
  PWMDemo(35);      // 35Hz
  PWMDemo(13);      // 13Hz
}

void PWMDemo(unsigned long frequency) {
  setFrequency(frequency);
  setDutyCycle(64);   // ~25%
  delay(4000);
  setDutyCycle(128);  // ~50%
  delay(4000);
  setDutyCycle(192);  // ~75%
  delay(4000);
}

void setDutyCycle(byte duty) {
  TCA0.SINGLE.CMP1 = map(duty, 0, 255, 0, Period);
  // map()はいまいちな関数で、もっと良い方法もあります。詳しくは
  // 誰か別の人が書いた別の解説をどうぞ。心当たりはありませんが ;)
}

void setFrequency(unsigned long freqInHz) {
  unsigned long tempperiod = (F_CPU / freqInHz);
  byte presc = 0;
  while (tempperiod > 65536 && presc < 7) {
    presc++;
    tempperiod = tempperiod >> (presc > 4 ? 2 : 1);
  }
  Period = tempperiod;
  TCA0.SINGLE.CTRLA = (presc << 1) | TCA_SINGLE_ENABLE_bm;
  TCA0.SINGLE.PER = Period;
}
