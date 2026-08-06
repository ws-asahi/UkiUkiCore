# Wazamono 太刀（Tachi）

**Pro Micro 後継機 — AVR64DU28 / USB-C**

Wazamono Tachi は、SparkFun Pro Micro と同じフォームファクタを USB ネイティブな AVR `AVR64DU28` で再設計したボードです（rev.3 で 32 ピンの AVR64DU32 から 28 ピンの AVR64DU28 へ変更し、外部水晶を撤去しました）。
USB-シリアル変換チップを搭載せず、マイコン単体で USB-C により PC と直接つながります。

> このページは Wazamono Tachi 1 機種のドキュメントです。コア全体の概要は [README](../../README.md) を参照してください。

---

## 概要

| 項目 | 内容 |
|------|------|
| 由来 | Pro Micro 後継 |
| MCU | AVR64DU28（28 ピン） |
| フォームファクタ | Pro Micro 互換（L 側 1×12 / R 側 1×12 ピンヘッダ） |
| USB | USB-C（USB 2.0 Full-Speed、マイコン内蔵） |
| クロック | 内蔵 OSCHF 24 MHz 固定（水晶レス。USB は PLL48M + SOF 自動チューニング） |
| 電源 | USB 5V、または RAW 入力（基板上 LDO Torex XC6702 で 5V 生成）。VCC は JP1 で 5V / 3.3V を選択 |
| 書き込み | USB CDC ブートローダ（STK500v1、1200bps タッチ） |

---

## ボード諸元（AVR64DU28）

| 項目 | 値 |
|------|----|
| Flash | 64 KB（うちスケッチ用 60 KB/USB ブートローダ 4 KB） |
| SRAM | 8 KB |
| EEPROM | 256 B |
| USERROW | 512 B |
| 最大動作周波数 | 24 MHz |
| USB | USB 2.0 Full-Speed デバイス（16 エンドポイントアドレス/最大 32 エンドポイント） |
| ADC | 10-bit 170 ksps × 1（17 チャネル） |
| DAC | なし（AC 内部の DACREF のみ） |
| タイマ | TCA0 ×1（PWM 3ch）、TCB ×2 / TCD は USB が占有のため非搭載 |
| USART | 2（USART0 / USART1） |
| SPI / TWI(I2C) | 各 1 |
| CCL（LUT） | 4 |
| イベントシステム | 6 チャネル |
| アナログコンパレータ（AC） | 1 |
| OPAMP | なし |

<sub>諸元はデータシート DS40002548A（AVR64DU28/32）に基づく。スケッチ用 Flash サイズ・SRAM サイズは boards.txt の設定値。</sub>

---

## ATmega32U4 との比較

Wazamono Tachi が置き換える Pro Micro / Arduino Leonardo は **ATmega32U4** を搭載しています。
両者とも USB 内蔵 AVR ですが、AVR64DU28 は新世代の **AVRxt コア**で、クロック・メモリ・周辺機能が大きく強化されています。

| 項目 | Wazamono Tachi (AVR64DU28) | Pro Micro 等 (ATmega32U4) |
|------|----------------------------|----------------------------|
| コア | AVRxt（命令タイミング改善） | 旧来 AVR |
| 最大クロック | 24 MHz（1.8–5.5V 全域） | 16 MHz（4.5V 以上）/3.3V では通常 8 MHz |
| Flash | 64 KB | 32 KB |
| SRAM | 8 KB | 2.5 KB |
| EEPROM | 256 B | 1 KB |
| USERROW | 512 B | - |
| ADC | 10-bit・17 ch・170 ksps | 10-bit・12 ch |
| タイマ | 16-bit TCA ×1 + TCB ×2 | 8/16/16/10-bit ×4 |
| USART | 2 | 1 |
| SPI / I2C | 各 1 | 各 1 |
| CCL（論理ブロック） | 4 LUT | なし |
| イベントシステム | 6 ch | なし |
| USB | Full-Speed デバイス（16 EP アドレス） | Full-Speed デバイス（6 EP） |

### 性能上の主な利点

