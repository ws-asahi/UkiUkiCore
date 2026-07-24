# Wazamono 太刀（Tachi）

**Pro Micro 後継機 — AVR64DU32 / USB-C**

Wazamono Tachi は、SparkFun Pro Micro と同じフォームファクタを USB ネイティブな AVR `AVR64DU32` で再設計したボードです。
USB-シリアル変換チップを搭載せず、マイコン単体で USB-C により PC と直接つながります。

> このページは Wazamono Tachi 1 機種のドキュメントです。コア全体の概要は [README](../../README.md) を参照してください。

---

## 概要

| 項目 | 内容 |
|------|------|
| 由来 | Pro Micro 後継 |
| MCU | AVR64DU32（TQFP-32） |
| フォームファクタ | Pro Micro 互換（L 側 1×12 / R 側 1×13 ピンヘッダ） |
| USB | USB-C（USB 2.0 Full-Speed、マイコン内蔵） |
| クロック | 24 MHz 水晶（既定）/ 内蔵オシレータ切替可 |
| 電源 | USB 5V、または基板上 LDO による 3.3V（J3 で切替） |
| 書き込み | USB CDC ブートローダ（STK500v1、1200bps タッチ） |

---

## ボード諸元（AVR64DU32）

| 項目 | 値 |
|------|----|
| Flash | 64 KB（うちスケッチ用 60 KB/USB ブートローダ 4 KB） |
| SRAM | 8 KB |
| EEPROM | 256 B |
| USERROW | 512 B |
| 最大動作周波数 | 24 MHz |
| USB | USB 2.0 Full-Speed デバイス（16 エンドポイントアドレス/最大 32 エンドポイント） |
| ADC | 10-bit 170 ksps × 1（21 チャネル） |
| DAC | なし（AC 内部の DACREF のみ） |
| タイマ | TCA0 ×1（PWM 3ch）、TCB ×2 / TCD は USB が占有のため非搭載 |
| USART | 2（USART0 / USART1） |
| SPI / TWI(I2C) | 各 1 |
| CCL（LUT） | 4 |
| イベントシステム | 6 チャネル |
| アナログコンパレータ（AC） | 1 |
| OPAMP | なし |

<sub>諸元はデータシート DS40002548A（AVR64DU32）に基づく。スケッチ用 Flash サイズ・SRAM サイズは boards.txt の設定値。</sub>

---

## ATmega32U4 との比較

Wazamono Tachi が置き換える Pro Micro / Arduino Leonardo は **ATmega32U4** を搭載しています。
両者とも USB 内蔵 AVR ですが、AVR64DU32 は新世代の **AVRxt コア**で、クロック・メモリ・周辺機能が大きく強化されています。

| 項目 | Wazamono Tachi (AVR64DU32) | Pro Micro 等 (ATmega32U4) |
|------|----------------------------|----------------------------|
| コア | AVRxt（命令タイミング改善） | 旧来 AVR |
| 最大クロック | 24 MHz（1.8–5.5V 全域） | 16 MHz（4.5V 以上）/3.3V では通常 8 MHz |
| Flash | 64 KB | 32 KB |
| SRAM | 8 KB | 2.5 KB |
| EEPROM | 256 B | 1 KB |
| USERROW | 512 B | - |
| ADC | 10-bit・21 ch・170 ksps | 10-bit・12 ch |
| タイマ | 16-bit TCA ×1 + TCB ×2 | 8/16/16/10-bit ×4 |
| USART | 2 | 1 |
| SPI / I2C | 各 1 | 各 1 |
| CCL（論理ブロック） | 4 LUT | なし |
| イベントシステム | 6 ch | なし |
| USB | Full-Speed デバイス（16 EP アドレス） | Full-Speed デバイス（6 EP） |

### 性能上の主な利点

