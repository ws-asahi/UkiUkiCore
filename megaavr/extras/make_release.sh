#!/usr/bin/env bash
# ============================================================================
#  make_release.sh - UkiUkiCore Boards Manager release packager
#
#  Builds the platform archive UkiUkiCore-<ver>.tar.bz2 from megaavr/ (same
#  layout as WazamonoCore releases: single root folder "UkiUkiCore-<ver>/"
#  containing boards.txt, platform.txt, cores/, variants/, ...), computes
#  SHA-256 + size, and writes them into docs/package_ukiuki_index.json.
#
#  Usage (from repo root or anywhere, Git Bash OK):
#      megaavr/extras/make_release.sh [version]
#  If [version] is omitted, the LAST "version=" line of megaavr/platform.txt
#  is used (the IDE-workaround override, e.g. "version=0.0.3").
#
#  After running:
#    1. Review + commit docs/package_ukiuki_index.json
#    2. git tag v<ver> && git push --tags
#    3. Create GitHub Release v<ver> and upload dist/UkiUkiCore-<ver>.tar.bz2
#       (the URL in the index points at
#        releases/download/v<ver>/UkiUkiCore-<ver>.tar.bz2)
#    4. Push main; GitHub Pages (docs/) serves the index at
#       https://ws-asahi.github.io/UkiUkiCore/package_ukiuki_index.json
#
#  NOTE: toolsDependencies (avr-gcc 15.2.0-wazamono1 / avrdude 8.1-wazamono1)
#  are downloaded from ws-asahi/wazamono-toolchain releases - shared with
#  WazamonoCore, nothing to rebuild here.
# ============================================================================
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
MEGAAVR="$REPO_ROOT/megaavr"
INDEX="$REPO_ROOT/docs/package_ukiuki_index.json"
DIST="$REPO_ROOT/dist"

VER="${1:-}"
if [ -z "$VER" ]; then
  VER="$(grep -E '^version=' "$MEGAAVR/platform.txt" | tail -1 | cut -d= -f2 | tr -d '[:space:]')"
fi
case "$VER" in
  *[!0-9.]*|"") echo "ERROR: bad version '$VER'"; exit 1;;
esac
echo "Packaging UkiUkiCore version: $VER"

NAME="UkiUkiCore-$VER"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT
mkdir -p "$STAGE/$NAME" "$DIST"

# Copy platform contents, excluding machine-local / junk files.
tar -C "$MEGAAVR" -cf - \
    --exclude='platform.local.txt' \
    --exclude='make_platform_local.bat' \
    --exclude='.DS_Store' \
    --exclude='*.pyc' \
    . | tar -C "$STAGE/$NAME" -xf -

TARBALL="$DIST/$NAME.tar.bz2"
rm -f "$TARBALL"
tar -C "$STAGE" -cjf "$TARBALL" "$NAME"

SHA="$(sha256sum "$TARBALL" | cut -d' ' -f1)"
SIZE="$(wc -c < "$TARBALL" | tr -d '[:space:]')"
echo "  archive : $TARBALL"
echo "  sha256  : $SHA"
echo "  size    : $SIZE"

python3 - "$INDEX" "$VER" "$SHA" "$SIZE" <<'PYEOF'
import json, sys
index_path, ver, sha, size = sys.argv[1:5]
d = json.load(open(index_path))
p = d["packages"][0]["platforms"][0]
p["version"] = ver
p["url"] = f"https://github.com/ws-asahi/UkiUkiCore/releases/download/v{ver}/UkiUkiCore-{ver}.tar.bz2"
p["archiveFileName"] = f"UkiUkiCore-{ver}.tar.bz2"
p["checksum"] = f"SHA-256:{sha}"
p["size"] = size
with open(index_path, "w") as f:
    json.dump(d, f, indent=2, ensure_ascii=False)
    f.write("\n")
print(f"Updated {index_path} -> platform {ver}")
PYEOF

echo "Done. Next: commit index, tag v$VER, upload $NAME.tar.bz2 to the GitHub Release."
