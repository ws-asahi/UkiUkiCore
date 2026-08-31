# Wazamono 剣(Tsurugi)

[English](WazamonoTsurugi.md) | **日本語**

**Arduino Uno R3 互換機 — AVR64DU32 / USB-C**

Wazamono Tsurugi は、Arduino Uno R3 と同じフォームファクタを USB ネイティブな AVR `AVR64DU32` で再設計したボードです。  
Uno R3 が別チップ(USB-シリアル変換)を必要としたのに対し、Tsurugi はマイコン単体で USB-C により PC と直接つながります。  
またUSB機能は Leonardo とも近いですが、ハードウェア上の制約で SPI 専用ピンヘッダが GPIO から独立していません。  
さらに、産業用途を想定した **DC ジャック(最大 24V)入力＋同期バックコンバータ**を搭載しています。  
 
このページは Wazamono Tsurugi 専用のドキュメントです。コア全体の概要は [README](../../README.ja.md) を参照してください。  
**状態: 試作中** ピン定義・ブートローダは変更される可能性があります。  
 

---

## 概要

| 項目 | 内容 |
|------|------|
| MCU | AVR64DU32 |
| フォームファクタ | Uno R3 互換 |
| USB | USB-C(USB 2.0 Full-Speed、マイコン内蔵) |
| クロック | 24 MHz 内蔵発振 |
| 電源 | USB 5V / DC ジャック DC7-24V |
| 書き込み | USB CDC ブートローダ(STK500v1) |

---

## ボード諸元(AVR64DU32)

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

<sub>諸元はデータシート DS40002548A(AVR64DU32)に基づく。スケッチ用 Flash サイズ・SRAM サイズは boards.txt の設定値。</sub>

---

## ATmega328P(Arduino Uno R3)および ATmega32U4(Arduino Leonardo)との比較

Wazamono Tsurugi が置き換える Arduino Uno R3 は **ATmega328P**(ネイティブ USB なし / 8-bit MCU )を搭載しています。  
ここでは性能として近い **ATmega32U4**(ネイティブ USB あり / 8-bit MCU )とも比較します。  
AVR64DU32 は新世代の **AVRxt コア**で、USB 内蔵・クロック・メモリ・周辺機能のすべてが強化されています。  

| 項目 | Wazamono Tsurugi (AVR64DU32) | Arduino Uno R3 (ATmega328P) | Arduino Leonardo (ATmega32U4) |
|------|------------------------------|------------------------------|------------------------------|
| コア | AVRxt(命令タイミング改善) | 旧来 AVR | 旧来 AVR |
| 最大クロック | 24 MHz | 16 MHz | 16 MHz |
| USB | マイコン内蔵(変換チップ不要) | なし(基板上に別の USB チップが必要) | マイコン内蔵(変換チップ不要) |
| Flash | 64 KB | 32 KB | 32 KB |
| SRAM | 8 KB | 2 KB | 2.5 KB |
| EEPROM | 256 B | 1 KB | 1 KB |
| USERROW | 512 B | - | - |
| ADC | 10-bit 170ksps 21ch| 10-bit 9.6ksps 6ch | 10-bit 9.6ksps 12ch |
| タイマ | 16-bit ×1 + 8-bit ×2 | 16bit ×1 + 8-bit ×2 | 16bit ×2 + 8-bit ×1 + 10-bit ×1 |
| USART | 2 | 1 | 1 |
| SPI | 2(1つはスレーブ可) | 1(スレーブ不可) | 1(スレーブ不可) |
| I2C | 1 | 1 | 1 |
| 外部割り込み | 全ピン | 2 | 4 | 
| CCL(LUT) | 4 | なし | なし |
| イベントシステム | 6 ch | なし | なし |
| アナログコンパレータ(AC) | 1 | なし | なし |

---

## R4MA1(Arduino Uno R4)との比較

Uno R4 が搭載するルネサス R4MA1 と比較すると多くの点でAVR64DU32は下回ります。  
ただし一部AVR64DU32が勝る点もあります。  

