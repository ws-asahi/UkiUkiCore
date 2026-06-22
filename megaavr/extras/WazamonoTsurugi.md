# Wazamono 剣（Tsurugi）

**Arduino Uno R3 後継機 — AVR64DU32 / USB-C**

Wazamono Tsurugi は、Arduino Uno R3 と同じピン配置・フォームファクタを USB ネイティブな AVR `AVR64DU32` で再設計したボードです。Uno R3 が別チップ（USB-シリアル変換）を必要としたのに対し、Tsurugi はマイコン単体で USB-C により PC と直接つながります。

> このページは Wazamono Tsurugi 1 機種のドキュメントです。コア全体の概要は [README](../../README.md) を参照してください。
> **状態: 開発中。** ピン定義・ブートローダは変更される可能性があります。

---

## 概要

| 項目 | 内容 |
|------|------|
| 由来 | Arduino Uno R3 後継 |
| MCU | AVR64DU32（TQFP-32） |
| フォームファクタ | Uno R3 互換（D0–D13、A0–A5、電源・AREF ヘッダ） |
| USB | USB-C（USB 2.0 Full-Speed、マイコン内蔵） |
| クロック | 24 MHz 水晶（既定）／内蔵オシレータ切替可 |
| 電源 | USB 5V、または基板上 LDO による 3.3V |
| 書き込み | USB CDC ブートローダ（STK500v1、1200bps タッチ） |

---

## ボード諸元（AVR64DU32）

| 項目 | 値 |
|------|----|
| Flash | 64 KB（うちスケッチ用 60 KB／USB ブートローダ 4 KB） |
| SRAM | 8 KB |
| EEPROM | 256 B |
| 最大動作周波数 | 24 MHz |
| USB | USB 2.0 Full-Speed デバイス（16 エンドポイントアドレス／最大 32 エンドポイント） |
| ADC | 10-bit 170 ksps × 1（21 チャネル） |
| DAC | なし（AC 内部の DACREF のみ） |
| タイマ | TCA0 ×1（PWM 3ch）、TCB ×2 ／ TCD は USB が占有のため非搭載 |
| USART | 2（USART0 / USART1） |
| SPI / TWI(I2C) | 各 1 |
| CCL（LUT） | 4 |
| イベントシステム | 6 チャネル |
| アナログコンパレータ（AC） | 1 |
| OPAMP | なし |

<sub>諸元はデータシート DS40002548A（AVR64DU32）に基づく。スケッチ用 Flash サイズ・SRAM サイズは boards.txt の設定値。</sub>

---

## ATmega328P（Arduino Uno R3）との比較

Wazamono Tsurugi が置き換える Arduino Uno R3 は **ATmega328P**（8-bit AVR、ネイティブ USB なし）を搭載しています。AVR64DU32 は新世代の **AVRxt コア**で、USB 内蔵・クロック・メモリ・周辺機能のすべてが強化されています。

| 項目 | Wazamono Tsurugi (AVR64DU32) | Arduino Uno R3 (ATmega328P) |
|------|------------------------------|------------------------------|
| コア | AVRxt（命令タイミング改善） | 旧来 AVR |
| 最大クロック | 24 MHz（1.8–5.5V 全域） | 20 MHz（4.5V 以上）／Uno は 16 MHz |
| **USB** | **マイコン内蔵**（CDC/HID/MIDI、変換チップ不要） | なし（基板上に別 USB チップが必要） |
| Flash | 64 KB | 32 KB |
| SRAM | 8 KB | 2 KB |
| EEPROM | 256 B | 1 KB |
| ADC | 10-bit・21 ch・170 ksps | 10-bit・6 ch（Uno ヘッダ） |
| タイマ | 16-bit TCA ×1 + TCB ×2 | 8/16/8-bit ×3 |
| USART / SPI / I2C | 2 / 1 / 1 | 1 / 1 / 1 |
| CCL（論理ブロック） | 4 LUT | なし |
| イベントシステム | 6 ch | なし |

### 性能上の主な利点

- **USB がマイコン内蔵** — Uno R3 は USB-シリアル変換用の別チップを基板に載せていましたが、Tsurugi は不要。`Serial` がそのまま USB CDC 仮想シリアルとなり、USB HID（キーボード/マウス）や USB-MIDI にもなれます。
- **クロックと処理速度** — 24 MHz 動作（Uno の 16 MHz 比で 1.5 倍）に加え、AVRxt コアは一部命令のタイミングが改善されています。
- **メモリ** — Flash 2 倍（64 KB）、SRAM 4 倍（8 KB）。
- **新世代の周辺機能** — CCL（4 論理ブロック）とイベントシステム（6 チャネル）により、CPU を介さないハードウェア信号処理が可能。ATmega328P にはいずれもありません。

