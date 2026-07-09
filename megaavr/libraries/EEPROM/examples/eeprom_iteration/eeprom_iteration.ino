/* eeprom_iteration(EEPROMの走査方法)
 *
 * EEPROMの全バイトを順に処理する、最も基本的な書き方を
 * 3パターン示すサンプル集です。
 *
 * 実行すること自体に意味はなく、書き方の紹介が目的です。
 *
 * 原作: Christopher Andrews 2015 (MITライセンス)
 * UkiUkiduino向けに日本語化。
 */

#include <EEPROM.h>

void setup() {

  /*
   * forループでEEPROMを走査する。
   */

  for (int index = 0 ; index < EEPROM.length() ; index++) {

    // 各バイトに1を足す
    EEPROM[index] += 1;
  }

  /*
   * whileループでEEPROMを走査する。
   */

  int index = 0;

  while (index < EEPROM.length()) {

    // 各バイトに1を足す
    EEPROM[index] += 1;
    index++;
  }

  /*
   * do-whileループでEEPROMを走査する。
   */

  int idx = 0;  // 上の'index'と名前が重ならないよう'idx'にした

  do {

    // 各バイトに1を足す
    EEPROM[idx] += 1;
    idx++;
  } while (idx < EEPROM.length());


} // setup関数の終わり

void loop() {}
