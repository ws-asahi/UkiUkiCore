# WazamonoCore

**Wazamono（業物）シリーズ専用 Arduino コア**
USB ネイティブな新世代 AVR（AVR DU シリーズ: `AVR64DU32` / `AVR32DU20`）を搭載した Arduino 互換ボード「Wazamono」シリーズのためのボードサポートパッケージ（Arduino core）です。

![platform](https://img.shields.io/badge/platform-AVR%20DU-blue)
![license](https://img.shields.io/badge/license-LGPL--2.1-green)
![version](https://img.shields.io/badge/core-v0.0.3-orange)
![based on](https://img.shields.io/badge/based%20on-DxCore-lightgrey)

Wazamono シリーズは、定番の Arduino 互換ボードを **USB を内蔵した新世代 AVR** で置き換えることを目指したボード群です。
USB-シリアル変換チップを別途搭載せず、マイコン単体で PC と直接つながります。
WazamonoCore は、これらのボードを Arduino IDE で開発するための専用コアで、[DxCore](https://github.com/SpenceKonde/DxCore) をベースに **Wazamono シリーズに必要な部分だけを残して再構成** しています。

> ⚠️ **開発版（0.0.3 系）です。** API・ボード定義・ブートローダは予告なく変更されることがあります。

---

## 対応ボード

| ボード | 由来 | MCU | フォームファクタ | 状態 |
|--------|------|-----|------------------|------|
| [**Wazamono 太刀（Tachi）**](megaavr/extras/WazamonoTachi.md) | Pro Micro 後継 | AVR64DU32 | Pro Micro 互換 / USB-C | 🚧 準備中 |
| [**Wazamono 剣（Tsurugi）**](megaavr/extras/WazamonoTsurugi.md) | Arduino Uno R3 後継 | AVR64DU32 | Uno R3 互換 / USB-C | 🚧 準備中  |
| [**Wazamono 苦無（Kunai）**](megaavr/extras/WazamonoKunai.md) | Seeeduino XIAO 後継 | AVR32DU20 | XIAO 互換 / USB-C | 🚧 準備中  |

> このコアには **Wazamono Tachi**、**Wazamono Tsurugi**、**Wazamono Kunai**の variant が含まれています。

---

## 心臓部 - AVR DU シリーズ

Wazamono シリーズは全機種が USB を内蔵した新世代 AVR「**AVR DU**」を採用しています。
USB-シリアル変換チップなしで PC と直接通信できることが最大の特長です。
Tachi / Tsurugi は **AVR64DU32**、小型の Kunai は **AVR32DU20** を搭載します。

| 項目 | AVR64DU32（Tachi / Tsurugi） | AVR32DU20（Kunai） |
|------|------------------------------|--------------------|
| Flash | 64 KB | 32 KB |
| SRAM | 8 KB | 4 KB |
| EEPROM | 256 B | 256 B |
| USERROW | 512 B | 512 B |
| 最大動作周波数 | 24 MHz | 24 MHz |
| **USB** | USB 2.0 Full-Speed **デバイス** | USB 2.0 Full-Speed **デバイス** |
| ADC | 10-bit 170 ksps × 1（21 チャネル） | 10-bit 170 ksps × 1（ピン数で制限） |
| タイマ | TCA0 ×1、TCB ×2 | TCA0 ×1、TCB ×2 |
| USART / SPI / I2C | 2 / 1 / 1 | 2 / 1 / 1 |
| CCL（LUT）/ EVSYS / AC | 4 / 6ch / 1 | 同等（周辺ブロックは共通） |
| パッケージ | 32 ピン（VQFN） | 20 ピン（VQFN） |

<sub>諸元はデータシート DS40002548A（AVR64DU32）/ DS40002576（AVR16/32DU ファミリ）に基づく。
両者はピン数・Flash・SRAM が主な差で、周辺機能ブロックは共通です</sub>

---

## データ記憶領域

AVR DU は用途の異なる複数の不揮発メモリ領域を持ちます。
旧 ATmega との大きな違いは、EEPROM が 256 B に減少した一方で、**チップ消去でも消えない USERROW（512 B）** など新しい領域が追加されている点です。
容量・構成は全対応ボード（AVR64DU32 / AVR32DU20）で共通です。

| 領域 | 容量 | 消去単位 | 書き換え耐久 | チップ消去で | 対応ライブラリ |
|------|------|----------|--------------|--------------|----------------|
| EEPROM | 256 B | バイト（1–32 B） | 10 万回 | 消える（EESAVE ヒューズで保持可） | `EEPROM.h` |
| USERROW | 512 B | 512 B ページ一括 | 1,000 回 | **残る** | `USERSIG.h` |
| Flash（APPDATA） | スケッチ領域の空き | 512 B ページ一括 | 1,000 回 | 消える | `Flash.h` |
| SIGROW | 読み取り専用 | — | — | — | 工場書き込みの 16 B 個体シリアル番号を含む |

**使い分けの目安:** 頻繁に書き換えるデータは耐久 10 万回の **EEPROM**、
シリアル番号・校正値など滅多に変えない製造データはリフラッシュ後も残る **USERROW**、
大きなデータの保存は **Flash ライブラリ**を使用してください。
USERROW / Flash はフラッシュ型（ページ一括消去・耐久 1,000 回）のため EEPROM の完全な代替にはなりません。
詳細は各ボードのドキュメント「データ記憶領域」を参照してください。

<sub>仕様はデータシート DS40002548A / DS40002576 に基づく。</sub>

---

## 特長

- **基礎性能の向上** - 動作クロック1.5倍、命令処理の高速化、
- **記憶領域の刷新** - 新しく USERROW / Flash などの記憶領域が追加（一方 EEPROM 容量は 256 B へ減少）
- **USB ネイティブ** - 追加の USB-シリアル変換チップが不要。`Serial` がそのまま USB仮想シリアルポートになります。
- **USB ブートローダ** - USB-CDC（STK500v1）経由でスケッチを書き込み。
- **HID / MIDI 対応** - USB キーボード・マウス等の HID、および USB-MIDI をサポート。
- **高い互換性** - 同系統の MCU を採用しているため UnoR3 や Leonardo / ProMicro のコードをほぼそのまま実装可能（一部ピン配列の変更や未対応のライブラリあり）。
- **各ピンの出力能力** - ピンあたりの電流出力は 20 mA を維持しており、UnoR4 の 8 mA では動かせない外部機器も動作可能。
- **全ピンアナログ入力対応** - 全てのデジタル入出力ピンでアナログ値の読取りが可能。
- **7系統の PWM 出力** - UnoR3 では 6 本だった PWM を 7 本に拡張。
- **UPDI 対応** - UPDI デバッガーなどで動作中の MCU にアクセス可能。
- **ネイティブ avr-gcc 対応** - DxCore と異なる点として最新の avr-gcc コンパイラを使用（今後も順次更新されます）。
- **64 bit 浮動小数点演算対応** - プログラムサイズは大きくなりますが 64 bit 浮動小数を扱えます（long doubleとして実装）。

---

## インストール

### ボードマネージャ経由（推奨）

1. Arduino IDE の **ファイル > 基本設定 > 追加のボードマネージャの URL** に以下を追加します。

   ```
   https://wazamono.ws-asahi.net/package_wazamono_index.json
   ```

2. **ツール > ボード > ボードマネージャ** で「**Wazamono**」を検索し、
   **Wazamono Boards (AVR DU series)** をインストールします。
3. コア本体に加えて、専用ツールチェーン（avr-gcc 15.2.0 / avrdude 8.1）が
   自動的にダウンロード・設定されます。追加の設定は不要です。

詳しい手順・手動インストール（開発者向け）は [Installation.md](Installation.md) を参照してください。

### 必要環境

- Arduino IDE 1.8.13 以降、または 2.x
- ブートローダーの書き換えにはUPDI プログラマ（PICkit 4/5、Atmel-ICE、jtag2updi 等）が必要になります。

> Linux をお使いの場合、Arduino IDE は必ず [arduino.cc](https://www.arduino.cc) 配布版を使用してください。ディストリのパッケージマネージャ版は改変されており、正常に動作しません。

---

## クイックスタート

1. **ツール > ボード > WazamonoCore** から **Wazamono Tachi (AVR64DU32)** を選択
2. USB ケーブルで接続し、書き込み

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

**USB シリアル:**
```cpp
void setup() {
  Serial.begin(115200);          // Serial = USB CDC（変換チップ不要）
}
void loop() {
  Serial.println(millis());
  delay(1000);
}
```

---

## 書き込み（ブートローダ経由）

1. ボードを USB で接続します。
2. Arduino IDE からスケッチを書き込みます。書き込み開始時に **1200bps タッチ**が行われ、自動的にブートローダへ遷移します。
3. 自動遷移しない場合は、**リセットボタンのダブルタップ**でブートローダに入れます。

---

## ライセンスとクレジット

WazamonoCore は [DxCore](https://github.com/SpenceKonde/DxCore)（© Spence Konde、LGPL 2.1）から派生した **製品専用フォーク**です。本コアも **LGPL 2.1** で配布されます。

- ベースコア: **DxCore** - © Spence Konde 2021–2022、および各 Arduino コア
- Wazamono 向けカスタマイズ・USB スタック・ボード定義: © Workshop Asahi 2026
- 「Wazamono（業物）」「Tachi（太刀）」「Tsurugi（剣）」「Kunai（苦無）」は Workshop Asahi の製品名です。

ライセンス全文は [LICENSE.md](LICENSE.md) を参照してください。一部のファイル・ライブラリは別ライセンスで提供される場合があり、その旨は各ファイル先頭に記載されています。
