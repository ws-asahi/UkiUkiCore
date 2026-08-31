# Wazamono 苦無(Kunai)

[English](WazamonoKunai.md) | **日本語**

**Seeeduino XIAO 互換機 — AVR32DU20 / USB-C**

Wazamono Kunai は、Seeeduino XIAO と同じ超小型フォームファクタを AVR `AVR32DU20` で再設計したボードです。
Seeeduino XIAO が搭載する SAMD21 より性能は低いですが、AVR マイコンの搭載により 5V / 3.3V 動作の切り替えと高出力の GPIO を使用できます。
  
このページは Wazamono Kunai 専用のドキュメントです。コア全体の概要は [README](../../README.ja.md) を参照してください。  
**状態: 試作中** ピン定義・ブートローダは変更される可能性があります。確定 BOM/回路図は準備中です。  

---

## 概要

| 項目 | 内容 |
|------|------|
| MCU | AVR32DU20(20 ピン) |
| フォームファクタ | Seeeduino XIAO 互換 |
| USB | USB-C(USB 2.0 Full-Speed、マイコン内蔵) |
| クロック | 24 MHz 内蔵発振 |
| 電源 | USB 5V / VIN 入力 4-6V (駆動電源は JP1 で 5V / 3.3V を選択) |
| 書き込み | USB CDC ブートローダ(STK500v1) |

---

## ボード諸元(AVR32DU20)

| 項目 | 値 |
|------|----|
| Flash | 32 KB(うちスケッチ用 28 KB/USB ブートローダ 4 KB) |
| SRAM | 4 KB |
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
| CCL(LUT) | 3 |
| イベントシステム | 4 チャネル |
| アナログコンパレータ(AC) | 1 |

<sub>諸元は AVR16/32DU ファミリデータシート(DS40002576)に基づく。</sub>

---

## SAMD21(Seeeduino XIAO)との比較

Wazamono Kunai が置き換える Seeeduino XIAO は **ATSAMD21G18** を搭載しています。  
AVR32DU20 は **8-bit の AVRxt コア**で、演算性能やメモリ容量では SAMD21 に及びませんが、  
**5V ネイティブ動作** と **高い出力電流値** ならではの周辺機能と扱いやすさで差別化します。  
また**同一基板上で 5V と 3.3V の動作電圧切り替え**が可能です。  

| 項目 | Wazamono Kunai (AVR32DU20) | Seeeduino XIAO (SAMD21G18) |
|------|----------------------------|----------------------------|
| コア | 8-bit AVRxt | 32-bit ARM Cortex-M0+ |
| 最大クロック | 24 MHz | 48 MHz |
| 動作電圧 | 5V / 3.3V | 3.3V のみ |
| Flash | 32 KB | 256 KB |
| SRAM | 4 KB | 32 KB |
| EEPROM | 256 B | なし(フラッシュエミュレーション) |
| ADC | 10-bit | 12-bit |
| DAC | なし | 10-bit ×1 |
| USB | Full-Speed デバイス(内蔵) | Full-Speed デバイス(内蔵) |
| USART | 2 | SERCOM ×6(機能振分) |
| SPI | 2(1つはホスト限定) | SERCOM ×6(機能振分) |
| I2C | 1 | SERCOM ×6(機能振分) |
| 外部割り込み | 全ピン | ほぼ全ピン |
| CCL(LUT) | 3 | なし |
| イベントシステム | 4 ch | なし |
| アナログコンパレータ(AC) | 1 | なし |

---

### 性能上の主な利点

- **AVR / Arduino-AVR エコシステム** — AVR マイコン向けの豊富なライブラリ・作例がそのまま、あるいは小修正で動きます。
- **新世代の周辺機能** — CCL(3 論理ブロック)、AC（1 アナログコンパレーター）、イベントシステム(4 チャネル)等で CPU を介さないハードウェア信号処理が可能。
- **ピンあたりの駆動能力** — AVR の堅牢な I/O により、20mA クラスの出力が可能です。Seeeduino XIAO では 7mA 程度です。
- **追加の UART** — 2 系統の UART シリアル通信を利用可能です。
- **RS-422/485 への対応** — USARTを使用して RS-485 通信が可能(外部の追加チップが必要)
- **複数電圧への対応** — AVRxtのコア特性を活かして 5V または 3.3V の切り替えが可能です。

