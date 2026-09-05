# UkiUkiduino

**Arduino Uno R3 互換機 — AVR64DU32 / USB-C**

UkiUkiduino は、定番の Arduino Uno R3 を USB ネイティブな AVR `AVR64DU32` で置き換えたボードです。  
USB-シリアル変換チップを搭載せず、マイコン単体で USB-C により PC と直接つながります。  
VTuber「浮々ゆにこ」のファングッズとして開発され、基板は白ベースのフルカラー印刷です。  

このページは UkiUkiduino 専用のドキュメントです。コア全体の概要・インストール・共通 API は [README](../../README.md) を参照してください。  
**状態: 試作中** ピン定義・ブートローダは変更される可能性があります。  

---

## 概要

| 項目 | 内容 |
|------|------|
| MCU | AVR64DU32 |
| フォームファクタ | Arduino Uno R3 互換 |
| USB | USB-C(USB 2.0 Full-Speed、マイコン内蔵) |
| クロック | 24 MHz 内蔵発振(水晶なし) |
| 電源 | USB 5V / DC ジャック 7–12V |
| オンボード LED | フルカラー LED(WS2812D-F5、`LED_BUILTIN` = D13)、電源 LED(白) |
| オンボードボタン | `BTN_BUILTIN` = D21(押下 = HIGH) |
| 書き込み | USB CDC ブートローダ(STK500v1) / UPDI(Power ヘッダ 1 番ピン) |

---

## Uno R3 / Leonardo との比較

UkiUkiduino の比較対象となる Arduino Uno R3 は 旧世代の8ビットマイコン **ATmega328P**（ネイティブUSB なし）を搭載しています。  
また同じく旧世代の Arduino Leonardo が搭載する **ATmega32U4** はネイティブUSBを搭載していますが  
ブートローダーで多くのプログラムメモリを消費します。  
AVR64DU32 は新世代の **AVRxt コア**で、USB 内蔵・クロック・メモリ・周辺機能の多くが強化されています。  

| 項目 | UkiUkiduino (AVR64DU32) | Arduino Uno R3 (ATmega328P) | Arduino Leonardo (ATmega32U4) |
|------|------------------------------|------------------------------|------------------------------|
| コア | AVRxt(命令タイミング改善) | 旧来 AVR | 旧来 AVR |
| 最大クロック | 24 MHz | 16 MHz | 16 MHz |
| USB | マイコン内蔵(変換チップ不要) | なし(基板上に別の USB チップが必要) | マイコン内蔵(変換チップ不要) |
| Flash | 64 KB | 32 KB | 32 KB |
| SRAM | 8 KB | 2 KB | 2.5 KB |
| EEPROM | 256 B | 1 KB | 1 KB |
| USERROW | 512 B | - | - |
| ADC | 10-bit 170ksps 21ch| 10-bit 9.6ksps 6ch | 10-bit 9.6ksps 12ch |
| タイマ | 16-bit ×3(TCA0 + TCB ×2) | 16bit ×1 + 8-bit ×2 | 16bit ×2 + 8-bit ×1 + 10-bit ×1 |
| USART | 2 | 1 | 1 |
| SPI | 2(1つはスレーブ可) | 1(スレーブ不可) | 1(スレーブ不可) |
| I2C | 1 | 1 | 1 |
| 外部割り込み | 全ピン | 2 | 4 | 
| CCL(LUT) | 4 | なし | なし |
| イベントシステム | 6 ch | なし | なし |
| アナログコンパレータ(AC) | 1 | なし | なし |


---

## ピンマッピング

Arduino Uno R3 と同じ番号付け（D0–D13、A0–A5）です。
A0–A5 はデジタル D14–D19 、D0～D13 は アナログ A6–A19 を兼ねます。  
また AREF 端子の D20 とオンボードボタンの D21 が追加されています。

