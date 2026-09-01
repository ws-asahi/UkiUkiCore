#!/usr/bin/env bash
# make_release.sh - WazamonoCore のリリース用 tar.bz2 を作り、
#                   docs/package_wazamono_index.json の checksum と size を更新する。
#
#   使い方: tools/make_release.sh 0.0.6
#
# Board Manager 用の索引は、配布する書庫の SHA-256 と実バイト数を持ちます。
# 手で書き換えると必ずどこかでずれるので、書庫の作成と索引の更新を
# 同じ 1 回の操作にまとめています。
set -euo pipefail

VERSION="${1:-}"
if [[ -z "$VERSION" ]]; then
  echo "使い方: $0 <version>   例: $0 0.0.6" >&2
  exit 1
fi

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

INDEX="docs/package_wazamono_index.json"
ARCHIVE="WazamonoCore-${VERSION}.tar.bz2"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

# --- platform.txt の version と一致しているか確認 -------------------------
# platform.txt は Windows で CRLF のまま取得されることがある。CR を落とさずに
# 比較すると、値の末尾に \r が付いたまま突き合わせることになり、一致するはずの
# 場合でも失敗する（しかも \r で行頭に戻るため、エラー文まで崩れて読めなくなる）。
PLATFORM_VER="$(sed -n 's/^version=\([0-9].*\)$/\1/p' megaavr/platform.txt | tail -1 | tr -d '\r[:space:]')"
if [[ "$PLATFORM_VER" != "$VERSION" ]]; then
  echo "エラー: megaavr/platform.txt の version は '$PLATFORM_VER' で、指定の '$VERSION' と一致しません。" >&2
  exit 1
fi

# --- 書庫の作成 -----------------------------------------------------------
# Board Manager は書庫内の単一のトップディレクトリを剥がし、その直下を
# プラットフォームの根として扱う。つまり boards.txt / platform.txt /
# cores / variants は「トップ直下」に無ければならない。megaavr を
# ディレクトリごと入れると 1 階層深くなり、arduino-cli が
# プラットフォームを見つけられずに導入が失敗する。
# v0.0.3 の書庫も megaavr の中身を展開した形になっている。
echo "書庫を作成: $ARCHIVE"
mkdir -p "$STAGE/WazamonoCore-${VERSION}"
cp -a megaavr/. "$STAGE/WazamonoCore-${VERSION}/"

find "$STAGE" \( -name '*.lst' -o -name '*.map' -o -name '*.o' -o -name '*.d' \
              -o -name '.DS_Store' -o -name 'platform.local.txt' \) -delete
find "$STAGE" -name '.git*' -prune -exec rm -rf {} + 2>/dev/null || true

# 展開後に platform.txt がトップ直下に来ているかを必ず確かめる。
if [[ ! -f "$STAGE/WazamonoCore-${VERSION}/platform.txt" ]]; then
  echo "エラー: platform.txt が書庫のトップ直下にありません。構造が不正です。" >&2
  exit 1
fi

tar -cjf "$ARCHIVE" -C "$STAGE" "WazamonoCore-${VERSION}"

SIZE="$(wc -c < "$ARCHIVE" | tr -d ' ')"
SHA="$(sha256sum "$ARCHIVE" | cut -d' ' -f1)"
echo "  size     = $SIZE"
echo "  sha256   = $SHA"

# --- 索引の更新 -----------------------------------------------------------
python3 - "$INDEX" "$VERSION" "$SHA" "$SIZE" <<'PY'
import json, sys
index, version, sha, size = sys.argv[1:5]
with open(index, encoding='utf-8') as f:
    doc = json.load(f)
pl = doc['packages'][0]['platforms'][0]
pl['version']         = version
pl['url']             = f"https://github.com/ws-asahi/WazamonoCore/releases/download/v{version}/WazamonoCore-{version}.tar.bz2"
pl['archiveFileName'] = f"WazamonoCore-{version}.tar.bz2"
pl['checksum']        = f"SHA-256:{sha}"
pl['size']            = str(size)
with open(index, 'w', encoding='utf-8') as f:
    json.dump(doc, f, indent=2, ensure_ascii=False)
    f.write('\n')
print(f"{index} を v{version} へ更新しました")
PY

cat <<EOS

次の手順:
  1. git add $INDEX && git commit
  2. git tag -a v${VERSION} -m "WazamonoCore ${VERSION}"
  3. GitHub の Releases で v${VERSION} を作成し、${ARCHIVE} を添付
  4. https://wazamono.ws-asahi.net/package_wazamono_index.json へ索引を反映
  5. Arduino IDE でボードマネージャから素の導入を確認
EOS
