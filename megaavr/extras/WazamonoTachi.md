# Wazamono 太刀（Tachi）

**Pro Micro 後継機 — AVR64DU32 / USB-C**

Wazamono Tachi は、SparkFun Pro Micro と同じフォームファクタを USB ネイティブな AVR `AVR64DU32` で再設計したボードです。
USB-シリアル変換チップを搭載せず、マイコン単体で USB-C により PC と直接つながります。

> このページは Wazamono Tachi 1 機種のドキュメントです。コア全体の概要は [README](../../README.md) を参照してください。
> **状態: 開発中。** ピン定義・ブートローダは変更される可能性があります。確定 BOM/回路図は準備中です。

---

## 概要

| 項目 | 内容 |
|------|------|
| MCU | AVR64DU32（32 ピン） |
| フォームファクタ | SparkFun Pro Micro 互換 |
| USB | USB-C（USB 2.0 Full-Speed、マイコン内蔵） |
| クロック | 24 MHz 内蔵オシレータ（USB接続時に自動調整） |
| 電源 | USB 5V、または RAW 入力（駆動電源は JP1 で 5V / 3.3V を選択） |
| 書き込み | USB CDC ブートローダ（STK500v1） |

---

## ボード諸元（AVR64DU32）

| 項目 | 値 |
|------|----|
| Flash | 64 KB（うちスケッチ用 60 KB/USB ブートローダ 4 KB） |
| SRAM | 8 KB |
| EEPROM | 256 B |
| USERROW | 512 B |
| 最大動作周波数 | 24 MHz |
| USB | USB 2.0 Full-Speed デバイス（In/Out 16 EP ずつ最大 32 EP） |
| ADC | 10-bit 170 ksps × 1（21 チャネル） |
| タイマ | TCA0 ×1（PWM 6ch）、TCB ×2 (TCB1 は通常 PWM 用) |
| USART | 2（Serial1 / Serial2） |
| SPI | 2（SPI / SPI1） |
| I2C | 1 |
| CCL（LUT） | 4 |
| イベントシステム | 3 チャネル |
| アナログコンパレータ（AC） | 1 |

<sub>諸元はデータシート DS40002548A（AVR64DU28 / 32）に基づく。スケッチ用 Flash サイズ・SRAM サイズは boards.txt の設定値。</sub>

---

## ATmega32U4 との比較

Wazamono Tachi が置き換える Pro Micro / Arduino Leonardo は **ATmega32U4** を搭載しています。
両者とも USB 内蔵 AVR ですが、AVR64DU32 は新世代の **AVRxt コア**で、クロック・メモリ・周辺機能が大きく強化されています。

| 項目 | Wazamono Tachi (AVR64DU32) | Pro Micro 等 (ATmega32U4) |
|------|----------------------------|----------------------------|
| コア | AVRxt（命令タイミング改善） | 旧来 AVR |
| 最大クロック | 24 MHz（1.8–5.5V 全域） | 16 MHz（4.5V 以上）/3.3V では通常 8 MHz |
| 動作電圧 | 5V / 3.3V（部品変更不要） | 5V / 3.3V（部品変更が必要） |
| Flash | 64 KB | 32 KB |
| SRAM | 8 KB | 2.5 KB |
| EEPROM | 256 B | 1 KB |
| USERROW | 512 B | - |
| ADC | 10-bit・21 ch・170 ksps | 10-bit・12 ch |
| タイマ | 16-bit TCA ×1 + TCB ×2 | 8/16/16/10-bit ×4 |
| USART | 2 | 1 |
| SPI | 2（1つはホスト限定） | 2 |
| I2C | 1 | 1 |
| CCL（LUT） | 4 | なし |
| イベントシステム | 3 ch | なし |
| アナログコンパレータ（AC） | 1 | なし |

### 性能上の主な利点

- **クロックと処理速度** — 24 MHz 動作（ATmega32U4は 16 MHz）に加え、AVRxt コアは一部命令のタイミングが改善されており、同一クロックでもわずかに高速です。
- **複数電圧への対応** — AVRxtのコア特性を活かして 5V または 3.3V の切り替えを**周辺部品の変更なしに可能**です。
- **メモリ** — Flash 2 倍（64 KB）、SRAM 約 3.2 倍（8 KB）。大きなバッファ、USB 複合デバイス、ライブラリを多用するスケッチで余裕が生まれます。
- **新世代の周辺機能** — CCL（4 論理ブロック）とイベントシステム（3 チャネル）により、CPU を介さないハードウェア信号処理が可能。
- **アナログ入力** — ADC チャネルが 12 -> 21 に増加し全チャネルがアナログ入力に対応します。
- **ピンあたりの駆動能力** — AVR の堅牢な I/O により、5V・20mA クラスの出力が可能です。
- **追加の UART** — 2 系統の UART シリアル通信を利用可能です。
- **RS-422/485 への対応** — USARTを使用して RS-485 通信が可能（外部の追加チップが必要）

