# UkiUkiduino（うきうきでゅいーの）

**Arduino Uno R3 互換 — AVR64DU32 / USB-C**

UkiUkiduino は、VTuber「浮々ゆにこ」公式ファングッズとして開発された、Arduino Uno R3 互換ボードです。USB ネイティブな AVR `AVR64DU32` を搭載し、Uno R3 が別チップ（USB-シリアル変換）を必要としたのに対し、マイコン単体で USB-C により PC と直接つながります。白ベース・フルカラープリント基板による特別デザインで、電子工作の入門にも本格的な開発にも使えます。

> このページは UkiUkiduino 1 機種のドキュメントです。コア全体の概要は [README](../../README.md) を参照してください。
> **状態: 開発中（Rev 0.1）。** ピン定義・ブートローダは変更される可能性があります。

---

## 概要

| 項目 | 内容 |
|------|------|
| 由来 | Arduino Uno R3 互換（VTuber「浮々ゆにこ」公式ファングッズ） |
| MCU | AVR64DU32-I/PT（TQFP-32） |
| フォームファクタ | Uno R3 互換（D0–D13、A0–A5、AREF、電源・ICSP ヘッダ） |
| USB | USB-C（USB 2.0 Full-Speed、マイコン内蔵） |
| クロック | 内蔵オシレータ 24 MHz（クリスタルレス。USB クロックは SOF に自動同期） |
| 電源 | USB 5V ／ DC ジャック 7–12V（リニアレギュレータで 5V 生成）／基板上 LDO で 3.3V（シールド用） |
| 書き込み | USB CDC ブートローダ（STK500v1、1200bps タッチ）／UPDI（Power ヘッダ 1 番ピン） |

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
| タイマ | TCA0 ×1（PWM 6ch）、TCB ×2 ／ TCD は USB が占有のため非搭載 |
| USART | 2（USART0 / USART1） |
| SPI / TWI(I2C) | 各 1 |
| CCL（LUT） | 4 |
| イベントシステム | 6 チャネル |
| アナログコンパレータ（AC） | 1 |
| OPAMP | なし |

<sub>諸元はデータシート DS40002548A（AVR64DU32）に基づく。スケッチ用 Flash サイズ・SRAM サイズは boards.txt の設定値。</sub>

---

## ATmega328P（Arduino Uno R3）との比較

UkiUkiduino が互換対象とする Arduino Uno R3 は **ATmega328P**（8-bit AVR、ネイティブ USB なし）を搭載しています。AVR64DU32 は新世代の **AVRxt コア**で、USB 内蔵・クロック・メモリ・周辺機能のすべてが強化されています。

| 項目 | UkiUkiduino (AVR64DU32) | Arduino Uno R3 (ATmega328P) |
|------|--------------------------|------------------------------|
| コア | AVRxt（命令タイミング改善） | 旧来 AVR |
| 動作クロック | 24 MHz（内蔵オシレータ、水晶不要） | 16 MHz（外部水晶/セラロック） |
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

- **USB がマイコン内蔵** — Uno R3 は USB-シリアル変換用の別チップを基板に載せていましたが、UkiUkiduino は不要。`Serial` がそのまま USB CDC 仮想シリアルとなり、USB HID（キーボード/マウス）や USB-MIDI にもなれます。
- **クリスタルレス** — USB 用 48 MHz（CLK_USB）は内蔵 PLL が生成し、USB の SOF に同期して自動調整されます。水晶振動子を搭載せず部品点数とコストを削減しています。
- **クロックと処理速度** — 24 MHz 動作（Uno の 16 MHz 比で 1.5 倍）に加え、AVRxt コアは一部命令のタイミングが改善されています。
- **メモリ** — Flash 2 倍（64 KB）、SRAM 4 倍（8 KB）。
- **新世代の周辺機能** — CCL（4 論理ブロック）とイベントシステム（6 チャネル）により、CPU を介さないハードウェア信号処理が可能。ATmega328P にはいずれもありません。
- **AREF（外部基準電圧）対応** — Uno R3 と同様に AREF 端子から外部基準電圧（VREFA = PD7）を入力できます。
- **BTN_BUILTIN 搭載** — Uno R3 にはなかったオンボードのユーザーボタン（D20）を搭載。追加部品なしで入力を試せます。
- タイマの構成上 Servo や Tone 使用時には TCB1 を使用し、そのため D3 ピンの PWM が無効化されます（Uno R3 では D3 と D11 が無効）。

