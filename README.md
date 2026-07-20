# UkiUkiCore

**UkiUkiduino専用 Arduino コア**
USB ネイティブな新世代 AVR（`AVR64DU32`）を搭載した Arduino Uno R3 互換ボード「UkiUkiduino」のためのボードサポートパッケージ（Arduino core）です。
UkiUkiduino は VTuber「浮々ゆにこ」のファングッズとして開発されたボードです。

![platform](https://img.shields.io/badge/platform-AVR%20DU-blue)
![license](https://img.shields.io/badge/license-LGPL--2.1-green)
![version](https://img.shields.io/badge/core-v0.0.1-orange)
![based on](https://img.shields.io/badge/based%20on-WazamonoCore%20%2F%20DxCore-lightgrey)

UkiUkiduino は、定番の Arduino Uno R3 を **USB を内蔵した新世代 AVR** で置き換えたボードです。
USB-シリアル変換チップを別途搭載せず、マイコン単体で PC と直接つながります。
UkiUkiCore は、このボードを Arduino IDE で開発するための専用コアで、[WazamonoCore](https://github.com/ws-asahi/WazamonoCore)から **UkiUkiduino に必要な部分だけを残して再構成** しています。

> ⚠️ **開発版（v0.0.3）です。** API・ボード定義・ブートローダは予告なく変更されることがあります。

---

## 対応ボード

| ボード | 由来 | MCU | フォームファクタ | 状態 |
|--------|------|-----|------------------|------|
| [**UkiUkiduino**](megaavr/extras/UkiUkiduino.md) | Arduino Uno R3 互換 | AVR64DU32 | Uno R3 互換 | 🚧 開発中 |

---

## 心臓部 - AVR DU シリーズ

UkiUkiduino は USB を内蔵した新世代 AVR「**AVR64DU32**」を採用しています。
USB-シリアル変換チップなしで PC と直接通信できることが最大の特長です。

| 項目 | AVR64DU32 |
|------|-----------|
| Flash | 64 KB |
| SRAM | 8 KB |
| EEPROM | 256 B |
| USERROW | 512 B |
| 最大動作周波数 | 24 MHz |
| **USB** | USB 2.0 Full-Speed **デバイス** |
| ADC | 10-bit 170 ksps × 1（21 チャネル） |
| タイマ | TCA0 ×1、TCB ×2 |
| USART / SPI / I2C | 2 / 1 / 1 |
| CCL（LUT）/ EVSYS / AC | 4 / 6ch / 1 |
| パッケージ | 32 ピン（TQFP-32） |

<sub>諸元はデータシート DS40002548A（AVR64DU32）に基づく。</sub>

---

## 特長

- **基礎性能の向上** - 動作クロック 1.5 倍、プログラム容量約 2 倍（※EEPROM 容量は 1KB から 256B へ減少）
- **USB ネイティブ** - 追加の USB-シリアル変換チップが不要。`Serial` がそのまま USB 仮想シリアルポートになります。
- **USB ブートローダ** - USB-CDC（STK500v1）経由でスケッチを書き込み。**1200bps タッチ**でブートローダへ自動遷移するため、Leonardo / ProMicro と同じ手順で書き込めます。
- **HID / MIDI 対応** - USB キーボード・マウス等の HID、および USB-MIDI をサポート。
- **高い互換性** - Uno R3 と同じピン配置・番号付けのため、既存のコードをほぼそのまま実装可能（一部ピン機能の変更や未対応のライブラリあり）。
- **各ピンの出力能力** - ピンあたりの電流出力は 20mA を維持しており、Uno R4 の 8mA では動かせない外部機器も動作可能。
- **全ピンアナログ入力対応** - 全てのデジタル入出力ピン（D0–D19）でアナログ値の読取りが可能。
- **7 系統の PWM 出力** - Uno R3 では 6 本だった PWM を 7 本に拡張
- **オンボードボタン搭載** - BTN_BUILTIN（D20）で追加部品なしに入力を試せます
- **フルカラーLED搭載** - LED_BUILTIN(D13)と連動してRGB色指定が可能なLEDが動作します。
- **UPDI 対応** - Power ヘッダの 1 番ピンから UPDI デバッガーで動作中の MCU にアクセス可能
- **ネイティブ avr-gcc 対応** - DxCore と異なる点として最新の avr-gcc コンパイラを使用（今後も順次更新されます）。


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

> Linux をお使いの場合、Arduino IDE は必ず [arduino.cc](https://www.arduino.cc) 配布版を使用してください。ディストリのパッケージマネージャ版は改変されており、正常に動作しません。

---

## クイックスタート

1. **ツール > ボード > ** から **UkiUkiduino** を選択
2. USB ケーブルで接続し、 UkiUkiduino が接続された COM ポートを選択して書き込み


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

## ライセンスとクレジット

UkiUkiCore は [WazamonoCore](https://github.com/ws-asahi/WazamonoCore)（© Workshop Asahi、[DxCore](https://github.com/SpenceKonde/DxCore)（© Spence Konde、LGPL 2.1）からの派生）をベースにした **製品専用フォーク**です。本コアも **LGPL 2.1** で配布されます。

- ベースコア: **DxCore** - © Spence Konde 2021–2022、および各 Arduino コア
- USB スタック・Wazamono 向けカスタマイズ: © Workshop Asahi 2026
- UkiUkiduino 向けボード定義・カスタマイズ: © Unicollabo
- 「UkiUkiduino」は [VEE](https://vee-official.jp) の製品名です。「浮々ゆにこ」に関する名称・キャラクターの権利は VEE に帰属します。

ライセンス全文は [LICENSE.md](LICENSE.md) を参照してください。一部のファイル・ライブラリは別ライセンスで提供される場合があり、その旨は各ファイル先頭に記載されています。
