# Wazamono 苦無（Kunai）

**Seeeduino XIAO 後継機 — AVR32DU20 / USB-C**

Wazamono Kunai は、Seeed の XIAO と同じ超小型フォームファクタ（21 × 17.8 mm）を、USB ネイティブな新世代 AVR `AVR32DU20` で再設計したボードです。
USB-シリアル変換チップを搭載せず、マイコン単体で USB-C により PC と直接つながります。
XIAO シリーズの定番である SAMD21 版（Seeeduino XIAO）を **5V / 3V 動作の AVR** で置き換えることを狙ったボードです。

> このページは Wazamono Kunai 1 機種のドキュメントです。コア全体の概要は [README](../../README.md) を参照してください。
> **状態: 開発中。** ピン定義・ブートローダは変更される可能性があります。確定 BOM/回路図は準備中です。

---

## 概要

| 項目 | 内容 |
|------|------|
| MCU | AVR32DU20（20 ピン） |
| フォームファクタ | Seeeduino XIAO 互換 |
| USB | USB-C（USB 2.0 Full-Speed、マイコン内蔵） |
| クロック | 24 MHz 内蔵オシレータ（USB接続時に自動調整） |
| 電源 | USB 5V または VIN入力（駆動電源は JP1 で 5V / 3.3V を選択） |
| 書き込み | USB CDC ブートローダ（STK500v1） |

---

## ボード諸元（AVR32DU20）

| 項目 | 値 |
|------|----|
| Flash | 32 KB（うちスケッチ用 28 KB/USB ブートローダ 4 KB） |
| SRAM | 4 KB |
| EEPROM | 256 B |
| USERROW | 512 B |
| 最大動作周波数 | 24 MHz |
| USB | USB 2.0 Full-Speed デバイス（In/Out 16 EP ずつ最大 32 EP） |
| ADC | 10-bit 170 ksps × 1（11 チャネル） |
| タイマ | TCA0 ×1（PWM 6ch）、TCB ×2 (TCB1 は通常 PWM 用) |
| USART | 2（Serial1 / Serial2） |
| SPI | 2（SPI / SPI1） |
| I2C | 1 |
| CCL（LUT） | 3 |
| イベントシステム | 2 チャネル |
| アナログコンパレータ（AC） | 1 |

<sub>諸元は AVR16/32DU ファミリデータシート（DS40002576）に基づく。</sub>

---

## SAMD21（Seeeduino XIAO）との比較

Wazamono Kunai が置き換える Seeeduino XIAO は **ATSAMD21G18**（ARM Cortex-M0+、3.3V 動作）を搭載しています。
AVR32DU20 は **8-bit の AVRxt コア**で、演算性能やメモリ容量では SAMD21 に及びませんが、
**5V ネイティブ動作** と **高い出力電流値** ならではの周辺機能と扱いやすさで差別化します。
また同一基板上で 5V と 3.3V の動作電圧切り替えが可能です。

| 項目 | Wazamono Kunai (AVR32DU20) | Seeeduino XIAO (SAMD21G18) |
|------|----------------------------|----------------------------|
| コア | 8-bit AVRxt | 32-bit ARM Cortex-M0+ |
| 最大クロック | 24 MHz | 48 MHz |
| 動作電圧 | 5V / 3.3V（1.8–5.5V） | 3.3V のみ（5V 非トレラント） |
| Flash | 32 KB | 256 KB |
| SRAM | 4 KB | 32 KB |
| EEPROM | 256 B | なし（フラッシュエミュレーション） |
| ADC | 10-bit | 12-bit |
| DAC | なし | 10-bit ×1 |
| USB | Full-Speed デバイス（内蔵） | Full-Speed デバイス（内蔵） |
| USART | 2 | SERCOM ×6（用途を割当 |
| SPI | 2（1つはホスト限定） | SERCOM ×6（用途を割当 |
| I2C | 1 | SERCOM ×6（用途を割当 |
| CCL（LUT） | 3 | なし |
| イベントシステム | 2 ch | なし |
| アナログコンパレータ（AC） | 1 | なし |

### Kunai を選ぶ理由

- **5V ネイティブ動作** — XIAO の最大の制約だった「3.3V のみ」を解消。5V ロジックのセンサ・モジュール・リレー等をレベル変換なしで直接駆動できます。
- AVR32DU20 は 1.8–5.5V の全域で 24 MHz 動作が可能です。
- **真の EEPROM** — 256 B の独立した EEPROM を搭載。SAMD21 はフラッシュエミュレーションのため、設定値の保存が手軽です。
- **AVR / Arduino-AVR エコシステム** — 古典 AVR 向けの豊富なライブラリ・作例がそのまま、あるいは小修正で動きます。
- `<avr/io.h>` レベルの低レベル制御も馴染みやすい構成です。
- **新世代の周辺機能** — CCL（3 論理ブロック）とイベントシステム（3 チャネル）により、CPU を介さないハードウェア信号処理が可能。
- **ピンあたりの駆動能力** — AVR の堅牢な I/O により、5V・20mA クラスの出力が可能です。
- **追加の UART** — 2 系統の UART シリアル通信を利用可能です。
- **RS-422/485 への対応** — USARTを使用して RS-485 通信が可能（外部の追加チップが必要）

