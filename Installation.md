# WazamonoCore のインストール

WazamonoCore は、Wazamono シリーズ（AVR64DU32 搭載ボード）を Arduino IDE で開発するための専用コアです。

## 対応 IDE

- **Arduino IDE 1.8.13 以降**（推奨）
- **Arduino IDE 2.x**

> **Linux をお使いの場合:** Arduino IDE は必ず [arduino.cc](https://www.arduino.cc) の配布版を使用してください。ディストリビューションのパッケージマネージャ版は改変されており、サードパーティのボードパッケージが正常に動作しないことが知られています。

---

## 手動インストール（hardware フォルダ）

現在の推奨インストール方法です。

1. このリポジトリを `git clone` するか、ZIP をダウンロードして展開します。

2. スケッチブックの `hardware` フォルダに、フォルダ名を **`WazamonoCore`** として配置します。階層は以下のようになります。

   ```
   <スケッチブック>/
     └─ hardware/
          └─ WazamonoCore/
               └─ megaavr/
                    ├─ boards.txt
                    ├─ platform.txt
                    ├─ cores/
                    ├─ variants/
                    └─ ...
   ```

   スケッチブックの場所は、Arduino IDE の「ファイル > 環境設定 > スケッチブックの保存場所」で確認できます。

   - Windows 例: `ドキュメント\Arduino\hardware\WazamonoCore\`
   - macOS 例: `~/Documents/Arduino/hardware/WazamonoCore/`
   - Linux 例: `~/Arduino/hardware/WazamonoCore/`

3. Arduino IDE を再起動します。

4. 「ツール > ボード」に **WazamonoCore** が表示され、**Wazamono Tachi (AVR64DU32)** を選択できます。

> `git clone` で配置する場合、フォルダ名が `WazamonoCore` になっていることを確認してください（リポジトリ名のままで問題ありません）。

---

## ボードマネージャ経由（今後対応予定）

ボードマネージャ（JSON URL）からのインストールは現在準備中です。対応次第、本ページに URL を記載します。

---

## 初回のブートローダ書き込み（必要な場合）

Wazamono の製品ボードは出荷時に USB ブートローダが書き込まれているため、通常は USB 接続するだけでスケッチを書き込めます。

自作・修理などでブートローダの書き込みが必要な場合は、UPDI プログラマを使用します。

- 対応プログラマ: PICkit 4 / 5、Atmel-ICE、jtag2updi など
- 接続先: UPDI パッド（直列 470Ω 経由で MCU の PF7 へ）
- 手順: 「ツール > 書き込み装置」でプログラマを選択し、「ブートローダを書き込む」を実行

---

## 動作確認

インストール後、以下のスケッチで動作を確認できます。

```cpp
void setup() {
  Serial.begin(115200);          // Serial = USB CDC（変換チップ不要）
  pinMode(LED_BUILTIN, OUTPUT);
}
void loop() {
  Serial.println(millis());
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(1000);
}
```

ボードを選択して書き込み、シリアルモニタ（115200 bps）に値が表示され、オンボード LED が点滅すれば成功です。
