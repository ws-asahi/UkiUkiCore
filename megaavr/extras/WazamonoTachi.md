# Wazamono 太刀(Tachi)

**Pro Micro 互換機 — AVR64DU32 / USB-C**

>  
> Wazamono Tachi は、SparkFun Pro Micro と同じフォームファクタを USB ネイティブな AVR `AVR64DU32` で再設計したボードです。  
> USB-シリアル変換チップを搭載せず、マイコン単体で USB-C により PC と直接つながります。  
>  
> このページは Wazamono Tachi 専用のドキュメントです。コア全体の概要は [README](../../README.md) を参照してください。  
> **状態: 試作中** ピン定義・ブートローダは変更される可能性があります。確定 BOM/回路図は準備中です。  
>  

---

## 概要

| 項目 | 内容 |
|------|------|
| MCU | AVR64DU32(32 ピン) |
| フォームファクタ | SparkFun Pro Micro 互換 |
| USB | USB-C(USB 2.0 Full-Speed、マイコン内蔵) |
| クロック | 24 MHz 内蔵発振(OSCHF、水晶レス) |
| 電源 | USB 5V、または RAW 入力(駆動電源は JP1 で 5V / 3.3V を選択) |
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
| タイマ | TCA0 ×1 / TCB ×2 |
| USART | 2 |
| SPI | 2 |
| I2C | 1 |
| 外部割り込み | 全ピン |
| CCL(LUT) | 4 |
| イベントシステム | 6 チャネル |
| アナログコンパレータ(AC) | 1 |

<sub>各領域の仕様・耐久回数はデータシート DS40002548A(§8 Memories/§11 NVMCTRL/電気的特性)に基づく。</sub>

---

## ATmega32U4(Pro Micro)との比較

>  
> Wazamono Tachi が置き換える Pro Micro は **ATmega32U4** を搭載しています。  
> Pro Micro は USB 内蔵 AVR ですが、AVR64DU32 は新世代の **AVRxt コア**で、クロック・メモリ・周辺機能が大きく強化されています。  
> また同一基板上で 5V と 3.3V の動作電圧切り替えが可能です。  
>  

| 項目 | Wazamono Tachi (AVR64DU32) | Pro Micro (ATmega32U4) |
|------|----------------------------|----------------------------|
| コア | AVRxt(命令タイミング改善) | 旧来 AVR |
| 最大クロック | 24 MHz(1.8–5.5V 全域) | 16 MHz(4.5V 以上)/3.3V では通常 8 MHz |
| 動作電圧 | 5V / 3.3V(部品変更不要) | 5V / 3.3V(部品変更が必要) |
| Flash | 64 KB | 32 KB |
| SRAM | 8 KB | 2.5 KB |
| EEPROM | 256 B | 1 KB |
| USERROW | 512 B | - |
| ADC | 10-bit 170ksps 12ch | 10-bit 9.6ksps 12ch |
| タイマ | 16-bit ×1 + 8-bit ×2 | 16bit ×2 + 8-bit ×1 + 10-bit ×1 |
| USART | 2 | 1 |
| SPI | 2(1つはスレーブ可) | 1(スレーブ不可) |
| I2C | 1 | 1 |
| 外部割り込み | 全ピン | 4 | 
| CCL(LUT) | 4 | なし |
| イベントシステム | 6 ch | なし |
| アナログコンパレータ(AC) | 1 | なし |

---

### 性能上の主な利点

- **クロックと処理速度** — 24 MHz 動作(ATmega32U4は 16 MHz)に加え、AVRxt コアは一部命令のタイミングが改善されており、同一クロックでもわずかに高速です。
- **メモリ** — Flash 2 倍(64 KB)、SRAM 約 3.2 倍(8 KB)。大きなバッファ、USB 複合デバイス、ライブラリを多用するスケッチで余裕が生まれます。
- **新世代の周辺機能** — CCL(4 論理ブロック)とイベントシステム(6 チャネル)により、CPU を介さないハードウェア信号処理が可能。
- **アナログ入力** — ADC チャネルが 21 に増加し全 GPIO がアナログ入力に対応します。
- **ピンあたりの駆動能力** — AVR の堅牢な I/O により、20mA クラスの出力が可能です。
- **追加の UART** — 2 系統の UART シリアル通信を利用可能です。
- **RS-422/485 への対応** — USARTを使用して RS-485 通信が可能(外部の追加チップが必要)。
- **複数電圧への対応** — AVRxtのコア特性を活かして 5V または 3.3V の切り替えを**周辺部品の変更なしに可能**です。

