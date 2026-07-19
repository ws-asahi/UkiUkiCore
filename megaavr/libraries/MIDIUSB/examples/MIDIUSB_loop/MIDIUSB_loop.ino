/*
 * MIDIUSB_loop.ino - MIDIループバック
 *
 * 原作: gurbrinder grewal (2015) / Arduino LLCによる改変
 * UkiUkiduino向けに日本語化
 *
 * 受信したMIDIコマンドをそのまま送り返します。ホスト側の
 * 送受信経路のテストに使えます。
 */

#include "MIDIUSB.h"

// 第1引数はイベント種別(0x09=ノートオン、0x08=ノートオフ)。
// 第2引数はノートオン/オフとチャネルの合成値。
// チャネルは0~15(ユーザー向け表記では通常1~16)。
// 第3引数はノート番号(48=中央のド)。
// 第4引数はベロシティ(64=普通、127=最強)。

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
  pinMode(LED_BUILTIN, OUTPUT);
}

// 第1引数はイベント種別(0x0B=コントロールチェンジ)。
// 第2引数はイベント種別とチャネルの合成値。
// 第3引数はコントロール番号(0~119)。
// 第4引数はコントロール値(0~127)。

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void loop() {
  //MidiUSB.accept();
  //delayMicroseconds(1);
  midiEventPacket_t rx;
  do {
    rx = MidiUSB.read();
    if (rx.header != 0) {
      // 受信したMIDIコマンドを送り返す
      MidiUSB.sendMIDI(rx);
      MidiUSB.flush();
    }
  } while (rx.header != 0);
}