- **クロックと処理速度** — 24 MHz 動作（ATmega32U4は 16 MHz）に加え、AVRxt コアは一部命令のタイミングが改善されており、同一クロックでもわずかに高速です。
- **複数電圧への対応** — AVRxtのコア特性を活かして 5V または 3.3V の切り替えを**周辺部品の変更なしに可能**です。3.3V 動作時、ATmega32U4 は通常 8 MHz に制限されるのに対し、AVR64DU32 は全電圧範囲で 24 MHz を維持できるため、3.3V では最大で約 3 倍の差になります。
- **メモリ** — Flash 2 倍（64 KB）、SRAM 約 3.2 倍（8 KB）。大きなバッファ、USB 複合デバイス、ライブラリを多用するスケッチで余裕が生まれます。
- **新世代の周辺機能** — CCL（3 つの論理ブロック）とイベントシステム（3 チャネル）により、CPU を介さないハードウェアレベルの信号処理・自動ルーティングが可能です。ATmega32U4 にはいずれもありません。
（Tachiでは4つのLUT出力すべてがピンに接続され、うちLUT2/LUT3は入力ピンも利用可能です。イベント出力は3系統＝EVOUTA/D/Fが使用可能です）
- **アナログ** — ADC チャネルが 12 → 21 に増加し、全チャネルがアナログ入力に対応します。
- **追加のUART** — 2系統のUARTシリアル通信を利用可能です。

### 留意点

- **EEPROM 容量は ATmega32U4 のほうが大きい**（1 KB 対 256 B）。多くの不揮発データを EEPROM に保存する用途では、保存方法の見直し（User Row やフラッシュの活用など）が必要になる場合があります（後述「データ記憶領域」参照）。

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

論理ピン番号（`D#`）と MCU ピン、機能の対応です。Pro Micro と同様に D11–D12・D22–D29 は欠番です（D13 は Tachi 独自の LED_BUILTIN として追加）。

| D# | MCU | アナログ別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PA5 | A11 | AIN25 | **Serial1 RX**（USART0 ALT1） |
| D1 | PA4 | A12 | AIN24 | **Serial1 TX**（USART0 ALT1） |
| D2 | PA2 | A13 | AIN22 | **I2C SDA** |
| D3 | PA3 | A14 | AIN23 | **I2C SCL**/~PWM(TCB1)/CCL(LUT0-OUT) |
| D4 | PD7 | A6 | AIN7 | SPI **SS**/Serial2 RX（USART1 ALT2） |
| D5 | PD0 | A15 | AIN0 | ~PWM(TCA0 WO0)/CCL(LUT2-IN0) |
| D6 | PD1 | A7 | AIN1 | ~PWM(TCA0 WO1)/CCL(LUT2-IN1) |
| D7 | PA6 | A16 | AIN26 | USART0 XCK |
| D8 | PA7 | A8 | AIN27 | USART0 XDIR/AC0 出力/EVOUTA/CLKOUT |
| D9 | PD2 | A9 | AIN2 | ~PWM(TCA0 WO2)/CCL(LUT2-IN2)/AC0 AINP0/EVOUTD |
| D10 | PD3 | A10 | AIN3 | ~PWM(TCA0 WO3)/CCL(LUT2-OUT)/AC0 AINN0 |
| D13 | PC3 | A19 | AIN31 | **LED_BUILTIN**（オンボード、Active-HIGH）/CCL(LUT1-OUT) |
| D14 | PD5 | A20 | AIN5 | SPI **MISO**/~PWM(TCA0 WO5) |
| D15 | PD6 | A21 | AIN6 | SPI **SCK**/Serial2 TX（USART1 ALT2） |
| D16 | PD4 | A22 | AIN4 | SPI **MOSI**/~PWM(TCA0 WO4) |
| D17 | PF4 | A23 | AIN20 | RX LED（オンボード、Active-LOW、ヘッダなし） |
| D18 | PF0 | **A0** | AIN16 | アナログ入力 A0/CCL(LUT3-IN0) |
| D19 | PF1 | **A1** | AIN17 | アナログ入力 A1/CCL(LUT3-IN1) |
| D20 | PF2 | **A2** | AIN18 | アナログ入力 A2/CCL(LUT3-IN2)/EVOUTF |
| D21 | PF3 | **A3** | AIN19 | アナログ入力 A3/CCL(LUT3-OUT) |
| D30 | PF5 | A30 | AIN21 | TX LED（オンボード、Active-LOW、ヘッダなし） |