- **クロックと処理速度** — 24 MHz 動作（ATmega32U4は 16 MHz）に加え、AVRxt コアは一部命令のタイミングが改善されており、同一クロックでもわずかに高速です。
- **複数電圧への対応** — AVRxtのコア特性を活かして 5V または 3.3V の切り替えを**周辺部品の変更なしに可能**です。3.3V 動作時、ATmega32U4 は通常 8 MHz に制限されるのに対し、AVR64DU28 は全電圧範囲で 24 MHz を維持できるため、3.3V では最大で約 3 倍の差になります。
- **メモリ** — Flash 2 倍（64 KB）、SRAM 約 3.2 倍（8 KB）。大きなバッファ、USB 複合デバイス、ライブラリを多用するスケッチで余裕が生まれます。
- **新世代の周辺機能** — CCL（3 つの論理ブロック）とイベントシステム（3 チャネル）により、CPU を介さないハードウェアレベルの信号処理・自動ルーティングが可能です。ATmega32U4 にはいずれもありません。
（Tachi rev.3 では LUT0/LUT1/LUT2 の出力がピンに接続され、LUT0/LUT2 は入力ピンも利用可能です。LUT3 は入力 2 本のみで、結果はイベント/リンク経由で使用します。イベント出力は 2 系統＝EVOUTA/D が使用可能です）
- **アナログ** — ADC チャネルが 12 → 17 に増加し、全チャネルがアナログ入力に対応します。
- **追加のUART** — 2系統のUARTシリアル通信を利用可能です。

### 留意点

- **EEPROM 容量は ATmega32U4 のほうが大きい**（1 KB 対 256 B）。多くの不揮発データを EEPROM に保存する用途では、保存方法の見直し（User Row やフラッシュの活用など）が必要になる場合があります（後述「データ記憶領域」参照）。

---

## データ記憶領域

AVR64DU28 には用途の異なる複数の不揮発メモリ領域があります。
ATmega と比べて EEPROM は小さくなりましたが（256 B）、代わりに **USERROW（使用者列）** などの新しい領域が使えます。

| 領域 | 容量 | 消去単位 | 書き換え耐久 | チップ消去（再書き込み）で | 対応ライブラリ |
|------|------|----------|--------------|----------------------------|----------------|
| EEPROM | 256 B | バイト（1–32 B） | 10 万回 | 消える（EESAVE ヒューズで保持可） | `EEPROM.h` |
| USERROW | 512 B | 512 B ページ一括 | 1,000 回 | **残る** | `USERSIG.h` |
| Flash（APPDATA） | スケッチ領域の空き | 512 B ページ | 1,000 回 | 消える | `Flash.h` |
| SIGROW | 読み取り専用 | — | — | — | 工場書き込みの 16 B 個体シリアル番号を含む |

<sub>各領域の仕様・耐久回数はデータシート DS40002548A（§8 Memories/§11 NVMCTRL/電気的特性）に基づく。</sub>

---

## ピンマッピング

論理ピン番号（`D#`）と MCU ピン、機能の対応です（rev.3 / AVR64DU28）。Pro Micro と同様に D11 以降が欠番になります（本家の D11–D12 に加え、旧版で LED に割り当てていた D13 も rev.3 では欠番）。

