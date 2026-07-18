# AnalogComp — Wazamono用アナログコンパレータライブラリ

AVR DUシリーズ内蔵のアナログコンパレータ(AC0)を、SerialやSDと同じ感覚で使えるようにした
WazamonoCore専用ライブラリです。インスタンス `AnalogComp` が用意済みなので、
`begin()` するだけで使い始められます。

```cpp
#include <AnalogComp.h>

void setup() {
  AnalogComp.begin();                    // +ピンと−ピンを比較
  // AnalogComp.begin(INTERNAL2V5);      // +ピンを内蔵基準電圧2.5Vと比較
  // AnalogComp.begin(VDD, 128);         // +ピンをVDDの1/2と比較
}

void loop() {
  if (AnalogComp.read()) {     // + > − のとき true
    ...
  }
}
```

## 入出力ピン(ハードウェア固定)

| | +入力 | −入力 | 出力 (`enableOutput()`) |
|---|---|---|---|
| **Tachi** | A1 (PD2) | A0 (PD3) | D4 (PA7) |
| **Tsurugi** | D9 (PD2) | D10 (PD3) | D8 (PA7) |
| **Kunai** | D6 (PD6) | D7 (PD7) | D0 (PA7) |

`setInputs(plus, minus)` で別の対応ピンへ切り替えられます
(+: PD2/PD6、−: PD3/PD0/PD7 のうちボードに実在するもの)。

## API

| メソッド | 説明 |
|---|---|
| `begin()` | +ピン vs −ピンで比較開始 |
| `begin(ref)` / `begin(ref, level)` | +ピン vs 基準電圧で比較開始。`ref`は`analogReference()`と同じ定数(`INTERNAL1V024/2V048/2V5/4V096`、`VDD`、`EXTERNAL`)。しきい値 = Vref × level ÷ 256(level省略時255≒Vrefそのもの) |
| `read()` | 比較結果。+ > − で `true` |
| `setThreshold(ref, level)` | 基準電圧・しきい値を変更(−入力を基準電圧側に切替) |
| `setInputs(plus, minus)` | 入力ピンの切り替え(非対応ピンなら `false`) |
| `setHysteresis(level)` | `AC_HYST_NONE/SMALL/MEDIUM/LARGE`(約0/10/25/50mV) |
| `enableOutput(invert)` / `disableOutput()` | 比較結果をPA7へ出力 |
| `attachInterrupt(fn, mode)` | `RISING/FALLING/CHANGE` で関数呼び出し |
| `detachInterrupt()` | 割り込み解除 |
| `end()` | 停止してピンを解放 |

## サンプル

- **ReadState** — ピン同士の比較結果をシリアルへ表示
- **Threshold** — 内蔵基準電圧2.5Vとの比較結果をLEDに表示
- **OutputPin** — 比較結果をPA7へハードウェア出力(スケッチ介在なし)
- **EdgeInterrupt** — しきい値クロスで割り込み

## 実装の出自

本ライブラリはAVR64DU28/32データシート(DS40002548A)とMicrochip公式デバイスヘッダのみを
情報源とした独立実装です。DxCore/megaTinyCoreのComparatorライブラリを含む既存の
コンパレータライブラリのコードは使用・参照していません。

## 注意

- 入力電圧はGND〜VDDの範囲で使用してください。
- `EXTERNAL`指定時はVREFAピン(PD7)へ基準電圧を入力します。
- ノイズのある信号では `setHysteresis()` の併用を推奨します(特に割り込み使用時)。
- 内蔵基準電圧の精度を保つため、しきい値はVDDより0.5V以上低くしてください。
