# Wazamono 苦無（Kunai）

**Seeeduino XIAO 後継機 — AVR32DU20 / USB-C**

Wazamono Kunai は、Seeed の XIAO と同じ超小型フォームファクタ（21 × 17.8 mm）を、USB ネイティブな新世代 AVR `AVR32DU20` で再設計したボードです。
USB-シリアル変換チップを搭載せず、マイコン単体で USB-C により PC と直接つながります。XIAO シリーズの定番である SAMD21 版（Seeeduino XIAO）を **5V 動作の AVR** で置き換えることを狙ったボードです。

> このページは Wazamono Kunai 1 機種のドキュメントです。コア全体の概要は [README](../../README.md) を参照してください。
> **状態: 開発中。** ピン定義・ブートローダは変更される可能性があります。確定 BOM/回路図は準備中です。

---

## 概要

| 項目 | 内容 |
|------|------|
| 由来 | Seeeduino XIAO（SAMD21 版）後継 |
| MCU | AVR32DU20（VQFN-20） |
| フォームファクタ | XIAO 互換（21 × 17.8 mm、両側 7 パッド + 裏面パッド） |
| USB | USB-C（USB 2.0 Full-Speed、マイコン内蔵） |
| クロック | 24 MHz 内蔵オシレータ **固定**（水晶なし、クロックメニューなし） |
| 電源 | USB 5V（VUSB は内蔵 3.3V レギュレータ） |
| 書き込み | USB CDC ブートローダ（STK500v1、1200bps タッチ） |

---

## ボード諸元（AVR32DU20）

| 項目 | 値 |
|------|----|
| Flash | 32 KB（うちスケッチ用 28 KB/USB ブートローダ 4 KB） |
| SRAM | 4 KB |
| EEPROM | 256 B |
| USERROW | 512 B |
| 最大動作周波数 | 24 MHz |
| USB | USB 2.0 Full-Speed デバイス |
| ADC | 10-bit 170 ksps × 1（チャネル数は 20 ピンパッケージにより制限） |
| DAC | なし（AC 内部の DACREF のみ） |
| タイマ | TCA0 ×1（PWM 6ch）、TCB ×2 / TCD は USB が占有のため非搭載 |
| USART | 2（USART0 / USART1） |
| SPI / TWI(I2C) | 各 1 |
| CCL（LUT）/イベントシステム/AC | AVR64DU32 と同じ周辺ブロックを搭載 |
| パッケージ | 20 ピン（VQFN 3×3） |

<sub>諸元は AVR16/32DU ファミリデータシート（DS40002576）に基づく。
周辺機能ブロックは AVR64DU32（DS40002548A）と共通だが、20 ピンのため外部に出せるピン数が少なく、同時に使える機能が制限されます。</sub>

---

## SAMD21（Seeeduino XIAO）との比較

Wazamono Kunai が置き換える Seeeduino XIAO は **ATSAMD21G18**（ARM Cortex-M0+、3.3V 動作）を搭載しています。
AVR32DU20 は **8-bit の AVRxt コア**で、生の演算性能やメモリ容量では SAMD21 に及びませんが、**5V ネイティブ動作** と **高い出力電流値** ならではの周辺機能と扱いやすさで差別化します。
また同一基板上で 5V と 3.3V の動作電圧切り替えが可能です。

| 項目 | Wazamono Kunai (AVR32DU20) | Seeeduino XIAO (SAMD21G18) |
|------|----------------------------|----------------------------|
| コア | 8-bit AVRxt | 32-bit ARM Cortex-M0+ |
| 最大クロック | 24 MHz | 48 MHz |
| **動作電圧** | **5V**（1.8–5.5V） | 3.3V のみ（5V 非トレラント） |
| Flash | 32 KB | 256 KB |
| SRAM | 4 KB | 32 KB |
| EEPROM | 256 B | なし（フラッシュエミュレーション） |
| ADC | 10-bit | 12-bit |
| DAC | なし | 10-bit ×1 |
| USB | Full-Speed デバイス（内蔵） | Full-Speed デバイス（内蔵） |
| シリアル系統 | USART ×2 / SPI ×1 / I2C ×1 | SERCOM ×6（用途を割当） |
| CCL（論理ブロック）/イベントシステム | あり | なし |

### Kunai を選ぶ理由

- **5V ネイティブ動作** — XIAO の最大の制約だった「3.3V のみ」を解消。5V ロジックのセンサ・モジュール・リレー等をレベル変換なしで直接駆動できます。AVR32DU20 は 1.8–5.5V の全域で 24 MHz 動作が可能です。
- **真の EEPROM** — 256 B の独立した EEPROM を搭載。SAMD21 はフラッシュエミュレーションのため、設定値の保存が手軽です。
- **AVR / Arduino-AVR エコシステム** — 古典 AVR 向けの豊富なライブラリ・作例がそのまま、あるいは小修正で動きます。`<avr/io.h>` レベルの低レベル制御も馴染みやすい構成です。
- **CCL / イベントシステム** — CPU を介さないハードウェアレベルの論理演算・信号ルーティングが可能。SAMD21 にはない機能です。
- **ピンあたりの駆動能力** — AVR の堅牢な I/O により、5V・20mA クラスの出力が可能です。

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

