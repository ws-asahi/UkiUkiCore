/*
 * MIDIUSB_buzzer.ino - MIDIで圧電ブザーを鳴らす
 *
 * 原作: Paulo Costa
 * UkiUkiduino向けに日本語化
 *
 * 受信したノートオンの音程で圧電ブザーを鳴らします。
 * ブザー(またはスピーカー+抵抗)をD9とGNDの間に接続し、
 * DAWやMIDIキーボードアプリから演奏してください。
 */

#include <MIDIUSB.h>
#include "pitchToFrequency.h"

#define BUZZ_PIN 9   // D9: 圧電ブザー接続ピン

const char* pitch_name(byte pitch) {
  static const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  return names[pitch % 12];
}

int pitch_octave(byte pitch) {
  return (pitch / 12) - 1;
}

void noteOn(byte channel, byte pitch, byte velocity) {
  tone(BUZZ_PIN, pitchFrequency[pitch]);   // ノート番号を周波数に変換して鳴らす

  Serial.print("Note On: ");
  Serial.print(pitch_name(pitch));
  Serial.print(pitch_octave(pitch));
  Serial.print(", channel=");
  Serial.print(channel);
  Serial.print(", velocity=");
  Serial.println(velocity);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  noTone(BUZZ_PIN);

  Serial.print("Note Off: ");
  Serial.print(pitch_name(pitch));
  Serial.print(pitch_octave(pitch));
  Serial.print(", channel=");
  Serial.print(channel);
  Serial.print(", velocity=");
  Serial.println(velocity);
}

void controlChange(byte channel, byte control, byte value) {
  Serial.print("Control change: control=");
  Serial.print(control);
  Serial.print(", value=");
  Serial.print(value);
  Serial.print(", channel=");
  Serial.println(channel);
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  midiEventPacket_t rx = MidiUSB.read();
  switch (rx.header) {
    case 0:
      break; // 保留中のイベントなし

    case 0x9:
      noteOn(
        rx.byte1 & 0xF,  // チャネル
        rx.byte2,        // 音程
        rx.byte3         // ベロシティ
      );
      break;

    case 0x8:
      noteOff(
        rx.byte1 & 0xF,  // チャネル
        rx.byte2,        // 音程
        rx.byte3         // ベロシティ
      );
      break;

    case 0xB:
      controlChange(
        rx.byte1 & 0xF,  // チャネル
        rx.byte2,        // コントロール番号
        rx.byte3         // 値
      );
      break;

    default:
      Serial.print("Unhandled MIDI message: ");
      Serial.print(rx.header, HEX);
      Serial.print("-");
      Serial.print(rx.byte1, HEX);
      Serial.print("-");
      Serial.print(rx.byte2, HEX);
      Serial.print("-");
      Serial.println(rx.byte3, HEX);
  }
}
