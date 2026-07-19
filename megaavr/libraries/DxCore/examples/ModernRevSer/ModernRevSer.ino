/* UkiUkiduinoに載っているAVR DUのシリコンリビジョンを読み取ります。
 * あわせてヒューズとシリアル番号も表示します。ヒューズとシリアル番号は
 * 「日付コードがシリアル番号に埋め込まれているか」を調べる試みの一環で、
 * 何らかの方法で日付コードを割り出せる可能性があります。
 * シリアル番号について面白い点:
 *    データバイトは値域を全部使っていないように見えます - 多くの
 *        バイトは16進表示すると10進の2桁になる、つまりBCDです。
 *    わずかなエントロピーもシリアル番号全体に均等には分布して
 *        いません。要するにこれは「シリアル番号」であって「シリアル
 *        番号のハッシュ」ではないので、推測不能であることを当てに
 *        しないでください。
 *    そのため、シリアル番号から追加の情報を推定できる可能性があります。
 *    実際、IOヘッダには各バイトの意味が書かれています! 実際に観測される
 *    値の類似性から、他の型番でも同様の番号体系と推測されます。
 *  LOTNUM0:LOTNUM1:LOTNUM2:LOTNUM3:LOTNUM4:LOTNUM5  ロット番号6バイト
 *  RANDOM :SCRIBE : XPOS0 : XPOS1 : YPOS0 : YPOS1   乱数1(SCRIBEは不明)、ウェハ上のダイのX/Y位置が各2バイト
 *   RES0  : RES1  : RES2  : RES3                    予約4バイト
 *
 * UkiUkiduino向けに日本語化
 */


#if (__AVR_ARCH__ < 100)
  #error "このスケッチは modern AVR(2016年以降のペリフェラル/命令タイミング刷新世代)専用です"
#endif
void setup() {
  Serial.begin(115200);
  Serial.println();
  delay(1000);
  Serial.print("REVID: ");
  char major = 0x40 + (SYSCFG.REVID >> 4);   // 例: REVID 0x12は"A2"と表示される
  Serial.print(major);
  Serial.println(SYSCFG.REVID & 0x0F);
  Serial.print("S/N: ");
  volatile uint8_t *mptr = &SIGROW_SERNUM0;
  showHex(*mptr++);
  for (byte i = 0; i < 15; i++) {
    Serial.print(':');
    showHex(*mptr++);
  }
  Serial.println();
  Serial.print("FUSES: ");
  mptr = FUSES_START;
  showHex(*mptr++);
  for (byte i = 1; i < 9; i++) {
    Serial.print(':');
    showHex(*mptr++);
  }
  Serial.println();
}

void showHex(const byte b) {
  char x = (b >> 4) | '0';
  if (x > '9') {
    x += 7;
  }
  Serial.write(x);
  x = (b & 0x0F) | '0';
  if (x > '9') {
    x += 7;
  }
  Serial.write(x);
}

void loop() {
  // 何もしない

}
