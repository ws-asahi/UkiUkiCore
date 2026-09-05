# UkiUkiCore

**UkiUkiduino シリーズ専用 Arduino コア**  
USB ネイティブな新世代 AVR(AVR DU シリーズ)を搭載した Arduino 互換ボード  
「UkiUkiduino」シリーズのためのボードサポートパッケージ(Arduino core)です。  
UkiUkiduino シリーズは VTuber「浮々ゆにこ」のファングッズとして開発されたボードです。  

![platform](https://img.shields.io/badge/platform-AVR%20DU-blue)
![license](https://img.shields.io/badge/license-LGPL--2.1-green)
![version](https://img.shields.io/badge/core-v0.0.4-orange)
![based on](https://img.shields.io/badge/based%20on-WazamonoCore%20%2F%20DxCore-lightgrey)

UkiUkiduino シリーズは、定番の Arduino 互換ボードを **USB を内蔵した新世代 AVR** で置き換えたボード群です。  
USB-シリアル変換チップを別途搭載せず、マイコン単体で PC と直接つながります。  
UkiUkiCore は、これらのボードを Arduino IDE で開発するための専用コアで、  
[WazamonoCore](https://github.com/ws-asahi/WazamonoCore)から **UkiUkiduino シリーズに必要な部分だけを残して再構成** しています。

> ⚠️ **開発版(v0.0.4)です。** API・ボード定義・ブートローダは予告なく変更されることがあります。

---

## 対応ボード

| ボード | MCU | フォームファクタ | 状態 |
|--------|-----|------------------|------|
| [**UkiUkiduino**](megaavr/extras/UkiUkiduino.md) | AVR64DU32 | Arduino Uno R3 互換 / USB-C | 🔧 試作中 |
| [**UkiUkiduino ProMicro**](megaavr/extras/UkiUkiduinoProMicro.md) | AVR64DU32 | Pro Micro 互換 / USB-C | 🔧 製造中 |

> ピン配置・オンボード LED / ボタン・電源など、ボードごとの詳細は上記の各ページを参照してください。  
> このページにはコア全体に共通する内容をまとめています。  

---

## 心臓部 - AVR64DU32

両機種とも USB を内蔵した新世代 AVR「**AVR64DU32**」を採用しています。  

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
- **全ピンアナログ入力対応** - 全てのデジタル入出力ピンでアナログ値の読取りが可能。  
- **拡張された PWM 出力** - TCA0 の 6 本に加え、LUT を使用した択一式 PWM(TCB1)を備え、  
  Uno R3 / Pro Micro 本来の PWM ピンをすべて再現した上で追加の PWM ピンを提供。  
- **UPDI 対応** - UPDI デバッガーで動作中の MCU にアクセス可能(UkiUkiduino: Power ヘッダ 1 番ピン / ProMicro: UPDI ヘッダ)。  
- **ネイティブ avr-gcc 対応** - DxCore と異なる点として最新の avr-gcc コンパイラを使用（今後も順次更新されます）。  
- **追加の シリアル通信** — 2 系統の UART シリアル通信と2系統の SPI を利用可能です。  
- **フルカラーLED搭載** - `LED_BUILTIN` と連動して `setBLEDColor()` で色指定できる LED を両機種に搭載。  
- **オンボードボタン搭載** - `BTN_BUILTIN` で追加部品なしに入力を試せます。  


---

### 留意点

- **EEPROM 容量は ATmega328P / ATmega32U4 のほうが大きい**(1 KB 対 256 B)。
- 多くの不揮発データを保存する用途では保存方法の見直し(User Row やフラッシュの活用)が必要になる場合があります
  (後述「データ記憶領域」参照)。


---

## インストール

### ボードマネージャ経由(推奨)

1. Arduino IDE の **ファイル > 基本設定 > 追加のボードマネージャの URL** に以下を追加します。

   ```
   https://ws-asahi.github.io/UkiUkiCore/package_ukiuki_index.json
   ```

2. **ツール > ボード > ボードマネージャ** で「**UkiUkiCore**」を検索してインストールします。
3. コア本体に加えて、専用ツールチェーン(avr-gcc 15.2.0 / avrdude 8.1)が自動的にダウンロード・設定されます。  
   追加の設定は不要です。

詳しい手順・手動インストール(開発者向け)は [Installation.md](Installation.md) を参照してください。

### 必要環境

- Arduino IDE 1.8.13 以降、または 2.x
- ブートローダーの書き換えには UPDI プログラマ(PICkit 4/5、Atmel-ICE、jtag2updi 等)が必要になります。

> Linux をお使いの場合、Arduino IDE は必ず [arduino.cc](https://www.arduino.cc) 配布版を使用してください。  
> ディストリのパッケージマネージャ版は改変されており、正常に動作しません。  

---

## クイックスタート

1. **ツール > ボード > UkiUkiCore** から **UkiUkiduino** または **UkiUkiduino ProMicro** を選択
2. USB ケーブルで接続し、ボードが接続された COM ポートを選択して書き込み

以下のスケッチは両機種で無改変で動作します(`LED_BUILTIN` / `BTN_BUILTIN` の番号はボードごとに異なりますが、名前で指定すれば共通です)。

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
  setBLEDColor(255, 0, 0); //赤色指定(標準色は黄色)
}
void loop() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(500);
}
```

---

## オンボードフルカラー LED(共通 API)

両機種のオンボード LED は WS2812 系のフルカラー LED で、`LED_BUILTIN` への `digitalWrite()` に連動して点灯・消灯します。  
点灯時の色と明るさは以下の API で変更できます(コア組み込み、include 不要)。

```cpp
setBLEDColor(Yellow);          // 色名で指定(既定の明るさ 40)
setBLEDColor(Yellow, 255);     // 色名 + 明るさ 0〜255
setBLEDColor(r, g, b);         // RGB 値をそのまま表示(明るさスケーリングなし)
```

色名: `Red` `Green` `Blue` `Yellow` `Orange` `Cyan` `Magenta` `Purple` `Pink` `White`  

> 点灯中に呼ぶと即時反映、消灯中に呼ぶと次の点灯から反映されます。  
> `digitalWrite(LED_BUILTIN, ...)` 1 回あたり約 0.33〜0.34 ms の LED 通信が入ります。  
> `BTN_BUILTIN` はプルダウン式で **押すと HIGH** です(一般的な Arduino のプルアップ式とは逆)。  

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

## 書き込み

- 通常は **USB ブートローダ**(USB-CDC / STK500v1)経由で書き込みます。1200 bps タッチで自動的にブートローダへ入ります。
- ブートローダに手動で入るには、RESET ボタンを **ダブルタップ**します(オンボードのフルカラー LED が黄色にブレス点灯します)。
- ブートローダ自体の書き換え(**ツール > ブートローダを書き込む**)には UPDI プログラマが必要です。

---

## ボード / MCU 識別マクロ

| マクロ |  用途 |
|--------|------|
| `ARDUINO_AVR_UKIUKIDUINO` | ボード識別用(UkiUkiduino) |
| `ARDUINO_AVR_UKIUKIDUINO_PROMICRO` | ボード識別用(UkiUkiduino ProMicro) |
| `__AVR_AVR64DU32__` | MCU 識別用 |
| `__AVR_DU__` | 製品グループ `"DU"` 識別用 |

```cpp
#if defined(ARDUINO_AVR_UKIUKIDUINO_PROMICRO)
  // ProMicro 固有の処理
#elif defined(ARDUINO_AVR_UKIUKIDUINO)
  // Uno R3 形 UkiUkiduino 固有の処理
#endif
```

---

## ライセンスとクレジット

UkiUkiCore は [WazamonoCore](https://github.com/ws-asahi/WazamonoCore)（© Workshop Asahi、[DxCore](https://github.com/SpenceKonde/DxCore)（© Spence Konde、LGPL 2.1）からの派生）をベースにした **製品専用フォーク**です。本コアも **LGPL 2.1** で配布されます。

- ベースコア: **DxCore** - © Spence Konde 2021–2022、および各 Arduino コア
- USB スタック・Wazamono 向けカスタマイズ: © Workshop Asahi 2026
- UkiUkiduino 向けボード定義・カスタマイズ: © Unicollabo

ライセンス全文は [LICENSE.md](LICENSE.md) を参照してください。一部のファイル・ライブラリは別ライセンスで提供される場合があり、その旨は各ファイル先頭に記載されています。