### 留意点

- **EEPROM 容量は ATmega328P のほうが大きい**（1 KB 対 256 B）。多くの不揮発データを保存する用途では保存方法の見直し（User Row やフラッシュの活用）が必要になる場合があります。
- **D7 と D20 はアナログ入力非対応**（PA0 / PA1 は ADC チャネルを持ちません）。このためアナログ別名 **A13 は欠番**です。
- **Tx / Rx LED が無い**（ピン不足による削減）。Serial に対する出力をボード上でモニターすることはできません。

---

## ピンマッピング

Arduino Uno R3 と同じ番号付け（D0–D13、A0–A5）です。A0–A5 はデジタル D14–D19 を兼ねます。

| D# | MCU | アナログ別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PA5 | A6 | AIN25 | **RX**（Serial1 / USART0 ALT1） |
| D1 | PA4 | A7 | AIN24 | **TX**（Serial1 / USART0 ALT1） |
| D2 | PA6 | A8 | AIN26 | USART0 XCK ／ SPI1 SCK |
| D3 | PF5 | A9 | AIN21 | ~PWM(TCB1 ALT1) ／ tone() |
| D4 | PF4 | A10 | AIN20 | 汎用 I/O（PWM なし。TCB0 は millis） |
| D5 | PD0 | A11 | AIN0 | ~PWM(TCA0 WO0) ／ CCL |
| D6 | PD1 | A12 | AIN1 | ~PWM(TCA0 WO1) ／ CCL |
| D7 | PA0 | —（ADC なし） | — | 汎用 I/O（A13 は欠番） |
| D8 | PA7 | A14 | AIN27 | 汎用 I/O ／ EVOUTA ／ AC0 OUT |
| D9 | PD2 | A15 | AIN2 | ~PWM(TCA0 WO2) ／ CCL ／ AC0 AINP0 ／ EVOUTD |
| D10 | PD3 | A16 | AIN3 | ~PWM(TCA0 WO3) ／ CCL ／ AC0 AINN0 ／ **SS** |
| D11 | PD4 | A17 | AIN4 | ~PWM(TCA0 WO4) ／ SPI **MOSI** |
| D12 | PD5 | A18 | AIN5 | ~PWM(TCA0 WO5) ／ SPI **MISO** |
| D13 | PD6 | A19 | AIN6 | SPI **SCK** ／ **LED_BUILTIN** |
| D14 / A0 | PF0 | A0 | AIN16 | アナログ A0 ／ CCL |
| D15 / A1 | PF1 | A1 | AIN17 | アナログ A1 ／ CCL |
| D16 / A2 | PF2 | A2 | AIN18 | アナログ A2 ／ CCL ／ EVOUTF |
| D17 / A3 | PF3 | A3 | AIN19 | アナログ A3 ／ CCL |
| D18 / A4 | PA2 | A4 | AIN22 | アナログ A4 ／ **SDA**（I2C） |
| D19 / A5 | PA3 | A5 | AIN23 | アナログ A5 ／ **SCL**（I2C） |
| D20 | PA1 | —（ADC なし） | — | **BTN_BUILTIN**（オンボードボタン） |
| AREF | PD7 | — | AIN7 | **VREFA**（外部基準電圧入力。ヘッダ AREF 端子） |