| 項目 | Wazamono Tsurugi (AVR64DU32) | Arduino Uno R4 (R4MA1) |
|------|------------------------------|------------------------------|
| コア | AVRxt | Arm Cortex-M4 |
| 最大クロック | 24 MHz | 48 MHz |
| USB | マイコン内蔵(変換チップ不要) | マイコン内蔵(変換チップ不要) |
| Flash | 64 KB | 256 KB |
| SRAM | 8 KB | 32 KB |
| EEPROM | 256 B | 8 KB(FLASHエミュレーション) |
| ADC | 10-bit 170ksps 21ch | 10-bit 1.85 Msps 25ch |
| タイマ | 16-bit ×1 + 8-bit ×2 | 32bit ×7 + 16-bit ×2 |
| USART | 2 | 4 |
| SPI | 2(1つはスレーブ可) | 4(3つはスレーブ可) |
| I2C | 1 | 4 |
| DAC | なし | 1 |
| 外部割り込み | **全ピン** | 14 |
| CCL(LUT) | **4** | なし |
| イベントシステム | **6 ch** | なし |
| アナログコンパレータ(AC) | **1** | なし |
| GPIO出力能力 | **20mA** | 8mA |

---

### 性能上の主な利点

- **USB がマイコン内蔵** — Uno R3 は USB-シリアル変換用の別チップを基板に載せていましたが、Tsurugi は不要。  
  `Serial` がそのまま USB CDC 仮想シリアルとなり、USB HID(キーボード/マウス)や USB-MIDI にもなれます。  
- **クロックと処理速度** — 24 MHz 動作(Uno の 16 MHz 比で 1.5 倍)に加え、
  AVRxt コアは一部命令のタイミングが改善されており、ベンチマークでは同一クロックでも約 12% 高速です。  
- **メモリ** — Flash 2 倍(64 KB)、SRAM 4 倍(8 KB)。  
- **新世代の周辺機能** — CCL(4 論理ブロック)とイベントシステム(6 チャネル)により、CPU を介さないハードウェア信号処理が可能。  
- **アナログ入力** — ADC チャネルが 21 に増加し全 GPIO がアナログ入力に対応します。  
- **ピンあたりの駆動能力** — AVR の堅牢な I/O により、20mA クラスの出力が可能です。  
- **追加の UART** — 2 系統の UART シリアル通信を利用可能です。  
- **RS-422/485 への対応** — USARTを使用して RS-485 通信が可能(外部の追加チップが必要)。  
- **広い入力電源範囲** — DC ジャックから最大 24V を入力でき、基板上のバックコンバーターで 5V を生成します。  

---

### 留意点

- **EEPROM 容量は ATmega328P / ATmega32U4 のほうが大きい**(1 KB 対 256 B)。
- 多くの不揮発データを保存する用途では保存方法の見直し(User Row やフラッシュの活用)が必要になる場合があります(後述「データ記憶領域」参照)。

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

| 機種 | MCU | Clock(MHz) | Dhrystone 2.1(5回平均) | CoreMark 1.0 Iterations/Sec(5回平均) |
|------|------|------|------|------|
| Tsurugi | AVR64DU32 | 24.0 | (未検証)  | (未検証) |
| Arduino Uno R3 | ATmega328P | 16.0 | 17307.590 | -(書き込み不可) |
| Arduino Leonardo | ATmega32U4 | 16.0 | 17207.268 | -(動作せず) |
| Arduino Uno R4 | RA4M1 | 48.0 | 62303.986 | 81.78 |

---

## ピンマッピング

Arduino Uno R3 と同じ番号付けです。  
全ての外部ピンが ADC に接続されています。  
基本的なピン番号は Uno R3 と同一で、元々 ADC を持たなかったピンには新たなピン番号が割り当てられています。  
そのため Leonardo とはピン配置が大きく異なります。  

