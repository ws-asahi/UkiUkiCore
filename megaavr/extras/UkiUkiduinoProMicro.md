# UkiUkiduino ProMicro

**Pro Micro 互換機 — AVR64DU32 / USB-C**

UkiUkiduino ProMicro は、UkiUkiduino シリーズの第二機種で、SparkFun Pro Micro と同じフォームファクタを  
USB ネイティブな AVR `AVR64DU32` で再設計したボードです。  
MCU・クロック・USB・書き込み方式・オンボード LED / ボタンの API は UkiUkiduino と共通で、  
ピン配置とフォームファクタを Pro Micro(ATmega32U4 版)に合わせてあります。  
キーボード等の組み込み用途(HID)を主眼にしているため、**電源は USB 5V 専用**の簡素な構成です。  

このページは UkiUkiduino ProMicro 専用のドキュメントです。コア全体の概要・インストール・共通 API は [README](../../README.md) を参照してください。  
**状態: 製造中** 初回ロット到着後に実機確認を行います。  

---

## 概要

| 項目 | 値 |
|------|----|
| フォームファクタ | Pro Micro 互換(35.0 × 17.8 mm、12 ピン × 2 列、USB-C 側に 2 mm 延長) |
| MCU / クロック / USB | UkiUkiduino と同一(AVR64DU32、内蔵 24 MHz、USB 2.0 FS) |
| 電源 | **USB 5V のみ**(レギュレータ非搭載。RAW / VCC ピンはともに +5V に直結) |
| オンボード LED | フルカラー LED(WS2812B、`LED_BUILTIN` = D17)、TX / RX LED(D30 / D31、負論理)、電源 LED |
| オンボードボタン | `BTN_BUILTIN` = **D22**(押下 = HIGH) |
| 書き込み | USB ブートローダー / UPDI(4 ピン UPDI ヘッダ: RESET / VCC / GND / UPDI) |

> ⚠️ **RAW ピンに 5V を超える電圧を加えないでください。** Pro Micro と異なりレギュレータがなく、  
> RAW は MCU の VDD に直結されています(絶対最大定格 5.5V)。外部 5V を RAW / VCC へ供給しながら  
> USB を接続することは、USB 側の理想ダイオード(CH213K)が逆流を防ぐため可能です。


---

## ピンマッピング

Pro Micro(SparkFun / ATmega32U4)と同じ番号付けです。D11〜D13 は存在しません。  
A0〜A3 は D18〜D21 を兼ね、その他のデジタルピンはアナログ A6〜A19 を兼ねます(A4 / A5 はありません)。  

| D# | MCU | アナログ別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PA5 | A11 | AIN25 | **RX**(Serial1) / **MOSI**(SPI1) |
| D1 | PA4 | A12 | AIN24 | **TX**(Serial1) / **MISO**(SPI1) |
| D2 | PA2 | A13 | AIN22 | **SDA**(I2C) |
| D3 | PA3 | A14 | AIN23 | **SCL**(I2C) / PWM(TCB1、既定の出口) |
| D4 | PC3 | A6 | AIN31 | PWM(TCB1 → CCL LUT1 経由) / AC0 AINP4 |
| D5 | PD0 | A15 | AIN0 | PWM / CCL(LUT2-IN0) |
| D6 | PD1 | A7 | AIN1 | PWM / CCL(LUT2-IN1) |
| D7 | PA6 | A16 | AIN26 | PWM(TCB1 → CCL LUT0 経由) / XCK(Serial1) / SCK(SPI1) |
| D8 | PA7 | A8 | AIN27 | XDIR(Serial1) / AC0 OUT / EVOUTA / CLKOUT |
| D9 | PD2 | A9 | AIN2 | PWM / CCL(LUT2-IN2) / AC AINP0 / EVOUTD |
| D10 | PD3 | A10 | AIN3 | PWM / CCL(LUT2-OUT) / AC AINN0 |
| D14 | PD5 | A17 | AIN5 | PWM / **MISO**(SPI) |
| D15 | PD6 | A18 | AIN6 | **SCK**(SPI) / TX(Serial2) |
| D16 | PD4 | A19 | AIN4 | PWM / **MOSI**(SPI) |
| D17 | PF5 | — | — | **LED_BUILTIN**(物理ピンなし) |
| D18 | PD7 | A0 | AIN7 | **A0** / SPI ハードウェア SS / RX(Serial2) / VREFA |
| D19 | PF1 | A1 | AIN17 | **A1** / CCL(LUT3-IN1) |
| D20 | PF2 | A2 | AIN18 | **A2** / CCL(LUT3-IN2) / EVOUTF |
| D21 | PF3 | A3 | AIN19 | **A3** / CCL(LUT3-OUT) |
| D22 | PF0 | — | AIN16 | **BTN_BUILTIN**(物理ピンなし) / CCL(LUT3-IN0) |
| D30 | PA0 | — | — | **LED_BUILTIN_TX**(TX LED、負論理、物理ピンなし) |
| D31 | PA1 | — | — | **LED_BUILTIN_RX**(RX LED、負論理、物理ピンなし) |

