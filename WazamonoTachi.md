# Wazamono 太刀（Tachi）

**Arduino Pro Micro 後継機 — AVR64DU32 / USB-C / Grove**

Wazamono Tachi は、Arduino Pro Micro と同じフォームファクタを USB ネイティブな AVR `AVR64DU32` で再設計したボードです。USB-シリアル変換チップを搭載せず、マイコン単体で USB-C により PC と直接つながります。

> このページは Wazamono Tachi 1 機種のドキュメントです。コア全体の概要は [README](../../README.md) を参照してください。

---

## 概要

| 項目 | 内容 |
|------|------|
| 由来 | Arduino Pro Micro 後継 |
| MCU | AVR64DU32（TQFP-32） |
| フォームファクタ | Pro Micro 互換（L 側 1×12 / R 側 1×13 ピンヘッダ） |
| USB | USB-C（USB 2.0 Full-Speed、マイコン内蔵） |
| クロック | 24 MHz 水晶（既定）／内蔵オシレータ切替可 |
| 電源 | USB 5V、または基板上 LDO による 3.3V（J3 で切替） |
| 拡張 | Grove I2C コネクタ（PA2/PA3） |
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

## ピンマッピング

論理ピン番号（`D#`）と MCU ピン、機能の対応です。Pro Micro と同様に D11–D13・D22–D29 は欠番です。

| D# | MCU | アナログ別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PD7 | A30 | AIN7 | **Serial1 RX**（USART1 ALT2）／イベント出力(EVOUTD) |
| D1 | PD6 | A31 | AIN6 | **Serial1 TX**（USART1 ALT2） |
| D2 | PA2 | A32 | AIN22 | **I2C SDA**（Grove）／Serial2 TX（USART0 ALT2）／EVOUTA |
| D3 | PA3 | A33 | AIN23 | **I2C SCL**（Grove）／Serial2 RX（USART0 ALT2）／~PWM(TCB1) |
| D4 | PA7 | A6 | AIN27 | SPI **SS** ／ AC0 出力 |
| D5 | PF0 | A34 | AIN16 | ~PWM(TCA0 WO0) ／ CCL |
| D6 | PF1 | A7 | AIN17 | ~PWM(TCA0 WO1) ／ CCL |
| D7 | PF2 | A35 | AIN18 | ~PWM(TCA0 WO2) ／ CCL ／ EVOUTF |
| D8 | PF3 | A8 | AIN19 | ~PWM(TCA0 WO3) ／ CCL |
| D9 | PF4 | A9 | AIN20 | ~PWM(TCA0 WO4) |
| D10 | PF5 | A10 | AIN21 | ~PWM(TCA0 WO5) |
| D14 | PA5 | A36 | AIN25 | SPI **MISO** |
| D15 | PA6 | A37 | AIN26 | SPI **SCK** |
| D16 | PA4 | A38 | AIN24 | SPI **MOSI** |
| D17 | PD5 | — | AIN5 | **LED_BUILTIN** ／ RX LED（オンボード、ヘッダなし） |
| D18 | PD3 | **A0** | AIN3 | アナログ入力 A0 |
| D19 | PD2 | **A1** | AIN2 | アナログ入力 A1 |
| D20 | PD1 | **A2** | AIN1 | アナログ入力 A2 |
| D21 | PD0 | **A3** | AIN0 | アナログ入力 A3 |
| D30 | PD4 | — | AIN4 | TX LED（オンボード、ヘッダなし） |

**ヘッダに出ない内部ピン:** PC3（VBUS 検出, AIN31）／PA0・PA1（24 MHz 水晶）／PF6（RESET）／PF7（UPDI）

> `~` は PWM 出力可能ピンを示します。`A0`–`A3` はボードのシルク表記に対応するアナログ入力です。各デジタルピンは ADC チャネルを持つため、A6–A10・A30–A38 としても参照できます。

---

## ペリフェラル割り当て

