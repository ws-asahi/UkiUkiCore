# EventSystem — チップ内配線ライブラリ(Wazamono専用)

**イベントシステム**は、チップの中に用意された「配線」です。信号をある場所から別の場所へ、
**CPUを一切使わずに**運びます。スケッチが何をしていても、CPUがスリープしていても動き続けます。

このライブラリは**接続を6本**、最初から用意しています:

```
EventSystem, EventSystem1, EventSystem2, EventSystem3, EventSystem4, EventSystem5
```

1本の接続は「**1つの送り元 → いくつでも送り先**」を運びます。

```cpp
#include <EventSystem.h>

void setup() {
  EventSystem.connect(8, 2);   // ピンD8の状態がそのままD2に出る。これだけ。
}
```

## 送り元(何を運ぶか)

| 指定 | 意味 |
|---|---|
| Arduinoピン番号 | そのピンのレベル(**同じポートから同時に2本まで** — ハードウェアの制約) |
| `EVENT_ANALOG_COMP` | **AnalogCompの比較結果**。`AnalogComp.begin()`するだけでよい |
| `EVENT_CUSTOM_LOGIC` | **CustomLogicの出力** |
| `EVENT_CUSTOM_LOGIC1` | CustomLogic1の出力(Tachi/Tsurugiのみ) |
| `EVENT_SOFTWARE` | 何も運ばない。`trigger()`を呼んだ瞬間に**1クロックのパルス**を送る |

## 送り先(どこへ届けるか)

**ピンへ** — 出せるピンはボードごとに固定です(ピン構成表どおり):

| | EVOUTA | EVOUTD | EVOUTF |
|---|---|---|---|
| **Tachi** | D2 | D0 | D7 |
| **Tsurugi** | D8 | D9 | A2 |
| **Kunai** | D0 | D7 | — |

**CustomLogicの入力へ** — `EVENT_TO_LOGIC_A` / `EVENT_TO_LOGIC_B`
(CustomLogic1へは `EVENT_TO_LOGIC1_A` / `_B`。Tachi/Tsurugiのみ)。
ロジック側は `CustomLogic.setInputINn(LOGIC_EVENT_A)` で受けます。

```cpp
// 「電圧が2.5Vを超えている AND ボタン」— 配線もCPUも使わない
AnalogComp.begin(INTERNAL2V5);
EventSystem.connect(EVENT_ANALOG_COMP, EVENT_TO_LOGIC_A);
CustomLogic.setInputIN0(LOGIC_EVENT_A);
CustomLogic.begin(AND);

// 任意のピン(D8)をCustomLogicのIN0として使う
EventSystem1.connect(8, EVENT_TO_LOGIC_B);
```

## API

| メソッド | 説明 |
|---|---|
| `connect(送り元, 送り先)` | 接続を作る。2回目以降は送り元を差し替え(送り先は維持) |
| `addDestination(送り先)` | 送り先を**追加**(1本の接続から複数へ同時配信) |
| `disconnect(送り先)` | その送り先だけ切る |
| `trigger()` | この接続に**1クロックのパルス**を送る(主に`EVENT_SOFTWARE`用) |
| `connected()` | 接続済みか |
| `end()` | 接続を解体し、使ったものをすべて返す |

すべての`connect`/`addDestination`は成否を`bool`で返します。`false`になるのは:
送り先がそのボードの固定ピンでない、同じポートのピン送り元が3本目、
送り先が別の接続に使用中、この接続のチャネルが他の機能(例: `CustomLogic.addOutput()`)に
使用中、Kunaiで`*_LOGIC1`系を指定 — のいずれかです。

## 特徴と注意

- **ピンを勝手に使いません** — ピンに触れるのは、そのピンを送り元/送り先に指定したときだけです。
  送り元ピンにはプルアップを設定します(GNDへのボタンがそのまま動く。駆動信号はプルアップに勝つ)。
- **CustomLogicと安全に共存** — `CustomLogic.setOutput()/addOutput()`も同じ6チャネルを使いますが、
  互いに「他者が設定したチャネル」を絶対に奪いません(CustomLogicは高番号側から、
  こちらは自分の固定番号だけを使うので、実用上ぶつかりません)。
- **`trigger()`のパルスは1クロック(24MHzで約42ns)** — LEDを直接繋いでも目には見えません。
  CustomLogicのラッチ(SoftwareTriggerサンプル)やカウンタのような「エッジで動くもの」に使います。
- TCA/TCB/RTC/USART/SPIのイベント機能は、他の機能との競合を避けるため**このライブラリでは提供しません**。

## サンプル

- **PinToPin** — ボタンをイベント出力ピンへ(最小の1行)
- **CompToPin** — AnalogCompのしきい値判定結果をピンへ(CPU不使用のインジケータ)
- **PinToLogic** — 任意のピンをCustomLogicの入力にする(KunaiでI2Cを潰さない実例)
- **SoftwareTrigger** — `trigger()`でCustomLogicのラッチをセットする

## 出自について

本ライブラリはWazamonoCoreのための**新規実装**です。AVR64DU28/32データシート
(DS40002548A)§16 EVSYS・§17 PORTMUX・§18 PORTのみを典拠とし、DxCoreのEventライブラリを
含む既存実装に由来するコードを含みません。
