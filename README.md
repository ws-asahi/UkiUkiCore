# UkiUkiCore

**UkiUkiduino専用 Arduino コア**  
USB ネイティブな新世代 AVR(AVR DU シリーズ)を搭載した Arduino Uno R3 互換ボード  
「UkiUkiduino」および Pro Micro 互換ボード「UkiUkiduino ProMicro」のための  
ボードサポートパッケージ（Arduino core）です。  
UkiUkiduino シリーズは VTuber「浮々ゆにこ」のファングッズとして開発されたボードです。  

![platform](https://img.shields.io/badge/platform-AVR%20DU-blue)
![license](https://img.shields.io/badge/license-LGPL--2.1-green)
![version](https://img.shields.io/badge/core-v0.0.4-orange)
![based on](https://img.shields.io/badge/based%20on-WazamonoCore%20%2F%20DxCore-lightgrey)

UkiUkiduino は、定番の Arduino Uno R3 を **USB を内蔵した新世代 AVR** で置き換えたボードです。  
USB-シリアル変換チップを別途搭載せず、マイコン単体で PC と直接つながります。  
UkiUkiCore は、このボードを Arduino IDE で開発するための専用コアで、  
[WazamonoCore](https://github.com/ws-asahi/WazamonoCore)から **UkiUkiduino に必要な部分だけを残して再構成** しています。

> ⚠️ **開発版（v0.0.4）です。** API・ボード定義・ブートローダは予告なく変更されることがあります。

---

## ボード概要

| 項目 | 値 |
|------|----|
| Flash | 64 KB(うちスケッチ用 60 KB/USB ブートローダ 4 KB) |
| SRAM | 8 KB |
| EEPROM | 256 B |
| USERROW | 512 B |
| 最大動作周波数 | 24 MHz |
| USB | USB 2.0 Full-Speed デバイス |
| USB-EP | IN16 / OUT16 (合計32) |
| ADC | 10-bit 170 ksps × 1 |
| タイマ | 16-bit TCA0 ×1 / 16-bit TCB ×2 |
| USART | 2 |
| SPI | 2 |
| I2C | 1 |
| 外部割り込み | 全ピン |
| CCL(LUT) | 4 |
| イベントシステム | 6 チャネル |
| アナログコンパレータ(AC) | 1 |
| 電源 | USB 5V / DC ジャック 7–12V |
| 書き込み | USB ブートローダー / UPDI |

<sub>諸元はデータシート DS40002548B（AVR64DU32）に基づく。</sub>

---

## インストール

### ボードマネージャ経由（推奨）

1. Arduino IDE の **ファイル > 基本設定 > 追加のボードマネージャの URL** に以下を追加します。

   ```
   https://ws-asahi.github.io/UkiUkiCore/package_ukiuki_index.json
   ```

2. **ツール > ボード > ボードマネージャ** で「**UkiUkiCore**」を検索してインストールします。
3. コア本体に加えて、専用ツールチェーン（avr-gcc 15.2.0 / avrdude 8.1）が自動的にダウンロード・設定されます。  
   追加の設定は不要です。

詳しい手順・手動インストール（開発者向け）は [Installation.md](Installation.md) を参照してください。

### 必要環境

- Arduino IDE 1.8.13 以降、または 2.x
- ブートローダーの書き換えには UPDI プログラマ（PICkit 4/5、Atmel-ICE、jtag2updi 等）が必要になります。

> Linux をお使いの場合、Arduino IDE は必ず [arduino.cc](https://www.arduino.cc) 配布版を使用してください。  
> ディストリのパッケージマネージャ版は改変されており、正常に動作しません。  

---

## クイックスタート

1. **ツール > ボード > UkiUkiCore** から **UkiUkiduino** または **UkiUkiduino ProMicro** を選択
2. USB ケーブルで接続し、ボードが接続された COM ポートを選択して書き込み


**Lチカ:**
```cpp
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}
void loop() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(500);
}
```

**ボタン連動:**
```cpp
void setup() {
  pinMode(BTN_BUILTIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}
void loop() {
  digitalWrite(LED_BUILTIN, digitalRead(BTN_BUILTIN));  //ボタン状態をLEDへ反映
}
```

**LED色変更:**
```cpp
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  setBLED(255, 0, 0); //赤色指定（標準色は黄色）
}
void loop() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(500);
}
```

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

## 特長

- **クロックと処理速度** — 24 MHz 動作(Uno の 16 MHz 比で 1.5 倍)に加え、  
  AVRxt コアは一部命令のタイミングが改善されており、ベンチマークでは同一クロックでも約 12% 高速です。  
- **記憶領域の刷新** - 新しく USERROW / Flash などの記憶領域が追加(一方 EEPROM 容量は 256 B へ減少)。  
- **USB ネイティブ** - 追加の USB-シリアル変換チップが不要。`Serial` がそのまま USB仮想シリアルポートになります。  
  `Serial` がそのまま USB CDC 仮想シリアルとなり、USB HID(キーボード/マウス)や USB-MIDI にもなれます。  
- **USB ブートローダ** - USB-CDC(STK500v1)経由でスケッチを書き込み。  
- **HID / MIDI 対応** - USB キーボード・マウス等の HID、および USB-MIDI などをサポート。  
- **高い互換性** - 同系統の MCU を採用しているため UnoR3 や ProMicro のコードをほぼそのまま実装可能。  
- **各ピンの出力能力** - ピンあたりの電流出力は 20 mA を維持しており、UnoR4 の 8 mA では動かせない外部機器も動作可能。  
- **全ピンアナログ入力対応** - 全てのデジタル入出力ピン（D0–D20）でアナログ値の読取りが可能。  
- **7 系統の PWM 出力** - Uno R3 では 6 本だった PWM を 7 本に拡張し、  
  更に LUT を使用した択一式 PWM でフレキシブルな運用が可能。  
- **UPDI 対応** - Power ヘッダの 1 番ピンから UPDI デバッガーで動作中の MCU にアクセス可能。  
- **ネイティブ avr-gcc 対応** - DxCore と異なる点として最新の avr-gcc コンパイラを使用（今後も順次更新されます）。  
- **追加の シリアル通信** — 2 系統の UART シリアル通信と2系統の SPI を利用可能です。  
- **フルカラーLED搭載** - LED_BUILTIN(D13)と連動してRGB色指定が可能なLEDが動作します。  
- **オンボードボタン搭載** - BTN_BUILTIN（D21）で追加部品なしに入力を試せます。  

---

### 留意点

- **EEPROM 容量は ATmega328P / ATmega32U4 のほうが大きい**(1 KB 対 256 B)。
- 多くの不揮発データを保存する用途では保存方法の見直し(User Row やフラッシュの活用)が必要になる場合があります
  (後述「データ記憶領域」参照)。

---

## データ記憶領域

AVR64DU32 には用途の異なる複数の不揮発メモリ領域があります。  
ATmega と比べて EEPROM は小さくなりましたが（256 B）、代わりに **USERROW（使用者列）** などの新しい領域が使えます。  

| 領域 | 容量 | 消去単位 | 書き換え耐久 | チップ消去(再書き込み)で | 対応ライブラリ |
|------|------|----------|--------------|----------------------------|----------------|
| EEPROM | 256 B | バイト(1–32 B) | 10 万回 | 消える(EESAVE ヒューズで保持可) | `EEPROM.h` |
| USERROW | 512 B | 512 B ページ一括 | 1,000 回 | **残る** | `USERSIG.h` |
| Flash(APPDATA) | スケッチ領域の空き | 512 B ページ | 1,000 回 | 消える | `Flash.h` |
| SIGROW | 読み取り専用 | — | — | — | 工場書き込みの 16 B 個体シリアル番号を含む |

<sub>各領域の仕様・耐久回数はデータシート DS40002548A(§8 Memories/§11 NVMCTRL/電気的特性)に基づく。</sub>

---

## ベンチマークテスト

他互換機種とのベンチマークテストの結果です。  

| 機種 | MCU | Clock(MHz) | Dhrystone 2.1(5回平均) |
|------|------|------|------|
| UkiUkiduino | AVR64DU32 | 24.0 | 28931.556 |
| Arduino Uno R3 | ATmega328P | 16.0 | 17307.590 |
| Arduino Leonardo | ATmega32U4 | 16.0 | 17207.268 |

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

## クロック

- UkiUkiduino は **水晶を搭載しない**設計で、システムクロックは内蔵 OSCHF から生成します(既定 **24 MHz**。下記の選択肢参照)。
- USB 用の 48 MHz(CLK_USB)は内蔵 PLL48M が生成し、USB の SOF に同期して自動調整されるため水晶なしでも USB は仕様通りに機能します。

> USB ホスト切断時は動作クロックの精度が内蔵オシレータ単体の精度になります。  

---

### クロック速度の選択肢

Arduino IDE の「ツール > Clock Speed」で次の 2 つを選べます。

| メニュー | F_CPU | 主な用途 |
|---------|-------|---------|
| 24 MHz internal(既定) | 24 MHz | 通常はこちら |
| 16 MHz internal | 16 MHz | classic AVR(16 MHz)とのタイミング互換、省電力 |

> PWM の周波数と `delayMicroseconds()` などの時間処理は F_CPU に追従します。  
> `millis()` / `micros()` はどちらの選択肢でも正しく動作します。  

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

## ボード / MCU 識別マクロ

| マクロ |  用途 |
|--------|------|
| `ARDUINO_AVR_UKIUKIDUINO` | ボード識別用(UkiUkiduino) |
| `ARDUINO_AVR_UKIUKIDUINO_PROMICRO` | ボード識別用(UkiUkiduino ProMicro、後述) |
| `__AVR_AVR64DU32__` | MCU 識別用 |
| `__AVR_DU__` | 製品グループ `"DU"` 識別用 |

---

## ソフトウェア互換性(Arduino Uno R3)

- UkiUkiduino は Uno R3 / Leonardo からの移植の手間を最小化することを目指しています。  
- 基本的には旧 megaAVR とほぼ同一の命令を持ちます。  
- Uno R3 と比較すると Serial は純粋な USB-CDC になっています。  
  D0 / D1 は Serial1 として使用できます(Leonardoと同等)。  

> レジスタ構成は大幅に変化しているため、レジスタを直接操作するプログラムの移植は難易度が高くなります。  

## UkiUkiduino ProMicro

UkiUkiduino シリーズの第二機種、**Pro Micro 互換ボード**です。  
MCU(AVR64DU32)・クロック・USB・書き込み方式・オンボード LED / ボタンの API は UkiUkiduino と共通で、  
ピン配置とフォームファクタが Pro Micro(ATmega32U4 版)に合わせてあります。  
キーボード等の組み込み用途(HID)を主眼にしているため、**電源は USB 5V 専用**の簡素な構成です。  

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

### ピンマッピング(ProMicro)

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

### LED とスイッチ(ProMicro)

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

### 移植のヒント(Pro Micro → UkiUkiduino ProMicro)

- ピン番号・`LED_BUILTIN` / `BTN_BUILTIN`・`Serial` / `Serial1` の意味は Pro Micro と揃えてあるので、  
  多くのスケッチは無改変で動きます。  
- `A6`〜`A10` は Pro Micro と同じ物理ピン(D4 / D6 / D8 / D9 / D10)を指します。  
- ボード判定は `#if defined(ARDUINO_AVR_UKIUKIDUINO_PROMICRO)` を使ってください。  
- 32U4 特有のレジスタ操作・`USBCON` の直接操作は動作しません。  

---

## ライセンスとクレジット

UkiUkiCore は [WazamonoCore](https://github.com/ws-asahi/WazamonoCore)（© Workshop Asahi、[DxCore](https://github.com/SpenceKonde/DxCore)（© Spence Konde、LGPL 2.1）からの派生）をベースにした **製品専用フォーク**です。本コアも **LGPL 2.1** で配布されます。

- ベースコア: **DxCore** - © Spence Konde 2021–2022、および各 Arduino コア
- USB スタック・Wazamono 向けカスタマイズ: © Workshop Asahi 2026
- UkiUkiduino 向けボード定義・カスタマイズ: © Unicollabo

ライセンス全文は [LICENSE.md](LICENSE.md) を参照してください。一部のファイル・ライブラリは別ライセンスで提供される場合があり、その旨は各ファイル先頭に記載されています。
