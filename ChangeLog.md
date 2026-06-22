# 変更履歴（Changelog）

WazamonoCore の変更履歴です。WazamonoCore は [DxCore](https://github.com/SpenceKonde/DxCore)（1.6 系）から派生した、Wazamono シリーズ専用の Arduino コアです。

---

## v0.0.2 — Wazamono Tsurugi（ベータ）追加

Arduino Uno R3 後継機 **Wazamono Tsurugi** のソフトウェア対応を追加しました。

### 追加

- **Wazamono Tsurugi** variant（`variants/WazamonoTsurugi`）を追加。AVR64DU32、Uno R3 互換ピン配置（D0–D13、A0–A5、AREF）。
  - 番号付けは Uno R3 標準（D0–D19 連続）。`LED_BUILTIN` = D13（PD6）。
  - シリアル: `Serial` = USB CDC、**`Serial0`** = D0/D1 ハードウェア UART（USART0 ALT1）。USART1 はチップ上に存在するが Tsurugi では使用可能ピンなし。
  - SPI（D11/D12/D13 = SPI0 ALT4、SS=D10）、I2C（A4/A5 = TWI0）を Uno R3 慣例に合わせて配置。
  - PWM: TCA0→PORTD（D5,D6,D9,D10,D11,D12）、TCB0 ALT1→D4。**millis は TCB1** に配置し TCB0 を D4 PWM に開放。
- **Wazamono Tsurugi** を `boards.txt` に追加（VID/PID アプリ `0x1209:0x0008` / ブートローダ `0x1209:0x0007`）。
- ボードドキュメント `extras/WazamonoTsurugi.md`（ATmega328P / Uno R3 との比較を含む）を追加。

### 既知の制限

- Tsurugi の確定 BOM・回路図、および専用ブートローダ hex（`usbcdcboot_wazamonotsurugi.hex`）は準備中です。
- VID/PID は開発用に pid.codes のテスト範囲を使用。製品出荷前に正式な VID/PID へ置き換えが必要。

---

## v0.0.1 — 初版

DxCore をベースに、Wazamono シリーズ専用コアとして再構成した最初のリリースです。

### コア構成

- DxCore（1.6 系）から派生。Wazamono シリーズ以外の MCU・ボードファミリ（DA / DB / DD / EA / EB および汎用 DU ボード等）の定義を削除。
- `boards.txt` を **Wazamono Tachi (AVR64DU32)** 1 機種に整理。製品向けにメニューを固定（チップ選択・各種オプションメニューを削除し、既定値を固定）。
- 「Clock Speed」メニューのみ残置。24 MHz 外部水晶（既定）／内蔵オシレータ（24/20/16 MHz）を選択可能。

### 対応ボード

- **Wazamono 太刀（Tachi）** — Pro Micro 後継、AVR64DU32、USB-C。variant `WazamonoTachi` を追加。
  - シリアル: `Serial` = USB CDC、`Serial1` = USART1（D0/D1、ALT2 固定）、`Serial2` = USART0（D2/D3、ALT2 固定、I2C と排他）。
  - SPI（PA4/PA5/PA6/PA7）、I2C（PA2/PA3）、PWM（TCA0→PORTF の D5–D10、TCB1 の D3）を割り当て。
  - `millis()` / `micros()` は TCB0 を使用し、TCB1（D3）と TCA0（D5–D10）を PWM に開放。
  - `LED_BUILTIN` = D17（PD5、RX LED）。

### USB

- USB 2.0 仕様とデータシート（DS40002548A）のみに基づくクリーンルーム実装の USB スタックを採用。
- USB CDC 仮想シリアル、USB HID、USB-MIDI に対応。
- USB CDC ブートローダ（STK500v1、1200bps タッチ）。リセットボタンのダブルタップでもブートローダへ遷移可能。

### ピン定義

- Wazamono Tachi の variant（`pins_arduino.h`）について、各ペリフェラルのピン割り当てを variant 側で確定。スケッチでの `swap()` 指定を不要化。
- アナログ入力 **A0–A3** を基板シルクに合わせて PD3 / PD2 / PD1 / PD0（D18–D21）へ割り当て。

### 既知の制限・今後の予定

- **Wazamono 剣（Tsurugi）**（Arduino Uno R3 後継）は回路設計中。対応でき次第、variant を追加予定。
- ボードマネージャ（JSON URL）からのインストールは未対応（手動インストールのみ）。
- VID/PID は開発用に pid.codes のテスト範囲（`0x1209:0x0006` / `0x1209:0x0005`）を使用。製品出荷前に正式な VID/PID へ置き換えが必要。