XIAO と同じパッド配置（D0–D10）に加え、オンボード LED を D13/D14 に割り当てています。各パッドは `A#` のアナログ別名も持ちます（ADC 入力のない D6/D7 を除く）。

| D# | MCU | アナログ別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PC3 | **A0** | AIN31 | **~PWM（TCB1 + CCL LUT1）** |
| D1 | PA7 | **A1** | AIN27 | SPI **SS** / AC0 出力 / EVOUTA / CLKOUT |
| D2 | PD6 | **A2** | AIN6 | **Serial2 TX**（USART1 ALT2）/ CCL LUT2-OUT |
| D3 | PD7 | **A3** | AIN7 | **Serial2 RX**（USART1 ALT2）/ EVOUTD |
| D4 | PA2 | **A4** | AIN22 | **I2C SDA** / ~PWM(TCA0 WO2) / Serial1 XCK / CCL LUT0-IN2 |
| D5 | PA3 | **A5** | AIN23 | **I2C SCL** / ~PWM(TCA0 WO3) / Serial1 XDIR / CCL LUT0-OUT |
| D6 | PA0 | — | — | **Serial1 TX**（USART0 既定位置）/ ~PWM(TCA0 WO0) / CCL LUT0-IN0 |
| D7 | PA1 | — | — | **Serial1 RX**（USART0 既定位置）/ ~PWM(TCA0 WO1) / CCL LUT0-IN1 |
| D8 | PA6 | **A8** | AIN26 | SPI **SCK** |
| D9 | PA5 | **A9** | AIN25 | SPI **MISO** / ~PWM(TCA0 WO5) |
| D10 | PA4 | **A10** | AIN24 | SPI **MOSI** / ~PWM(TCA0 WO4) |
| D13 | PD4 | **A13** | AIN4 | **LED_BUILTIN** / USB-CDC **RX** アクティビティ LED |
| D14 | PD5 | **A14** | AIN5 | USB-CDC **TX** アクティビティ LED |

**パッドに出ない内部ピン:** PF6（RESET）/PF7（UPDI）

> `~` は PWM 出力可能ピンを示します。XIAO は通常全てのピンで PWM を実行可能ですが Kunai では機能が限定されます。
> XIAO では通常 A0–A10 がすべて存在しますが、Kunai では **A6/A7 が欠番**です（D6=PA0・D7=PA1 に ADC 入力がないため）。

---

## ペリフェラル割り当て

variant 側でピン割り当てが確定済みのため、スケッチで `swap()` を指定する必要はありません。

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ（仮想 COM） |
| `Serial1` | USART0（既定位置） | D6(TX) / D7(RX) | XIAO の TX/RX パッド相当のハードウェア UART（Uno R4 と同じ命名） |
| `Serial2` | USART1（ALT2 固定） | D2(TX) / D3(RX) | 予備 UART |

> `Serial1` の実体は USART0 です（`Serial0` でも同じポートに届きます）。水晶を持たない Kunai では PA0/PA1 が空くため、USART0 をチップ既定位置のまま XIAO の TX/RX パッドに配置できています。
> USART0 は多機能で、2つ目の SPI や RS-485 を使用する際の XDIR 信号（D5）・XCK（D4）を持ちます。

### SPI（ホスト）

| 信号 | ピン |
|------|------|
| MOSI | D10（PA4） |
| MISO | D9（PA5） |
| SCK | D8（PA6） |
| SS | D1（PA7） |

SPI0 既定位置（PORTA）に配置。ボードは SPI ホストで、チップセレクトは任意の GPIO を使用してください（ハードウェア SS の PA7/D1 は SSD=1 運用のためソフトウェア CS として自由に使えます）。

### I2C（Wire）

| 信号 | ピン |
|------|------|
| SDA | D4（PA2） |
| SCL | D5（PA3） |

I2C は **TWI0 の既定位置（PA2/PA3）** そのままで XIAO の A4/A5 位置に一致します。PORTMUX の変更は不要で、**通常の `Wire.begin()` でそのまま D4/D5 が SDA/SCL になります**。

### PWM（`analogWrite()`）

- **D4, D5, D6, D7, D9, D10** … TCA0（PORTA へ割り当て、WO0–WO5 = PA0–PA5）
- **D0** … TCB1 の 8bit PWM 波形を **CCL LUT1 経由**で出力（PC3 = LUT1-OUT）