| ピン名 | MCU | ピン別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PA5 | A6 | AIN25 | **RX**(Serial1) / **MOSI**(SPI1)|
| D1 | PA4 | A7 | AIN24 | **TX**(Serial1) / **MISO**(SPI1)|
| D2 | PA7 | A8 | AIN27 | **XDIR**(Serial1) / **OUT**(AnalogComp) / EVOUTA / CLKOUT |
| D3 | PA6 | A9 | AIN26 | ~PWM(TCB1 + LUT0) / **XCK**(Serial1) / **CLK**(SPI1)|
| D4 | PC3 | A10 | AIN31 | ~PWM(TCB1 + LUT1) |
| D5 | PD0 | A11 | AIN0 | ~PWM(TCA0 WO0) / **IN0**(CustomLogic) |
| D6 | PD1 | A12 | AIN1 | ~PWM(TCA0 WO1) / **IN1**(CustomLogic) |
| D7 | PF5 | A13 | AIN21 | ~PWM(**TCB1 WO 直結**・択一) |
| D8 | PF4 | A14 | AIN20 | - |
| D9 | PD2 | A15 | AIN2 | ~PWM(TCA0 WO2) / **IN2**(CustomLogic) / **P**(AnalogComp)|
| D10 | PD3 | A16 | AIN3 | ~PWM(TCA0 WO3) / **OUT**(CustomLogic) / **N**(AnalogComp) |
| D11 | PD4 | A17 | AIN4 | ~PWM(TCA0 WO4) / **MOSI**(SPI) |
| D12 | PD5 | A18 | AIN5 | ~PWM(TCA0 WO5) / **MISO**(SPI) |
| D13 | PD6 | A19 | AIN6 | LED_BUILTIN / **SCK**(SPI) / **TX**(Serial2) |
| AREF | PD7 | D20 / A20 | AIN7 | AREF **(GPIOと兼用)** / **SS**(SPI) / **RX**(Serial2) |
| D30 | PA0 | - | - | **LED_BUILTIN_TX**(USB-CDCと連動)|
| D31 | PA1 | - | - | **LED_BUILTIN_RX**(USB-CDCと連動)|
| A0 | PF0 | D14 | AIN16 | **IN0**(CustomLogic1) |
| A1 | PF1 | D15 | AIN17 | **IN1**(CustomLogic1) |
| A2 | PF2 | D16 | AIN18 | **IN2**(CustomLogic1) / EVOUTF |
| A3 | PF3 | D17 | AIN19 | **OUT**(CustomLogic1) |
| A4 | PA2 | D18 | AIN22 | **SDA**(I2C) |
| A5 | PA3 | D19 | AIN23 | **SCL**(I2C) |

>  
> **D3 / D4 / D7 の PWM は択一**です。  
> 1 本の TCB1 波形をいずれかのピンに出し分ける構造のため、最後に `analogWrite()` したピンが出口になります(既定は D3)。  
> 3 本同時に異なる PWM は出せません。周波数・デューティは 3 ピンで共通です。  
> tone などで TCB1 を使用する時は 3 つとも PWM が無効化されます。  
>  
> **AREF は GPIO D20 / A20 や SPI SS(スレーブ側)として使えます**。  
> それぞれの機能は排他利用です。  
>  
> D30,D31は物理ピンを持ちません。  
>  

---

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ(仮想 COM) |
| `Serial1` | USART0 | D0(RX) / D1(TX) | Uno R3 互換ハードウェア UART |
| `Serial2` | USART1 | AREF(RX) / D13(TX) | 追加 UART |

>  
> Serial1は XCK(D3) / XDIR(D2) と併用して **RS-485 の方向制御や SPI ホストモードにも対応**。  
>  
> `Serial` は `USBSerial` の別名として定義されており USB-CDC を利用します。  
>  

---

### SPI

| オブジェクト | SPI | SPI1 |
| 信号 | ピン(スレーブ可) | ピン(ホストのみ) |
|------|------|------|
| MOSI | D11 | D0 |
| MISO | D12 | D1 |
| SCK | D13 | D3 |
| SS | AREF | なし |

