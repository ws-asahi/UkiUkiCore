/* 例3: 高速8ビットPWM
 * 参考: https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/TakingOverTCA0.md
 *
 * スプリットモードを無効にした高速PWMのデモです。8ビットの分解能を
 * 保てる最高のPWM周波数はCLK_PER/256。UkiUkiduinoの24MHzなら
 * 93.75kHzです。完全な8ビット分解能を保てる次の周波数はその半分に
 * なります。それ以上の周波数では分解能を下げる必要があります
 * (中間の周波数にも使える手法は例2参照)。周波数が固定なら、map()を
 * 使わず0~PERの値を直接与える方が滑らかでおすすめです。ちなみに
 * 93.75kHzで足りるなら、スプリットモードを無効にする必要すら
 * ありません(イベント入力やバッファリング等が要る場合を除く)。
 *
 * UkiUkiduino向けに日本語化・単板化
 */

#if defined(MILLIS_USE_TIMERA0)
  #error "このスケッチはTCA0を占有します。millisにTCA0を使う設定では使えません。"
#endif

/* PWMはTCA0のWO2に出る。UkiUkiduinoのTCA0はPORTD配置なのでD9(PD2)。 */
#define DEMO_OUT_PIN 9                            // D9 (PD2, TCA0 WO2)
#define DEMO_TCA_MUX PORTMUX_TCA0_PORTD_gc


void setup() {
  // D9(PD2)へPWMを出力する
  pinMode(DEMO_OUT_PIN, OUTPUT);
  takeOverTCA0();

  PORTMUX.TCAROUTEA = (PORTMUX.TCAROUTEA & ~(PORTMUX_TCA0_gm)) | DEMO_TCA_MUX;
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP2EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc); // シングルスロープPWM、WO2に出力
  TCA0.SINGLE.PER = 0x00FF; // 0x00FF(255)までカウント = 8ビットPWM
  // 24MHzでは93.75kHzのPWMになる
  TCA0.SINGLE.CMP2 = 0;
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; // 分周なしでタイマを有効化する
}

void loop() { // 動作している証拠に出力を変化させてみる
  static byte pass = 0;
  static unsigned int duty = 255;
  TCA0.SINGLE.CMP2 = duty--; // loopのたびにデューティ比を1段下げる
  delay(100);  // オシロやLEDでデューティの変化が見えるように
  if (!duty) {
    if (pass == 0) {
      // 1周目が終わったら100kHzへ上げる
      pass = 1;
      duty = 199;
      TCA0.SINGLE.PER = 199;
    } else if (pass == 1) {
      // 次はご要望の62kHz(実際は62.11kHz)
      pass = 2;
      duty = 322;
      TCA0.SINGLE.PER = 322;
    } else { // そして最初へ戻る
      pass = 0;
      duty = 255;
      TCA0.SINGLE.PER = 255;
    }
  }
}
