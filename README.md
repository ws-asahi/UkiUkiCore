# WazamonoCore

**Wazamono（業物）シリーズ専用 Arduino コア**
USB ネイティブな新世代 AVR `AVR64DU32` を搭載した Arduino 互換ボード「Wazamono」シリーズのためのボードサポートパッケージ（Arduino core）です。

![platform](https://img.shields.io/badge/platform-AVR64DU32-blue)
![license](https://img.shields.io/badge/license-LGPL--2.1-green)
![version](https://img.shields.io/badge/core-v0.0.1-orange)
![based on](https://img.shields.io/badge/based%20on-DxCore-lightgrey)

Wazamono シリーズは、定番の Arduino 互換ボードを **USB を内蔵した新世代 AVR** で置き換えることを目指したボード群です。USB-シリアル変換チップを別途搭載せず、マイコン単体で PC と直接つながります。WazamonoCore は、これらのボードを Arduino IDE で開発するための専用コアで、[DxCore](https://github.com/SpenceKonde/DxCore) をベースに **Wazamono シリーズに必要な部分だけを残して再構成** しています（他の AVR ファミリ向けの定義は削除済み）。

> ⚠️ **開発版（v0.0.1）です。** API・ボード定義・ブートローダは予告なく変更されることがあります。

---

## 対応ボード

| ボード | 由来 | MCU | フォームファクタ | 状態 |
|--------|------|-----|------------------|------|
| **Wazamono 太刀（Tachi）** | Arduino Pro Micro 後継 | AVR64DU32 | Pro Micro 互換 / USB-C | ✅ 対応済み |
| **Wazamono 剣（Tsurugi）** | Arduino Uno R3 後継 | AVR64DU32 | Uno R3 互換 | 🚧 開発中 |

> 現在このコアに含まれているボード定義（variant）は **Wazamono Tachi** です。Tsurugi は回路設計が進行中で、対応でき次第このコアに追加します。

---

## 共通の心臓部 — AVR64DU32

Wazamono シリーズは全機種が **AVR64DU32** を採用しています。USB を内蔵した数少ない AVR で、PC と直接通信できることが最大の特長です。

| 項目 | AVR64DU32 |
|------|-----------|
| Flash | 64 KB |
| SRAM | 8 KB |
| EEPROM | 256 B |
| 最大動作周波数 | 24 MHz |
| **USB** | USB 2.0 Full-Speed **デバイス**（16 エンドポイントアドレス／最大 32 エンドポイント） |
| ADC | 10-bit 170 ksps × 1（32 ピン版で 21 チャネル） |
| DAC | なし（AC 内部の DACREF のみ） |
| タイマ | TCA0 ×1（PWM 3ch）、TCB ×2 ／ **TCD は USB が占有するため非搭載** |
| USART | 2（USART0 / USART1） |
| SPI / TWI(I2C) | 各 1 |
| CCL（LUT） | 4 |
| イベントシステム | 6 チャネル |
| アナログコンパレータ（AC） | 1 |
| OPAMP | なし |
| パッケージ | 32 ピン（TQFP 7×7 / VQFN 5×5） |

<sub>諸元はデータシート DS40002548A（AVR64DU32）に基づく。AVR16DU32 / AVR32DU32 は Flash・SRAM のみ異なります。</sub>

---

## 特長

- **USB ネイティブ** — 追加の USB-シリアル変換チップが不要。`Serial` がそのまま USB CDC 仮想シリアルポートになります。
- **USB ブートローダ** — USB CDC（STK500v1）経由でスケッチを書き込み。**1200bps タッチ**でブートローダへ自動遷移するため、Arduino Leonardo / Pro Micro と同じ手順で書き込めます。
- **HID / MIDI 対応** — USB キーボード・マウス等の HID、および USB-MIDI をサポート。
- **`swap()` 指定不要** — Wazamono の各ボードは UART のピン割り当てが variant 側で確定済み。スケッチで `Serial1.swap()` 等を呼ぶ必要はありません。
- **製品専用に最小化** — DxCore から Wazamono シリーズ以外のボード／MCU 定義を取り除き、メニューを製品向けに固定。迷わず選べます。

---

## インストール

詳しい手順は [Installation.md](Installation.md) を参照してください。

### 手動インストール（hardware フォルダ）

1. このリポジトリを clone するか、ZIP をダウンロードして展開します。
2. スケッチブックの `hardware` フォルダに、フォルダ名を **`WazamonoCore`** として配置します。

   ```
   <スケッチブック>/hardware/WazamonoCore/megaavr/...
   ```

   - Windows 例: `ドキュメント\Arduino\hardware\WazamonoCore\`
   - macOS / Linux 例: `~/Documents/Arduino/hardware/WazamonoCore/`

3. Arduino IDE を再起動します。

> ボードマネージャ（JSON URL）からのインストールは今後対応予定です。

### 必要環境

- Arduino IDE 1.8.13 以降、または 2.x
- 初回のみ、UPDI プログラマ（PICkit 4/5、Atmel-ICE、jtag2updi 等）でブートローダの書き込みが必要になる場合があります。

> Linux をお使いの場合、Arduino IDE は必ず [arduino.cc](https://www.arduino.cc) 配布版を使用してください。ディストリのパッケージマネージャ版は改変されており、正常に動作しません。

---

## クイックスタート

1. **ツール > ボード > WazamonoCore** から **Wazamono Tachi (AVR64DU32)** を選択
2. **クロック** は通常「24 MHz external crystal (default)」のまま
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

## シリアルポート構成（Wazamono Tachi）

Arduino Leonardo / Pro Micro と同じ「**`Serial` = USB**」方式です。

| オブジェクト | 実体 | ピン | 用途 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ（仮想 COM ポート） |
| `Serial1` | USART1 | D0(RX) / D1(TX) | Pro Micro 互換のハードウェア UART |
| `Serial2` | USART0 | D2(SDA) / D3(SCL) | 予備 UART（Grove I2C とピン共有・**排他利用**） |

> `Serial2` は Grove I2C（`Wire`）とピンを共有します。どちらか一方のみ使用できます。

---

## 主要ペリフェラルのピン（Wazamono Tachi）

| 機能 | ピン |
|------|------|
| I2C（Grove） | SDA = D2(PA2) / SCL = D3(PA3) |
| SPI | MOSI = D16(PA4) / MISO = D14(PA5) / SCK = D15(PA6) / SS = D4(PA7) |
| PWM | D5–D10（TCA0）、D3（TCB1） |
| アナログ入力 | A0–A3、A6–A10、追加チャネル A30–A38 |
| LED_BUILTIN | D17（オンボード RX LED） |

ピン配置の詳細は各ボードのドキュメントを参照してください。

- [Wazamono Tachi ボードドキュメント](megaavr/extras/WazamonoTachi.md)

---

## 書き込み（ブートローダ経由）

1. ボードを USB で接続します。
2. Arduino IDE からスケッチを書き込みます。書き込み開始時に **1200bps タッチ**が行われ、自動的にブートローダへ遷移します。
3. 自動遷移しない場合は、**リセットボタンのダブルタップ**でブートローダに入れます。

<sub>開発用 VID/PID は pid.codes のテスト範囲（アプリ: `0x1209:0x0006`）を使用しています。製品出荷前に正式な VID/PID へ置き換えてください。</sub>

---

## ライセンスとクレジット

WazamonoCore は [DxCore](https://github.com/SpenceKonde/DxCore)（© Spence Konde、LGPL 2.1）から派生した **製品専用フォーク**です。本コアも **LGPL 2.1** で配布されます。

- ベースコア: **DxCore** — © Spence Konde 2021–2022、および各 Arduino コア
- Wazamono 向けカスタマイズ・USB スタック・ボード定義: © Workshop Asahi 2026
- 「Wazamono」「太刀」「剣」は Workshop Asahi の製品名です。

ライセンス全文は [LICENSE.md](LICENSE.md) を参照してください。一部のファイル・ライブラリは別ライセンスで提供される場合があり、その旨は各ファイル先頭に記載されています。