### 留意点

- **メモリと演算性能は SAMD21 が上**(SAMD21 はシンプルな演算性能で数倍、メモリ容量も 8 倍)。
- **ADC は 10-bit**(SAMD21 は 12-bit ADC)。また一部ピンで ADC を搭載しません。
- **PWM は 8-bit で 7 点まで、DAC なし**(SAMD21 は 全ピンで PWM 対応 + D0 に 10-bit DAC)。
- **LED_BUILTINが無い** (ピン不足による削減)。Tx / Rx LED をユーザー LED として駆動できます。

---

## データ記憶領域

AVR32DU20 には用途の異なる複数の不揮発メモリ領域があります。    
しかし SAMD21 に比べて容量や書き換え耐性で大きく劣ります。  
大きなデータを保存する用途には向きません。  

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
| Kunai (5.0V) | AVR32DU20 | 24.0 | (未検証) | (未検証) |
| Kunai (3.3V) | AVR32DU20 | 24.0 | (未検証) | (未検証) |
| Seeeduino XIAO | SAMD21G18 | 48.0 | 46547.108| 51.73 |

---

## ピンマッピング

Seeeduino XIAO と同じ番号付けです。  
ただし SAMD21 とは異なり一部の GPIO は PWM や ADC の機能を持ちません。  
PWM 非対応：D1, D3  
ADC 非対応：D6, D7  
またハードウェア上の制約により独立した LED_BUILTIN がありません。  

| ピン名 | MCU | ピン別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PC3 | A0 | AIN31 | ~PWM(TCB1 + LUT1)|
| D1 | PA7 | A1 | AIN27 | **SS**(SPI) / **OUT**(AnalogComp)/ EVOUTA / CLKOUT |
| D2 | PD6 | A2 | AIN6 | **TX**(Serial2) / ~PWM(TCB1 + LUT2)/ **SCK**(SPI)/ **OUT**(AnalogComp)|
| D3 | PD7 | A3 | AIN7 | **RX**(Serial2) / **AREF** / **SS**(SPI) / EVOUTD |
| D4 | PA2 | A4 | AIN22 | **I2C SDA** / ~PWM(TCA0 WO2)/ **XCK**(Serial1) / **CLK**(SPI1) / **IN2**(CustomLogic) |
| D5 | PA3 | A5 | AIN23 | **I2C SCL** / ~PWM(TCA0 WO3)/ **XDIR**(Serial1) / **OUT**(CustomLogic)/ **OUT**(AnalogComp) / EVOUTA / CLKOUT |
| D6 | PA0 | — | — | **TX**(Serial1) / ~PWM(TCA0 WO0) / **MISO**(SPI1) / **IN0**(CustomLogic)|
| D7 | PA1 | — | — | **RX**(Serial1) / ~PWM(TCA0 WO1) / **MOSI**(SPI1) / **IN1**(CustomLogic)|
| D8 | PA6 | A8 | AIN26 | ~PWM(TCB1 WO) / **SCK**(SPI) |
| D9 | PA5 | A9 | AIN25 | ~PWM(TCA0 WO5) / **MISO**(SPI) |
| D10 | PA4 | A10 | AIN24 | ~PWM(TCA0 WO4) / **MOSI**(SPI)  |
| D11 | PD4 | A11 | AIN4 | **LED_BUILTIN** / **LED_BUILTIN_TX**(USB-CDCと連動)|
| D12 | PD5 | A12 | AIN5 | **LED_BUILTIN_RX**(USB-CDCと連動) |

