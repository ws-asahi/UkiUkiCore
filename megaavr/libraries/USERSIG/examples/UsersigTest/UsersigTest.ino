/***********************************************************************
 * UsersigTest - USERSIGライブラリを実機のUSERROWで検証する
 *
 * USERROW(ユーザー署名領域)はチップ消去の影響を受けず、通常の
 * スケッチ書き込みでも消えない小さな不揮発領域です。スケッチでは
 * なく「基板そのもの」に属する情報 - シリアル番号、校正値、基板
 * リビジョンなど - の保存場所として意図されています。
 *
 * AVR DUシリーズでは0x1200に512バイトあり、DA/DB/DD系の32バイトの
 * 16倍です。ライブラリ内に8ビット前提のインデックスが残っていると
 * 壊れるため、このテストで検証します。
 *
 * USERROWはフラッシュ的な性質を持ち、書き込みはビットを落とす
 * (1→0)ことしかできません。ビットを1へ戻すには行全体の消去が
 * 必要なため、ライブラリはそのような書き込みをRAMコピーに保留します:
 *
 *   USERSIG.write(idx, val) が1を返す → USERROWへ直接書き込まれた
 *                           が0を返す → 保留。USERSIG.flush()を呼ぶこと
 *   USERSIG.flush()                   → 消去+再書き込み。変化した
 *                                        バイト数を返す
 *   USERSIG.erase()                   → USERROW全体を消去
 *
 * 警告: このスケッチはUSERROWの消去と再書き込みを行います(1回の
 * 実行で2回の消去/書き込みサイクル)。USERROWの書き換え寿命は有限
 * です - テストとして実行し、ループで回さないでください。
 ***********************************************************************/
#include <USERSIG.h>

/* テストデータの配置。検査同士が互いを隠さないよう離してある。 */
#define MARKER_INDEX  500          /* 永続性マーカー(6バイト)             */
#define STRUCT_INDEX  480          /* put()/get()テスト用の構造体(12バイト) */
#define MARKER_MAGIC  0x47495355UL /* "USIG" */

struct Marker {
  uint32_t magic;
  uint16_t runs;
};

struct Cal {                       /* 実用でありそうなペイロードの例      */
  uint32_t serial;
  int16_t  offset;
  char     tag[6];
};

uint8_t tests = 0, failures = 0;

/* ライブラリを経由せずUSERROWを直接読む。インデックスを切り詰める
 * バグ(例: 300→44)を持つライブラリにテストが騙されないようにするため。 */
static inline uint8_t raw(uint16_t idx) {
  return *((volatile uint8_t *)(USER_SIGNATURES_START + idx));
}

void check(const __FlashStringHelper *what, bool ok) {
  tests++;
  if (!ok) {
    failures++;
  }
  Serial.print(ok ? F("  PASS  ") : F("  FAIL  "));
  Serial.println(what);
}

