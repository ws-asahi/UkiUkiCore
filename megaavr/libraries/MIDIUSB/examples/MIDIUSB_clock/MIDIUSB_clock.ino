/*
 * MIDIUSB_clock.ino - MIDIクロック同期
 *
 * ソフトウェアから送られるMIDIクロック(パルス)メッセージに
 * 同期するビートクロックの簡単な例です。
 *
 * LeonardoとAbletonでの動作確認に基づくサンプルです:
 * 環境設定のMIDI Syncで本デバイスのOutputを選び、Syncボタンを
 * 有効化、クロックタイプをPatternへ変更します。多くの場合
 * Sync Delayの調整も必要です。
 *
 * 原作: Ernest Warzocha (2016)
 * UkiUkiduino向けに日本語化
 */

#include "MIDIUSB.h"

// 4分音符あたりのパルス数。1拍=24パルス。
// テンポはソフト側のBPMに従う。
int ppqn = 0;

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
  Serial.begin(115200);
}

void loop() {

  midiEventPacket_t rx;

  do {
    rx = MidiUSB.read();

    // パルスを数え、拍ごとにノートを送る
    if(rx.byte1 == 0xF8){
       ++ppqn;

       if(ppqn == 24){
          noteOn(1,48,127);
          MidiUSB.flush();
          ppqn = 0;
       };
    }
    // クロック開始バイト
    else if(rx.byte1 == 0xFA){
      noteOn(1,48,127);
      MidiUSB.flush();
      ppqn = 0;
    }
    // クロック停止バイト
    else if(rx.byte1 == 0xFC){
      noteOff(1,48,0);
      MidiUSB.flush();
      ppqn = 0;
    };

  } while (rx.header != 0);

}