---

### 留意点

- **EEPROM 容量は ATmega32U4 のほうが大きい**(1 KB 対 256 B)。
- 多くの不揮発データを保存する用途では保存方法の見直し(User Row やフラッシュの活用)が必要になる場合があります(後述「データ記憶領域」参照)。

---

## データ記憶領域

>  
> AVR64DU32 には用途の異なる複数の不揮発メモリ領域があります。  
> ATmega と比べて EEPROM は小さくなりましたが(256 B)、代わりに **USERROW(使用者列)** などの新しい領域が使えます。  
>  

| 領域 | 容量 | 消去単位 | 書き換え耐久 | チップ消去(再書き込み)で | 対応ライブラリ |
|------|------|----------|--------------|----------------------------|----------------|
| EEPROM | 256 B | バイト(1–32 B) | 10 万回 | 消える(EESAVE ヒューズで保持可) | `EEPROM.h` |
| USERROW | 512 B | 512 B ページ一括 | 1,000 回 | **残る** | `USERSIG.h` |
| Flash(APPDATA) | スケッチ領域の空き | 512 B ページ | 1,000 回 | 消える | `Flash.h` |
| SIGROW | 読み取り専用 | — | — | — | 工場書き込みの 16 B 個体シリアル番号を含む |

<sub>各領域の仕様・耐久回数はデータシート DS40002548A(§8 Memories/§11 NVMCTRL/電気的特性)に基づく。</sub>

---

## ピンマッピング

>  
> SparkFun Pro Micro と同じ番号付けです。  
> 全ての外部ピンが ADC に接続されています。  
> 基本的なピン番号は Pro Micro と同一で、元々 ADC を持たなかったピンには新たなピン番号が割り当てられています。  
>  

| ピン名 | MCU | ピン別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PA5 | A12 | AIN25 | **RX**(Serial1) / **MOSI**(SPI1)|
| D1 | PA4 | A13 | AIN24 | **TX**(Serial1) / **MISO**(SPI1)|
| D2 | PA2 | A14 | AIN22 | **SDA**(I2C) |
| D3 | PA3 | A15 | AIN23 | ~PWM(TCB1 WO) / **SCL**(I2C) |
| D4 | PC3 | A6 | AIN31 | ~PWM(TCB1 + LUT1) |
| D5 | PD0 | A16 | AIN0 | ~PWM(TCA0 WO0) / **IN0**(CustomLogic) |
| D6 | PD1 | A7 | AIN1 | ~PWM(TCA0 WO1) / **IN1**(CustomLogic) |
| D7 | PA6 | A17 | AIN26 | ~PWM(TCB1 + LUT0) / **XCK**(Serial1)/ **CLK**(SPI1) |
| D8 | PA7 | A8 | AIN27 | **XDIR**(Serial1) / **OUT**(AnalogComp) / EVOUTA / CLKOUT |
| D9 | PD2 | A9 | AIN2 | ~PWM(TCA0 WO2) / **IN2**(CustomLogic) / **P**(AnalogComp)|
| D10 | PD3 | A10 | AIN3 | ~PWM(TCA0 WO3) / **OUT**(CustomLogic) / **N**(AnalogComp) |
| D14 | PD5 | A18 | AIN5 | ~PWM(TCA0 WO5) / **MISO**(SPI) |
| D15 | PD6 | A19 | AIN6 | **SCK**(SPI) / **TX**(Serial2) |
| D16 | PD4 | A20 | AIN4 | ~PWM(TCA0 WO4) / SPI **MOSI** |
| D17 | PF3 | A21 | AIN19 | **LED_BUILTIN**(基板上ユーザー LED) / **OUT**(CustomLogic1) |
| A0 | PD7 | D18 | AIN7 | SPI **SS** / **RX**(Serial2) / **AREF** |
| A1 | PF0 | D19 | AIN16 | **IN0**(CustomLogic1) |
| A2 | PF1 | D20 | AIN17 | **IN1**(CustomLogic1) |
| A3 | PF2 | D21 | AIN18 | **IN2**(CustomLogic1) / EVOUTF |
| A4 | PF4 | D22 | AIN20 | テストパッド TP1(ヘッダなし) |
| A5 | PF5 | D23 | AIN21 | テストパッド TP2(ヘッダなし) |
| D30 | PA0 | - | - | **LED_BUILTIN_TX**(USB-CDC と連動) |
| D31 | PA1 | - | - | **LED_BUILTIN_RX**(USB-CDC と連動) |