variant 側でピン割り当てが確定済みのため、スケッチで `swap()` を指定する必要はありません。

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ（仮想 COM） |
| `Serial1` | USART1（ALT2 固定） | D0(RX) / D1(TX) | Pro Micro 互換ハードウェア UART |
| `Serial2` | USART0（ALT2 固定） | D2(SDA) / D3(SCL) | 予備 UART。Grove I2C とピン共有・**排他利用** |

> `Serial0` は DxCore 内部での USART0 の名称です。ユーザ向けには `Serial2` を使用してください。

### SPI（ホスト）

| 信号 | ピン |
|------|------|
| MOSI | D16（PA4） |
| MISO | D14（PA5） |
| SCK | D15（PA6） |
| SS | D4（PA7） |

チップセレクトは任意の GPIO を使用してください（SS ピンは AC0 出力と共用）。

### I2C（Grove コネクタ）

| 信号 | ピン |
|------|------|
| SDA | D2（PA2） |
| SCL | D3（PA3） |

Grove コネクタに直結されています。前述のとおり `Serial2`（USART0）とピンを共有します。

### PWM（`analogWrite()`）

- **D5–D10** … TCA0（PORTF へ割り当て、WO0–WO5）
- **D3** … TCB1
- `millis()` / `micros()` は **TCB0** を使用するため、TCB1（D3）と TCA0（D5–D10）は PWM に使用できます。

### アナログ入力

- シルク表記の **A0–A3**（= D18–D21 = PD3/PD2/PD1/PD0）
- 各デジタルピンも ADC チャネルを持ち、A6–A10・A30–A38 として参照可能

---

## クロック

USB 用の 48 MHz（CLK_USB）は内蔵 PLL48M が生成し、USB の SOF に同期して自動調整されます。**システムクロックの選択とは独立**しているため、内蔵オシレータ動作でも USB は機能します。

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
- **3.3V 動作:** 基板上の LDO（Torex XC6503A331、3.3V/500mA）。
- **電圧切替:** ジャンパパッド **J3**（5V / 3.3V）。AVR64DU32 は 1.8–5.5V の全範囲で 24 MHz 動作が可能です。
- USB データライン（D+/D-）は TVS（Toshiba DF2B6M4CT）で ESD 保護。

---

## LED とスイッチ

| 部品 | 色 | 接続 | 用途 |
|------|----|----|------|
| 電源 LED | 黄緑 | 電源ライン | 通電表示 |
| TX LED | 橙 | D30（PD4） | 送信表示（オンボード） |
| RX LED | 橙 | D17（PD5） | 受信表示。`LED_BUILTIN` |
| リセット | — | RESET（PF6） | タクトスイッチ |

`LED_BUILTIN` は **D17（PD5、RX LED）** です。

---

## 書き込み

1. ボードを USB で接続します。
2. Arduino IDE からスケッチを書き込みます。書き込み開始時に **1200bps タッチ**が行われ、USB CDC ブートローダへ自動遷移します。
3. 自動遷移しない場合は、**リセットボタンのダブルタップ**でブートローダに入れます。

初回のみ、または USB ブートローダを書き込み直す場合は、UPDI プログラマ（PICkit 4/5、Atmel-ICE、jtag2updi 等）を UPDI パッド（PF7、直列 470Ω 経由）に接続して書き込みます。

<sub>開発用 VID/PID は pid.codes のテスト範囲（アプリ `0x1209:0x0006` / ブートローダ `0x1209:0x0005`）を使用しています。製品出荷前に正式な VID/PID へ置き換えてください。</sub>

---

## 主要部品

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
| R3–R5 | 抵抗 5.1kΩ | — | USB-C CC（Rd）／VBUS 検出 |

---

## 公式ドキュメント

データシート・エラッタは予告なく更新されます。常に最新版を参照してください（Microchip PCN システムで更新通知を受け取れます）。

- AVR64DU32 製品ページ: <https://www.microchip.com/en-us/product/AVR64DU32>
- データシート: DS40002548A（AVR64DU32）
