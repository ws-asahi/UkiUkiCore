# Wazamono 剣（Tsurugi）

**Arduino Uno R3 後継機 — AVR64DU32 / USB-C**

Wazamono Tsurugi は、Arduino Uno R3 と同じピン配置・フォームファクタを USB ネイティブな AVR `AVR64DU32` で再設計したボードです。
Uno R3 が別チップ（USB-シリアル変換）を必要としたのに対し、Tsurugi はマイコン単体で USB-C により PC と直接つながります。
さらに、産業用途を想定した **DC ジャック（最大 24V）入力＋同期バックコンバータ**を搭載しています。

> このページは Wazamono Tsurugi 1 機種のドキュメントです。コア全体の概要は [README](../../README.md) を参照してください。
> **状態: 開発中。** ピン定義・ブートローダは変更される可能性があります。

---

## 概要

| 項目 | 内容 |
|------|------|
| MCU | AVR64DU32（32 ピン） |
| フォームファクタ | Uno R3 互換 |
| USB | USB-C（USB 2.0 Full-Speed、マイコン内蔵） |
| クロック | 24 MHz 水晶 |
| 電源 | USB 5V / DC ジャック（最大 24V、同期バックで 5V 生成）|
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

<sub>諸元はデータシート DS40002548A（AVR64DU32）に基づく。スケッチ用 Flash サイズ・SRAM サイズは boards.txt の設定値。</sub>

---

## ATmega328P（Arduino Uno R3）との比較

Wazamono Tsurugi が置き換える Arduino Uno R3 は **ATmega328P**（8-bit AVR、ネイティブ USB なし）を搭載しています。
AVR64DU32 は新世代の **AVRxt コア**で、USB 内蔵・クロック・メモリ・周辺機能のすべてが強化されています。

| 項目 | Wazamono Tsurugi (AVR64DU32) | Arduino Uno R3 (ATmega328P) |
|------|------------------------------|------------------------------|
| コア | AVRxt（命令タイミング改善） | 旧来 AVR |
| 最大クロック | 24 MHz（1.8–5.5V 全域） | 20 MHz（4.5V 以上）/Uno は 16 MHz |
| USB | マイコン内蔵（変換チップ不要） | なし（基板上に別の USB チップが必要） |
| Flash | 64 KB | 32 KB |
| SRAM | 8 KB | 2 KB |
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

- **USB がマイコン内蔵** — Uno R3 は USB-シリアル変換用の別チップを基板に載せていましたが、Tsurugi は不要。
- `Serial` がそのまま USB CDC 仮想シリアルとなり、USB HID（キーボード/マウス）や USB-MIDI にもなれます。
- **クロックと処理速度** — 24 MHz 動作（Uno の 16 MHz 比で 1.5 倍）に加え、AVRxt コアは一部命令のタイミングが改善されています。
- **メモリ** — Flash 2 倍（64 KB）、SRAM 4 倍（8 KB）。
- **新世代の周辺機能** — CCL（4 論理ブロック）とイベントシステム（3 チャネル）により、CPU を介さないハードウェア信号処理が可能。
- **アナログ入力** — ADC チャネルが 12 -> 21 に増加し全チャネルがアナログ入力に対応します。
- **追加の UART** — 2 系統の UART シリアル通信を利用可能です。
- **RS-422/485 への対応** — USARTを使用して RS-485 通信が可能（外部の追加チップが必要）
- **広い入力電源範囲** — DC ジャックから最大 24V を入力でき、基板上の同期バックで 5V を生成します（Uno R3 のリニアレギュレータより高効率）。
- タイマの構成上ServoやTone使用時にはTCB1を使用し、そのためD3ピンのPWMが無効化されます（Uno R3ではD3とD11が無効）。

### 留意点

- **EEPROM 容量は ATmega328P のほうが大きい**（1 KB 対 256 B）。
- 多くの不揮発データを保存する用途では保存方法の見直し（User Row やフラッシュの活用）が必要になる場合があります（後述「データ記憶領域」参照）。

- **Tx / Rx LEDが無い** (ピン不足による削減)。 Serialに対する出力をボード上でモニターすることはできません。

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

Arduino Uno R3 と同じ番号付け（D0–D13、A0–A5）です。A0–A5 はデジタル D14–D19 を兼ねます。