| D# | MCU | アナログ別名 | ADC ch | 主な機能 |
|----|-----|--------------|--------|----------|
| D0 | PA5 | A12 | AIN25 | **Serial1 RX**（USART0 ALT1） |
| D1 | PA4 | A13 | AIN24 | **Serial1 TX**（USART0 ALT1） |
| D2 | PA2 | A14 | AIN22 | **I2C SDA** |
| D3 | PA3 | A15 | AIN23 | **I2C SCL**/~PWM(TCB1)/CCL(LUT0-OUT) |
| D4 | PF1 | A6 | AIN17 | 汎用 I/O/CCL(LUT3-IN1) |
| D5 | PD0 | A16 | AIN0 | ~PWM(TCA0 WO0)/CCL(LUT2-IN0) |
| D6 | PD1 | A7 | AIN1 | ~PWM(TCA0 WO1)/CCL(LUT2-IN1) |
| D7 | PA1 | — | — | 汎用 I/O/CCL(LUT0-IN1)（ADC なし） |
| D8 | PC3 | A8 | AIN31 | CCL(LUT1-OUT) |
| D9 | PD2 | A9 | AIN2 | ~PWM(TCA0 WO2)/CCL(LUT2-IN2)/AC0 AINP0/EVOUTD |
| D10 | PD3 | A10 | AIN3 | ~PWM(TCA0 WO3)/CCL(LUT2-OUT)/AC0 AINN0 |
| D14 | PD5 | A18 | AIN5 | SPI **MISO**/~PWM(TCA0 WO5) |
| D15 | PD6 | A19 | AIN6 | SPI **SCK**/Serial2 TX（USART1 ALT2） |
| D16 | PD4 | A20 | AIN4 | SPI **MOSI**/~PWM(TCA0 WO4) |
| D17 | PA0 | — | — | **LED_BUILTIN**（= RX LED、Active-LOW、ヘッダなし）/CCL(LUT0-IN0)（ADC なし） |
| D18 | PD7 | **A0** | AIN7 | アナログ入力 A0/SPI **SS**/Serial2 RX（USART1 ALT2）/VREFA |
| D19 | PF0 | **A1** | AIN16 | アナログ入力 A1/CCL(LUT3-IN0) |
| D20 | PA6 | **A2** | AIN26 | アナログ入力 A2/USART0 XCK |
| D21 | PA7 | **A3** | AIN27 | アナログ入力 A3/USART0 XDIR/AC0 出力/EVOUTA/CLKOUT |

**ヘッダに出ない内部ピン:** PF6（RESET）/PF7（UPDI）

> `~` は PWM 出力可能ピンを示します。`A0`–`A3` はボードのシルク表記に対応するアナログ入力です。PWM ピンは本家 Pro Micro（D3/D5/D6/D9/D10）と完全に一致し、さらに D14/D16 にも PWM を出力できます（SPI と排他）。アナログ別名も本家（A6=D4, A7=D6, A8=D8, A9=D9, A10=D10）に一致します。A11 は意図的に**未定義**です（Leonardo の A11=D12 と取り違えて別ピンへ静かに繋がる事故を防ぐため。A17 も予約欠番）。D18（A0）は VREFA を兼ねるため、外部アナログ基準電圧を使う場合は A0/SS/Serial2 RX が使えなくなります。

---

### シリアルポート

| オブジェクト | 実体 | ピン | 備考 |
|--------------|------|------|------|
| `Serial` | USB CDC | USB-C | シリアルモニタ（仮想 COM） |
| `Serial1` | USART0（ALT1 固定） | D0(RX) / D1(TX) | Pro Micro 互換ハードウェア UART。XCK(D20)/XDIR(D21) 付きのフル機能位置で、RS-485 の方向制御や USART-SPI ホストモードにも対応 |
| `Serial2` | USART1（ALT2 固定） | D18(RX) / D15(TX) | 予備 UART。SPI（SS/SCK）とピン共有・**排他利用** |

> 本家 Pro Micro と同じく、D0/D1 のハードウェア UART が `Serial1` です。`Serial0` は DxCore 内部での USART0 の名称で、`Serial1` と同一オブジェクトを指します。

### SPI（ホスト）

| 信号 | ピン |
|------|------|
| MOSI | D16（PD4） |
| MISO | D14（PD5） |
| SCK | D15（PD6） |
| SS | D18（PD7） |

チップセレクトは任意の GPIO を使用してください（SPI0 ALT4 位置。SCK/SS は `Serial2` と共用・排他利用）。

> **クライアント（受信側）動作:** ハードウェア SS（D18/PD7）が実ピンにあるため、付属の **SPISlave ライブラリ**（ESP8266 互換 API）で SPI クライアントとしても動作できます。詳細は [libraries/SPISlave](../libraries/SPISlave/README.md) を参照。

### I2C

| 信号 | ピン |
|------|------|
| SDA | D2（PA2） |
| SCL | D3（PA3） |

この版では I2C は UART とピンを共有しません。`Wire`・`Serial1`・`Serial2` は同時に使用できます。