>  
> **クライアント(受信側)動作:** ハードウェア SS(AREF) が実ピンにあるため、  
> 付属の **SPISlave ライブラリ**（ESP8266 互換 API）で SPI スレーブとしても動作できます。  
> その間 AREF ピンは SS 入力となり、外部基準電圧(`analogReference(EXTERNAL)`)・GPIO D20/A20・Serial2 とは排他です。  
>  
> 詳細は [libraries/SPISlave](../libraries/SPISlave/README.md) を参照。  
>  
>  

---

### I2C(Wire)

| 信号 | ピン |
|------|------|
| SDA | A4 |
| SCL | A5 |

>  
> Uno R3 と同じ A4/A5 に配置されています。**Leonardo とは異なります。**  
> 通常の `Wire.begin()` でそのまま使えます。  
>  

---

### PWM(`analogWrite()`)

- **D5 / D6 / D9 / D10 / D11 / D12** - TCA0
- **D3 / D4 / D7** - TCB1 の 8bit PWM 波形を直接または LUT 経由で出力(排他使用)
- Uno R3 に対して D12 へ PWM 機能が追加されています。

>  
> **排他 PWM:** TCB1 が他の用途に使われている間、`analogWrite(D3)`(またはD4, D7) は PWM をあきらめて単純な HIGH/LOW 出力(127 を閾値)に切り替わります。  
> - `tone()` は TCB1 を使うため、実行中は D3, D4, D7 の PWM が停止します。  
> これは Uno R3 の Timer2(`tone()` 実行中は D3, D11 が停止)に相当する挙動です。  
>  

---

### アナログ入力

- 10-bit ADC
- Uno R3 ヘッダの **A0–A5**(= D14–D19)
- 各デジタルピンも ADC チャネルを持ち、A6–A20 として参照可能

> 
> 入力チャネルは複数あっても ADC は一つしかないので多チャンネルで同時に `analogRead` を実行すると安定性が劣化します(megaAVR と同じ挙動)。
> 

---

### クロック出力(CLKOUT)

- メインクロック(CLK_PER)を **D2** へ出力できます。外部 IC へのクロック供給、他 MCU との同期、実クロックの測定に使えます。
- 付属の **ClockOut ライブラリ**で `ClockOut.begin()` / `ClockOut.end()` により開閉します(詳細は [libraries/ClockOut](../libraries/ClockOut/README.md))。

>  
> 24MHz の連続矩形波は EMI 源になるため、必要な期間だけ有効化する運用を推奨します。  
> D2 は AC0 出力・EVOUTA と共用のため、それらが使用中は `begin()` が `false` を返します。  
>  

---

## クロック

- Tsurugi は **水晶を搭載しない**設計で、システムクロックは内蔵 OSCHF から生成します(既定 **24 MHz**。下記の選択肢参照)。
- USB 用の 48 MHz(CLK_USB)は内蔵 PLL48M が生成し、USB の SOF に同期して自動調整されるため水晶なしでも USB は仕様通りに機能します。

>  
> USB ホスト切断時は動作クロックの精度が内蔵オシレータ単体の精度になります。  
>  

---

### クロック速度の選択肢

Arduino IDE の「ツール > Clock Speed」で次の 2 つを選べます。

| メニュー | F_CPU | 主な用途 |
|---------|-------|---------|
| 24 MHz internal(既定) | 24 MHz | 通常はこちら |
| 16 MHz internal | 16 MHz | classic AVR(16 MHz)とのタイミング互換、省電力 |

>  
> PWM の周波数と `delayMicroseconds()` などの時間処理は F_CPU に追従します。  
> `millis()` / `micros()` はどちらの選択肢でも正しく動作します。  
>  

---

## 電源

Tsurugi は **2 系統の電源入力**を持ち、いずれからでも 5V を得られます。

