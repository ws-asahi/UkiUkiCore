# CustomLogic — Wazamono用ハードウェアロジックゲートライブラリ

AVR DUシリーズ内蔵のCCL(Configurable Custom Logic)を、SerialやSDと同じ感覚で
使えるようにしたWazamonoCore専用ライブラリです。`begin()`した瞬間から
**CPUを一切使わずに**ハードウェアだけで論理ゲートが動き続けます。

```cpp
#include <CustomLogic.h>

void setup() {
  CustomLogic.begin(AND);                  // OUT = IN0 AND IN1
  // CustomLogic.begin(OR, OR);            // 3入力OR: (IN0 OR IN1) OR IN2
  // CustomLogic.begin(AND, OR);           // (IN0 AND IN1) OR IN2
  // CustomLogic.begin(NOP, OR);           // IN1 OR IN2(IN0不使用)
  // CustomLogic.begin(NOP);               // IN0のバッファ(OUT=IN0)
  // CustomLogic.beginTruthTable(0x96, 3); // 任意の真理値表(3入力XOR)
}

void loop() {
  // 何も書かなくてもゲートは動き続けます
}
```

## ユニットとピン(ハードウェア固定)

| | IN0 | IN1 | IN2 | OUT | OUT(代替) |
|---|---|---|---|---|---|
| **CustomLogic** (Tachi) | A3 (PD0) | A2 (PD1) | A1 (PD2) | A0 (PD3) | D1 (PD6) |
| **CustomLogic** (Tsurugi) | D5 (PD0) | D6 (PD1) | D9 (PD2) | D10 (PD3) | D13 (PD6) |
| **CustomLogic** (Kunai) | D4 (PA0) | D5 (PA1) | D3 (PA2) | D2 (PA3) | D8 (PA6) |
| **CustomLogic1** (Tachi) | D5 (PF0) | D6 (PF1) | D7 (PF2) | D8 (PF3) | — |
| **CustomLogic1** (Tsurugi) | A0 (PF0) | A1 (PF1) | A2 (PF2) | A3 (PF3) | — |

- Kunaiは1ユニットのみ(CustomLogic1はありません)
- 入力ピンは自動的にプルアップされるので、ボタンをGNDへ直結するだけで実験できます
  (駆動されたロジック信号はプルアップに勝つのでそのまま接続可)

## API

| メソッド | 説明 |
|---|---|
| `begin(logic1)` | 2入力ゲート: OUT = IN0 (logic1) IN1。`LogicType`は`AND/OR/XOR/NAND/NOR/XNOR/NOT/NOP`(NOT=IN0のインバータ、NOP=IN0のバッファ。いずれも1入力) |
| `begin(logic1, logic2)` | 3入力ロジック: OUT = (IN0 logic1 IN1) logic2 IN2。例: `begin(AND, OR)`=A・B+C。**NOPで片側を省略可**: `begin(NOP, x)`=IN0不使用でIN1 (x) IN2、`begin(x, NOP)`=IN2不使用(`begin(x)`と同じ)。NOTは合成不可、(NOP, NOP)は無効 |
| `beginTruthTable(table, n)` | 真理値表を直接指定。bit iが「入力の並びが数値iのときの出力」(IN2=bit2, IN1=bit1, IN0=bit0) |
| `setInputIN0(src)` / `setInputIN1(src)` / `setInputIN2(src)` | その入力の**取得元**を変更(既定は`LOGIC_PIN`)。`setInput(番号, src)`でも同じ |
| `setOutput(pin)` | 結果の**出力先ピン**を変更(そのピンのみに出力) |
| `addOutput(pin)` | **出力先を追加**(複数ピンへ同時出力) |
| `disableOutput()` | 出力ピンなし(割り込み・他ユニットへの供給のみ) |
| `read()` | 現在の出力ピンの状態 |
| `attachInterrupt(fn, mode)` | 出力変化で関数呼び出し(`RISING/FALLING/CHANGE`) |
| `detachInterrupt()` | 割り込み解除 |
| `end()` | 停止してピンを解放 |

## 入力の取得元(`LogicInput`)

入力はピンからでなくても構いません。**チップ内部の配線**なので、ピンも配線もCPU時間も消費しません。

| 定数 | 意味 |
|---|---|
| `LOGIC_PIN` | そのユニットのINnピン(**既定**) |
| `LOGIC_ANALOG_COMP` | **AnalogCompの比較結果**(AC0出力)。`AnalogComp.begin()`するだけでよく、`enableOutput()`は不要 |
| `LOGIC_OWN_OUTPUT` | **自分自身の出力**。ラッチや発振回路が作れる(CustomLogicのみ) |
| `LOGIC_OTHER_UNIT` | **もう一方のユニットの出力**。2段構成の論理が作れる(Tachi/Tsurugiのみ) |
| `LOGIC_EVENT_A` / `LOGIC_EVENT_B` | イベント接続経由。**任意のピン**を入力にできる(EventSystemライブラリで配線) |