> 
> **D0 / D2 / D8 の PWM は択一**です。  
> 1 本の TCB1 波形をいずれかのピンに出し分ける構造のため、最後に `analogWrite()` したピンが出口になります(既定は D0)。  
> 3 本同時に異なる PWM は出せません。周波数・デューティは 3 ピンで共通です。  
> tone などで TCB1 を使用する時は 3 つとも PWM が無効化されます。  
>
> **D3 は AREFとして使用できます**。  
> AREF として使用する時は他の用途には使用できません。  
>
> D11, D12は物理ピンを持ちません。  
> また D13 はピン不足のため非実装です。  
> 

---

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ(仮想 COM) |
| `Serial1` | USART0 | D7(RX) / D6(TX) | Seeeduino XIAO 互換ハードウェア UART |
| `Serial2` | USART1 | D3(RX) / D2(TX) | 追加 UART |

> 
> Serial1は XCK(D4) / XDIR(D5) と併用して **RS-485 の方向制御や SPI ホストモードにも対応**。  
>  
> `Serial` は `USBSerial` の別名として定義されており USB-CDC を利用します。  
> 

---

| オブジェクト | SPI | SPI1 |
| 信号 | ピン(スレーブ可) | ピン(ホストのみ) |
|------|------|------|
| MOSI | D10 | D7 |
| MISO | D9 | D6 |
| SCK | D8 | D4 |
| SS | D3 | なし |

>  
> **クライアント(受信側)動作:** ハードウェア SS(D3) が実ピンにあるため、  
> 付属の **SPISlave ライブラリ**（ESP8266 互換 API）で SPI スレーブとしても動作できます。  
> その間 D3 ピンは SS 入力となり、外部基準電圧(`analogReference(EXTERNAL)`)・Serial2 とは排他です。
>  
> 詳細は [libraries/SPISlave](../libraries/SPISlave/README.md) を参照。  
>  

### I2C(Wire)

| 信号 | ピン |
|------|------|
| SDA | D4 |
| SCL | D5 |

> 
> Seeduino XIAO と同じ D4/D5 に配置されています。  
> 通常の `Wire.begin()` でそのまま使えます。  
> 

### PWM(`analogWrite()`)

- **D4, D5, D6, D7, D9, D10** - TCA0
- **D0 / D2 / D8** - TCB1 の 8bit PWM 波形を直接または LUT 経由で出力(排他使用)

>  
> **排他 PWM:** TCB1 が他の用途に使われている間、`analogWrite(D0)`(またはD2, D8) は PWM をあきらめて単純な HIGH/LOW 出力(127 を閾値)に切り替わります。  
> - `tone()` は TCB1 を使うため、実行中は D0, D2, D8 の PWM が停止します。  
>  
> Seeeduino XIAO は全ての GPIO で PWM 出力が可能ですが、Kunai では PWM出力 にいくつかの制限があります。
>  

### アナログ入力

- 10-bit ADC
- パッドの **A0–A10**(A6/A7 は欠番)

> ハードウェア仕様上の制約から D6 / D7 にはアナログ入力がありません。

---

### クロック出力(CLKOUT)

- メインクロック(CLK_PER)を **D1** へ出力できます。外部 IC へのクロック供給、他 MCU との同期、実クロックの測定に使えます。
- 付属の **ClockOut ライブラリ**で `ClockOut.begin()` / `ClockOut.end()` により開閉します(詳細は [libraries/ClockOut](../libraries/ClockOut/README.md))。

>  
> 24MHz の連続矩形波は EMI 源になるため、必要な期間だけ有効化する運用を推奨します。  
> D1 は AC0 出力・EVOUTA と共用のため、それらが使用中は `begin()` が `false` を返します。  
>  

---

## クロック

- Kunai は **水晶を搭載しない**設計で、システムクロックは内蔵 OSCHF から生成します(既定 **24 MHz**。下記の選択肢参照)。
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

- **USB-C(5V):** 理想ダイオードで逆流保護し、外部電源との併用時もホストを破損させません。
- **VIN 入力(5V):** 入力された電圧と JP1 のハンダ付けの組み合わせで動作電圧を切り替えることができます。
- **電圧切替:** ジャンパパッド **JP1** で VCC を 5V / 3.3V から選択します。  