| D# | MCU | アナログ別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PA5 | A6 | AIN25 | **RX**（Serial1 / USART0 ALT1） |
| D1 | PA4 | A7 | AIN24 | **TX**（Serial1 / USART0 ALT1） |
| D2 | PA7 | A8 | AIN27 | USART0 XDIR / AC0 OUT / EVOUTA / CLKOUT |
| D3 | PA6 | A9 | AIN26 | ~PWM(**TCB1 + CCL LUT0**) / USART0 XCK / tone() と共用 |
| D4 | PF4 | A10 | AIN20 | 汎用 I/O（PWM なし。TCB0 は millis） |
| D5 | PD0 | A11 | AIN0 | ~PWM(TCA0 WO0) / CCL |
| D6 | PD1 | A12 | AIN1 | ~PWM(TCA0 WO1) / CCL |
| D7 | PC3 | A13 | AIN31 | 汎用 I/O（実測で VDD 駆動を確認） |
| D8 | PF5 | A14 | AIN21 | 汎用 I/O（TCB1 WO 駐機位置 ※D3 PWM の注記参照） |
| D9 | PD2 | A15 | AIN2 | ~PWM(TCA0 WO2) / CCL / AC0 AINP0 / EVOUTD |
| D10 | PD3 | A16 | AIN3 | ~PWM(TCA0 WO3) / CCL / AC0 AINN0 / **SS** |
| D11 | PD4 | A17 | AIN4 | ~PWM(TCA0 WO4) / SPI **MOSI** |
| D12 | PD5 | A18 | AIN5 | ~PWM(TCA0 WO5) / SPI **MISO** |
| D13 | PD6 | A19 | AIN6 | SPI **SCK** / **LED_BUILTIN**
| D14 / A0 | PF0 | A0 | AIN16 | アナログ A0 / CCL |
| D15 / A1 | PF1 | A1 | AIN17 | アナログ A1 / CCL |
| D16 / A2 | PF2 | A2 | AIN18 | アナログ A2 / CCL / EVOUTF |
| D17 / A3 | PF3 | A3 | AIN19 | アナログ A3 / CCL |
| D18 / A4 | PA2 | A4 | AIN22 | アナログ A4 / **SDA**（I2C） |
| D19 / A5 | PA3 | A5 | AIN23 | アナログ A5 / **SCL**（I2C） |

> 各デジタルピンは ADC チャネルを持つため、A6–A19 としても参照できます。

---

## ペリフェラル割り当て

variant 側でピン割り当てが確定済みのため、スケッチで `swap()` を指定する必要はありません。

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ（仮想 COM）。日常の `Serial.print()` は Uno と同じ感覚で使えます |
| `Serial1` | USART0（ALT1 固定） | D0(RX) / D1(TX) | Pro Micro 互換ハードウェア UART。XCK(D7)/XDIR(D8) 付きのフル機能位置で、RS-485 の方向制御や USART-SPI ホストモードにも対応 |

> 本家 Pro Micro と同じく、D0/D1 のハードウェア UART が `Serial1` です。
> `Serial0` は DxCore 内部での USART0 の名称で、`Serial1` と同一オブジェクトを指します。

### SPI

| オブジェクト | SPI | SPI1 |
| 信号 | ピン(ホストのみ) | ピン(ホストのみ) |
|------|------|------|
| MOSI | D11（PD4） | D0（PA5） |
| MISO | D12（PD5） | D1（PA4） |
| SCK | D13（PD6） | D3（PA6） |
| SS | なし | なし |

### I2C（Wire）

| 信号 | ピン |
|------|------|
| SDA | A4（PA2） |
| SCL | A5（PA3） |

Uno R3 と同じ A4/A5 に配置されています。
通常の `Wire.begin()` でそのまま使えます。

### PWM（`analogWrite()`）

- **D5, D6, D9, D10, D11, D12** … TCA0（PORTD へ割り当て、WO0–WO5）
- **D3** … TCB1 の 8bit PWM 波形を **CCL LUT0 経由**で出力（PA6 = LUT0-OUT 代替位置）
- Uno R3に対してD12へPWM機能が追加されています。

> **自動無効化:** TCB1 が他の用途に使われている間、`analogWrite(D3)` は PWM をあきらめて単純な HIGH/LOW 出力（127 を閾値）に切り替わります。
> - `tone()` は TCB1 を使うため、実行中は D3 の PWM のみ停止します。
> - LUT0 を CCL レジスタ直接操作で使用中も同様に D3 の PWM が無効化されます（設定を上書きすることはありません）。
> これは Uno R3 の Timer2（`tone()` 実行中は D3/D11 が停止）に相当する挙動です。