> 
> **D3 / D4 / D7 の PWM は択一**です。  
> 1 本の TCB1 波形をいずれかのピンに出し分ける構造のため、最後に `analogWrite()` したピンが出口になります(既定は D3)。  
> 3 本同時に異なる PWM は出せません。周波数・デューティは 3 ピンで共通です。  
> tone などで TCB1 を使用する時は 3 つとも PWM が無効化されます。  
> 
> **A0 / D18 は AREF または SPI SS (スレーブ側) として使用できます**。  
> それぞれの機能は排他利用です。  
> 
> D17 / D30 / D31 は物理ピンを持ちません(基板上 LED)。A4 / A5 はヘッダに出ておらず、裏面テストパッドのみです。  
> D30 / D31(PA0 / PA1)は ADC チャネルを持ちません。  
> 

---

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ(仮想 COM) |
| `Serial1` | USART0 | D0(RX) / D1(TX) | Pro Micro 互換ハードウェア UART |
| `Serial2` | USART1 | D18(RX) / D15(TX) | 予追加 UART |

> 
> Serial1は XCK(D7) / XDIR(D8) と併用して **RS-485 の方向制御や SPI ホストモードにも対応**。  
>  
> `Serial` は `USBSerial` の別名として定義されており USB-CDC を利用します。  
> 

---

### SPI

| オブジェクト | SPI | SPI1 |
| 信号 | ピン(スレーブ可) | ピン(ホストのみ) |
|------|------|------|
| MOSI | D16 | D0 |
| MISO | D14 | D1 |
| SCK | D15 | D7 |
| SS | A0 | なし |

>  
> **クライアント(受信側)動作:** ハードウェア SS(A0) が実ピンにあるため、  
> 付属の **SPISlave ライブラリ**（ESP8266 互換 API）で SPI スレーブとしても動作できます。  
>  
> 詳細は [libraries/SPISlave](../libraries/SPISlave/README.md) を参照。  
>  

---

### I2C(Wire)

| 信号 | ピン |
|------|------|
| SDA | D2 |
| SCL | D3 |

> 
> Pro Micro と同じ D2/D3 に配置されています。  
> 通常の `Wire.begin()` でそのまま使えます。  
> 

---

### PWM(`analogWrite()`)

- **D5 / D6 / D9 / D10 / D14 / D16** - TCA0
- **D3 / D4 / D7** - TCB1 の 8bit PWM 波形を直接(D3)または CCL LUT 経由(D4: LUT1、D7: LUT0)で出力(排他使用)
- Pro Micro に対して D14 / D16 へ PWM 機能が追加されており、D3 を使用しない時は D4 または D7 で PWM を出力できます。

> 
> **排他 PWM:** TCB1 が他の用途に使われている間、`analogWrite(D3)`(または D4, D7) は PWM をあきらめて単純な HIGH/LOW 出力(127 を閾値)に切り替わります。  
> `tone()` は TCB1 を使うため、実行中は D3, D4, D7 の PWM が停止します。  
> これは Pro Micro の Timer3(`tone()` 実行中は D5 が停止)に相当する挙動です。  
> 

### アナログ入力

- シルク表記の **A0–A3**(= D18–D21)
- **A4 / A5**(= D22 / D23)は裏面テストパッド TP1 / TP2 のみ(SparkFun Pro Micro の A4 / A5 = 22 / 23 と番号互換)
- 各デジタルピンも ADC チャネルを持ち、A6–A21 として参照可能(D30 / D31 を除く)

---

## クロック