### 留意点

- **メモリと演算性能は SAMD21 が上**（Flash 256KB 対 32KB、SRAM 32KB 対 4KB、48MHz 32-bit 対 24MHz 8-bit）。大きなバッファや重い処理、TinyML 等の用途では SAMD21 / nRF52840 系が有利です。Kunai は「小型・5V・シンプルな AVR」を求める用途向けです。
- **ADC は 10-bit**（SAMD21 は 12-bit ADC）。
- **PWM は 8-bitで6点まで、DAC なし**（SAMD21 は 全ピンで PWM 対応 + D0 に 10-bit DAC）。
- **アナログ入力 A4/A5 は存在しません**。
- **Tx / Rx LEDが無い** (ピン不足による削減)。 Serialに対する出力をボード上でモニターすることはできません。

---

## データ記憶領域

AVR32DU20 には用途の異なる複数の不揮発メモリ領域があります。
ATmega と比べて EEPROM は小さくなりましたが（256 B）、代わりに **USERROW（使用者列）** などの新しい領域が使えます。

| 領域 | 容量 | 消去単位 | 書き換え耐久 | チップ消去（再書き込み）で | 対応ライブラリ |
|------|------|----------|--------------|----------------------------|----------------|
| EEPROM | 256 B | バイト（1–32 B） | 10 万回 | 消える（EESAVE ヒューズで保持可） | `EEPROM.h` |
| USERROW | 512 B | 512 B ページ一括 | 1,000 回 | **残る** | `USERSIG.h` |
| Flash（APPDATA） | スケッチ領域の空き | 512 B ページ | 1,000 回 | 消える | `Flash.h` |
| SIGROW | 読み取り専用 | — | — | — | 工場書き込みの 16 B 個体シリアル番号を含む |

<sub>各領域の仕様・耐久回数はデータシート DS40002548A（§8 Memories/§11 NVMCTRL/電気的特性）に基づく。</sub>

---

## ピンマッピング

Seeeduino XIAO と同じ番号付けです。
ただし PWM は一部の GPIO のみ使用可能で、D6 / D7 は ADC を持ちません。

| ピン名 | MCU | ピン別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PC3 | A0 | AIN31 | ~PWM（TCB1 + LUT1）|
| D1 | PA7 | A1 | AIN27 | **SS**（SPI）/ **OUT**（AnalogComp）/ EVOUTA / CLKOUT |
| D2 | PD6 | A2 | AIN6 | **TX**（Serial2）/ ~PWM（TCB1 + LUT2）/ **SCK**（SPI）/ **OUT**（AnalogComp）|
| D3 | PD7 | A3 | AIN7 | **RX**（Serial2）/ **AREF** / EVOUTD |
| D4 | PA2 | A4 | AIN22 | **I2C SDA** / ~PWM（TCA0 WO2）/ **XCK**（Serial1）/ **CLK**（SPI1） / **IN2**（CustomLogic） |
| D5 | PA3 | A5 | AIN23 | **I2C SCL** / ~PWM（TCA0 WO3）/ **XDIR**（Serial1）/ **OUT**（CustomLogic）/ **OUT**（AnalogComp） / EVOUTA / CLKOUT |
| D6 | PA0 | — | — | **TX**（Serial1）/ ~PWM（TCA0 WO0）/ **IN0**（CustomLogic）|
| D7 | PA1 | — | — | **RX**（Serial1）/ ~PWM（TCA0 WO1）/ **IN1**（CustomLogic）|
| D8 | PA6 | A8 | AIN26 | ~PWM（TCB1 WO）/ SPI **SCK** |
| D9 | PA5 | A9 | AIN25 | ~PWM（TCA0 WO5）/ SPI **MISO** |
| D10 | PA4 | A10 | AIN24 | ~PWM（TCA0 WO4）/ SPI **MOSI**  |
| D11 | PD4 | A11 | AIN4 | **LED_BUILTIN** / **LED_BUILTIN_TX**（USB-CDCと連動）|
| D12 | PD5 | A12 | AIN5 | **LED_BUILTIN_RX**（USB-CDCと連動） |


> **D0 / D2 / D8 の PWM は択一**です。
> 1 本の TCB1 波形をいずれかのピンに出し分ける構造のため、最後に `analogWrite()` したピンが出口になります（既定は D0）。
> 3 本同時に異なる PWM は出せません。周波数・デューティは 3 ピンで共通です。
> tone などで TCB1 を使用する時は 3 つとも PWM が無効化されます。
>
> **D2 / A2 は AREFとして使用できます**。
> AREF として使用する時は他の用途には使用できません。
>
> D17,D30は物理ピンを持ちません。


---

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ（仮想 COM） |
| `Serial1` | USART0（ALT1 固定） | D7(RX) / D6(TX) | Pro Micro 互換ハードウェア UART。XCK(D4)/XDIR(D5) 付きのフル機能位置で、RS-485 の方向制御や USART-SPI ホストモードにも対応 |
| `Serial2` | USART1（ALT2 固定） | D3(RX) / D2(TX) | 予備 UART。SPI（SS/SCK）とピン共有・**排他利用** |