**ヘッダの専用ピン:** ICSP（PD4/PD5/PD6 = MOSI/MISO/SCK + RESET）／Power ヘッダ 1 番ピン = **UPDI**（PF7）。
**内部・非ヘッダピン:** PC3（LED_BUILTIN ドライバ, AIN31）／PF6（RESET）。

> `~` は PWM 出力可能ピン。D7・D20 を除く各デジタルピンは ADC チャネルを持つため、A6–A19 としても参照できます（A13 は欠番）。

---

## ペリフェラル割り当て

variant 側でピン割り当てが確定済みのため、スケッチで `swap()` を指定する必要はありません。

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ（仮想 COM）。日常の `Serial.print()` は Uno と同じ感覚で使えます |
| `Serial1` | USART0（ALT1 固定） | D0(RX) / D1(TX) | Uno R3 の D0/D1 ハードウェア UART |

> `Serial` は USB CDC なので、シリアルモニタ用途は Uno R3 と同じく `Serial` を使います。外部機器と D0/D1 でやり取りする場合のみ `Serial1` を使ってください。

### SPI（ホスト）

| 信号 | ピン |
|------|------|
| MOSI | D11（PD4） |
| MISO | D12（PD5） |
| SCK | D13（PD6） |
| SS | D10（PD3、Uno 慣例） |

Uno R3 と同じ D11–D13 / D10 に配置されています（SPI0 ALT4）。
ボードは SPI ホストで、チップセレクトはソフトウェア制御（任意の GPIO）です。
ALT4 のハードウェア SS は PD7（= AREF）ですが、`SPI0.CTRLB.SSD = 1`（Client Select Disable）で運用するため、AREF 端子の電位によってホスト→クライアントに切り替わることはありません。

### I2C（Wire）

| 信号 | ピン |
|------|------|
| SDA | A4（PA2） |
| SCL | A5（PA3） |

Uno R3 と同じ A4/A5 に配置されています（TWI0 既定位置）。
通常の `Wire.begin()` でそのまま使えます。

### PWM（`analogWrite()`）

- **D5, D6, D9, D10, D11, D12** … TCA0（PORTD へ割り当て、WO0–WO5）
- **D3** … TCB1（ALT1 = PF5）
- Uno R3 に対して D12 へ PWM 機能が追加されています。

> **Uno R3 / tone() との関係:** `tone()` はタイマー **TCB1** を使います。
TCB1 は D3 PWM と共用のため、`tone()` 実行中は D3 の PWM のみ停止します（TCA0 の PWM と millis は動作継続）。
これは Uno R3 の Timer2（`tone()` 実行中は D3 および D11 が無効）に相当する挙動です。

### アナログ入力

- Uno R3 ヘッダの **A0–A5**（= D14–D19）
- D7・D20 を除く各デジタルピンも ADC チャネルを持ち、A6–A19 として参照可能（**A13 は欠番**）
- **AREF 端子（PD7 / VREFA）** から外部基準電圧を入力できます

---

## クロック

**クリスタルレス構成**です。システムクロックは内蔵オシレータ（OSCHF）、USB 用の 48 MHz（CLK_USB）は内蔵 PLL48M が生成し USB の SOF に同期して自動調整されます。**システムクロックの選択とは独立**しているため、内蔵オシレータ動作でも USB は全機能が動作します。

| 選択肢 | クロック源 | 備考 |
|--------|-----------|------|
| 24 MHz internal（既定） | 内蔵 OSCHF | 製品の標準設定 |
| 20 MHz internal | 内蔵 OSCHF | |
| 16 MHz internal | 内蔵 OSCHF | Uno R3 と同じ動作周波数 |

> 外部水晶は搭載していません（PA0/PA1 は D7 / D20 として使用しています）。

---

## 電源

UkiUkiduino は **2 系統の電源入力**を持ち、いずれからでも 5V を得られます。両方を同時に接続しても安全です。

