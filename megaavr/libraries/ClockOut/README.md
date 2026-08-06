# ClockOut — メインクロック出力（CLKOUT）ライブラリ

Wazamono ボードの **CLK_PER（周辺/CPU クロック）を CLKOUT ピンへ出力**するライブラリです。`Serial` などと同じく `begin()` で出力開始、`end()` で停止します。

```cpp
#include <ClockOut.h>

void setup() {
  ClockOut.begin();                        // CLK_PER の出力開始
  Serial.println(ClockOut.frequency());    // 24000000（24MHz 動作時）
}
```

## CLKOUT ピン

CLKOUT は **PA7 固定**で代替位置がありません。

| ボード | CLKOUT ピン |
|--------|-------------|
| Wazamono Tachi | **D21** |
| Wazamono Tsurugi | **D2** |
| Wazamono Kunai | **D1** |

## 主な用途

- **外部 IC へのクロック供給** — コーデック / ADC / ロジック IC に系のクロックを分配し、2 個目の水晶や発振器を省く
- **他 MCU との同期** — 相手の外部クロック入力（XTALHF1）へ入れて同期動作させる
- **実クロックの実測** — オシロや周波数カウンタで系のクロックを確認。特に**水晶を持たない Kunai** は OSCHF を USB フレーム信号に対して自動チューニングするため、チューニングが効いているかの確認手段として実用的です
- **生産時検査** — 内蔵オシレータの精度を基準器と比較する検査ポイント

## API

| メソッド | 説明 |
|----------|------|
| `bool begin()` | CLKOUT 出力開始。PA7 が他の周辺機能に取られている場合は**何も変更せず `false` を返す** |
| `void end()` | 出力停止。PA7 を入力に戻す |
| `bool isRunning()` | 実際に出力中かどうか。レジスタの CLKOUT ビットを直接読むため、**CFD（クロック故障検出）によるハードウェア自動停止も検出**できます |
| `uint32_t frequency()` | 出力周波数 [Hz]（= CLK_PER = `F_CPU`） |
| `uint8_t pin()` | CLKOUT の Arduino ピン番号（Tachi=21 / Tsurugi=2 / Kunai=1） |

## 分周指定が無い理由

CLKOUT が出すのは **CLK_PER そのもの**です。この経路にある分周器は CLK_MAIN プリスケーラ（`CLKCTRL.MCLKCTRLB`）だけで、これは **CPU クロックも同時に変えてしまいます**（`millis()`・USB・UART ボーレートがすべてずれます）。

したがって本ライブラリは分周引数を提供しません。**任意の周波数をピンに出したい場合は CLKOUT ではなく TCA/TCB の PWM 出力**（`analogWrite()` / `tone()`）**や CCL** を使ってください。

## PA7 の競合

PA7 には次の機能が集中しています。

| 機能 | 検出 | 備考 |
|------|------|------|
| AC0 コンパレータ出力 | **`begin()` が拒否** | `AC0.CTRLA` の OUTEN を確認 |
| イベント出力 EVOUTA | **`begin()` が拒否** | EVSYS のユーザ割当と PORTMUX 位置を確認 |
| SPI SS（**Kunai のみ**） | **`begin()` が拒否** | SPI0 が有効なら拒否（`SPISlave` 使用中を含む） |
| USART0 XDIR（ALT1 位置） | 検出しない | RS-485 用 DIR。使用時は手動で排他管理してください |
| スケッチによる通常の GPIO 使用 | 検出不可 | `pin()` で番号を取得して自分で管理してください |

## EMI・消費電流に関する注意

24MHz の連続した矩形波は**強い EMI 源**であり、ピンドライバの消費電流も増えます。製品では以下を推奨します。

- 配線は最短に。長いパターンやヘッダへ引き出したままの常時出力は VCCI 対応の観点で不利です
- 必要な期間だけ `begin()` / `end()` で開閉する（サンプル `ClockOut_OnDemand` 参照）
- 未使用時は有効化しない（リセット既定値は無効）

## 実装メモ（DS40002548A 12章）

- 出力信号は **CLK_PER**（12.2.2 信号説明: `CLKOUT — Digital output — CLK_PER output`）
- 制御は `CLKCTRL.MCLKCTRLA` の bit7（CLKOUT）。このレジスタは **CCP 保護**付きのため、書き込み値を事前にレジスタへロードしてから CCP キーを書き、さらに割り込みを止めて保護ウィンドウを守っています（保護ウィンドウ内でのリード・モディファイ・ライトは完了しません）
- **CFD（クロック故障検出）**でメインクロックの CLKSEL がオーバーライドされると、**CLKOUT ビットはハードウェアが自動的にクリア**します。`isRunning()` はこれを反映します
- データシートは CLKOUT の override がピン方向まで設定するかを明記していないため、`begin()` は明示的に `pinMode(OUTPUT)` を実行します（どちらの挙動でも無害）

## サンプル

- **ClockOut_Basic** — 出力開始、周波数とピン番号の表示、CFD による停止の監視
- **ClockOut_OnDemand** — 外部デバイスが必要とする期間だけ出力する EMI 配慮の使い方
