# UkiUkiCore

**UkiUkiduino（うきうきでゅいーの）専用 Arduino コア**
USB ネイティブな新世代 AVR（`AVR64DU32`）を搭載した Arduino Uno R3 互換ボード「UkiUkiduino」のためのボードサポートパッケージ（Arduino core）です。
UkiUkiduino は VTuber「浮々ゆにこ」公式ファングッズとして開発されたボードです。

![platform](https://img.shields.io/badge/platform-AVR%20DU-blue)
![license](https://img.shields.io/badge/license-LGPL--2.1-green)
![version](https://img.shields.io/badge/core-v0.0.1-orange)
![based on](https://img.shields.io/badge/based%20on-WazamonoCore%20%2F%20DxCore-lightgrey)

UkiUkiduino は、定番の Arduino Uno R3 を **USB を内蔵した新世代 AVR** で置き換えたボードです。
USB-シリアル変換チップを別途搭載せず、マイコン単体で PC と直接つながります。
UkiUkiCore は、このボードを Arduino IDE で開発するための専用コアで、[WazamonoCore](https://github.com/ws-asahi/WazamonoCore)（[DxCore](https://github.com/SpenceKonde/DxCore) ベース）から **UkiUkiduino に必要な部分だけを残して再構成** しています。

> ⚠️ **開発版（v0.0.1）です。** API・ボード定義・ブートローダは予告なく変更されることがあります。

---

## 対応ボード

| ボード | 由来 | MCU | フォームファクタ | 状態 |
|--------|------|-----|------------------|------|
| [**UkiUkiduino**](megaavr/extras/UkiUkiduino.md) | Arduino Uno R3 互換 | AVR64DU32 | Uno R3 互換 / USB-C | 🚧 開発中 |

> このコアには **UkiUkiduino** の variant が含まれています。

---

## 心臓部 - AVR DU シリーズ

UkiUkiduino は USB を内蔵した新世代 AVR「**AVR64DU32**」を採用しています。
USB-シリアル変換チップなしで PC と直接通信できることが最大の特長です。

| 項目 | AVR64DU32 |
|------|-----------|
| Flash | 64 KB |
| SRAM | 8 KB |
| EEPROM | 256 B |
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
- **ほぼ全ピンアナログ入力対応** - D7・D20 を除く全てのデジタル入出力ピンでアナログ値の読取りが可能。
- **7 系統の PWM 出力** - Uno R3 では 6 本だった PWM を 7 本に拡張
- **AREF 対応** - Uno R3 と同様に外部基準電圧（VREFA）を使用可能
- **オンボードボタン搭載** - BTN_BUILTIN（D20）で追加部品なしに入力を試せます
- **UPDI 対応** - Power ヘッダの 1 番ピンから UPDI デバッガーで動作中の MCU にアクセス可能
- **ネイティブ avr-gcc 対応** - DxCore と異なる点として最新の avr-gcc コンパイラを使用（今後も順次更新されます）。

---

## インストール

詳しい手順は [Installation.md](Installation.md) を参照してください。

### 手動インストール（hardware フォルダ）

1. このリポジトリを clone するか、ZIP をダウンロードして展開します。
2. スケッチブックの `hardware` フォルダに、フォルダ名を **`UkiUkiCore`** として配置します。

   ```
   <スケッチブック>/hardware/UkiUkiCore/megaavr/...
   ```

   - Windows 例: `ドキュメント\Arduino\hardware\UkiUkiCore\`
   - macOS / Linux 例: `~/Documents/Arduino/hardware/UkiUkiCore/`

3. Arduino IDE を再起動します。

> ボードマネージャ（JSON URL）からのインストールは今後対応予定です。

### 必要環境

- Arduino IDE 1.8.13 以降、または 2.x
- ブートローダーの書き換えには UPDI プログラマ（PICkit 4/5、Atmel-ICE、jtag2updi 等）が必要になります。

> Linux をお使いの場合、Arduino IDE は必ず [arduino.cc](https://www.arduino.cc) 配布版を使用してください。ディストリのパッケージマネージャ版は改変されており、正常に動作しません。

---

## クイックスタート

1. **ツール > ボード > UkiUkiCore** から **UkiUkiduino (AVR64DU32)** を選択
2. **クロック** は通常「24 MHz internal (default)」のまま
3. USB ケーブルで接続し、書き込み

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

<sub>開発用 VID/PID は pid.codes のテスト範囲を使用しています。製品出荷までに pid.codes から正式な VID/PID を取得し置き換えます。</sub>

---

## 動作検証済みのライブラリ

UkiUkiCore（AVR64DU32）上で **動作を確認した** 主要なサードパーティライブラリです。
※ただしそのままでは動作せず若干の変更が必要になります。

| ライブラリ | 用途 | 備考 |
|------------|------|------|
| [HID-Project](https://github.com/NicoHood/HID)（NicoHood） | USB HID（Keyboard / Mouse / Gamepad / RawHID / BootKeyboard / System など） | 各機能の動作を確認（BootKeyboard の `getLeds`、System HID のリモートウェイクアップ含む）。AVR-DU に存在しない 32U4 固有レジスタ `UEDATX` を参照する箇所があり、ガードを追加する小修正が必要（上流 NicoHood/HID へ [PR #472](https://github.com/NicoHood/HID/pull/472) 提出済み）。 |
| [MIDIUSB](https://github.com/arduino-libraries/MIDIUSB)（Arduino） | USB-MIDI（MIDI メッセージの送受信） | RX / TX 双方向を MIDI-OX で確認。単体構成および IAD ベースのコンポジット構成で動作。`megaavr` アーキテクチャ対応の修正が必要（上流 arduino-libraries/MIDIUSB へ [PR #132](https://github.com/arduino-libraries/MIDIUSB/pull/132) 提出済み・CLA 署名済み）。 |

---

## 移植不可能なライブラリ

UkiUkiCore（AVR-DU）へ **移植できないことが確認された** サードパーティライブラリを記録します。
更新が停止しており、かつクラシック AVR 固有のレジスタに密結合しているなど、AVR-DU への対応追加（または上流への PR）が現実的でないものをここに挙げます。
今後、同様に確認されたものを追記していきます。

> **掲載基準**: メンテナンスが停止しており、かつ AVR-DU の現行ペリフェラル API では対応の追加・移植が困難なもの。代替が存在する場合は併記します。

| ライブラリ | 状態 | 移植不可能な理由 | 推奨代替 |
|------------|------|------------------|----------|
| [analogComp](https://github.com/leomil72/analogComp)（leomil72） | 更新停止（v1.2.4 / 2018年） | クラシック AVR の `ACSR` / `ADCSRB` / `ADMUX` レジスタ前提の実装で、AVR-DU の AC0（`AC0.CTRLA` / `MUXCTRL` / `DACREF` / `STATUS`）とは構造が全く異なる。`analogComp.h` が対応 MCU をマクロでゲートしており、非対応部品ではビルド自体が通らない。UNO R4 等の非クラシック系も非対応で、作者も約 8 年非活動のため上流 PR も現実的でない。 | **Comparator** ライブラリ（DxCore / UkiUkiCore 同梱・MCUdude 作） |

> アナログコンパレータ（AC）・CCL・EVSYS の各ペリフェラル **自体** は、DxCore 由来の **Comparator / Logic / Event** ライブラリで利用できます。ここに挙げているのは、それらを置き換えようとして移植不可能と判明した特定のサードパーティライブラリのみです。

---

## ライセンスとクレジット

UkiUkiCore は [WazamonoCore](https://github.com/ws-asahi/WazamonoCore)（© Workshop Asahi、[DxCore](https://github.com/SpenceKonde/DxCore)（© Spence Konde、LGPL 2.1）からの派生）をベースにした **製品専用フォーク**です。本コアも **LGPL 2.1** で配布されます。

- ベースコア: **DxCore** - © Spence Konde 2021–2022、および各 Arduino コア
- USB スタック・Wazamono 向けカスタマイズ: © Workshop Asahi 2026
- UkiUkiduino 向けボード定義・カスタマイズ: © Workshop Asahi 2026
- 「UkiUkiduino」は Workshop Asahi の製品名です。「浮々ゆにこ」に関する名称・キャラクターの権利は権利者に帰属します。

ライセンス全文は [LICENSE.md](LICENSE.md) を参照してください。一部のファイル・ライブラリは別ライセンスで提供される場合があり、その旨は各ファイル先頭に記載されています。