- **USB-C（5V）:** 理想ダイオード（Torex XC8110AA01）で保護しています。逆流防止に加え、突入電流制限（起動時 150mA）・過電流フォールドバック制限・短絡時 50mA 抑制・サーマルシャットダウンを内蔵し（IEC 62368-1:2023 認証品）、ホストポートを保護します。
- **DC ジャック（7–12V、推奨 7–9V）:** φ5.5/2.1mm の DC ジャック（DC1）から入力し、ショットキー（SS34）で逆接続保護後、リニアレギュレータ（AMS1117-5.0）で 5V を生成します。リニア方式のため入力電圧が高いほど発熱が増えます。**大きな負荷（数百 mA 以上）を駆動する場合は 7–9V での使用を推奨**します（Uno R3 と同様の制約です）。
- **3.3V（シールドピン用）:** 基板上の LDO（AMS1117-3.3）。
- **VUSB:** AVR64DU32 内蔵の USB レギュレータを使用（外付け 3.3V 供給なし、デカップリング 0.47µF）。
- USB データライン（D+/D-）は USBLC6-2SC6 で ESD 保護。

---

## LED とスイッチ

| 部品 | 接続 | 用途 |
|------|------|------|
| 電源 LED | 電源ライン（D3／赤） | 通電表示 |
| LED_BUILTIN | D13（PD6）→ PC3 へミラー（D4／黄・3mm 砲弾型） | オンボード LED（Uno R3 慣例） |
| リセット | RESET（PF6） | タクトスイッチ（SW1） |
| **BTN_BUILTIN** | **D20（PA1）** | オンボードユーザーボタン（SW2） |

`LED_BUILTIN` は **D13（PD6）** です。D13 は SPI SCK と共用のため、Uno R3 と同様に SPI 使用中は LED が SCK のトラフィックで点滅します。

> **LED 駆動の仕組み（PC3 ハードミラー）:** UkiUkiduino は D13（PD6）の状態を **イベントシステム + CCL LUT1** で **PC3** にハードウェアミラーし、PC3 側で実際の LED を点灯させます。
> 経路: `PD6 ピンレベル → PORTD EVGEN → EVSYS CH0 → CCL LUT1（バッファ）→ LUT1-OUT = PC3 → LED`。
> CPU を介さないハードウェアパスで、`digitalWrite(13, …)` でも SPI（SCK）でも PD6 を追従します。これにより Uno R3 で必要だった D13 のバッファ用オペアンプを置き換え、LED が SCK ラインに負荷をかけないようにしています。
> EVSYS チャネル 0 と CCL LUT1 を消費するため、Event / Logic ライブラリを使うスケッチではこれらを避けてください。

**BTN_BUILTIN（D20）** は 1kΩ でプルダウンされており、**押すと HIGH** になります。プルアップ設定は不要で、`digitalRead(BTN_BUILTIN)` がそのまま使えます。

```cpp
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BTN_BUILTIN, INPUT);   // 基板上でプルダウン済み
}
void loop() {
  digitalWrite(LED_BUILTIN, digitalRead(BTN_BUILTIN));  // 押している間だけ点灯
}
```

---

## 書き込み

1. ボードを USB で接続します。
2. Arduino IDE からスケッチを書き込みます。書き込み開始時に **1200bps タッチ**が行われ、USB CDC ブートローダへ自動遷移します。
3. 自動遷移しない場合は、**リセットボタンのダブルタップ**でブートローダに入れます。

初回のみ、または USB ブートローダを書き込み直す場合は、UPDI プログラマ（PICkit 4/5、Atmel-ICE、SerialUPDI 等）を **Power ヘッダの 1 番ピン（UPDI = PF7）** に接続して書き込みます。GND・5V も同じ Power ヘッダから取れるため、専用パッドや治具なしで書き込めます。