| D# | MCU | アナログ別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PA5 | A6 | AIN25 | **RX**(Serial1) / **MOSI**(SPI1)|
| D1 | PA4 | A7 | AIN24 | **TX**(Serial1) / **MISO**(SPI1)|
| D2 | PA7 | A8 | AIN27 | XDIR（Serial1） / AC0 OUT / EVOUTA |
| D3 | PA6 | A9 | AIN26 | PWM（TCB1 → CCL LUT0 経由） / XCK（Serial1） / SCK（SPI1） |
| D4 | PC3 | A10 | AIN31 | PWM（TCB1 → CCL LUT1 経由） / AC0 AINP4 |
| D5 | PD0 | A11 | AIN0 | PWM / CCL（LUT2-IN0） |
| D6 | PD1 | A12 | AIN1 | PWM / CCL（LUT2-IN1） |
| D7 | PF5 | A13 | AIN21 | PWM（TCB1 WO 直結） |
| D8 | PF4 | A14 | AIN20 |  |
| D9 | PD2 | A15 | AIN2 | PWM / CCL（LUT2-IN2）/ AC AINP0 / EVOUTD |
| D10 | PD3 | A16 | AIN3 | PWM / CCL（LUT2-OUT）/ AC AINN0  |
| D11 | PD4 | A17 | AIN4 | PWM / **MOSI**（SPI） |
| D12 | PD5 | A18 | AIN5 | PWM / **MISO**（SPI） |
| D13 | PD6 | A19 | AIN6 | **LED_BUILTIN** / **SCK**（SPI） / TX（Serial2） |
| D14 | PF0 | A0 | AIN16 | CCL（LUT3-IN0） |
| D15 | PF1 | A1 | AIN17 | CCL（LUT3-IN1） |
| D16 | PF2 | A2 | AIN18 | CCL（LUT3-IN2） |
| D17 | PF3 | A3 | AIN19 | CCL（LUT3-OUT） |
| D18 | PA2 | A4 | AIN22 | アナログ A4 / **SDA**（I2C） |
| D19 | PA3 | A5 | AIN23 | アナログ A5 / **SCL**（I2C） |
| D20 | PD7 | A20 | AIN7 | **AREF**（VREFA 外部基準電圧入力）/ GPIO / SPI ハードウェア SS / RX（Serial2） |
| D21 | PA1 | — | — | **BTN_BUILTIN** |
| A0 | PF0 | D14 | AIN16 | **IN0**(CustomLogic1) |
| A1 | PF1 | D15 | AIN17 | **IN1**(CustomLogic1) |
| A2 | PF2 | D16 | AIN18 | **IN2**(CustomLogic1) / EVOUTF |
| A3 | PF3 | D17 | AIN19 | **OUT**(CustomLogic1) |
| A4 | PA2 | D18 | AIN22 | **SDA**(I2C) |
| A5 | PA3 | D19 | AIN23 | **SCL**(I2C) |

> **D3 / D4 / D7 の PWM は択一**です。  
> 1 本の TCB1 波形をいずれかのピンに出し分ける構造のため、  
> 最後に `analogWrite()` したピンが出口になります(既定は D3)。  
> 3 本同時に異なる PWM は出せません。周波数・デューティは 3 ピンで共通です。  
> tone などで TCB1 を使用する時は 3 つとも PWM が無効化されます。  
>  
> **AREF は GPIO D20 / A20 や SPI SS(スレーブ側)として使えます**。  
> それぞれの機能は排他利用です。  
>  
> D21 は物理ピンを持ちません。  
>  
> オンボード LED は D13 への digitalWrite に連動して動作します。 
> D13 への入力および SPI 動作等では動作しません。 
> また専用の命令で点灯色の色を変更可能です。

---

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ(仮想 COM) |
| `Serial1` | USART0 | D0(RX) / D1(TX) | Uno R3 互換ハードウェア UART |
| `Serial2` | USART1 | AREF(RX) / D13(TX) | 追加 UART |

> Serial1は XCK(D3) / XDIR(D2) と併用して **RS-485 の方向制御や SPI ホストモードにも対応**。  
>  
> `Serial` は `USBSerial` の別名として定義されており USB-CDC を利用します。  

---

### SPI

| オブジェクト | SPI | SPI1 |
| 信号 | ピン(スレーブ可) | ピン(ホストのみ) |
|------|------|------|
| MOSI | D11 | D0 |
| MISO | D12 | D1 |
| SCK | D13 | D3 |
| SS | AREF | なし |

> **クライアント(受信側)動作:** ハードウェア SS(AREF) が実ピンにあるため、  
> 付属の **SPISlave ライブラリ**（ESP8266 互換 API）で SPI スレーブとしても動作できます。  
> その間 AREF ピンは SS 入力となり、外部基準電圧(`analogReference(EXTERNAL)`)・GPIO D20/A20・Serial2 とは排他です。  
>  
> 詳細は [libraries/SPISlave](../libraries/SPISlave/README.md) を参照。  

---

### I2C(Wire)

| 信号 | ピン |
|------|------|
| SDA | A4 |
| SCL | A5 |

> Uno R3 と同じ A4/A5 に配置されています。**Leonardo とは異なります。**  
> 通常の `Wire.begin()` でそのまま使えます。  

---

### PWM(`analogWrite()`)

- **D5 / D6 / D9 / D10 / D11 / D12** - TCA0
- **D3 / D4 / D7** - TCB1 の 8bit PWM 波形を直接または LUT 経由で出力(排他使用)
- Uno R3 に対して D12 へ PWM 機能が追加されています。

