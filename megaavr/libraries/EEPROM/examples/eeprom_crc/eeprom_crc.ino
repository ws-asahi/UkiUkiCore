/* eeprom_crc(EEPROMのCRC検証)
 *
 * 原作: Christopher Andrews
 * CRCアルゴリズムはpycrc(MITライセンス)により生成。
 *
 * CRCはデータが変化・破損していないかを調べる簡単な方法です。
 * このサンプルはEEPROMの内容から直接CRC値を計算します。
 * EEPROMオブジェクトを配列のように(EEPROM[i]の形で)扱えることを
 * 示すのが主な目的です。
 *
 * メモ: avr-libcにはアセンブラで書かれた高速なCRC16実装
 * (util/crc16.h)があり、実用にはそちらの方が適しています。
 *
 * UkiUkiduino向けに日本語化。
 */

#include <Arduino.h>
#include <EEPROM.h>

void setup() {

  // USBシリアルを開始する
  Serial.begin(115200);

  // CRCを計算する対象データの長さを表示する
  Serial.print("EEPROM length: ");
  Serial.println(EEPROM.length());

  // eeprom_crc()の計算結果を表示する
  Serial.print("CRC32 of EEPROM data: 0x");
  Serial.println(eeprom_crc(), HEX);
  Serial.print("\n\nDone!");
}

void loop() {
  /* 何もしない */
}

unsigned long eeprom_crc(void) {

  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = ~0L;

  for (int index = 0 ; index < EEPROM.length()  ; ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}
