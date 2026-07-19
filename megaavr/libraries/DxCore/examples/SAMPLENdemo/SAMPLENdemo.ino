/* ADCサンプル時間(SAMPLEN)のデモ
 *
 * A0とA1の間に1MΩの抵抗を接続してください。A0が「非常に高インピー
 * ダンスな入力」になり、A1がそれを駆動する構成になります。
 *
 * このスケッチはA1を反転させた直後にA0を数回読み、最初の測定値を
 * 表示します。出力はExcel等でのグラフ化に向く形式です(.csv保存)。
 *
 * 重要なのは「ADCが最後に読んだ電圧が何だったか」です。例えば高
 * インピーダンス源とGND付近の電圧を交互に読むと、SAMPLENが小さい
 * 場合は測定値が低い側へ、直前に高い電圧を測っていた場合は高い側へ
 * ずれます。裏を返せば、同じ高インピーダンス・緩変化の電圧を測り
 * 続けるなら、複数のアナログ電圧を切り替える場合より短いサンプル
 * 時間で済むということです。
 *
 * 最後に、変化がLOW→HIGHのときの方がHIGH→LOWより遅いこと
 * (原作者いわく「かなり奇妙」)、LOWでは0まで落ち切るのにHIGHでは
 * 上限まで届き切らないことにも注目してください。
 *
 * UkiUkiduino向けに日本語化
 */
#define FIRST_PIN A0   // テスト対象のアナログ入力
#define SECOND_PIN A1  // 隣で駆動する側のピン
#if !defined(ADC_LOWLAT_bm) // AVR DA/DB/DDでのレジスタ名
  #define SAMPLENREG (ADC0.SAMPCTRL)
#else
  #define SAMPLENREG (ADC0.CTRLE) // AVR DU/EA/EB/SDでは名前が違う
#endif

void setup() {
  pinMode(SECOND_PIN, OUTPUT);
  digitalWrite(SECOND_PIN, LOW);
  SAMPLENREG = 0xFF;
  analogReadResolution(ADC_NATIVE_RESOLUTION); // Dx系は通常12ビットだが、DUとSDは10ビット。
  Serial.begin(115200);
  delay(1000);
  analogRead(FIRST_PIN);
}

void loop() {
  SAMPLENREG++;
  Serial.print("Sampctrl=");
  Serial.println(SAMPLENREG);
  Serial.flush();
  digitalWrite(SECOND_PIN, 1);
  Serial.print(analogRead(FIRST_PIN));
  Serial.print(' ');
  Serial.println(analogRead(FIRST_PIN));
  Serial.flush();
  digitalWrite(SECOND_PIN, 0);
  Serial.print(analogRead(FIRST_PIN));
  Serial.print(' ');
  Serial.println(analogRead(FIRST_PIN));
  Serial.flush();
  if (SAMPLENREG == 255) {
    while (1);
  }
}