### アナログ入力

- Uno R3 ヘッダの **A0–A5**（= D14–D19）
- 各デジタルピンも ADC チャネルを持ち、A6–A19 として参照可能

---

## クロック

Tsurugi は **外付けの水晶を搭載**した設計で、システムクロックは **24 MHz 固定**です。
USB 用の 48 MHz（CLK_USB）は内蔵 PLL48M が生成し USB の SOF に同期して自動調整されます。

### クロック出力（CLKOUT）

メインクロック（CLK_PER）を **D2（PA7）** へ出力できます。外部 IC へのクロック供給、他 MCU との同期、実クロックの測定に使えます。付属の **ClockOut ライブラリ**で `ClockOut.begin()` / `ClockOut.end()` により開閉します（詳細は [libraries/ClockOut](../libraries/ClockOut/README.md)）。

> 24MHz の連続矩形波は EMI 源になるため、必要な期間だけ有効化する運用を推奨します。PA7 は AC0 出力・EVOUTA と共用のため、それらが使用中は `begin()` が `false` を返します。

---

## 電源

Tsurugi は **2 系統の電源入力**を持ち、いずれからでも 5V を得られます。

- **USB-C（5V）:** 理想ダイオードで逆流保護し、ホストを破損させずに 5V を供給します。
- **DC ジャック（最大 24V）:** φ5.5/2.1mm の DC ジャック（J10）から入力し、ショットキーで逆接続保護後、**同期バックコンバータ**で 5Vを生成します。
- EN 分圧（100k/15k）により起動電圧は約 9.2V です。入力側 TVS で過電圧から保護します。
- **3.3V（シールドピン用）:** 基板上の LDO。
- **VUSB（USB トランシーバ 3.3V）:** 上記の外部 3.3V LDO から供給します。

---

## LED とスイッチ

| 部品 | 接続 | 用途 |
|------|------|------|
| 電源 LED | 電源ライン（D3/黄緑） | 通電表示 |
| LED_BUILTIN | D13（PD6）→ MOSFET（D4/橙） | オンボード LED（Uno R3 慣例） |
| リセット | RESET（PF6） | タクトスイッチ |

`LED_BUILTIN` は **D13（PD6）** です。D13 は SPI SCK と共用のため、Uno R3 と同様に SPI 使用中は LED が SCK のトラフィックで点滅します。

---

## 書き込み

1. ボードを USB で接続します。
2. Arduino IDE からスケッチを書き込みます。書き込み開始時に **1200bps タッチ**が行われ、USB CDC ブートローダへ自動遷移します。
3. 自動遷移しない場合は、**リセットボタンのダブルタップ**でブートローダに入れます。

初回のみ、または USB ブートローダを書き込み直す場合は、UPDI プログラマ（PICkit 4/5、Atmel-ICE、jtag2updi 等）を UPDI パッド（PF7）に接続して書き込みます。

<sub>開発用 VID/PID は pid.codes のテスト範囲（アプリ `0x1209:0x0008` / ブートローダ `0x1209:0x0007`）を使用しています。製品出荷前に正式な VID/PID へ置き換えてください。</sub>

> **ブートローダ hex について:** Tsurugi 用のブートローダ hex（`usbcdcboot_wazamonotsurugi.hex`）はボード固有（VID/PID・LED ピンが Tachi と異なり、LED は PD6＝D13/アクティブ HIGH）のため別途ビルドが必要です。`megaavr/bootloaders/usbcdcboot/` で `build_wazamono.bat`（または `.sh`）を実行すると Tachi/Kunai とともに生成されます。

---

## ソフトウェア互換性（Arduino Uno R3）

Tsurugi は Uno R3 からの移植の手間を最小化することを目指しています。
基本的には旧 megaAVR とほぼ同一の命令を持ちます。

> レジスタ構成は大幅に変化しているため、レジスタを直接操作するプログラムの移植は難易度が高くなります。

---

## 主要部品

> Tsurugi の確定 BOM・回路図は現在準備中です。確定次第このページに追記します。

---

## 公式ドキュメント

- AVR64DU32 製品ページ: <https://www.microchip.com/en-us/product/AVR64DU32>
- データシート: DS40002548A（AVR64DU32）
- 降圧 DC-DC: Renesas ISL854102 データシート