**ヘッダに出ない内部ピン:** PA0・PA1（24 MHz 水晶）/PF6（RESET）/PF7（UPDI）

> `~` は PWM 出力可能ピンを示します。`A0`–`A3` はボードのシルク表記に対応するアナログ入力です。PWM ピンは本家 Pro Micro（D3/D5/D6/D9/D10）と完全に一致し、さらに D14/D16 にも PWM を出力できます（SPI と排他）。アナログ別名も本家（A6=D4, A7=D6, A8=D8, A9=D9, A10=D10）に一致します。

---

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ（仮想 COM） |
| `Serial1` | USART0（ALT1 固定） | D0(RX) / D1(TX) | Pro Micro 互換ハードウェア UART。XCK(D7)/XDIR(D8) 付きのフル機能位置で、RS-485 の方向制御や USART-SPI ホストモードにも対応 |
| `Serial2` | USART1（ALT2 固定） | D4(RX) / D15(TX) | 予備 UART。SPI（SS/SCK）とピン共有・**排他利用** |

> 本家 Pro Micro と同じく、D0/D1 のハードウェア UART が `Serial1` です。`Serial0` は DxCore 内部での USART0 の名称で、`Serial1` と同一オブジェクトを指します。

### SPI（ホスト）

| 信号 | ピン |
|------|------|
| MOSI | D16（PD4） |
| MISO | D14（PD5） |
| SCK | D15（PD6） |
| SS | D4（PD7） |

チップセレクトは任意の GPIO を使用してください（SPI0 ALT4 位置。SCK/SS は `Serial2` と共用・排他利用）。

### I2C

| 信号 | ピン |
|------|------|
| SDA | D2（PA2） |
| SCL | D3（PA3） |

この版では I2C は UART とピンを共有しません。`Wire`・`Serial1`・`Serial2` は同時に使用できます。

### PWM（`analogWrite()`）

- **D5/D6/D9/D10/D14/D16** … TCA0（PORTD へ割り当て、WO0–WO5）
- **D3** … TCB1
- `millis()` / `micros()` は **TCB0** を使用するため、TCB1（D3）と TCA0 は PWM に使用できます。
- 本家 Pro Micro の PWM ピン（D3/D5/D6/D9/D10）を完全再現。D14/D16 は SPI 使用時には PWM に使えません。

### アナログ入力

- シルク表記の **A0–A3**（= D18–D21 = PF0/PF1/PF2/PF3）
- 各デジタルピンも ADC チャネルを持ち、A6–A16・A19–A23・A30 として参照可能

--

## クロック

USB 用の 48 MHz（CLK_USB）は内蔵 PLL48M が生成し、USB の SOF に同期して自動調整されます。
**システムクロックの選択とは独立**しているため、内蔵オシレータ動作でも USB は機能します。
そのため外部水晶を取り除いた廉価版の構築も可能です。

ボードメニュー「Clock Speed」で選択できます。

| 選択肢 | クロック源 | 備考 |
|--------|-----------|------|
| 24 MHz external crystal（既定） | 外部 24 MHz 水晶（PA0/PA1） | 製品の標準設定 |
| 24 MHz internal | 内蔵 OSCHF | 水晶なしで動作確認する場合 |
| 20 MHz internal | 内蔵 OSCHF | |
| 16 MHz internal | 内蔵 OSCHF | |

外部水晶を選択した場合、PA0/PA1 は GPIO として使用できなくなります。

---

## 電源

