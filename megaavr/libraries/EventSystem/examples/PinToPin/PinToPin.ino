/* EventSystem / PinToPin(ピンからピンへ)
 *
 * イベントシステムの最小の使用例です。あるピンのレベルが、チップ内部の
 * 配線だけを通って別のピンにそのまま現れます。外部配線も、loop()内の
 * コードも不要で、CPUがスリープしていても動き続けます。
 *
 * このスケッチでは、オンボードボタン(BTN_BUILTIN = D20)の状態を
 * イベント出力ピンD8へ届けます。D8にLED(+抵抗、GNDへ)を接続すると、
 * ボタンを押している間だけLEDが点灯します。
 *
 * メモ: connect()は入力側ピンに内部プルアップを設定しますが、D20には
 * 基板上の1kΩプルダウンがあり、そちらが支配するため「押すとHIGH」の
 * 動作のままです(未押下=LOW=消灯)。
 *
 * どのピンが使える?
 *
 * 出力側 - イベント出力機能を持つ固定ピンのみ(1出力につき1本):
 *   D8, D9, A2
 *
 * 入力側 - 任意のピン。ただし同時に使えるのは「1ポートにつき2本まで」
 * (ポートごとのイベントジェネレータが2本というハードウェア制限で、
 * 全EventSystem接続で共有されます)。ポートとピンの対応は:
 *   PORTA: D0  D1  D2  D8  D20 A4  A5
 *   PORTC: D7
 *   PORTD: D5  D6  D9  D10 D11 D12 D13
 *   PORTF: D3  D4  A0  A1  A2  A3
 * 例: D0とD1を入力源にするのはOK。さらにD2(PORTAの3本目)を足そうと
 * すると、そのconnect()はfalseを返します。
 */
#include <EventSystem.h>

void setup() {
  Serial.begin(115200);

  bool ok = EventSystem.connect(BTN_BUILTIN, 8);   // D20(ボタン) -> D8(EVOUTA)

  Serial.println(ok ? F("connected - D8 now follows the button")
                    : F("connect() failed"));
}

void loop() {
  /* 何もすることはありません - 接続は純粋なハードウェアです。 */
}