> **注:** Uno R3 の仕様では Power ヘッダ 1 番ピンは Reserved（未接続）です。UkiUkiduino ではここを UPDI に割り当てているため、万一このピンへ信号を接続するシールドを使う場合はご注意ください。

<sub>開発用 VID/PID は pid.codes のテスト範囲を使用しています。製品出荷までに pid.codes から正式な VID/PID を取得し置き換えます。</sub>

> **ブートローダ hex について:** UkiUkiduino 用のブートローダ hex（`usbcdcboot_ukiukiduino.hex`）はボード固有（VID/PID。LED は PC3／アクティブ HIGH）のため専用ビルドが必要です。`megaavr/bootloaders/usbcdcboot/` のビルドスクリプトで生成します。

---

## ソフトウェア互換性（Arduino Uno R3）

UkiUkiduino は Uno R3 からの移植の手間を最小化することを目指しています。
`Serial` は USB CDC、I2C は A4/A5、SPI は D11–D13、`LED_BUILTIN` は D13、AREF は外部基準電圧入力と、Uno R3 の作法がそのまま通用します。

> 注: レジスタを直接読み出す動作に関しては変更が必要です。

---

## 主要部品

確定 BOM の主要部品です。JLCPCB での量産を前提に、標準部品（Basic 中心）で構成しています。

| 記号 | 種別 | 型番 (MPN) | LCSC | 備考 |
|------|------|-----------|------|------|
| U4 | MCU | Microchip AVR64DU32-I/PT（TQFP-32） | 委託調達 | メインマイコン |
| U3 | 理想ダイオード | Torex XC8110AA01MR-G（SOT-25） | C3235547 | USB 電源 逆流防止・突入/過電流/短絡保護 |
| U1 | レギュレータ | AMS1117-5.0（SOT-223） | C6187 | DC ジャック 5V 生成 |
| U2 | レギュレータ | AMS1117-3.3（SOT-223） | C6186 | シールド 3.3V ピン用 |
| USB1 | USB コネクタ | TYPE-C-31-M-12（USB-C 16P） | C165948 | USB 2.0 FS |
| D1 | ESD 保護 | USBLC6-2SC6（SOT-23-6） | C2827654 | USB D+/D- |
| D2 | ショットキー | SS34（SMA） | C8678 | DC 入力 逆接続保護 |
| D3 | LED（赤・0603） | — | C2286 | 電源表示 |
| D4 | LED（黄・3mm 砲弾型） | Everlight 204-10UYD/S530-A3-L | C85160 | LED_BUILTIN（PC3 駆動） |
| DC1 | DC ジャック | DC-005-2.5A-2.0（φ5.5/2.1mm） | C319099 | DC 7–12V 入力 |
| SW1, SW2 | タクトスイッチ | TS-1187A-B-A-B | C318884 | リセット／BTN_BUILTIN |
| J5 | ピンヘッダ | 2×3 / 2.54mm | — | ICSP / SPI ブレークアウト |
| R1, R2 | 抵抗 5.1kΩ（0402） | — | C25905 | USB-C CC（Rd） |
| R3–R5 | 抵抗 1Ω（0603） | — | C22936 | レギュレータ出力 ESR 補償 |

> 上記のほか、デカップリング用 MLCC（0.1µF ×5、0.47µF ×1）、レギュレータ入出力容量（22µF ×4）、LED 電流制限・ボタン用抵抗（1kΩ ×3、330Ω ×1）、Uno 互換ピンソケット類を実装します。
> レギュレータの出力コンデンサは低 ESR の MLCC（22µF）に 1Ω を直列接続し、安定動作に必要な ESR を確保しています。
> 完全な BOM は別途部品リストを参照してください。

---

## 公式ドキュメント

- AVR64DU32 製品ページ: <https://www.microchip.com/en-us/product/AVR64DU32>
- データシート: DS40002548A（AVR64DU32）
- Torex XC8110/XC8111 データシート（USB 電源保護）