### PWM（`analogWrite()`）

- **D5/D6/D9/D10/D14/D16** … TCA0（PORTD へ割り当て、WO0–WO5）
- **D3** … TCB1
- `millis()` / `micros()` は **TCB0** を使用するため、TCB1（D3）と TCA0 は PWM に使用できます。
- 本家 Pro Micro の PWM ピン（D3/D5/D6/D9/D10）を完全再現。D14/D16 は SPI 使用時には PWM に使えません。

### アナログ入力

- シルク表記の **A0–A3**（= D18–D21 = PD7/PF0/PA6/PA7）
- ADC を持つ各デジタルピンも A6–A16・A18–A20 として参照可能（D7/D17 = PA1/PA0 は ADC なし。A11/A17 は欠番）

--

## クロック

Tachi rev.3 は **水晶を搭載しない**設計で、システムクロックは内蔵 OSCHF の **24 MHz 固定**です（ボードメニューにクロック選択はありません）。
USB 用の 48 MHz（CLK_USB）は内蔵 PLL48M が生成し、USB の SOF に同期して自動調整されるため、水晶なしでも USB は仕様通りに機能します。
水晶用だった PA0/PA1 は GPIO として使用されています（PA0 = D17 = LED_BUILTIN、PA1 = D7）。

> USB ホスト切断時は動作クロックの精度が内蔵オシレータ単体の精度になります。

### クロック出力（CLKOUT）

メインクロック（CLK_PER）を **D21（PA7）** へ出力できます。外部 IC へのクロック供給、他 MCU との同期、実クロックの測定に使えます。付属の **ClockOut ライブラリ**で `ClockOut.begin()` / `ClockOut.end()` により開閉します（詳細は [libraries/ClockOut](../libraries/ClockOut/README.md)）。

> 24MHz の連続矩形波は EMI 源になるため、必要な期間だけ有効化する運用を推奨します。PA7 は AC0 出力・EVOUTA と共用のため、それらが使用中は `begin()` が `false` を返します。

---

## 電源

rev.3 は Pro Micro と同様に **2 系統の電源入力**を持ちます。

- **USB-C（5V）:** 理想ダイオード（Torex XC8110AA01）で逆流保護し、外部電源との併用時もホストを破損させません。
- **RAW 入力:** 基板上の高耐圧 LDO（**Torex XC6702D501**、入力 4.5–36V / サージ 46V 耐性 / 300mA / 過電流・過熱保護・ソフトスタート内蔵）で 5V を生成します。ドロップアウトを考慮した**実用最低入力は約 6.5V**、上限は放熱で決まります（推奨 6.5–16V）。誤って 24V の AC アダプタを接続しても破壊せず過熱保護で停止します。
- **VUSB（USB トランシーバ 3.3V）:** 基板上の LDO（**Torex XC6503D331**）から供給します（電源構成 3s。内蔵 USB レギュレータは無効 = `USBVREG = 0`。全 Wazamono ボード共通の構成）。
- **電圧切替:** ジャンパパッド **JP1** で VCC を 5V / 3.3V から選択します。AVR64DU28 は 1.8–5.5V の全範囲で 24 MHz 動作が可能です。
- USB データライン（D+/D-）は TVS（Toshiba DF2B6M4CT）で ESD 保護。

<sub>電圧選択のためには JP1 のパッドを望む電圧側とはんだ付けします。USB 給電時は XC6702 の内部ボディダイオード経由で RAW 端子に約 4.4V が現れます（本家 Pro Micro と同じ挙動）。RAW から外部機器へ給電しないでください。</sub>

---

## LED とスイッチ

| 部品 | 色 | 接続 | 用途 |
|------|----|----|------|
| 電源 LED | 黄緑 | 電源ライン | 通電表示 |
| LED_BUILTIN | 橙 | D17（PA0、Active-LOW） | ユーザー LED 兼 USB-CDC **RX** アクティビティ表示 |
| リセット | — | RESET（PF6） | タクトスイッチ |