> 
> 電圧選択のために裏面の JP1 のパッドを望む電圧側とはんだ付けします。  
> 
> 5V 設定で XIAO 専用アクセサリーを使用すると破損する可能性が高いです。  
> JP1 の設定と周辺機器の組み合わせには注意してください。  
> 

---

## LED とスイッチ

| 部品 | 色 | 接続 | 用途 |
|------|----|----|------|
| 電源 LED | 緑 | 電源ライン | 通電表示 |
| LED_BUILTIN_TX | 赤 | D11(Active-LOW) | USB-CDC 送信アクティビティ |
| LED_BUILTIN_RX | 赤 | D12(Active-LOW) | USB-CDC 受信アクティビティ |
| LED_BUILTIN | - | D11(Active-LOW) | 実体は無いが互換性のため TX に振り替えられる |
| リセット | RESET | タクトスイッチ |

> 
> Rx / Tx LED は CDC 受信の瞬間に約 100ms のパルスで点灯し、スケッチからの `digitalWrite(D11, ...)` と共存します。  
>  
> Pro Micro 系スケッチ互換の `TXLED1`/`TXLED0`/`RXLED1`/`RXLED0` マクロ(1=点灯、0=消灯)も定義済みです。  
> USB-CDC の通信中は約 100ms のアクティビティパルスが variant 側から上書きされます。  
>  
> ハードウェア仕様上の制約から LED_BUILTIN(D13) は実装されていません。  
> 互換性のため LED_BUILTIN は D11 に接続されています。
>

---

## 書き込み

1. ボードを USB で接続します。
2. Arduino IDE からスケッチを書き込みます。書き込み開始時に **1200bps タッチ**が行われ、USB CDC ブートローダへ自動遷移します。
3. 自動遷移しない場合は、**リセットパッドをジャンパ線でダブルタップ**することでブートローダに入れます。

初回のみ、または USB ブートローダを書き込み直す場合は、UPDI プログラマ(PICkit 4/5、Atmel-ICE、jtag2updi 等)を UPDI パッドに接続して書き込みます。

<sub>開発用 VID/PID は pid.codes のテスト範囲(アプリ `0x1209:0x0006` / ブートローダ `0x1209:0x0005`)を使用しています。</sub>

---

## ボード / MCU 識別マクロ

| マクロ |  用途 |
|--------|------|
| `ARDUINO_AVR_KUNAI` | ボード識別用 |
| `__AVR_AVR32DU20__` | MCU 識別用 |
| `__AVR_DU__` | 製品グループ `"DU"` 識別用 |

---

## ソフトウェア・ハードウェア互換性(Seeeduino XIAO)

- Kunai は新型 8-bit AVR マイコンを搭載するため SAMD21 とは互換性の無い場面が多いです。
- 基本的には Arduino IDE で開発可能ですが、レジスタを直接参照するような動作では互換性が失われます。
- 一方旧 megaAVR や Tachi / Tsurugi を含む新型 AVR とは高い互換性を持ちます。

XIAOと比較して以下の機能が異なります。  

- **LED_BUILTIN なし** : 独立した LED が無く 代わりに TX LED が動作します。
- **SWCLK / SWDIO なし** : USB 端子裏のパッドがSWCL, SWDIOではなく、代わりに UPDI と HV RESET (UPDI v2 用パッド) になっています。
- **5V動作が可能** : 5V 動作時には XIAO の 3.3V 供給ピンに 5V が供給されるため、耐圧 3.3V の周辺機器は破壊される可能性があります。
- **PWM は最大7系統** : D1, D3 は PWM に完全非対応。D0, D2, D8 の PWM は排他式。
- **一部ピンで ADC なし** : A6, A7 ピンは ADC なし。
- **変数サイズの不一致** : int は 4 -> 2 バイト。double は 8 -> 4バイト(long doubleは 8 バイトで一致)。 

---

## 主要部品

> Kunai の確定 BOM・回路図は現在準備中です。確定次第このページに追記します。

---

## 公式ドキュメント

- AVR32DU20 製品ページ: <https://www.microchip.com/en-us/product/AVR32DU20>
- データシート: DS40002576B(AVR16/32DU ファミリ)