> **D3 / D4 / D7 の PWM は択一**です(UkiUkiduino の D3 / D4 / D7 と同じ仕組み)。  
> 最後に `analogWrite()` したピンが出口になり、`tone()` 実行中は 3 本とも停止します。  
> Pro Micro 本来の PWM ピン(D3 / D5 / D6 / D9 / D10)はすべて使え、さらに D4 / D7 / D14 / D16 が加わります。  
>  
> **Pro Micro に AREF ピンはありません。** 外部基準電圧を使う場合は A0(PD7 = VREFA)に入力し、  
> その間 A0 / SPI SS / Serial2 RX は使えません。  
>  
> オンボード LED は D17 への digitalWrite に連動します。D17 に物理ピンはありません。  

| 機能 | UkiUkiduino | UkiUkiduino ProMicro |
|------|-------------|----------------------|
| `Serial1`(USART0) | D0(RX) / D1(TX) | D0(RX) / D1(TX) |
| `Serial2`(USART1) | AREF(RX) / D13(TX) | A0(RX) / D15(TX) |
| SPI(MOSI / MISO / SCK) | D11 / D12 / D13 | D16 / D14 / D15 |
| SPI ハードウェア SS(SPISlave) | AREF(D20) | A0(D18) |
| I2C(SDA / SCL) | A4 / A5 | D2 / D3 |
| クロック出力(ClockOut) | D2 | D8 |
| イベント出力 EVOUTA / EVOUTD / EVOUTF | D2 / D9 / A2 | D8 / D9 / A2 |
| CustomLogic1(LUT3) IN0 / IN1 / IN2 / OUT | A0 / A1 / A2 / A3 | D22(ボタン) / A1 / A2 / A3 |
| `LED_BUILTIN` | D13 | D17 |
| `BTN_BUILTIN` | D21 | D22 |
| TX / RX LED | なし | D30 / D31(`TXLED1` 等の 32U4 互換マクロあり) |


---

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ(仮想 COM)。送受信で TX / RX LED が点灯 |
| `Serial1` | USART0 | D0(RX) / D1(TX) | Pro Micro 互換ハードウェア UART |
| `Serial2` | USART1 | A0(RX) / D15(TX) | 追加 UART。SPI の SCK / SS と共用(排他) |

> Serial1 は XCK(D7) / XDIR(D8) と併用して **RS-485 の方向制御や SPI ホストモード(SPI1)にも対応**。  

---

### SPI

| 信号 | SPI | SPI1(USART0) |
|------|------|------|
| MOSI | D16 | D0 |
| MISO | D14 | D1 |
| SCK | D15 | D7 |
| SS | A0(D18) | なし |

> **クライアント(受信側)動作:** ハードウェア SS が A0(PD7)にあるため、付属の **SPISlave ライブラリ**で SPI スレーブとしても動作できます。  
> その間 A0 はアナログ入力・Serial2 RX と排他です。  

---

### I2C(Wire)

| 信号 | ピン |
|------|------|
| SDA | D2 |
| SCL | D3 |

> Pro Micro と同じ D2 / D3 に配置されています。**Uno 形 UkiUkiduino(A4 / A5)とは異なります。**  
> Wire と Serial1 / Serial2 / SPI はピンを共有しないので、すべて同時に使えます。  

---

### PWM(`analogWrite()`)

- **D5 / D6 / D9 / D10 / D14 / D16** - TCA0(D14 / D16 は SPI と排他)
- **D3 / D4 / D7** - TCB1 の 8bit PWM 波形を直接または LUT 経由で出力(排他使用)
- Pro Micro の PWM ピン(D3 / D5 / D6 / D9 / D10)はすべて再現されています。

> `tone()` は TCB1 を使うため、実行中は D3 / D4 / D7 の PWM が停止します(TCA0 側は継続)。  

---

### アナログ入力

- 10-bit ADC
- Pro Micro ヘッダの **A0–A3**(= D18–D21)
- 各デジタルピンも ADC チャネルを持ち、A6–A19 として参照可能(A4 / A5 はありません)

