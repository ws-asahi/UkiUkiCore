# ChangeLog

UkiUkiCore の変更履歴です。UkiUkiCore は VTuber「浮々ゆにこ」ファングッズ向け Arduino 互換機
UkiUkiduino（Uno R3 互換）/ UkiUkimicro（Pro Micro 互換）のための Arduino コアで、
[WazamonoCore](https://github.com/ws-asahi/WazamonoCore)（DxCore 1.6 系）から派生しています。

---

## v0.0.5 — 2026-09-14 — UkiUkimicro 追加、Ethernet シールド対応、実機検証の反映

UkiUkiduino の実機で GPIO / ADC / PWM / ボタン / LED / Serial1↔Serial2 / SPI（SD カード・
W5100 Ethernet シールド）/ ウォッチドッグの動作を検証し、そこで見つかった問題を修正した
リリースです。Pro Micro 互換機 **UkiUkimicro** の variant とブートローダーが加わりました。

### 破壊的変更

- **`BTN_BUILTIN` を D32 に変更**（UkiUkiduino: 旧 D21 = PA1、UkiUkimicro: PF0）。
  シリーズ共通の番号にするためで、UkiUkiduino は D21〜D31、UkiUkimicro は D22〜D29 が
  欠番になります。UkiUkimicro の RESET / UPDI は index 33 / 34 です。
  スケッチでは `BTN_BUILTIN` マクロを使っていれば変更不要です。
- **Pro Micro 互換機の名称を「UkiUkiduino ProMicro」から「UkiUkimicro」に変更**。
  開発途中の名称で、v0.0.4 には含まれていないため互換エイリアスはありません。
  FQBN `ukiukimicro`、`ARDUINO_AVR_UKIUKIMICRO`、`UKIUKIMICRO_PINOUT`、
  `variants/UkiUkimicro/`、USB 製品名 "UkiUkimicro" / "UkiUkimicro Bootloader"。
- **手動インストールのツールチェーン配置方法を変更**（下記「ツール」参照）。
  `make_platform_local.bat` と `platform.local.txt` は廃止です。

### ボード

- **UkiUkimicro（Pro Micro 互換）を追加**: AVR64DU32、フルカラー LED（XL-5050RGBC-WS2812B、
  PF4、`LED_BUILTIN` = D17 のミラー）、TX/RX LED（D30/D31）、ボタン（D32）。
  Serial1 = D1/D0、Serial2 = D15(TX)/D18=A0(RX)、SPI = D16/D14/D15。
  ブートローダー `usbcdcboot_ukiukimicro.hex`（VID:PID 0x1209:0x000D、テスト用 PID）。
- **Clock メニューに 20 MHz / 12 MHz を追加**（24 / 20 / 16 / 12 MHz、既定 24 MHz、両ボード）。
  オンボード WS2812 ドライバを F_CPU ごとにサイクル計算した nop 数へ一般化し、
  16 MHz 選択時の `#error` を解消。各クロックの実タイミングは実バイナリの逆アセンブルで
  データシートの範囲内であることを確認済み（実機での LED 点灯確認は 24 MHz のみ）。
  USB は CLK_USB（48 MHz）が OSCHF の固定 4 MHz 出力 + PLL48M から作られ主クロックと独立
  （CLK_PER ≥ 12 MHz、DS40002548B 28.3.1.1）のため、全クロックで USB-CDC が動作します。

### コア

- **ウォッチドッグ互換シム `wdt_compat.h` を WazamonoCore から取り込み**。配布中の
  toolchain（15.2.0-wazamono2）の avr-libc 2.3.2 は WDT 修正（#1065/#1068/#1069）を含まず、
  AVR DU では `wdt_enable()` / `wdt_disable()` の書き込みが CCP ウィンドウ内の
  read-modify-write で黙って落ち、`WDTO_2S` が 0.5 秒になっていました。
  `wdt_enable(WDTO_2S)` / `wdt_reset()` / `wdt_disable()` / `MCUSR & _BV(WDRF)` が
  Uno / Pro Micro と同じ書き方で動くことを実機で確認済み。

### ライブラリ

- **Ethernet（W5100 / W5200 / W5500）を同梱**（arduino-libraries/Ethernet 2.0.2 ベース、
  サンプル 13 本を日本語化）。24 MHz の AVR DU では公式版の `SPISettings(14000000)` が
  12 MHz に丸められ、W5100 が応答しません（レジスタ読み値が 1 ビットずれる。Uno R3 は
  16 MHz → 8 MHz なので顕在化しない）。同梱版は `ARDUINO_ARCH_MEGAAVR` で 8 MHz を要求
  （F_CPU 12/16/20/24 MHz で実効 6/8/5/6 MHz）し、チップセレクトを OUTSET/OUTCLR の
  アトミック書き込みに変更。API は公式版と同一。UkiUkiduino に Uno 用シールドを直挿し
  （CS = D10、SD CS = D4）して W5100 認識・固定 IP・TCP 接続まで実機確認済み。

### ツール / インストール

- **`tools/setup_toolchain.sh` / `tools/setup_toolchain.bat` を追加**。
  `docs/package_ukiuki_index.json` の `toolsDependencies` を読み、wazamono-toolchain の
  リリースから avr-gcc / avrdude を取得（SHA-256 検証）して `hardware/UkiUkiCore/tools/`
  に配置します。Arduino IDE 2 / arduino-cli はこの場所を
  `{runtime.tools.<name>-<version>.path}` として登録するため、コンパイル・スケッチ書き込み・
  ブートローダー書き込みの全てでこのツールが使われます。
  従来の `platform.local.txt` はコンパイルにしか効かず、書き込み時には IDE 側の別 avrdude
  （arduino:avr 更新後は 8.0.0-arduino1）が選ばれる問題がありました。
- `platform.txt` のツール参照をバージョン固定（`avr-gcc-15.2.0-wazamono2` /
  `avrdude-8.1-wazamono2`）に変更。ボードマネージャ版・手動版で同じ解決になります。
- `make_platform_local.bat` を削除。
- `tools/make_release.sh`: インデックスに過去バージョンを残す（ダウングレード可能）、
  ボード一覧を boards.txt から生成、toolchain 版のコメントを wazamono2 に更新。

### ドキュメント

- README とボード別ページ（`megaavr/extras/UkiUkiduino.md` / `UkiUkimicro.md`）の 3 分割に
  再構成。通信（Serial / SPI / I2C）の節を追加。
- サンプルスケッチの日本語化: ClockOut / SPISlave（第 13 弾）、UkiUkimicro のピン注記
  （第 14 弾）、Ethernet 13 本（第 15 弾）。

### 既知の制約（実機検証で確認、仕様として据え置き）

- 24 MHz では USART の最低ボーレートが約 1465 bps で、`Serial1.begin(300)` は約 1659 bps に
  化けます（`begin()` が 4·F_CPU/baud を uint16 に切り詰めるため）。300 / 1200 bps は非対応です。
  影響が小さいためコア側の修正は行いません。
- 2 Mbps は生成・受信できますが割り込み処理が追いつかず、連続受信の実用上限は 1 Mbps 程度です。

---

## v0.0.4 — 2026-09-02

- toolchain を `15.2.0-wazamono2` / `avrdude 8.1-wazamono2` に更新し、ボードマネージャ
  配布を整備（`tools/make_release.sh`、`docs/package_ukiuki_index.json`）。
- UkiUkiduino の variant を WazamonoCore Tsurugi 最新版ベースに再構成
  （ピン再割当、D21 ボタン、Serial2、CCL 経由の TCB1 PWM）。
- オンボード LED（WS2812D-F5-12mA-C1）のドライバとブートローダーの DFU 表示を
  データシートのタイミングに適合。電源 LED を白（KT-0603W）に変更。
- WazamonoCore 上流（2026-07-06〜09-02）をマージ。Wazamono 専用ドキュメントを削除し、
  ボード詳細を README に統合。

## v0.0.3 — 2026-07-19

- 初期公開版。UkiUkiduino（AVR64DU32、Uno R3 互換）の variant、USB-CDC ブートローダー、
  ボードマネージャ用インデックス。