> 本家 Pro Micro と同じく、D0/D1 のハードウェア UART が `Serial1` です。
> `Serial0` は DxCore 内部での USART0 の名称で、`Serial1` と同一オブジェクトを指します。

### SPI（ホスト）

| 信号 | ピン |
|------|------|
| MOSI | D10（PA4） |
| MISO | D9（PA5） |
| SCK | D8（PA6） |
| SS | D1（PA7） |

SPI0 既定位置（PORTA）に配置。ボードは SPI ホストで、チップセレクトは任意の GPIO を使用してください（ハードウェア SS の PA7/D1 は SSD=1 運用のためソフトウェア CS として自由に使えます）。

> **クライアント（受信側）動作:** ハードウェア SS（D1/PA7）が実ピンにあるため、付属の **SPISlave ライブラリ**（ESP8266 互換 API）で SPI クライアントとしても動作できます。詳細は [libraries/SPISlave](../libraries/SPISlave/README.md) を参照。

### I2C（Wire）

| 信号 | ピン |
|------|------|
| SDA | D4（PA2） |
| SCL | D5（PA3） |

I2C は **TWI0 の既定位置（PA2/PA3）** そのままで XIAO の A4/A5 位置に一致します。
通常の `Wire.begin()` でそのまま使えます。

### PWM（`analogWrite()`）

- **D4, D5, D6, D7, D9, D10** … TCA0（PORTA へ割り当て、WO0–WO5 = PA0–PA5）
- **D0** … TCB1 の 8bit PWM 波形を **CCL LUT1 経由**で出力（PC3 = LUT1-OUT 既定位置）

> **自動無効化:** TCB1 が他の用途に使われている間、`analogWrite(D0)` は PWM をあきらめて単純な HIGH/LOW 出力（127 を閾値）に切り替わります。
> - `tone()` は TCB1 を使うため、実行中は D0 の PWM のみ停止します。
> - LUT1 を CCL レジスタ直接操作で使用中も同様です（CustomLogic ライブラリの Kunai 用ユニットは LUT0 なので競合しません）。

### アナログ入力

- パッドの **A0–A5・A8–A10**（A6/A7 は欠番）と、LED ピンの **A13/A14**

> ハードウェア仕様上の制約から D6 / D7 にはアナログ入力がありません。

---

## 電源

- **USB-C（5V）:** 理想ダイオードで逆流保護し、外部電源との併用時もホストを破損させません。
- **VIN 入力:** 基板上の高耐圧 LDO で 5V を生成します。ドロップアウトを考慮した**実用最低入力は約 6.5V**、上限は放熱で決まります（推奨 6.5–16V）。
- 誤って 24V の AC アダプタを接続しても破壊せず過熱保護で停止します。
- **VUSB（USB トランシーバ 3.3V）:** 基板上の LDO から供給します。
- **電圧切替:** ジャンパパッド **JP1** で VCC を 5V / 3.3V から選択します。AVR64DU32 は 1.8–5.5V の全範囲で 24 MHz 動作が可能です。

<sub>電圧選択のためには JP1 のパッドを望む電圧側とはんだ付けします</sub>

---

## LED とスイッチ

| 部品 | 接続 | 用途 |
|------|------|------|
| TX LED | D11（PD4） | USB-CDC **TX** アクティビティ表示 |
| RX LED | D12（PD5） | USB-CDC **RX** アクティビティ表示 |

Rx / Tx LED は CDC 受信の瞬間に約 100ms のパルスで点灯し、スケッチからの `digitalWrite(D11, ...)` と共存します。

> ハードウェア仕様上の制約から LED_BUILDIN（D13）は実装されていません。

---

## 書き込み

1. ボードを USB で接続します。
2. Arduino IDE からスケッチを書き込みます。書き込み開始時に **1200bps タッチ**が行われ、USB CDC ブートローダへ自動遷移します。
3. 自動遷移しない場合は、**リセットのダブルタップ**でブートローダに入れます。

初回のみ、または USB ブートローダを書き込み直す場合は、UPDI プログラマ（PICkit 4/5、Atmel-ICE、jtag2updi 等）を UPDI パッド（PF7）に接続して書き込みます。

<sub>開発用 VID/PID は pid.codes のテスト範囲（アプリ `0x1209:0x000A` / ブートローダ `0x1209:0x0009`）を使用しています。製品出荷前に正式な VID/PID へ置き換えてください。</sub>

---

## 主要部品

> Kunai の確定 BOM・回路図は現在準備中です。確定次第このページに追記します。

---

## 公式ドキュメント

データシート・エラッタは予告なく更新されます。常に最新版を参照してください（Microchip PCN システムで更新通知を受け取れます）。

- AVR32DU20 製品ページ: <https://www.microchip.com/en-us/product/AVR32DU20>
- データシート: DS40002576（AVR16/32DU ファミリ）
