# 変更履歴（Changelog）

WazamonoCore の変更履歴です。WazamonoCore は [DxCore](https://github.com/SpenceKonde/DxCore)（1.6 系）から派生した、Wazamono シリーズ専用の Arduino コアです。

---

## v0.0.6 — Tachi の Pro Micro 互換性検証と、そこで判明した不具合の修正 / Tsurugi ピンマップ rev.3

Wazamono Tachi rev.4 の実機（3.3V 個体・5V 個体）で Pro Micro (ATmega32U4) との互換性を全域にわたり検証し、その過程で見つかったコアの不具合を修正しました。あわせて Tsurugi のハードウェア rev.C 対応と、SerialUPDI の avrdude 8.1 経由への変更を含みます。

### 全ボードに影響する修正

検証の過程で見つかった、Wazamono 3 機種すべて（および DxCore 系の他機種）に影響する不具合です。

- **`analogWrite(pin, 0)` / `analogWrite(pin, 255)` がピンを出力にしない** (`wiring_pwm.c`): Arduino AVR コアは 0/255 の分岐より前に `pinMode(pin, OUTPUT)` を呼び、アナログ出力ピンは `pinMode()` 不要であることを明示していますが、本コアは端点分岐から方向を設定せずに return していました。`pinMode()` を呼んでいないピンでは `analogWrite(pin, 0)` が Hi-Z のまま、`analogWrite(pin, 255)` はプルアップが有効になるだけで駆動しません。通常の PWM 経路は末尾で `_setOutput()` を通るため、影響は端点 2 値のみですが、そこは「フェードを 0 から始める」「ON/OFF を `analogWrite()` で書く」といった `pinMode()` を省きやすい場所です。
- **`HardwareSerial::getPin()` が常に `NOT_A_PIN` を返す** (`UART.cpp`): `_getPin()` の引数順が宣言と食い違っており、`mux_count` に要求ピン番号、`pin` に mux 総数が渡っていました。`pin > 3` のガードに掛かるため、USART0 の mux 選択肢が 5 つ以上ある機種（AVR DU を含む）では全インデックスが `NOT_A_PIN` になります。通常の UART 利用では `getPin()` を呼ばないため表面化しませんが、`mspiBegin()` / `syncBegin()` は XCK ピンの方向と反転を設定するためだけにこれを使うので、**USART の SPI ホストモードと同期モードがエラーも出さずに無動作**になっていました。
- **`noTone()` 後に TCB の PWM が復帰しない** (`timers.h`): `Tone.cpp` の `disableTimer()` には TCB を PWM モードへ戻す処理がありますが、`ENABLE_TCB_PWM` で囲まれており、この定数がコアのどこにも定義されていませんでした。`init_TCBs()` は millis 以外の TCB を 8bit PWM モードに設定しているため本コアは実際に TCB を PWM に使っており、復帰処理が消えた結果 **一度 `tone()` を鳴らすと該当ピンの `analogWrite()` がリセットまで死ぬ**状態でした。Tachi では millis が TCB0 のため `tone()` は TCB1 を取り、犠牲になるのは D3 です。

### AVR DU 系の修正

- **ADC の内部チャネルが実 I/O ピンに化ける** (`wiring_analog.c`): `analogRead()` / `_analogReadEnh()` がチャネルを `0x3F` でマスクしていました。AVR DU の MUXPOS は 7bit で、内部チャネルは 0x40 (GND) / 0x42 (TEMPSENSE) / 0x44 (VDDDIV10) にあり、tinyAVR/Dx の 0x30–0x33 とは配置が異なります。マスクの結果 `ADC_GROUND` → AIN0、`ADC_TEMPERATURE` → AIN2、`ADC_VDDDIV10` → AIN4 となり、**エラーも返さずに無関係な I/O ピンを読んで**いました（実機では VDD 実測が 1.4V、内蔵温度が 415℃ と表示）。不正チャネル判定も 0x30–0x33 前提だったため、DU の正しい値のみを受理するよう修正しています。
- **`analogClockSpeed()` が異常値を返す** (`wiring_analog.c`): `int16_t` の PROGMEM テーブルを `pgm_read_byte_near()`（下位バイトのみ）で読み、比較と戻り値ではコード空間のアドレスをデータ空間で直接参照していました。24MHz で正解 2000kHz のところ −9227kHz を返し、ADC クロックを指定した場合には誤ったプリスケーラが選ばれます。

### Tachi

- **Pro Micro (32U4) 互換マクロを追加**: `TX_RX_LED_INIT` / `TXLED0` / `TXLED1` / `RXLED0` / `RXLED1`（TX = PC3/D30、RX = PF3/D17、いずれもアクティブ LOW）、`SerialUSB`、および `A4`(22) / `A5`(23) / `A11`(29)。A4/A5/A11 は SparkFun の promicro variant が未配線ピン向けに定義しているもので、本コアでは未定義だったためスケッチがコンパイルできませんでした。同じ番号を割り当てましたが、Tachi 側ではいずれもピンテーブルの欠番なので、コンパイルは通り実行時は安全な no-op になります（別の実ピンに着地することはありません）。
- `SERIAL_PORT_HARDWARE_OPEN` を `Serial2` から **`Serial1` へ変更**。SparkFun の値と一致し、定義上も D0/D1 の Serial1 が「最初の空きポート」として正当です（Serial2 は SPI の SCK/SS と排他）。

### USB CDC

- **Leonardo `Serial_` の CDC アクセサを追加** (`USBSerial.h`): `dtr()` / `rts()` / `baud()` / `stopbits()` / `paritytype()` / `numbits()` / `readBreak()` と 2 つの enum を、ArduinoCore-avr と同じ名前・意味・生値で実装しました。状態自体は `usb_cdc.c` が保持していたため、不足していたアクセサの追加と、従来 ZLP を返して破棄していた `SEND_BREAK` の wValue のラッチを行っています。ホストの指定ボーレートを `Serial.baud()` で拾って `Serial1.begin()` へ渡す USB-シリアル変換ブリッジという定番のスケッチが、コンパイルすら通らない状態でした。

### SPISlave

- **プライムした返信の先頭 2 バイトが上書きできない**: BUFWR=1 では DREIF が Low の間の DATA 書き込みが黙って捨てられるため（DS40002548B 図 26-4）、`begin()` が仕込んだゼロを `setData()` が置き換えられず、最初のトランザクションでゼロが 2 バイト先行していました。プライム前に ENABLE を一巡させて送信経路を空にします。
- **BUFOVF が残留して受信経路がジャムする**: ホストが割り込み処理より速くクロックするとクライアントはオーバーフローし、BUFOVF は受信 FIFO を読み出すまで `end()` / `begin()` を跨いでも消えません。この状態では RXCIF が立たなくなるため、SPI 割り込みが返信を補充せずシフトレジスタが MOSI を 1 バイト遅れでエコーし、SS 立ち上がりハンドラは空のトランザクションを報告します。**一度高速で失敗すると、以後リセットまで低速でも通信できません**でした。`begin()` / `end()` / プライム経路で DATA を 3 回読んで FIFO を空にします。
- `SPISlave.h` のコメントを修正（Tachi の SS は PD7 = D18 = A0。旧リビジョンの D4 表記でした）。

### 非正準ピン配列への対応

- **`_getPin()` が RX/XDIR をピン番号の +1 で導出していた** (`UART.cpp`): PORT のビット位置としては正しい関係ですが、返すのは Arduino ピン番号なので、variant がポートのビット順にピンを振っている場合しか成立しません。Tachi は TX=PA4=D1 / RX=PA5=D0 なので `getPin(1)` が D2（PA2 = Grove の SDA）を返していました。`NONCANONICAL_PIN_NUMBERS` が定義されている場合は同一ポート内でビット位置を進めて逆引きします。正準配列の variant は従来どおりです。

### 検証で確認された仕様（変更なし）

Tachi rev.4 実機で確認し、修正不要と判断した項目です。詳細は WazamonoTachi.md を参照してください。

- ピンマップ 36 エントリ・アナログ別名・予約ピンの no-op が設計どおり
- PWM は D3/D5/D6/D9/D10 が Pro Micro と同一集合（D14/D16 が追加、SPI と排他）。全 7ch が 1470.6Hz で統一（32U4 は 490/980Hz 混在）
- クロックは水晶レスながら実測 ±0.1% 以内（n=2、5V/3.3V 両方）。Serial1 の総合ボーレート誤差は最大 0.08% で、8N1 の許容 ±2% に対して 25 倍以上の余裕。オートチューンは 1 ステップ 0.4% の分解能に対し素の精度が上回るため実質的に無動作
- ADC は 10bit ネイティブで `analogRead()` は 0–1023（32U4 と同一）。内部参照 4 種が 0.3% 以内で相互整合。差動 ADC なし（32U4 にはある）／累積で 13bit まで拡張可
- 外部割り込みはヘッダ 18 本すべてで RISING/FALLING/CHANGE/LOW が使用可（32U4 は 5 本のみ、LOW は INT0-3 限定）。応答は約 5〜7us で、PORT のビット位置に対し 0.33us/bit の線形な差（r=0.995、下位ビットほど速い）
- USART0 の SPI ホストモードと SPI0 クライアントモードによる同一個体間通信が 4 モードすべてで成立。安定上限は 1MHz（制限要因は SPI ハードではなく同一 CPU 上の割り込み競合）

### 既知の非互換（設計上、修正しません）

- `tone()` が潰す PWM ピンが異なります（32U4 は Timer1 で D9/D10、Tachi は TCB1 で **D3**）
- `digitalPinToInterrupt()` が恒等写像のため `NOT_AN_INTERRUPT` を返しません。欠番ピンを渡しても `attachInterrupt()` 側が弾いて no-op になりますが、`NOT_AN_INTERRUPT` で分岐するスケッチはその分岐に入りません
- EEPROM が 1KB → **256B**（DS40002548B 表 8-3。AVR DU 全品種で 256B 固定）。SRAM は 2.5KB → 8KB、Flash は 32KB → 64KB
- `analogReference()` の選択肢が異なります（32U4 の `INTERNAL` / `INTERNAL1V1` / `INTERNAL2V56` は**意図的に未定義**。近い値へ黙って読み替えると測定値が静かに狂うため、コンパイルエラーで気付ける方を選んでいます）
- A0 (PD7) が VREFA を兼ねるため、外部リファレンス使用時は A0 / SPI SS / Serial2 RX を同時に失います
- `LED_BUILTIN` = 17（32U4 は 13、いずれも配線されないピン）、`SS` = 18（32U4 は 17 = RX LED）、`A6`–`A10` の数値がピン自身の番号（32U4 は Leonardo の複製番号 24–28。指すピンは同一）

### ブートローダ

- Tachi のブートローダ hex は **avr64du32 / LED = PF3 / VREG = 0** 向けで生成済みです。今回の互換性検証はすべてこの hex を書き込んだ rev.4 実機で実施しました。v0.0.5 の「`usbcdcboot_wazamonotachi.hex` は avr64du28/PA0 向けのまま」という注記は解消済みです（Tachi は AVR64DU28 の採用を検討した時期がありましたが、実装面積の都合で AVR64DU32 に戻しています）。

### ツール

- **SerialUPDI が Python なしで動作するように**: 書込装置 `SerialUPDI - *` の3項目を `prog.py`（pymcuprog）から avrdude 8.1 の `-c serialupdi` へ切り替え。手動 / git インストールでは `megaavr/tools/python3/` が存在せず、ブートローダ書き込みが `executable file not found in %PATH%` で失敗していた問題を解消します。
- 併せて、上流 DxCore の AVR DU セクションが定義していなかった `bootloader.serupdifuse5` に依存しなくなりました。ヒューズ書き込みは PICkit / nEDBG / Atmel-ICE と同じ `avrdudefuse5` 経路に一本化されます。
- `megaavr/tools/prog.py` と `megaavr/tools/libs/` はツリーに残っていますが、どのレシピからも参照されません（`bootloader.pymcuprogstring` も同様に未使用）。

### Tsurugi ピンマップ rev.3（破壊的変更を含む）

Tsurugi のハードウェア rev.C に対応する変更です。Tachi / Kunai には影響しません。

- ピンマップ rev.3: **D4=PC3（旧 D7）/ D7=PF4（旧 D4）を入れ替え**。D4 が CCL LUT1-OUT の既定位置になります。A10/A13 は D 番号に追従します。
- **AREF（PD7）を D20/A20 として公開**: 外部基準電圧を使わないときは GPIO / SPI0 ハードウェア SS(ALT4) / Serial2 RX として使用可能（モダン AVR の構造的特徴。Uno R3 では不可能）。
- **Serial2 追加**: USART1 ALT2 固定（TX=D13/PD6、RX=D20/PD7）。D13(SCK)・AREF と共用のため、SPI 使用中・外部基準使用中の併用は不可。
- **択一式 TCB1 PWM**: `analogWrite()` の出口を D3（LUT0 ALT）/ D4（LUT1 既定）/ D7（TCB1 WO 直結）から選択。最後に書いたピンが出口を取得（既定 D3）。コアに `WAZAMONO_TCB1_PWMMUX` 機構を追加（Tachi/Kunai の単一経路 `WAZAMONO_TCB1_LUTPWM_*` は無変更）。
- **D7/D8 入替（基板 rev.C 最終版に追従）**: D7=PF5（AIN21、TCB1 WO 直結の PWM 出口）/ D8=PF4（AIN20、PWM なし）。A13/A14 は D 番号に追従。
- **水晶レス化**: boards.txt からクロック選択メニューを削除し、内蔵 OSCHF 24 MHz 固定に（Uno R4 と同方針）。USB の 48 MHz は従来どおり PLL48M + SOF 自動同調。
- **USB-CDC TX/RX LED**: 水晶撤去で空いた PA0（TX）/ PA1（RX）をアクティビティ LED として駆動（アクティブ LOW、100ms ワンショット、Pro Micro 慣例）。**D30（TX）/ D31（RX）**として `digitalWrite()` で直接制御可能（D21–D29 は欠番の no-op）。`PIN_LED_TX` / `PIN_LED_RX` / `LED_BUILTIN_TX` / `LED_BUILTIN_RX` に加え、Leonardo 互換の `TXLED1/TXLED0/RXLED1/RXLED0` マクロも定義。

### ドキュメント

- WazamonoTsurugi.md をピンマップ rev.3 へ更新（D4/D7、D20/AREF、Serial2、択一 PWM、クロック節、LED 表、電源節の EN 分圧値 100k/100k・起動約 5.0 V への修正）。

---

## v0.0.5 — ピンマップ rev.4（Tachi を AVR64DU32 へ戻す）

Tachi のハードウェア rev.4 に対応する変更です。**Tachi は burn-bootloader のやり直しが必要です**（MCU が avr64du28 → avr64du32 へ戻るため）。Tsurugi / Kunai に変更はありません。

### Tachi（破壊的変更）

- MCU を **AVR64DU28 → AVR64DU32（TQFP-32）** へ戻しました。実装面積の都合で TQFP が必要だったこと、28 ピン化のコスト削減効果（約 20 円）に対して機能削減が見合わなかったためです。水晶レス（内蔵 OSCHF 24 MHz 固定）は継続します。
- ピンマップ rev.4: D4=**PF4**(A6)、D7=**PA6**(A17/XCK)、D8=**PA7**(A8/XDIR/AC0 出力/EVOUTA/CLKOUT)、D17=**PF3**(A21)、D18=PD7(A0/SPI SS/Serial2 RX/VREFA)、D19=PF0(A1)、D20=**PF1**(A2)、D21=**PF2**(A3/EVOUTF)、**D30=PC3(A34) を復活**。欠番は D11–D13・D22–D29。
- LED を Pro Micro 準拠の 2 灯に戻しました（いずれもアクティブ LOW）: **RX LED = D17（PF3）= `LED_BUILTIN` = `LED_BUILTIN_RX`**、**TX LED = D30（PC3）= `LED_BUILTIN_TX`**。専用のユーザー LED（旧 D13）は設けません。
- **PA0・PA1 は基板上未接続、PF5 は非公開（予約）**: 隠しインデックス（NOT_A_PORT/NOT_A_PIN）とし、誤って操作しても no-op になります。AIN21（PF5）は使用不可で `AIN21` マクロも定義しません。
- EVOUTF（PF2 = D21）復活。LUT3 がフル利用可能に（IN0/IN1/IN2 = A1/A2/A3、OUT = D17）。LUT1-OUT は D30（TX LED）。
- アナログ別名: A0–A3 = D18–D21、A6–A10 は Pro Micro 互換、A11 は引き続き意図的欠番、A12–A17 = D0–D7、A18–A21 = D14–D17、A34 = D30。

### ブートローダ

- ビルドマトリクス更新: Tachi = `avr64du32` / LED **PF3**・アクティブ LOW / VREG=0。
- **注意: `bootloaders/hex/usbcdcboot_wazamonotachi.hex` は avr64du28/PA0 向けのままです。** wazamono-toolchain で `build_wazamono.(sh|bat)` を実行して再生成し、コミットしてから rev.4 基板へ burn-bootloader してください。Tsurugi / Kunai の hex は有効なままです。

### ドキュメント

- WazamonoTachi.md のピン表を rev.4 へ更新（諸元見出しの AVR64DU28 表記修正、重複した空の I2C 表を削除）。README のボードメニュー名を「Wazamono Tachi (Pro Micro)」へ。
- ライブラリ README（EventSystem / CustomLogic / ClockOut）と例スケッチ（CompToPin / PinToPin / MultipleOutputs / TCA0Demo1–4）の Tachi 行を rev.4 値へ更新（EVOUTA=D8 など）。

---

## v0.0.4 — ピンマップ rev.3（Tachi の AVR64DU28 化 / VUSB 供給の統一）

ハードウェア rev.3 に対応する変更です。**3 ボードとも burn-bootloader のやり直しが必要です**（Tachi は MCU が変わるため必須、Tsurugi は VREG 設定変更のため必須）。

### Tachi（破壊的変更）

- MCU を **AVR64DU32（32 ピン）→ AVR64DU28（28 ピン）** へ変更（コスト削減）。`boards.txt` の `build.mcu=avr64du28`。
- **外部水晶を撤去**。クロックは内蔵 OSCHF 24 MHz 固定になり、Clock Speed メニューを削除（Kunai と同じ構成）。PA0/PA1 は GPIO として開放。
- ピンマップ rev.3: D4=PF1(A6)、D7=PA1（ADC なし）、D8=PC3(A8)、D14–D16=SPI（変更なし）、D17=PA0、D18=PD7(**A0**/SPI SS/Serial2 RX/VREFA)、D19=PF0(A1)、D20=PA6(A2/XCK)、D21=PA7(A3/XDIR/EVOUTA/CLKOUT)。**D11–D13 は欠番**（旧 D13 の専用 LED は廃止）、旧 D30（TX LED）も廃止。
- LED は **D17（PA0）の 1 灯のみ**: `LED_BUILTIN` = `LED_BUILTIN_RX`、アクティブ LOW、USB-CDC 受信アクティビティ表示を兼用。`LED_BUILTIN_TX` は未定義（参照はコンパイルエラー）。
- アナログ別名: A0–A3 = D18–D21、A6–A10 は Pro Micro 互換のまま、**A11 は意図的に未定義**（Leonardo の A11=D12 との取り違え防止）、A12–A16 = D0–D5、A17 予約欠番、A18–A20 = D14–D16。
- EVOUTF を削除（PF2 が存在しないため）。イベント出力は EVOUTA（D21）/EVOUTD（D9）の 2 系統。
- Serial2 の RX が D4 → **D18** へ移動（TX は D15 のまま）。SPI の SS 表記も D4 → **D18**（同じ PD7）。

### Tsurugi

- ピンマップの変更はありません。
- **VUSB（USB トランシーバ 3.3V）の供給を内蔵 USB レギュレータから基板上の外部 3.3V LDO（NJM2881F33）へ変更**し、Tachi / Kunai と回路構成を統一（電源構成 3s）。`boards.txt` から `-DUSB_VREG_INTERNAL` を削除し、ブートローダも VREG=0 でビルドするよう変更。

### Kunai

- オンボード LED の番号と役割を XIAO 準拠へ変更: **D11 = PD4 = LED_BUILTIN + TX アクティビティ LED / D12 = PD5 = RX アクティビティ LED**（旧 D13/D14 から番号変更、TX/RX の役割も入替）。アナログ別名 A13/A14 → A11/A12。
- インデックス 13/14 は欠番になり、XIAO のユーザー LED「13」への `digitalWrite(13, ...)` は無害な no-op になります。

### ブートローダ

- ビルドマトリクスを更新: Tachi = `avr64du28` / LED **PA0**・アクティブ LOW / VREG=0、Tsurugi = VREG=0（変更）、Kunai = 変更なし。
- **注意: `bootloaders/hex/` の hex はこのリリース時点で未再生成です。** wazamono-toolchain（avr-gcc 15.2.0-wazamono1）で `build_wazamono.(sh|bat)` を実行して hex を更新してから burn-bootloader を行ってください。旧 `usbcdcboot_wazamonotachi.hex` は avr64du32/PC3 向けのため rev.3 基板では使用できません。

### ドキュメント

- WazamonoTachi.md を rev.3 へ全面改訂（電源系: RAW 入力 + Torex XC6702D501 / XC8110 / XC6503D331、BOM から水晶を削除）。
- ライブラリ README（EventSystem / CustomLogic / ClockOut / SPISlave）と例スケッチ（CompToPin / PinToPin / MultipleOutputs / TCA0Demo1–4 / ServoMaxTest）のボード別ピン表を現行 variant に一致するよう修正（3 ボードとも rev.1 時代の値が残存していたものを含む）。

---

## v0.0.3 — ウォッチドッグの Pro Micro 互換対応

SparkFun Pro Micro（ATmega32U4）向けのウォッチドッグコードを、無修正でビルド・動作できるようにしました。

### 追加

- コアに `cores/dxcore/wdt_compat.h` を追加し、`Arduino.h` から自動インクルード。`#include <avr/wdt.h>` を用いる古典 AVR のコードがそのまま動作します。
  - `wdt_enable(WDTO_*)` … 古典 `WDTO_*`（15ms〜8s）を AVR DU の `WDT.CTRLA` PERIOD 符号へ正しく変換（WDTO_2S→2.0s 等）。avr-libc 版は変換しないため誤った時間や未定義になる問題を解消。
  - `wdt_disable()` … `WDT_PERIOD_OFF_gc` を書き込み WDT を停止。
  - `wdt_reset()` … `WDR` 命令（古典・モダン共通、avr-libc 版をそのまま使用）。
  - `WDTO_15MS`〜`WDTO_8S` を古典値（0〜9）で定義。
- データシート DS40002548A §21.3.6「SYNCBUSY=1 の間は `WDT.CTRLA` 書き込み禁止」に従い、書き込み前に SYNCBUSY を待機。連続した `wdt_enable()`/`wdt_disable()` でも確実に反映されます（素朴な実装で起きる「2回目の書き込みが無視される」問題を回避）。

### 補足

- 既存の `DxCore` ライブラリの `ResetWithWDT()` 等はそのまま利用可能です。
- 本互換層はコア全体（Tachi / Tsurugi 両方）に適用されます。
- `MCUSR`（古典 AVR のリセットフラグ）は本対応の対象外です。`MCUSR = 0;` を含むコードは別途リセットフラグ互換が必要です。

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