> **D0 PWM の仕組み:** D0（PC3）は TCB の WO ピンではなく **LUT1 の出力ピン**です。`analogWrite(D0, x)` は空いている TCB1 を 8bit PWM モードで走らせ、その内部波形を CCL LUT1 の内部入力（INSEL1 = TCB）としてスルー出力します（コア機構 `wazamono_lutpwm`）。本物のハードウェア PWM であり CPU 負荷はありません。TCB1 自身の WO ピン位置は ALT1（PF5）へ退避しますが、**PF5 は 20 ピン品に存在しない**ため完全に無害です。
>
> **自動無効化:** TCB1 または LUT1 が他の用途に使われている間、`analogWrite(D0)` は PWM をあきらめて単純な HIGH/LOW 出力（127 を閾値）に切り替わります。`tone()` は TCB1 を使うため、実行中は D0 の PWM のみ停止します（TCA0 の PWM と millis は動作継続）。LUT1 を CCL レジスタ直接操作で使用中も同様です（CustomLogic ライブラリの Kunai 用ユニットは LUT0 なので競合しません）。
- `millis()` / `micros()` は **TCB0**。
- `tone()` と `Servo` はどちらも TCB1 を使うため排他です（実行中は D0 PWM も停止）。

> PWM 非対応: D1(PA7)・D2(PD6)・D3(PD7)・D8(PA6)・D13(PD4)・D14(PD5)。

### アナログ入力

- パッドの **A0–A5・A8–A10**（A6/A7 は欠番）と、LED ピンの **A13/A14**

> アナログ入力非対応: D6(PA0)・D7(PA1)。

---

## クロック

USB 用の 48 MHz（CLK_USB）は内蔵 PLL48M が生成し、USB の SOF に同期して自動調整されます。
**システムクロックの選択とは独立**しているため、内蔵オシレータ動作でも USB は機能します。

Kunai は **水晶を搭載しない**設計で、システムクロックは内蔵 OSCHF の **24 MHz 固定**です。
水晶用の PA0/PA1 は GPIO（D6/D7 = Serial1 TX/RX）として使用されています。

> USB ホスト切断時は動作クロックが安定しない場合があります。

---

## 電源

- **入力:** USB-C（5V）。
- **VUSB（USB トランシーバ 3.3V）:** AVR DU の**内蔵 USB レギュレータ**から供給します（boards.txt で `-DUSB_VREG_INTERNAL` を指定）。
- **VBUS 検出:** なし（PC3 は D0 として使用。USB 接続状態はソフトウェアでは USB フレームの有無で判断します）。
- AVR32DU20 は 1.8–5.5V の全範囲で 24 MHz 動作が可能です。

> Kunai の確定 BOM・回路図は現在準備中です。

---

## LED とスイッチ

| 部品 | 接続 | 用途 |
|------|------|------|
| LED_BUILTIN | D13（PD4） | オンボード LED（USB-CDC **RX** アクティビティ表示を兼用） |
| TX LED | D14（PD5） | USB-CDC **TX** アクティビティ表示 |
| リセット | RESET（PF6） | リセット入力 |

`LED_BUILTIN` は **D13（PD4）** です（XIAO 同様、ヘッダ/パッドには出ていません）。
アクティブ LOW なので D13 が LOW に設定された時に点灯します。

XIAO と同様、D13 の LED は USB CDC（`Serial`）の **受信アクティビティ表示**を兼ね、D14 の LED が**送信アクティビティ**を表示します（Pro Micro の RX/TX LED 相当。実装は [wazamono_kunai_init.cpp](../variants/WazamonoKunai/wazamono_kunai_init.cpp) の weak フック上書き）。スケッチから `digitalWrite(D13, ...)` した場合は、CDC 通信の瞬間だけ短いパルスが重なります。

---

## 書き込み

1. ボードを USB で接続します。
2. Arduino IDE からスケッチを書き込みます。書き込み開始時に **1200bps タッチ**が行われ、USB CDC ブートローダへ自動遷移します。
3. 自動遷移しない場合は、**リセットのダブルタップ**でブートローダに入れます。

初回のみ、または USB ブートローダを書き込み直す場合は、UPDI プログラマ（PICkit 4/5、Atmel-ICE、jtag2updi 等）を UPDI パッド（PF7）に接続して書き込みます。

<sub>開発用 VID/PID は pid.codes のテスト範囲（アプリ `0x1209:0x000A` / ブートローダ `0x1209:0x0009`）を使用しています。製品出荷前に正式な VID/PID へ置き換えてください。</sub>

> **ブートローダ hex について:** Kunai 用のブートローダ hex（`usbcdcboot_wazamonokunai.hex`）はボード固有（MCU が AVR32DU20、VID/PID・LED ピンが他機種と異なる）のため別途ビルドが必要です。`megaavr/bootloaders/usbcdcboot/` で `build_wazamono.bat`（または `.sh`）を実行すると Tachi/Tsurugi とともに生成されます。

---

## 主要部品

> Kunai の確定 BOM・回路図は現在準備中です。確定次第このページに追記します。
> 中心部品は **AVR32DU20（VQFN-20）** で、USB-C コネクタ・USB D+/D- の ESD/TVS 保護・電源デカップリングを基本構成とします。

---

## 公式ドキュメント

データシート・エラッタは予告なく更新されます。常に最新版を参照してください（Microchip PCN システムで更新通知を受け取れます）。

- AVR32DU20 製品ページ: <https://www.microchip.com/en-us/product/AVR32DU20>
- データシート: DS40002576（AVR16/32DU ファミリ）
