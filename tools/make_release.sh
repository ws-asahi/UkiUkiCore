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
#      tools/make_release.sh [version]
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
#  NOTE: toolsDependencies (avr-gcc 15.2.0-wazamono2 / avrdude 8.1-wazamono2)
#  are downloaded from ws-asahi/wazamono-toolchain releases - shared with
#  WazamonoCore, nothing to rebuild here.  Keep them in sync with the
#  {runtime.tools.<name>-<version>.path} pins in megaavr/platform.txt.
#
#  The index keeps every released platform version (newest first) so users can
#  downgrade from the Boards Manager; an existing entry for [version] is
#  replaced in place, a new version is inserted at the top.
# ============================================================================
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
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

python3 - "$INDEX" "$VER" "$SHA" "$SIZE" "$MEGAAVR/boards.txt" <<'PYEOF'
import json, sys, re
index_path, ver, sha, size, boards_txt = sys.argv[1:6]
# board display names for the Boards Manager entry, straight from boards.txt
boards = [{"name": m.group(1).strip()}
          for m in re.finditer(r"^[A-Za-z0-9_]+\.name=(.+)$", open(boards_txt, encoding="utf-8").read(), re.M)]
import copy
d = json.load(open(index_path))
plats = d["packages"][0]["platforms"]
existing = [p for p in plats if p["version"] == ver]
if existing:
    p = existing[0]
else:
    p = copy.deepcopy(plats[0])   # newest entry is the template (boards, toolsDependencies, ...)
    plats.insert(0, p)
p["version"] = ver
p["url"] = f"https://github.com/ws-asahi/UkiUkiCore/releases/download/v{ver}/UkiUkiCore-{ver}.tar.bz2"
p["archiveFileName"] = f"UkiUkiCore-{ver}.tar.bz2"
p["checksum"] = f"SHA-256:{sha}"
p["size"] = size
p["boards"] = boards
with open(index_path, "w") as f:
    json.dump(d, f, indent=2, ensure_ascii=False)
    f.write("\n")
print(f"Updated {index_path} -> platform {ver}")
PYEOF

echo "Done. Next: commit index, tag v$VER, upload $NAME.tar.bz2 to the GitHub Release."