---

### クロック出力(CLKOUT)

- メインクロック(CLK_PER)を **D8** へ出力できます(ClockOut ライブラリ)。D8 は AC0 出力・EVOUTA と共用です。

---

## 電源

- **USB-C(5V)専用**。USB VBUS は理想ダイオード IC(CH213K、電流制限約 1.3 A、逆流阻止)を通して +5V となり、MCU と各部に供給されます。
- **RAW / VCC ピンはともに +5V に直結**されています。レギュレータは搭載していません。
  - RAW / VCC から外部 5V を供給しながら USB を接続しても、CH213K が USB 側への逆流を防ぐため問題ありません。
  - **RAW に 5.5 V を超える電圧を加えないでください**(MCU の絶対最大定格)。Pro Micro のように 6–12 V を入力すると MCU が破損します。
- 3.3 V 出力はありません。

---

## LED とスイッチ

| 部品 | 接続 | 用途 |
|------|------|------|
| **LED_BUILTIN** | D17(Active-HIGH) | ユーザー用フルカラー LED(WS2812B) |
| **LED_BUILTIN_TX / RX** | D30 / D31(Active-LOW) | USB シリアル送受信表示(Pro Micro と同じ挙動) |
| **BTN_BUILTIN** | **D22**(Push-HIGH) | ユーザー用ボタン |
| リセット | RESET | ボタン(ダブルタップでブートローダ) |

> `setBLEDColor()` による色・明るさ指定、`digitalWrite(LED_BUILTIN, HIGH/LOW)` による点灯・消灯、  
> ブートローダ DFU 時の黄色ブレス点灯は UkiUkiduino と同一です。  
> UkiUkiduino ライブラリのサンプル(BLEDColorCycle 等)はそのまま動作します。  
>  
> TX / RX LED は USB-CDC の送受信で自動点灯しますが、スケッチから `digitalWrite(D30, LOW)` で  
> 点灯させることもできます(負論理)。  


---

## 書き込み

- USB ブートローダ(USB-CDC / STK500v1)。RESET ダブルタップでブートローダに入り、フルカラー LED が黄色にブレス点灯します。
- ブートローダ書き換えは基板端の 4 ピン UPDI ヘッダ(RESET / VCC / GND / UPDI)から UPDI プログラマで行います。

---

## ボード識別マクロ

| マクロ |  用途 |
|--------|------|
| `ARDUINO_AVR_UKIUKIDUINO_PROMICRO` | ボード識別用 |

---

## ソフトウェア互換性(Pro Micro)

- ピン番号・`LED_BUILTIN` / `BTN_BUILTIN`・`Serial` / `Serial1` の意味は Pro Micro と揃えてあるので、  
  多くのスケッチは無改変で動きます。  
- `A6`〜`A10` は Pro Micro と同じ物理ピン(D4 / D6 / D8 / D9 / D10)を指します。  
- ボード判定は `#if defined(ARDUINO_AVR_UKIUKIDUINO_PROMICRO)` を使ってください。  
- 32U4 特有のレジスタ操作・`USBCON` の直接操作は動作しません。  


---

## 主要部品

| 部品 | 型番 | 備考 |
|------|------|------|
| MCU | AVR64DU32(T)-I/PT | TQFP-32 |
| USB コネクタ | Hirose CX90M-16P | ミッドマウント USB-C 16P |
| USB 保護 | USBLC6-2SC6 | ESD 保護 |
| USB 電源スイッチ | WCH CH213K | 理想ダイオード / 電流制限 |
| フルカラー LED | XINGLIGHT XL-5050RGBC-WS2812B | 5050 SMD、GRB 順、データ線 PF4(330 Ω 直列) |
| 電源 LED | KT-0603W(白) | 5 V から 1 kΩ |
| TX / RX LED | KT-0603R(赤) ×2 | 5 V から 1 kΩ、PA0 / PA1(負論理) |
| ボタン | TS-1088-AR02016 ×2 | RESET / BTN_BUILTIN |
| 基板 | 35.0 × 17.78 mm、1.2 mm 厚 | 市販 USB-C 版 Pro Micro と同寸 |

---

## 公式ドキュメント

- AVR64DU32 製品ページ: <https://www.microchip.com/en-us/product/AVR64DU32>
- データシート: DS40002548(AVR64DU28/32)
- LED データシート: XINGLIGHT XL-5050RGBC-WS2812B(LCSC C2843785)
- USB コネクタ: Hirose CX90M-16P Design Guide(推奨板厚 0.8 mm、本機は 1.2 mm で実装実績あり)
