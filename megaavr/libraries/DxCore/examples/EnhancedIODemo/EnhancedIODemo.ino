/*********************\\*****************************//************************
                       \\   拡張I/O APIデモ         //
                        ^^-------------------------^^

AVR DUシリーズのI/Oハードウェアは、従来のクラシックAVRのものより
高機能です。このコアは、新しいピンI/O機能を活用するための簡単な
I/O関数をいくつか提供しています。このファイルはその働きと呼び出し方を
手短に実演します。

例ではD9とD8を使います(UkiUkiduinoでは通常のGPIOです)。

このスケッチはそのまま使うためのものではありません。出発点、あるいは
コピー&ペースト元の資料として使ってください。

UkiUkiduino向けに日本語化
******************************************************************************/

#define DEMO_PIN 9   // D9
#define DEMO_PIN2 8  // D8

void setup() {


}


void loop() {
  openDrainBitbang(0x0DF0AD8B); // 人間の読み順(エンディアン)なら0x8BADF00D
}

/*-----------------------------------------------------------------------------
openDrain(pin,value) と openDrainFast(pin,value)

openDrain()自体はmodern AVR固有の機能ではなく、「欠けていた」デジタル
I/O関数です。プルアップ付きで使うには、先にpinModeでINPUT_PULLUPにして
からopenDrain()を呼びます - クラシックAVRの挙動を踏襲し、コアは指示
されない限りピンを入力のままにすることを思い出してください。
  使い方:
    openDrain(DEMO_PIN, LOW);
    openDrain(DEMO_PIN, FLOATING);
    openDrain(DEMO_PIN, CHANGE);
    openDrainFast(DEMO_PIN, LOW);
    openDrainFast(DEMO_PIN, FLOATING);
    openDrainFast(DEMO_PIN, CHANGE);

  LOWはピンをOUTPUTにします。
  FLOATINGはピンをINPUTにします。プルアップが有効か外付けプルアップが
あれば、他のデバイスがLOWへ引っ張っていない限りピンはHIGHへ吊られ、
なければフロートします。
  CHANGEは方向をトグルします。

  Fast系のI/O関数はすべて、ピンに定数を渡す必要があり、値もできるだけ
  定数にすべきです。両方が定数で値がCHANGE以外なら、cbi/sbi 1命令
  (フラッシュ2バイト・1クロック)に最適化されます。この関数は
  ((always_inline))ですが、2バイト1命令ならその方が常に効率的です。
  値がCHANGEの場合は2命令(うち1つは倍サイズのSTS)で6バイト・
  3クロックになります。

-----------------------------------------------------------------------------*/

void openDrainBitbang(uint32_t data) {
  pinMode(DEMO_PIN, INPUT_PULLUP);
  pinMode(DEMO_PIN2, INPUT_PULLUP);
  openDrain(DEMO_PIN, FLOATING);
  openDrain(DEMO_PIN2, FLOATING);
  // これで両ピンともプルアップ付きのオープンドレインになった。
  // ここでは一方をクロック、他方をデータに使い、I2Cのように
  // 「ピンがHIGHへ戻るのを確認しながら」通信する体の何かをやる。
  // ピンの立ち上がり待ち以外の間、コードは進む
  for (uint8_t i = 0; i < 32; i++) {
    while (digitalReadFast(DEMO_PIN) != HIGH || digitalReadFast(DEMO_PIN2) != HIGH);
    // 両ピンがHIGHへ吊られるのを待つ - ループはおそらく回らないが、
    // 配線容量が大きい/プルアップが弱い/他デバイスがLOW保持
    // (I2Cのクロックストレッチ等)の可能性はある
    _NOPNOP(); // 受信側がこちらと同じ状態を見られるよう4クロック待つ。もっと待ってもよい
    _NOPNOP();
    if (((uint8_t)data) & 0x01) {
      openDrainFast(DEMO_PIN2, LOW); // データ線をセットする - おそらくcbi, sbrc, sbiにコンパイルされる
      // (コンパイラに期待する動作は確かにそれだが、そこまで賢くないこともある)
    }
    _NOPNOP(); // 受信側がこちらと同じ状態を見られるよう4クロック待つ。もっと待ってもよい
    _NOPNOP();
    openDrainFast(DEMO_PIN, LOW);
    data >>= 1;  // すぐ解放すると、LOWの時間が1マイクロ秒に満たなくなる。
    // ここでこの計算を挟むこと自体が1/4マイクロ秒ほどの遅延になる。
    openDrainFast(DEMO_PIN, FLOATING);
    openDrainFast(DEMO_PIN2, FLOATING);
    // ピンを解放する。
  }

}