> **排他 PWM:** TCB1 が他の用途に使われている間、`analogWrite(D3)`(またはD4, D7) は  
> PWM をあきらめて単純な HIGH/LOW 出力(127 を閾値)に切り替わります。  
> - `tone()` は TCB1 を使うため、実行中は D3, D4, D7 の PWM が停止します。  
> これは Uno R3 の Timer2(`tone()` 実行中は D3, D11 が停止)に相当する挙動です。  

---

### アナログ入力

- 10-bit ADC
- Uno R3 ヘッダの **A0–A5**(= D14–D19)
- 各デジタルピンも ADC チャネルを持ち、A6–A20 として参照可能

> 入力チャネルは複数あっても ADC は一つしかないので多チャンネルで同時に `analogRead` を実行すると安定性が劣化します(megaAVR と同じ挙動)。  
> ただし切り替え速度の向上により影響は最小限になっています。  

---

### クロック出力(CLKOUT)

- メインクロック(CLK_PER)を **D2** へ出力できます。外部 IC へのクロック供給、他 MCU との同期、実クロックの測定に使えます。
- 付属の **ClockOut ライブラリ**で `ClockOut.begin()` / `ClockOut.end()` により開閉します(詳細は [libraries/ClockOut](../libraries/ClockOut/README.md))。

> 24MHz の連続矩形波は EMI 源になるため、必要な期間だけ有効化する運用を推奨します。  
> D2 は AC0 出力・EVOUTA と共用のため、それらが使用中は `begin()` が `false` を返します。 


---

## 電源

UkiUkiduino は **2 系統の電源入力**を持ち、いずれからでも 5V を得られます。  

- **USB-C(5V):** 理想ダイオードで逆流保護し、ホストを破損させずに 5V を供給します。  
  理想ダイオードを採用しているので USB 駆動時でもアナログ入力の基準電圧を 5V として動作できます。  
  (通常のダイオードで保護する場合、アナログ基準電圧が低下し外部から 5V を入力した時の値がズレます)
- **DC ジャック（7–12V、推奨 7–9V）:** φ5.5/2.1mm の DC ジャックから入力し、ダイオードで逆接続保護後、リニアレギュレータで 5V を生成します。  
  リニア方式のため入力電圧が高いほど発熱が増えます。  
  **大きな負荷（数百 mA 以上）を駆動する場合は 7–9V での使用を推奨**します。  
- **3.3V(シールドピン用):** 基板上の LDO。  


---

## LED とスイッチ

| 部品 | 接続 | 用途 |
|------|------|------|
| **LED_BUILTIN** | D13(Active-HIGH) | ユーザー用フルカラー LED |
| **BTN_BUILTIN** | **D21**(Push-HIGH) | ユーザー用ボタン |
| リセット | RESET | ボタン |

> LED_BUILTIN は `setBLED` 命令により点灯色を指定可。  
>  
> BTN_BUILTIN は `digitalRead(BTN_BUILTIN)` または `digitalRead(D21)` で読み取り可能。  
> ボタン押下で HIGH 、離すと LOW になる。


---

## ボード識別マクロ

| マクロ |  用途 |
|--------|------|
| `ARDUINO_AVR_UKIUKIDUINO` | ボード識別用 |

---

## ソフトウェア互換性(Arduino Uno R3)

- UkiUkiduino は Uno R3 / Leonardo からの移植の手間を最小化することを目指しています。  
- 基本的には旧 megaAVR とほぼ同一の命令を持ちます。  
- Uno R3 と比較すると Serial は純粋な USB-CDC になっています。  
  D0 / D1 は Serial1 として使用できます(Leonardoと同等)。  

> レジスタ構成は大幅に変化しているため、レジスタを直接操作するプログラムの移植は難易度が高くなります。  

---

## 主要部品

| 部品 | 型番 | 備考 |
|------|------|------|
| MCU | AVR64DU32-I/PT | TQFP-32 |
| USB コネクタ | USB-C 16P(TYPE-C-31-M-12) | トップマウント |
| USB 保護 | USBLC6-2SC6 | ESD 保護 |
| USB 電源スイッチ | XC8110 | 理想ダイオード / ソフトスタート |
| レギュレータ | 5 V / 3.3 V LDO | DC ジャック入力用 / シールド 3.3 V 用 |
| フルカラー LED | WS2812D-F5-12mA-C1 | 5 mm 砲弾型、RGB 順、データ線 PA0 |
| 電源 LED | KT-0603W(白) | 3.3 V から 330 Ω |

---

## 公式ドキュメント

- AVR64DU32 製品ページ: <https://www.microchip.com/en-us/product/AVR64DU32>
- データシート: DS40002548(AVR64DU28/32)