- Tachi は **水晶を搭載しない**設計で、システムクロックは内蔵 OSCHF の **24 MHz 固定**です。
- USB 用の 48 MHz(CLK_USB)は内蔵 PLL48M が生成し、USB の SOF に同期して自動調整されるため水晶なしでも USB は仕様通りに機能します。

> 
> USB ホスト切断時は動作クロックの精度が内蔵オシレータ単体の精度になります。  
> 

---

## 電源

- **USB-C(5V):** 理想ダイオードで逆流保護し、外部電源との併用時もホストを破損させません。
- **RAW 入力:** 基板上の高耐圧 LDO で 5V を生成します。ドロップアウトを考慮した**実用最低入力は約 6.5V**、上限は放熱で決まります(推奨 6.5–16V)。
- 誤って DC24V の電源を接続しても破壊せず過熱保護で停止します。
- **電圧切替:** ジャンパパッド **JP1** で VCC を 5V / 3.3V から選択します。AVR64DU32 は 3.3 / 5V の全範囲で 24 MHz 動作が可能です。
- Pro Micro(ATmega32U4) では 3.3V 動作時に水晶発振器の変更と 8MHz 動作を強制されますがその制約はありません。

> 
> 電圧選択のためには裏面の JP1 のパッドを望む電圧側とはんだ付けします。  
> 

---

## LED とスイッチ

| 部品 | 色 | 接続 | 用途 |
|------|----|----|------|
| 電源 LED | 黄緑 | 電源ライン | 通電表示 |
| LED_BUILTIN | 橙 | D17 = PF3(Active-LOW) | ユーザー LED(コアは触りません) |
| LED_BUILTIN_TX | 橙 | D30 = PA0(Active-LOW) | USB-CDC 送信アクティビティ |
| LED_BUILTIN_RX | 橙 | D31 = PA1(Active-LOW) | USB-CDC 受信アクティビティ |

> 
> Rx / Tx LED は CDC 送受信の瞬間に約 100ms のパルスで点灯し、スケッチからの `digitalWrite(D30, ...)` / `digitalWrite(D31, ...)` と共存します。  
> Pro Micro 系スケッチ互換の `TXLED1`/`TXLED0`/`RXLED1`/`RXLED0` マクロ(1=点灯、0=消灯)も定義済みです。  
> USB-CDC の通信中は約 100ms のアクティビティパルスが variant 側から上書きされる点も 32U4 コアと同じ挙動です。  
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

| マクロ | 定義元 | 用途 |
|--------|--------|------|
| `WAZAMONO_AVR_TACHI` | variant(`pins_arduino.h`) | **ボード識別(正式)**。Tsurugi / Kunai は `WAZAMONO_AVR_TSURUGI` / `WAZAMONO_AVR_KUNAI` |
| `ARDUINO_WAZAMONO_AVR_TACHI` | Arduino ビルドシステム(`boards.txt` の `build.board`) | Arduino 慣例の `ARDUINO_<board>` 形式 |
| `__AVR_AVR64DU32__` | コンパイラ(`-mmcu`) | MCU 識別(`__AVR_ATmega32U4__` 相当)。Kunai は `__AVR_AVR32DU20__` |
| `__AVR_DU__` / `_AVR_FAMILY` / `_AVR_PINCOUNT` | コア(`core_devices.h`) | ファミリ `"DU"` / ピン数 `32` |

```cpp
#if defined(WAZAMONO_AVR_TACHI)
  // Tachi 固有の処理
#elif defined(WAZAMONO_AVR_TSURUGI)
  // Tsurugi 固有の処理
#endif
```

---

## ソフトウェア互換性(Pro Micro)

- Tachi は Pro Micro からの移植の手間を最小化することを目指しています。
- 基本的には旧 megaAVR とほぼ同一の命令を持ちます。

> 
> レジスタ構成は大幅に変化しているため、レジスタを直接操作するプログラムの移植は難易度が高くなります。
> 

---

## 主要部品

> 
> Tachi の確定 BOM・回路図は現在準備中です。確定次第このページに追記します。
> 

---

## 公式ドキュメント

データシート・エラッタは予告なく更新されます。常に最新版を参照してください(Microchip PCN システムで更新通知を受け取れます)。

- AVR64DU32 製品ページ: <https://www.microchip.com/en-us/product/AVR64DU32>
- データシート: DS40002548A(AVR64DU232)
