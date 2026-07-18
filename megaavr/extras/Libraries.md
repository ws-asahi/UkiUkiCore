# WazamonoCore 同梱ライブラリ

WazamonoCoreは、Wazamonoボード(Tachi・Tsurugi・Kunai — いずれもAVR DUシリーズ)で使うライブラリをすべて同梱しています。DU固有の周辺機能ライブラリは共通の設計に従います: `Serial`と同じく**定義済みオブジェクト**、**ボードごとに固定されたピン**を**Arduinoピン番号**で扱い、I/Oヘッダを開く必要はありません。`AnalogComp`・`CustomLogic`・`EventSystem`は互いに組み合わせて使えるよう作られています。DUシリーズに存在しない周辺機能(TCD・ZCD・OPAMP・PTC・MVIO)のライブラリは含みません。

## Wazamono固有

### Flash
[Flash readme](../libraries/Flash/README.md) 実行中のスケッチからプログラムフラッシュへ書き込みます。Wazamonoボードでは、USB CDCブートローダが4KBブート区画の末尾に持つ小さなスタブ(`spm z+; ret`)経由で書き込みます。`Flash.checkWritable()`が書き込み前にブートローダを検証し、4KB境界より下(ブートローダ自身)への書き込みはすべて拒否されます。消去単位は512バイトのフラッシュページです。自分のコードと衝突しないよう、**高位アドレスから**使ってください。FlashDemo・FlashWriteTestサンプルで実機検証済みです。

### USERSIG
[USERSIG readme](../libraries/USERSIG/README.md) USERROW(ユーザ署名領域)は0x1200にある**512バイト**の不揮発メモリで、チップ消去や通常のスケッチ書き込みでも消えません — シリアル番号・校正値・ボード設定の置き場所に最適です。APIはEEPROM互換(`read`/`write`/`update`/`get`/`put`)。USERROWは全体消去しかできないため、ビットを1に戻す必要がある書き込みはRAM(512バイト)にバッファされ、`flush()`で反映されます。`write()`は直接書けたとき1、`flush()`待ちのとき0を返します。書き換え耐久は有限です — ループ内で書かないでください。UsersigTestサンプルが全512バイトを実機で検査します。

### DxCore
[DxCore readme](../libraries/DxCore/README.md) チップ設定まわりの補助ラッパー(PWMTestサンプルの置き場所でもあります)。APIの一部はDUに存在しない周辺機能(MVIO・OPAMP)に触れており、互換のため維持しつつ、ボードパッケージ公開前にスリム化する予定です。

## 周辺機能ライブラリ
DUのアナログコンパレータ・カスタムロジック・イベントシステムを、同じ様式で相互接続できる3つの小さなライブラリとして提供します。

### AnalogComp
[AnalogComp readme](../libraries/AnalogComp/README.md) オンチップのアナログコンパレータ(AC0)を1つの定義済みオブジェクトとして提供: 2つの電圧の比較、内蔵基準電圧との比較(`begin(INTERNAL2V5)`など、細かなレベル指定も可)、ヒステリシス、結果の読み取り、AC出力ピンへの出力、出力変化での`attachInterrupt()`式コールバック。比較結果は**ピンもCPUも使わずに**`CustomLogic`(`LOGIC_ANALOG_COMP`)や`EventSystem`(`EVENT_ANALOG_COMP`)へ直接渡せます。

### CustomLogic
[CustomLogic readme](../libraries/CustomLogic/README.md) CCL(Configurable Custom Logic)のルックアップテーブルを、固定ピンの定義済みユニットとして提供: ゲート(`AND`/`OR`/`XOR`/`NAND`/`NOR`/`XNOR`/`NOT`/`NOP`)か任意の3入力真理値表を選ぶだけで、結果が**CPU時間ゼロ**のハードウェアとして動きます。入力はユニットのピンのほか、アナログコンパレータ、自分自身の出力(ラッチが作れます)、もう一方のユニット、`EventSystem`経由の任意ピンから取れます(`setInputINn()`)。結果は専用OUTピン・代替ピン・ボードのイベント出力ピンへ**複数同時に**出せます(`setOutput()`/`addOutput()`)。出力変化の`attachInterrupt()`式コールバックも使えます。