- **入力:** USB-C（5V）。理想ダイオード（Torex XC8110AA01）で逆流保護。
従来のProMicroに対して、ホストの破損を防ぎつつ 5V 電圧を供給可能。
- **3.3V 動作:** 基板上の LDO（Torex XC6503A331、3.3V/500mA）。
- USB データライン（D+/D-）は TVS（Toshiba DF2B6M4CT）で ESD 保護。
- **電圧切替:** ジャンパパッド **J3**（5V / 3.3V）。AVR64DU32 は 1.8–5.5V の全範囲で 24 MHz 動作が可能です。

<sub>電圧選択のためにはJ3のパッドを望む電圧側とはんだ付けします。
1mmピッチのピンヘッダを取り付けて切り替えることも可能です。</sub>

---

## LED とスイッチ

| 部品 | 色 | 接続 | 用途 |
|------|----|----|------|
| 電源 LED | 黄緑 | 電源ライン | 通電表示 |
| LED_BUILTIN | 黄緑 | D13（PC3、Active-HIGH） | ユーザー LED（Uno/Leonardo の D13 相当） |
| TX LED | 橙 | D30（PF5、Active-LOW） | 送信表示（オンボード） |
| RX LED | 橙 | D17（PF4、Active-LOW） | 受信表示（オンボード） |
| リセット | — | RESET（PF6） | タクトスイッチ |

`LED_BUILTIN` は **D13（PC3）** の専用 LED です（本家 Pro Micro には D13 がなく RX LED が代用でしたが、Tachi では独立させています）。

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

### ウォッチドッグ

古典 AVR（ATmega32U4）の `<avr/wdt.h>` を用いたコードがそのままビルド・動作します。AVR DU のウォッチドッグはレジスタ構成が異なりますが、コアが互換層（`wdt_compat.h`）を自動適用し、`WDTO_*` 定数を DU の時間設定へ正しく変換します。

```cpp
#include <avr/wdt.h>

void setup() {
  wdt_enable(WDTO_2S);   // 2秒のタイムアウト（Pro Micro と同じ書き方）
}

void loop() {
  // 処理が2秒以内に終わるなら…
  wdt_reset();           // ウォッチドッグをリセット（餌やり）
}
```

`wdt_enable(WDTO_*)` / `wdt_reset()` / `wdt_disable()` がそのまま使えます。タイムアウトは 15ms〜8s（`WDTO_15MS`〜`WDTO_8S`）。

> 注: `MCUSR`（リセット要因フラグ）は古典 AVR 固有のため対象外です。`MCUSR = 0;` を含む初期化コードは別途読み替えが必要です（DU では `RSTCTRL.RSTFR`）。

---

## 主要部品

確定 BOM の主要部品です。
**MCU 以外全て日本製部品で統一**されています。

| 記号 | 種別 | 型番 | 備考 |
|------|------|------|------|
| U1 | MCU | AVR64DU32-I/PT（TQFP-32） | メインマイコン |
| U2 | 理想ダイオード | Torex XC8110AA01MR | USB 電源逆流保護 |
| U3 | LDO | Torex XC6503A331MR | 3.3V / 500mA |
| Y1 | 水晶振動子 | Kyocera CX3225SB24000D0GPSCC | 24 MHz |
| D1, D2 | ESD 保護 | Toshiba DF2B6M4CT | USB D+/D- TVS |
| USB1 | USB コネクタ | Hirose CX90M-16P | USB-C ミッドマウント |
| J3 | 電圧切替パッド | — | 5V / 3.3V |
| R1 | 抵抗 470Ω | — | UPDI 直列 |
| R3–R5 | 抵抗 5.1kΩ | — | USB-C CC（Rd）/VBUS 検出 |

---

## 公式ドキュメント

データシート・エラッタは予告なく更新されます。常に最新版を参照してください（Microchip PCN システムで更新通知を受け取れます）。

- AVR64DU32 製品ページ: <https://www.microchip.com/en-us/product/AVR64DU32>
- データシート: DS40002548A（AVR64DU32）