### 留意点

- **EEPROM 容量は ATmega32U4 のほうが大きい**（1 KB 対 256 B）。
- 多くの不揮発データを保存する用途では保存方法の見直し（User Row やフラッシュの活用）が必要になる場合があります（後述「データ記憶領域」参照）。

---

## データ記憶領域

AVR64DU32 には用途の異なる複数の不揮発メモリ領域があります。
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

論理ピン番号（`D#`）と MCU ピン、機能の対応です（rev.4 / AVR64DU32）。Pro Micro と同様に D11–D13・D22–D29 が欠番になります（本家の D11–D12 に加え、D13 の専用 LED は設けません）。

| D# | MCU | アナログ別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PA5 | A12 | AIN25 | **Serial1 RX**（USART0 ALT1） |
| D1 | PA4 | A13 | AIN24 | **Serial1 TX**（USART0 ALT1） |
| D2 | PA2 | A14 | AIN22 | **I2C SDA** |
| D3 | PA3 | A15 | AIN23 | **I2C SCL**/~PWM(TCB1) |
| D4 | PF4 | A6 | AIN20 | 汎用 I/O |
| D5 | PD0 | A16 | AIN0 | ~PWM(TCA0 WO0)/CCL(LUT2-IN0) |
| D6 | PD1 | A7 | AIN1 | ~PWM(TCA0 WO1)/CCL(LUT2-IN1) |
| D7 | PA6 | A17 | AIN26 | USART0 XCK/CCL(LUT0-OUT 代替) |
| D8 | PA7 | A8 | AIN27 | USART0 XDIR/AC0 出力/EVOUTA/CLKOUT |
| D9 | PD2 | A9 | AIN2 | ~PWM(TCA0 WO2)/CCL(LUT2-IN2)/AC0 AINP0/EVOUTD |
| D10 | PD3 | A10 | AIN3 | ~PWM(TCA0 WO3)/CCL(LUT2-OUT)/AC0 AINN0 |
| D14 | PD5 | A18 | AIN5 | SPI **MISO**/~PWM(TCA0 WO5) |
| D15 | PD6 | A19 | AIN6 | SPI **SCK**/Serial2 TX（USART1 ALT2） |
| D16 | PD4 | A20 | AIN4 | SPI **MOSI**/~PWM(TCA0 WO4) |
| D17 | PF3 | A21 | AIN19 | **RX LED（= LED_BUILTIN）**（Active-LOW、ヘッダなし）/CCL(LUT3-OUT) |
| D18 | PD7 | **A0** | AIN7 | アナログ入力 A0/SPI **SS**/Serial2 RX（USART1 ALT2）/VREFA |
| D19 | PF0 | **A1** | AIN16 | アナログ入力 A1/CCL(LUT3-IN0) |
| D20 | PF1 | **A2** | AIN17 | アナログ入力 A2/CCL(LUT3-IN1) |
| D21 | PF2 | **A3** | AIN18 | アナログ入力 A3/CCL(LUT3-IN2)/EVOUTF |
| D30 | PC3 | A34 | AIN31 | **TX LED（= LED_BUILTIN_TX）**（Active-LOW、ヘッダなし）/CCL(LUT1-OUT) |

**基板上未接続（予約）:** PA0・PA1・PF5（AIN21 は使用不可）　**ヘッダに出ない内部ピン:** PF6（RESET）/PF7（UPDI）

> ADC を持つ各デジタルピンは A12–A21・A34 としても参照できます（A4/A5/A11 は欠番）。D18（A0）は VREFA を兼ねるため、外部アナログ基準電圧を使う場合は A0/SS/Serial2 RX が使えなくなります。

---

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ（仮想 COM） |
| `Serial1` | USART0（ALT1 固定） | D0(RX) / D1(TX) | Pro Micro 互換ハードウェア UART。XCK(D7)/XDIR(D8) 付きのフル機能位置で、RS-485 の方向制御や USART-SPI ホストモードにも対応 |
| `Serial2` | USART1（ALT2 固定） | D18(A0) / D15(TX) | 予備 UART。SPI（SS/SCK）とピン共有・**排他利用** |

> 本家 Pro Micro と同じく、D0/D1 のハードウェア UART が `Serial1` です。
> `Serial0` は DxCore 内部での USART0 の名称で、`Serial1` と同一オブジェクトを指します。

### SPI

| オブジェクト | SPI | SPI1 |
| 信号 | ピン（クライアント可）| ピン（ホストのみ）|
|------|------|------|
| MOSI | D16（PD4） | D0（PA5） |
| MISO | D14（PD5） | D1（PA4） |
| SCK | D15（PD6） | D7（PA6） |
| SS | A0（PD7） | なし |

チップセレクトは任意の GPIO を使用してください（SPI0 ALT4 位置。SCK/SS は `Serial2` と共用・排他利用）。