```cpp
// 電圧が2.5Vを超えている AND ボタンが押されていない → 出力HIGH(CPU不使用)
AnalogComp.begin(INTERNAL2V5);
CustomLogic.setInputIN0(LOGIC_ANALOG_COMP);
CustomLogic.begin(AND);

// 2段構成: (IN0₁ OR IN1₁) AND IN1₀   ※Tachi/Tsurugi
CustomLogic1.begin(OR);
CustomLogic.setInputIN0(LOGIC_OTHER_UNIT);
CustomLogic.begin(AND);

// 任意のピン(D8)をIN0へ、イベント経由で       ※EventSystemライブラリ併用
EventSystem.connect(8, EVENT_TO_LOGIC_A);
CustomLogic.setInputIN0(LOGIC_EVENT_A);
CustomLogic.begin(AND);
```

**ピンを使わない入力は、そのピンに一切触れません**(プルアップも設定しません)。
たとえばKunaiでIN0/IN1をイベント経由にすれば、重なっているI2Cピン(D4/D5)をそのまま
I2Cとして使えます。

### `LOGIC_OWN_OUTPUT`と`LOGIC_OTHER_UNIT`の関係

CCLはLUTペアの**偶数側**の出力をフィードバックします。CustomLogicが偶数側
(Tachi/Tsurugi=LUT2、Kunai=LUT0)なので自分の出力を見られますが、CustomLogic1は
奇数側(LUT3)なので、そこで見えるのは**CustomLogicの出力**です。これはそのまま
`LOGIC_OTHER_UNIT`の意味なので、`setInput()`は
CustomLogic1での`LOGIC_OWN_OUTPUT`と、Kunaiでの`LOGIC_OTHER_UNIT`を`false`で拒否します。

## 出力先(`setOutput` / `addOutput`)

結果は専用OUTピン以外にも出せます。**イベントシステムの設定はライブラリが行う**ので、
EventSystemライブラリを併用する必要はありません。

| 出力経路 | 説明 |
|---|---|
| 専用OUTピン | 既定。`begin()`しただけで出力される |
| 代替OUTピン | 上表の「OUT(代替)」。CustomLogic1にはありません |
| **イベント出力ピン(EVOUT)** | 下表のピン。**専用OUTピンと同時に使えます** |

**イベント出力ピン(ボードごとに固定 — ピン構成表どおり)**

| | EVOUTA | EVOUTD | EVOUTF |
|---|---|---|---|
| **Tachi** | D2 (PA2) | D0 (PD7) | D7 (PF2) |
| **Tsurugi** | D8 (PA7) | D9 (PD2) | A2 (PF2) |
| **Kunai** | D0 (PA7) | D7 (PD7) | — |

```cpp
CustomLogic.begin(AND);
CustomLogic.addOutput(2);      // 専用OUTピン + D2 の2箇所へ同時出力(Tachi)
// CustomLogic.setOutput(2);   // D2 のみへ出力(専用OUTピンは使わない)
// CustomLogic.disableOutput();// どのピンにも出さない(割り込み・連結のみ)
```

最大で**専用OUTピン1本 + 上表のイベント出力ピン(各1本)**まで同時出力できます
(Tachi/Tsurugiで最大4本、Kunaiで最大3本)。

- イベントチャネルは**高番号側(CH5→)から空きを自動確保**します。EventSystemの接続
  (EventSystem=CH0から順の固定番号)や、他の手段で設定済みのチャネルは決して奪いません。
- **代替OUTピンの注意**: Tachiでは**D1(Serial1のTX)**、Tsurugiでは**D13(SPIのSCK)**と
  兼用です。TsurugiのD13はオンボードLEDにオペアンプ経由で接続されているため、D13へ出力すると
  **オンボードLEDが論理結果を表示**します(SPIとは併用不可)。TsurugiのPD7はAREFのため
  イベント出力の対象外です。

## サンプル

- **TwoInputAND** — 2入力ANDゲート(ボタン2個+LED)
- **ThreeInputOR** — 3入力OR。ゲート名を書き換えるだけで他のゲートに
- **TruthTable** — 真理値表の直接指定(3入力XOR=0x96)
- **EdgeInterrupt** — ゲート出力の変化で割り込み
- **DualUnits** — 2ユニット同時動作(Tachi/Tsurugi)
- **AnalogCompInput** — AnalogCompの結果を入力にする(電圧しきい値 AND ボタン)
- **SetResetLatch** — 自分の出力を入力にしてSRラッチ(ハードだけで「記憶」)
- **MultipleOutputs** — 同じ結果を専用OUTピンとイベント出力ピンへ同時出力

## 注意

- **AnalogCompとのピン共有(Tachi/Tsurugi)**: CustomLogicのIN2/OUT(PD2/PD3)は
  AnalogCompの初期入力と同じピンです。ただし`AnalogComp.begin(INTERNAL2V5)`のように
  **内蔵基準電圧と比較する場合は−入力(PD3)を使わない**ので衝突しません
  (AnalogCompInputサンプル参照)。2入力ゲートならIN2のピン(PD2)にも触れません。
- **KunaiのI2Cとの共有**: KunaiのCustomLogic(PA0〜PA3)はI2Cピンと重なります。
  `setInputIN0(LOGIC_EVENT_A)`などでピンを使わない入力にすれば共存できます。
- ゲートの応答はハードウェア直結です(フィルタなし)。チャタリングのある機械接点を
  割り込みで数える場合はその点に留意してください。

## 実装の出自

本ライブラリはAVR64DU28/32データシート(DS40002548A)とMicrochip公式デバイスヘッダのみを
情報源とした独立実装です。DxCore/megaTinyCoreのLogicライブラリを含む既存の
CCLライブラリのコードは使用・参照していません。
