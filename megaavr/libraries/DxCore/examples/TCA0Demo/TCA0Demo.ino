/* 例1: シングルモード16ビットPWM(デュアルスロープ+割り込み)
 * 参考: https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/TakingOverTCA0.md
 *
 * 「ISRでデューティ比を書き換える」こと自体は実用的とは言えませんが、
 * TCA0の割り込み設定方法を示すのが目的です。
 *
 * UkiUkiduino向けに日本語化・単板化
 */

#if defined(MILLIS_USE_TIMERA0)
  #error "このスケッチはTCA0を占有します。millisにTCA0を使う設定では使えません。"
#endif

unsigned int DutyCycle = 0;
/* PWMはTCA0のWO1に出る。UkiUkiduinoのTCA0はPORTD配置なのでD6(PD1)。 */
uint8_t OutputPin = 6;                            // D6 (PD1, TCA0 WO1)
#define DEMO_TCA_MUX PORTMUX_TCA0_PORTD_gc


void setup() {
  pinMode(OutputPin, OUTPUT);
  takeOverTCA0(); // 以前必要だった「タイマ停止+リセット」の代わりにこれを呼ぶ
  PORTMUX.TCAROUTEA   = (PORTMUX.TCAROUTEA & ~(PORTMUX_TCA0_gm)) | DEMO_TCA_MUX; // WO出力をこのボードのPWMポートへ配置する
  TCA0.SINGLE.CTRLB   = (TCA_SINGLE_CMP1EN_bm | TCA_SINGLE_WGMODE_DSBOTTOM_gc); // デュアルスロープPWM、OVF割り込みはBOTTOMで発生、WO1にPWM
  TCA0.SINGLE.PER     = 0xFFFF;               // 0xFFFFまでフルにカウントする。
  //                                             UkiUkiduinoの24MHz・分周なしで約183HzのPWMになる。
  TCA0.SINGLE.CMP1    = DutyCycle;            // 0 - 65535
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;    // オーバーフロー割り込みを有効化する
  TCA0.SINGLE.CTRLA   = TCA_SINGLE_ENABLE_bm; // 分周なしでタイマを有効化する
}

void loop() { // ここでは何もしない
}

ISR(TCA0_OVF_vect) {    // オーバーフローのたびCMP1を増やす。全周期(7分強)でデューティが一巡する。
  TCA0.SINGLE.CMP1      = DutyCycle++; // デュアルスロープBOTTOMモードなのでOVFはパルス途中のTOPでなく末尾のBOTTOMで発生する
  TCA0.SINGLE.INTFLAGS  = TCA_SINGLE_OVF_bm; // 割り込みフラグのクリアを忘れずに。忘れると割り込みが連続発生する!
}
