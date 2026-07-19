/* FlashWriteTest - AVR DU用USB CDCブートローダのSPMエントリスタブを
 * 経由したフラッシュ書き込みの実機検証
 *
 * 更新済みブートローダ(v0x1A01以降)を載せた基板での期待出力:
 *   checkWritable() = 0 (FLASHWRITE_OK)、続いてERASE/WRITE/GUARDがPASS。
 * スタブのない旧ブートローダではcheckWritable()が非ゼロを返し、
 * 書き込みエントリを呼ぶ前にテストを中止します。
 *
 * UkiUkiduino向けに日本語化
 */
#include <Flash.h>

#define TEST_ADDRESS (PROGMEM_SIZE - 512UL)  /* フラッシュ最終ページ */

uint16_t failures = 0;

void check(const char *what, bool ok) {
  Serial.print(what);
  Serial.println(ok ? ": PASS" : ": FAIL");
  if (!ok) failures++;
}

void setup() {
  Serial.begin(115200);
  for (byte i = 10; i; i--) { delay(500); if (Serial) break; }
  Serial.println(F("\n=== Flash write test (CDC bootloader SPM entry) ==="));
  Serial.print(F("checkWritable() = "));
  uint8_t w = Flash.checkWritable();
  Serial.println(w);
  if (w != FLASHWRITE_OK) {
    Serial.println(F("Not writable - stopping before any SPM call."));
    return;
  }

  Serial.print(F("erasePage returned "));
  Serial.println(Flash.erasePage(TEST_ADDRESS, 1));
  bool blank = true;
  for (uint16_t i = 0; i < 512; i++) {
    if (Flash.readByte(TEST_ADDRESS + i) != 0xFF) { blank = false; break; }
  }
  check("ERASE  (page reads 0xFF)", blank);

  for (uint16_t i = 0; i < 16; i++) {
    Flash.writeWord(TEST_ADDRESS + 2 * i, 0xA000 + i);
  }
  bool wok = true;
  for (uint16_t i = 0; i < 16; i++) {
    if (Flash.readWord(TEST_ADDRESS + 2 * i) != (0xA000 + i)) { wok = false; break; }
  }
  check("WRITE  (16 words pattern)", wok);

  check("GUARD  (byte 32 still 0xFF)", Flash.readByte(TEST_ADDRESS + 32) == 0xFF);

  Serial.print(F("Result: "));
  Serial.println(failures ? F("*** FAIL ***") : F("ALL PASS"));
}
void loop() {}