### 留意点

- **EEPROM 容量は ATmega328P のほうが大きい**（1 KB 対 256 B）。多くの不揮発データを保存する用途では保存方法の見直し（User Row やフラッシュの活用）が必要になる場合があります。
- **PWM ピンが Uno R3 と一部異なります**（後述）。Uno R3 の D3 には PWM がなく、代わりに D4・D12 で PWM が使えます。
- TCD・DAC・OPAMP は非搭載です（USB が TCD を占有）。

---

## ピンマッピング

Arduino Uno R3 と同じ番号付け（D0–D13、A0–A5）です。A0–A5 はデジタル D14–D19 を兼ねます。

| D# | MCU | アナログ別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PA5 | A6 | AIN25 | **RX**（Serial0 / USART0 ALT1） |
| D1 | PA4 | A7 | AIN24 | **TX**（Serial0 / USART0 ALT1） |
| D2 | PA6 | A8 | AIN26 | USART0 XCK |
| D3 | PA7 | A9 | AIN27 | AC0 出力 / EVOUTA |
| D4 | PF4 | A11 | AIN20 | ~PWM(TCB0) |
| D5 | PD0 | A13 | AIN0 | ~PWM(TCA0 WO0) / CCL |
| D6 | PD1 | A14 | AIN1 | ~PWM(TCA0 WO1) / CCL |
| D7 | PF5 | A12 | AIN21 | 汎用 I/O |
| D8 | PC3 | A10 | AIN31 | 汎用 I/O（VUSB 電源ドメイン） |
| D9 | PD2 | A15 | AIN2 | ~PWM(TCA0 WO2) / CCL / EVOUTD |
| D10 | PD3 | A16 | AIN3 | ~PWM(TCA0 WO3) / CCL / **SS** |
| D11 | PD4 | A17 | AIN4 | ~PWM(TCA0 WO4) / SPI **MOSI** |
| D12 | PD5 | A18 | AIN5 | ~PWM(TCA0 WO5) / SPI **MISO** |
| D13 | PD6 | A19 | AIN6 | SPI **SCK** / **LED_BUILTIN** |
| D14 / A0 | PF0 | A0 | AIN16 | アナログ A0 / CCL |
| D15 / A1 | PF1 | A1 | AIN17 | アナログ A1 / CCL |
| D16 / A2 | PF2 | A2 | AIN18 | アナログ A2 / CCL / EVOUTF |
| D17 / A3 | PF3 | A3 | AIN19 | アナログ A3 / CCL |
| D18 / A4 | PA2 | A4 | AIN22 | アナログ A4 / **SDA**（I2C） |
| D19 / A5 | PA3 | A5 | AIN23 | アナログ A5 / **SCL**（I2C） |

**ヘッダの専用ピン:** AREF = PD7（VREFA）。**内部ピン:** PA0/PA1（24 MHz 水晶）、PF6（RESET）、PF7（UPDI）。

> `~` は PWM 出力可能ピン。各デジタルピンは ADC チャネルを持つため、A6–A19 としても参照できます。

---

## ペリフェラル割り当て

variant 側でピン割り当てが確定済みのため、スケッチで `swap()` を指定する必要はありません。

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ（仮想 COM）。日常の `Serial.print()` は Uno と同じ感覚で使えます |
| `Serial0` | USART0（ALT1 固定） | D0(RX) / D1(TX) | Uno R3 の D0/D1 ハードウェア UART |

> **名前について:** この基板では D0/D1 の UART が **USART0** に配線されているため、ハードウェア UART は `Serial1` ではなく **`Serial0`** です（Tachi とは名前が異なります）。`Serial` は USB CDC なので、シリアルモニタ用途は Uno R3 と同じく `Serial` を使います。外部機器と D0/D1 でやり取りする場合のみ `Serial0` を使ってください。
> USART1 はチップ上には存在しますが、Tsurugi では使用可能なピンがありません（DU の USART1 は PD6/PD7 のみで、ここでは SPI SCK / AREF に割り当て済み）。

### SPI（ホスト）