void dump(uint16_t start, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    if ((i & 0x0F) == 0) {
      Serial.print(F("\n  0x"));
      uint16_t a = start + i;
      if (a < 0x100) Serial.print('0');
      if (a < 0x10)  Serial.print('0');
      Serial.print(a, HEX);
      Serial.print(F(": "));
    }
    uint8_t b = raw(start + i);
    if (b < 0x10) Serial.print('0');
    Serial.print(b, HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < 40 && !Serial; i++) {
    delay(50);                     /* USB CDCホストの接続完了を待つ      */
  }
  delay(200);

  Serial.println(F("\n=== USERSIG (USERROW) test ==="));
  Serial.print(F("USER_SIGNATURES_START : 0x"));
  Serial.println((uint16_t)USER_SIGNATURES_START, HEX);
  Serial.print(F("USER_SIGNATURES_SIZE  : "));
  Serial.println((uint16_t)USER_SIGNATURES_SIZE);
  Serial.print(F("USERSIG.length()      : "));
  Serial.println((uint16_t)USERSIG.length());

  /* 安全ゲート。length()がデバイスの実サイズと食い違う場合、ライブラリの
   * どこかに8ビットのインデックス/カウンタが残っている: write()は誤った
   * バイトに着地し、flush()は512バイトのUSERROWで永久ループする。
   * 何かに触れる前にここで停止する。 */
  if ((uint16_t)USERSIG.length() != (uint16_t)USER_SIGNATURES_SIZE) {
    Serial.println(F("\n!! The library reports a size that is not the device's USERROW size."));
    Serial.println(F("!! It is not safe for a 512-byte USERROW - a flush() would hang here."));
    Serial.println(F("!! Stopping before any write."));
    return;
  }

  /* この基板でテストを実行したことがあるか? 消去の前にマーカーを読む。 */
  Marker prev;
  USERSIG.get(MARKER_INDEX, prev);
  Serial.println();
  if (prev.magic == MARKER_MAGIC) {
    Serial.print(F("Marker from a previous run found - test has run "));
    Serial.print(prev.runs);
    Serial.println(F(" time(s) on this board."));
    Serial.println(F("(The USERROW survived the reset/upload.)"));
  } else {
    Serial.println(F("No marker yet: first run on this board, or the USERROW is blank."));
  }

  Serial.println(F("\nUSERROW before the test (first 32 and last 32 bytes):"));
  dump(0, 32);
  dump(USER_SIGNATURES_SIZE - 32, 32);

  /* ---- 1. 消去 ------------------------------------------------------ */
  Serial.println(F("\n[1] erase()"));
  USERSIG.erase();
  bool blank = true;
  for (uint16_t i = 0; i < USER_SIGNATURES_SIZE; i++) {
    if (raw(i) != 0xFF) {
      blank = false;
      break;
    }
  }
  check(F("all 512 bytes read 0xFF"), blank);

  /* ---- 2. 直接書き込み(255超のインデックスを含む) ------------------- */
  Serial.println(F("\n[2] write() on an erased USERROW - goes straight to flash"));
  const uint16_t idx[6] = {0, 1, 255, 256, 300, 511};
  const uint8_t  val[6] = {0xA5, 0x5A, 0x11, 0x22, 0x33, 0x44};
  bool direct = true, stored = true;
  for (uint8_t i = 0; i < 6; i++) {
    if (USERSIG.write(idx[i], val[i]) != 1) {
      direct = false;
    }
  }
  check(F("every write() returned 1 (written immediately)"), direct);

  for (uint8_t i = 0; i < 6; i++) {
    if (raw(idx[i]) != val[i]) {           /* raw: 真のアドレスを見る */
      stored = false;
    }
  }
  check(F("bytes landed at the right addresses (0,1,255,256,300,511)"), stored);
  check(F("read() agrees with the USERROW"),
        USERSIG.read(300) == 0x33 && USERSIG.read(511) == 0x44);

  /* ---- 3. 消去が必要な書き込みは保留される --------------------------- */
  Serial.println(F("\n[3] write() that needs a bit set back to 1 - buffered"));
  int8_t r = USERSIG.write(0, 0xFF);       /* 0xA5→0xFFには消去が必要 */
  check(F("write() returned 0 (deferred until flush)"), r == 0);
  check(F("read() already reports the pending value"), USERSIG.read(0) == 0xFF);
  check(F("the USERROW itself is still unchanged"), raw(0) == 0xA5);

  /* ---- 4. flush ------------------------------------------------------ */
  Serial.println(F("\n[4] flush() - erase and rewrite the row from the buffer"));
  int16_t changed = USERSIG.flush();
  Serial.print(F("  flush() reported changed bytes: "));
  Serial.println(changed);
  check(F("exactly one byte was reported as changed"), changed == 1);
  check(F("the new value reached the USERROW"), raw(0) == 0xFF);
  check(F("the other bytes survived the erase/rewrite"),
        raw(1) == 0x5A && raw(255) == 0x11 && raw(256) == 0x22 &&
        raw(300) == 0x33 && raw(511) == 0x44);

  /* ---- 5. 255超のインデックスへの構造体put()/get() -------------------- */
  Serial.println(F("\n[5] put()/get() a 12-byte object at index 480"));
  Cal out = {0xDEADBEEF, -1234, "DUtst"};
  USERSIG.put(STRUCT_INDEX, out);
  USERSIG.flush();                          /* 保留分があれば確定する */
  Cal in;
  USERSIG.get(STRUCT_INDEX, in);
  check(F("the object survived the round trip"),
        in.serial == out.serial && in.offset == out.offset &&
        strcmp(in.tag, out.tag) == 0);
  check(F("it really is stored at index 480"),
        raw(STRUCT_INDEX) == (uint8_t)(out.serial & 0xFF));

  /* ---- 6. 永続性マーカー --------------------------------------------- */
  Serial.println(F("\n[6] persistence marker"));
  Marker next;
  next.magic = MARKER_MAGIC;
  next.runs  = (prev.magic == MARKER_MAGIC) ? (uint16_t)(prev.runs + 1) : 1;
  USERSIG.put(MARKER_INDEX, next);
  USERSIG.flush();
  Marker back;
  USERSIG.get(MARKER_INDEX, back);
  check(F("marker written and read back"),
        back.magic == MARKER_MAGIC && back.runs == next.runs);

  Serial.println(F("\nUSERROW after the test (first 32 and last 32 bytes):"));
  dump(0, 32);
  dump(USER_SIGNATURES_SIZE - 32, 32);

  /* ---- 結果まとめ ----------------------------------------------------- */
  Serial.print(F("\nResult: "));
  Serial.print(tests - failures);
  Serial.print('/');
  Serial.print(tests);
  Serial.println(failures ? F(" - *** FAILURES ***") : F(" - ALL PASS"));
  Serial.print(F("Run count stored in the USERROW: "));
  Serial.println(next.runs);
  Serial.println(F("Reset the board, or upload another sketch and this one again,"));
  Serial.println(F("then re-run: the run count must keep increasing - that is the"));
  Serial.println(F("USERROW outliving both the reset and the upload."));
}

void loop() {
  /* 何もしない - USERROWの書き換え寿命は有限なので、テストは一度だけ実行する。 */
}
