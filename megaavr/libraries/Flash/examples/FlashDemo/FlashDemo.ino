/* FlashDemo - アプリからのフラッシュ自己書き換えデモ
 *
 * Flashライブラリの消去/ワード書き込み/バイト書き込み/配列書き込み/
 * 読み出し/マップドポインタの各APIを一通り実演します。
 * UkiUkiduinoではCDCブートローダのSPMエントリ経由で書き込みます。
 *
 * UkiUkiduino向けに日本語化
 */
#include <Flash.h>

#define BASE_ADDRESS (PROGMEM_SIZE - (0x800))
// フラッシュ末尾の2048バイト手前

uint16_t testArray[] = {0xBADD, 0xBEEF, 0xD00D};
char testArray2[15] = "Hello Flash";
uint8_t testArray3[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // ついでにLEDも点滅させる
  Serial.begin(115200);
  for (uint8_t i = 0; i < 40 && !Serial; i++) {
    delay(50);                     /* USB CDCホストの接続完了を待つ */
  }
  delay(200);
  // ユーザーがシリアルポートへ接続する猶予を与える
  for (byte i = 3; i; i--) {
    digitalWrite(LED_BUILTIN, CHANGE);
    Serial.print('.'); // 固まって見えないよう何か表示する
    delay(1000);
  }
  digitalWrite(LED_BUILTIN, LOW);
  Demo();
  // デモが終わったら、あとは何もしない。
}


void Demo() {
  readHalfPage(BASE_ADDRESS);
  Serial.println(F("First, make sure we can write the flash")); // まずフラッシュへ書けることを確認する
  byte check = Flash.checkWritable();
  if (check != FLASHWRITE_OK) {
    Serial.print(F("Error: "));
    Serial.println(Flash.checkWritable());
  }
  Serial.print(F("Erase the page: "));
  Serial.println(Flash.erasePage(BASE_ADDRESS, 1));
  readHalfPage(BASE_ADDRESS);
  Serial.println(F("Write some stuff to it:"));
  Serial.print(F("Word 0xCFF3 at base + 0: "));
  Serial.println(Flash.writeWord(BASE_ADDRESS, 0xCFF3));
  Serial.print(F("the number 1056 at base + 4: "));
  Serial.println(Flash.writeWord(BASE_ADDRESS + 4, 1056));
  // 1056は16進でも2進でも見つけやすい: 0x0420 = 00000100 00100000
  Serial.print(F("Word 0x8001 at base +7: "));
  Serial.println(Flash.writeWord(BASE_ADDRESS + 7, 0x8001));
  Serial.println(F(" Failed - words writes must be aligned"));
  Serial.print(F("Byte 00 at base + 8: "));
  Serial.println(Flash.writeByte(BASE_ADDRESS + 8, 0));
  Serial.print(F("Byte 12 at base + 9: "));
  Serial.println(Flash.writeByte(BASE_ADDRESS + 9, 0x12));
  // バイト書き込みはアラインメント不要。
  Serial.print(F("3 words starting at base + 12: "));
  Serial.println(Flash.writeWords(BASE_ADDRESS + 12, testArray, 3));
  Serial.print(F("10-byte long array starting at base + 0x20: "));
  Serial.println(Flash.writeBytes(BASE_ADDRESS + 0x20, testArray3, 10));
  Serial.print(F("15-byte long character array at base + 0x40: "));
  Serial.println(Flash.writeBytes(BASE_ADDRESS + 0x40, (uint8_t *)testArray2, 15));
  Serial.println(F("After writing, flash looks like"));
  readHalfPage(BASE_ADDRESS);
  Serial.println(F("Read back the words we wrote at base + 12: "));
  Serial.println(Flash.readWord(BASE_ADDRESS + 12), HEX);
  Serial.println(Flash.readWord(BASE_ADDRESS + 14), HEX);
  Serial.println(Flash.readWord(BASE_ADDRESS + 16), HEX);
  Serial.println(F("Get a pointer to that string, cast it to char*, and print it"));
  uint8_t *ptr = Flash.mappedPointer(BASE_ADDRESS + 0x40);
  if (ptr != NULL) {
    Serial.println((char *)ptr);
    Serial.print(F("No check for addresses being erased - write 0x18 to base + 0x43: "));
    Serial.println(Flash.writeByte(BASE_ADDRESS + 0x43, 0x18));
    Serial.println(F("Flash bits only go 1 -> 0 without an erase, so that 'l' (0x6C) becomes"));
    Serial.println(F("0x6C & 0x18 = 0x08 - a backspace, which your terminal will act on:"));
    Serial.println((char *)ptr);
  }
}


void loop() { /* 何もしない */
}

// フラッシュ256バイトを整形表示する
void readHalfPage(uint32_t StartAddress) {
  StartAddress &= 0x0001FF00;
  Serial.print(F("0x"));
  Serial.print(StartAddress, HEX);
  Serial.println(F(":"));
  for (unsigned long i = StartAddress; i < (StartAddress + 0x100); i++) {
    Serial.printHex(Flash.readByte(i)); // DxCoreのヘルパー関数 - 先頭の0も表示する。
    if ((i & 31) != 31) {
      // 1行32バイト、間にスペース(スペースは31個)
      Serial.print(' ');
    } else {
      // 行末の1個だけは代わりに改行を打つ。
      Serial.println();
    }
  }
  Serial.println();
}