| 信号 | ピン |
|------|------|
| MOSI | D11（PD4） |
| MISO | D12（PD5） |
| SCK | D13（PD6） |
| SS | D10（PD3、Uno 慣例） |

Uno R3 と同じ D10–D13 に配置されています（SPI0 ALT4）。ボードは SPI ホストで、チップセレクトはソフトウェア制御（任意の GPIO）です。ALT4 のハードウェア SS（PD7）は AREF に転用しています。

### I2C（Wire）

| 信号 | ピン |
|------|------|
| SDA | A4（PA2） |
| SCL | A5（PA3） |

Uno R3 と同じ A4/A5 に配置されています（TWI0 既定）。

### PWM（`analogWrite()`）

- **D5, D6, D9, D10, D11, D12** … TCA0（PORTD へ割り当て、WO0–WO5）
- **D4** … TCB0（ALT1 = PF4）
- `millis()` / `micros()` は **TCB1** を使用するため、TCB0（D4）と TCA0（D5–D12 のうち上記）を PWM に使用できます。

> **Uno R3 との違い:** Uno R3 の PWM は D3,5,6,9,10,11 ですが、Tsurugi は **D4,5,6,9,10,11,12** です（D3 には PWM がありません）。

### アナログ入力

- Uno R3 ヘッダの **A0–A5**
- 各デジタルピンも ADC チャネルを持ち、A6–A19 として参照可能

---

## クロック

USB 用の 48 MHz（CLK_USB）は内蔵 PLL48M が生成し USB の SOF に同期して自動調整されます。**システムクロックの選択とは独立**しているため、内蔵オシレータ動作でも USB は機能します。

| 選択肢 | クロック源 | 備考 |
|--------|-----------|------|
| 24 MHz external crystal（既定） | 外部 24 MHz 水晶（PA0/PA1） | 製品の標準設定 |
| 24 MHz internal | 内蔵 OSCHF | 水晶なしで動作確認する場合 |
| 20 MHz internal | 内蔵 OSCHF | |
| 16 MHz internal | 内蔵 OSCHF | Uno R3 と同じ動作周波数 |

外部水晶を選択した場合、PA0/PA1 は GPIO として使用できなくなります。

---

## 電源

- **入力:** USB-C（5V）。
- **3.3V 動作:** 基板上の LDO（3.3V）。AVR64DU32 は 1.8–5.5V の全範囲で 24 MHz 動作が可能です。
- USB データライン（D+/D-）は TVS で ESD 保護。

> Tsurugi の確定 BOM・回路図は現在準備中です。確定次第このページに追記します。

---

## LED とスイッチ

| 部品 | 接続 | 用途 |
|------|------|------|
| LED_BUILTIN | D13（PD6、SCK と共用） | オンボード LED（Uno R3 慣例） |
| リセット | RESET（PF6） | タクトスイッチ |

`LED_BUILTIN` は **D13（PD6）** です。D13 は SPI SCK と共用のため、SPI 使用時は LED が SCK の通信で点滅します（Uno R3 と同じ挙動）。

---

## 書き込み

1. ボードを USB で接続します。
2. Arduino IDE からスケッチを書き込みます。書き込み開始時に **1200bps タッチ**が行われ、USB CDC ブートローダへ自動遷移します。
3. 自動遷移しない場合は、**リセットボタンのダブルタップ**でブートローダに入れます。

初回のみ、または USB ブートローダを書き込み直す場合は、UPDI プログラマ（PICkit 4/5、Atmel-ICE、jtag2updi 等）を UPDI パッド（PF7）に接続して書き込みます。

<sub>開発用 VID/PID は pid.codes のテスト範囲（アプリ `0x1209:0x0008` / ブートローダ `0x1209:0x0007`）を使用しています。製品出荷前に正式な VID/PID へ置き換えてください。</sub>

> **ブートローダ hex について:** Tsurugi 用のブートローダ hex（`usbcdcboot_wazamonotsurugi.hex`）はボード固有（VID/PID・LED ピンが Tachi と異なる）のため別途ビルドが必要です。コアにはまだ含まれていません。動作確認のみであれば、ブートローダ書き込み済みの基板に対し USB からの STK500v1 アップロードが可能です。

---

## 公式ドキュメント

- AVR64DU32 製品ページ: <https://www.microchip.com/en-us/product/AVR64DU32>
- データシート: DS40002548A（AVR64DU32）