- **USB-C(5V):** 理想ダイオードで逆流保護し、ホストを破損させずに 5V を供給します。
- **DC ジャック(最大 24V):** φ5.5/2.1mm の DC ジャックから入力し、ショットキーで逆接続保護後、**同期バックコンバータ**で DC5V 600mAを生成します。
- **3.3V(シールドピン用):** 基板上の LDO。

>  
> 表面的な +5V 供給能力は Uno R3 (5V 1A)に劣りますが、9V 以上で給電した場合 Uno R3 / Leonardo では発熱によって供給能力が低下します。  
> 一方 Tsurugi はバックコンバーターで 5V を供給するためどの電圧でも安定して 600mA を供給することが可能で、9V 以上では供給能力が逆転します。  
> 7V 程度の領域では Uno R3 の供給能力を下回ります。  
>  

---

## LED とスイッチ

| 部品 | 接続 | 用途 |
|------|------|------|
| 電源 LED | 緑 | 電源ライン | 通電表示 |
| LED_BUILTIN | 白 | D13(Active-HIGH) | ユーザー LED(コアは触りません) |
| LED_BUILTIN_TX | 赤 | D30(Active-LOW) | USB-CDC 送信アクティビティ |
| LED_BUILTIN_RX | 赤 | D31(Active-LOW) | USB-CDC 受信アクティビティ |
| リセット | RESET | タクトスイッチ |

>  
> Rx / Tx LED は CDC 受信の瞬間に約 100ms のパルスで点灯し、スケッチからの `digitalWrite(D30, ...)` と共存します。  
>  
> Pro Micro 系スケッチ互換の `TXLED1`/`TXLED0`/`RXLED1`/`RXLED0` マクロ(1=点灯、0=消灯)も定義済みです。  
> USB-CDC の通信中は約 100ms のアクティビティパルスが variant 側から上書きされる点も 32U4 コアと同じ挙動です。  
>  
> LED_BUILTIN (D13) は SPI SCK と共用のため、Uno R3 と同様に SPI 使用中は LED が SCK のトラフィックで点滅します。  
>  

---

## 書き込み

1. ボードを USB で接続します。
2. Arduino IDE からスケッチを書き込みます。書き込み開始時に **1200bps タッチ**が行われ、USB CDC ブートローダへ自動遷移します。
3. 自動遷移しない場合は、**リセットボタンのダブルタップ**でブートローダに入れます。

初回のみ、または USB ブートローダを書き込み直す場合は、UPDI プログラマ(PICkit 4/5、Atmel-ICE、jtag2updi 等)を UPDI ピンに接続して書き込みます。

<sub>開発用 VID/PID は pid.codes のテスト範囲(アプリ `0x1209:0x0008` / ブートローダ `0x1209:0x0007`)を使用しています。</sub>

---

## ボード / MCU 識別マクロ

| マクロ |  用途 |
|--------|------|
| `ARDUINO_AVR_TSURUGI` | ボード識別用 |
| `__AVR_AVR64DU32__` | MCU 識別用 |
| `__AVR_DU__` | 製品グループ `"DU"` 識別用 |

---

## ソフトウェア互換性(Arduino Uno R3)

- Tsurugi は Uno R3 / Leonardo からの移植の手間を最小化することを目指しています。  
- 基本的には旧 megaAVR とほぼ同一の命令を持ちます。  
- Uno R3 と比較すると Serial は純粋な USB-CDC になっています。  
  D0 / D1 は Serial1 として使用できます(Leonardoと同等)。  

>  
> レジスタ構成は大幅に変化しているため、レジスタを直接操作するプログラムの移植は難易度が高くなります。  
>  

---

## 主要部品

>  
> Tsurugi の確定 BOM・回路図は現在準備中です。確定次第このページに追記します。  
>  

---

## 公式ドキュメント

- AVR64DU32 製品ページ: <https://www.microchip.com/en-us/product/AVR64DU32>
- データシート: DS40002548B(AVR64DU32)