rev.3 のユーザー LED は **D17（PA0）の 1 灯のみ**です。本家 Pro Micro と同じく RX LED がユーザー LED（`LED_BUILTIN` = `LED_BUILTIN_RX` = D17）を兼ねます。TX LED は搭載しません（`LED_BUILTIN_TX` は未定義。参照するとコンパイルエラーになります）。CDC 受信の瞬間に約 100ms のパルスで点灯し、スケッチからの `digitalWrite(D17, ...)` と共存します。

--

## 書き込み

1. ボードを USB で接続します。
2. Arduino IDE からスケッチを書き込みます。書き込み開始時に **1200bps タッチ**が行われ、USB CDC ブートローダへ自動遷移します。
3. 自動遷移しない場合は、**リセットボタンのダブルタップ**でブートローダに入れます。

初回のみ、または USB ブートローダを書き込み直す場合は、UPDI プログラマ（PICkit 4/5、Atmel-ICE、jtag2updi 等）を UPDI パッドに接続して書き込みます。

<sub>開発用 VID/PID は pid.codes のテスト範囲（アプリ `0x1209:0x0006` / ブートローダ `0x1209:0x0005`）を使用しています。製品出荷前に正式な VID/PID へ置き換えてください。</sub>

---

## ソフトウェア互換性（Pro Micro）

Tachi は Pro Micro からの移植の手間を最小化することを目指しています。

### ウォッチドッグ

古典 AVR（ATmega32U4）の `<avr/wdt.h>` を用いたコードがそのままビルド・動作します。AVR DU のウォッチドッグはレジスタ構成が異なりますが、コアが互換層（`wdt_compat.h`）を自動適用し、`WDTO_*` 定数を DU の時間設定へ正しく変換します。

```cpp
#include <avr/wdt.h>

void setup() {
  wdt_enable(WDTO_2S);   // 2秒のタイムアウト（Pro Micro と同じ書き方）
}

void loop() {
  // 処理が2秒以内に終わるなら…
  wdt_reset();           // ウォッチドッグをリセット（餌やり）
}
```

`wdt_enable(WDTO_*)` / `wdt_reset()` / `wdt_disable()` がそのまま使えます。タイムアウトは 15ms〜8s（`WDTO_15MS`〜`WDTO_8S`）。

> 注: `MCUSR`（リセット要因フラグ）は古典 AVR 固有のため対象外です。`MCUSR = 0;` を含む初期化コードは別途読み替えが必要です（DU では `RSTCTRL.RSTFR`）。

---

## 主要部品

確定 BOM の主要部品です。
**MCU 以外全て日本製部品で統一**されています。

| 記号 | 種別 | 型番 | 備考 |
|------|------|------|------|
| U1 | MCU | AVR64DU28（28 ピン） | メインマイコン（rev.3 で 32 ピンから変更） |
| U2 | 理想ダイオード | Torex XC8110AA01MR-G | USB 電源逆流保護 |
| U3 | LDO（RAW→5V） | Torex XC6702D501ER-G | 入力 4.5–36V / 300mA / 保護・ソフトスタート内蔵 |
| U4 | LDO（5V→3.3V） | Torex XC6503D331GR-G | VUSB / 3.3V 系供給 |
| D1, D2 | ESD 保護 | Toshiba DF2B6M4CT | USB D+/D- TVS |
| D3, D4 | LED | — | 電源 LED / LED_BUILTIN（D17） |
| USB1 | USB コネクタ | CX90M-16P | USB-C ミッドマウント |
| JP1 | 電圧切替パッド | — | VCC = 5V / 3.3V |
| J3 | UPDI パッド | — | 1:RESET / 2:VCC / 3:GND / 4:UPDI |
| R3, R4 | 抵抗 5.1kΩ | — | USB-C CC（Rd） |

<sub>rev.3 では外部水晶（旧 Y1）を撤去しました。</sub>

---

## 公式ドキュメント

データシート・エラッタは予告なく更新されます。常に最新版を参照してください（Microchip PCN システムで更新通知を受け取れます）。

- AVR64DU28 製品ページ: <https://www.microchip.com/en-us/product/AVR64DU28>
- データシート: DS40002548A（AVR64DU28/32）