> **クライアント（受信側）動作:** ハードウェア SS（A0/PD7）が実ピンにあるため、付属の **SPISlave ライブラリ**（ESP8266 互換 API）で SPI クライアントとしても動作できます。
> 詳細は [libraries/SPISlave](../libraries/SPISlave/README.md) を参照。

### I2C（Wire）

| 信号 | ピン |
|------|------|
| SDA | D2（PA2） |
| SCL | D3（PA3） |

Pro Micro と同じ D2/D3 に配置されています。
通常の `Wire.begin()` でそのまま使えます。

### PWM（`analogWrite()`）

- **D5/D6/D9/D10/D14/D16** … TCA0（PORTD へ割り当て、WO0–WO5）
- **D3** … TCB1
- `millis()` / `micros()` は **TCB0** を使用するため、TCB1（D3）と TCA0 は PWM に使用できます。
- 本家 Pro Micro の PWM ピン（D3/D5/D6/D9/D10）を完全再現。D14/D16 は SPI 使用時には PWM に使えません。

> **自動無効化:** TCB1 が他の用途に使われている間、`analogWrite(D3)` は PWM をあきらめて単純な HIGH/LOW 出力（127 を閾値）に切り替わります。
> - `tone()` は TCB1 を使うため、実行中は D3 の PWM のみ停止します。
> これは Pro Micro の Timer2（`tone()` 実行中は D3/D11 が停止）に相当する挙動です。

### アナログ入力

- シルク表記の **A0–A3**（= D18–D21）
- 各デジタルピンも ADC チャネルを持ち、A6–A21 として参照可能

--

## クロック

Tachi は **水晶を搭載しない**設計で、システムクロックは内蔵 OSCHF の **24 MHz 固定**です。
USB 用の 48 MHz（CLK_USB）は内蔵 PLL48M が生成し、USB の SOF に同期して自動調整されるため水晶なしでも USB は仕様通りに機能します。

> USB ホスト切断時は動作クロックの精度が内蔵オシレータ単体の精度になります。

---

## 電源

- **USB-C（5V）:** 理想ダイオードで逆流保護し、外部電源との併用時もホストを破損させません。
- **RAW 入力:** 基板上の高耐圧 LDO で 5V を生成します。ドロップアウトを考慮した**実用最低入力は約 6.5V**、上限は放熱で決まります（推奨 6.5–16V）。
- 誤って 24V の AC アダプタを接続しても破壊せず過熱保護で停止します。
- **VUSB（USB トランシーバ 3.3V）:** 基板上の LDO から供給します。
- **電圧切替:** ジャンパパッド **JP1** で VCC を 5V / 3.3V から選択します。AVR64DU32 は 1.8–5.5V の全範囲で 24 MHz 動作が可能です。

<sub>電圧選択のためには JP1 のパッドを望む電圧側とはんだ付けします</sub>

---

## LED とスイッチ

| 部品 | 色 | 接続 | 用途 |
|------|----|----|------|
| 電源 LED | 黄緑 | 電源ライン | 通電表示 |
| RX_LED | 橙 | D17（PF3、Active-LOW） | ユーザー LED 兼 USB-CDC **RX** アクティビティ表示 |
| TX_LED | 橙 | D30（PC3、Active-LOW） | ユーザー LED 兼 USB-CDC **TX** アクティビティ表示 |

Rx / Tx LED は CDC 受信の瞬間に約 100ms のパルスで点灯し、スケッチからの `digitalWrite(D17, ...)` と共存します。

--

## 書き込み

1. ボードを USB で接続します。
2. Arduino IDE からスケッチを書き込みます。書き込み開始時に **1200bps タッチ**が行われ、USB CDC ブートローダへ自動遷移します。
3. 自動遷移しない場合は、**リセットボタンのダブルタップ**でブートローダに入れます。

初回のみ、または USB ブートローダを書き込み直す場合は、UPDI プログラマ（PICkit 4/5、Atmel-ICE、jtag2updi 等）を UPDI パッドに接続して書き込みます。

<sub>開発用 VID/PID は pid.codes のテスト範囲（アプリ `0x1209:0x0006` / ブートローダ `0x1209:0x0005`）を使用しています。製品出荷前に正式な VID/PID へ置き換えてください。</sub>

---

## ソフトウェア互換性（Pro Micro）

Tachi は Pro Micro からの移植の手間を最小化することを目指しています。
基本的には旧 megaAVR とほぼ同一の命令を持ちます。

> レジスタ構成は大幅に変化しているため、レジスタを直接操作するプログラムの移植は難易度が高くなります。

---

## 主要部品

> Tachi の確定 BOM・回路図は現在準備中です。確定次第このページに追記します。

---

## 公式ドキュメント

データシート・エラッタは予告なく更新されます。常に最新版を参照してください（Microchip PCN システムで更新通知を受け取れます）。

- AVR64DU32 製品ページ: <https://www.microchip.com/en-us/product/AVR64DU32>
- データシート: DS40002548A（AVR64DU28/32）