### EventSystem
[EventSystem readme](../libraries/EventSystem/README.md) チップ内部の「配線」です。6本の定義済み接続(`EventSystem`〜`EventSystem5`)が、それぞれ1つの送り元 — Arduinoピン、AnalogCompの結果、CustomLogicの出力、またはソフトウェアの`trigger()`パルス — を任意個の送り先(ボード固定のイベント出力ピン、CustomLogicのイベント入力)へ運びます。`EventSystem.connect(8, 2);` だけで完結します。ピン送り元は**同一ポート同時2本まで**(ハードウェアの性質)。タイマ/USART/SPIのイベント機能は競合回避のため提供しません。

## USBクラスライブラリ
WazamonoボードはネイティブUSBデバイスです。CDCシリアルポート(`Serial`)に加え、同梱の2つのクラスライブラリでHIDデバイスやMIDI楽器として振る舞えます。いずれもAVR DU対応を上流へ提出済みのWazamonoフォーク版です(出自は各readmeの同梱注記を参照)。

### HID-Project
[HID-Project readme](../libraries/HID-Project/Readme.md) NicoHood氏の拡張HIDライブラリ(ws-asahi/HIDフォークから同梱、MIT): BootKeyboard/BootMouse、Keyboard、Mouse、AbsoluteMouse、Consumer(メディアキー)、System、Gamepad、RawHID。サンプル13本を同梱。

### MIDIUSB
[MIDIUSB readme](../libraries/MIDIUSB/README.adoc) Arduino公式MIDIUSBライブラリ(ws-asahi/MIDIUSBフォークから同梱、LGPL 2.1): ボードがUSB-MIDI楽器として認識され、MIDIイベントパケットを送受信できます。サンプル5本を同梱。

### HID
HID-Projectの土台となる低レベルのPluggableUSB HIDトランスポート(`HID_`)。スケッチから直接は使いません。

## 標準Arduinoライブラリ

### EEPROM
[EEPROM readme](../libraries/EEPROM/README.md) DUの**256バイト**内蔵EEPROMの標準API。消去はバイト単位です(USERROWと違い、バッファのような仕掛けは不要)。他アーキテクチャのEEPROM内部実装を前提にしたライブラリには注意してください。

### SPI
[SPI readme](../libraries/SPI/README.md) SPI0による標準SPIマスタAPI。各Wazamonoボードはシルク印刷どおりにSPIのピン割り付けを固定しているため(Tsurugi: UnoのD11-D13位置)、スケッチで`swap()`を呼ぶ必要はありません(呼ばないでください)。

### Wire
[Wire readme](../libraries/Wire/README.md) TWIのマスタ/スレーブ(デュアルモード対応)。標準API一式に加え、ジェネラルコール受信、第2アドレス、アドレスマスクに対応します。**内蔵プルアップは自動では有効になりません** — バスにプルアップ抵抗が無い場合は`Wire.usePullups()`を呼んでください(本来は実抵抗を付けるのが正道です)。

### SD
[SD readme](../libraries/SD/README.adoc) 標準のArduino SDカードライブラリ(SPI経由のFAT16/FAT32)。サンプル7本を同梱。

### SoftwareSerial
公式megaavrコアから無改変で継承していますが、**使わないのが最善**です: どのボードにもUSB CDCポート(`Serial`)と固定ピンのハードウェアUSARTがあります。SoftwareSerialは使用ピンの割り込みを占有し、ビットバンギングでCPU時間を消費します。

## 汎用ハードウェア

### Servo
[Servoリファレンス](https://www.arduino.cc/reference/en/libraries/servo/) megaTinyCore/DxCore系の改良版再実装: TCA0プリスケーラに依存しないため**PWM周波数を変えてもサーボが壊れず**、ISRのタイミングも改善されています。ライブラリマネージャ版のServoが同梱版より優先されてしまう場合は`#include <Servo_DxCore.h>`に変えてください — APIは同一です。

### tinyNeoPixel
[tinyNeoPixelの解説](tinyNeoPixel.md) WS2812系(NeoPixel)制御を2種類で提供: `tinyNeoPixel`(Adafruit互換、動的バッファ)と`tinyNeoPixel_Static`(フレームバッファを自分で宣言するためRAM消費がコンパイル結果に表示され、mallocも使いません)。show()のタイミングはこれらのパーツのAVRxt命令タイミング向けに書かれており、サポートする全クロックで成立します。
