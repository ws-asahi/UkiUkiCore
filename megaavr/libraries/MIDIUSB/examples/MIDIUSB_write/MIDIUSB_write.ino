/*
 * MIDIUSB_write.ino - MIDIメッセージの送信
 *
 * 原作: gurbrinder grewal (2015) / Arduino LLCによる改変
 * UkiUkiduino向けに日本語化
 *
 * 中央のドを繰り返し弾くUSB-MIDIデバイスになります。DAWや
 * MIDIモニタで受信して確認してください。
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
  Serial.println("Sending note on");
  noteOn(0, 48, 64);   // チャネル0、中央のド、普通の強さ
  MidiUSB.flush();
  delay(500);
  Serial.println("Sending note off");
  noteOff(0, 48, 64);  // チャネル0、中央のド、普通の強さ
  MidiUSB.flush();
  delay(1500);

  // controlChange(0, 10, 65); // チャネル0のコントローラ10を65にする
}
