#!/usr/bin/env bash
# ============================================================================
#  tools/setup_toolchain.sh  -  UkiUkiCore 手動インストール用ツールチェーン配置スクリプト
#
#  docs/package_ukiuki_index.json(ボードマネージャ用インデックス)を読み、最新の
#  platform が要求する avr-gcc / avrdude(toolsDependencies)をダウンロードして、
#  このディレクトリ(hardware/UkiUkiCore/tools/)に Arduino IDE が認識する形で配置します:
#
#      <sketchbook>/hardware/UkiUkiCore/
#          megaavr/                        <- コア(platform.txt など)
#          tools/avr-gcc/<version>/        <- bin/avr-gcc ...
#          tools/avrdude/<version>/        <- bin/avrdude, etc/avrdude.conf
#
#  Arduino IDE 2 / arduino-cli は hardware/<VENDOR>/tools/<name>/<version>/ を
#  {runtime.tools.<name>-<version>.path} として登録するため、コンパイル・
#  スケッチ書き込み・ブートローダ書き込みの全てでこのツールが使われます。
#  (platform.local.txt は不要です。あれば削除します。)
#
#  使い方:  tools/setup_toolchain.sh [--force] [--index FILE]
#      --force       既に配置済みでも削除して入れ直す
#      --index FILE  参照するインデックス(既定: ../docs/package_ukiuki_index.json)
#
#  対応ホスト: インデックスに当該ホスト用のアーカイブがあるツールだけを配置します
#      (現状: avr-gcc は Linux x86_64 / Windows のみ、avrdude は Linux/macOS/Windows)
#  必要なコマンド: curl(または wget), tar, sha256sum(または shasum), python3(または jq)
# ============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$SCRIPT_DIR")"
DEST="$SCRIPT_DIR"
INDEX="${ROOT}/docs/package_ukiuki_index.json"
FORCE=0

while [ $# -gt 0 ]; do
  case "$1" in
    --force) FORCE=1 ;;
    --index) shift; INDEX="$1" ;;
    -h|--help) sed -n '2,27p' "$0"; exit 0 ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
  shift
done
[ -f "$INDEX" ] || { echo "ERROR: index not found: $INDEX" >&2; exit 1; }

# ---- ホスト判定 -------------------------------------------------------------
os="$(uname -s)"; arch="$(uname -m)"
case "$os" in
  Linux)  case "$arch" in x86_64) HOST="x86_64-pc-linux-gnu";; aarch64|arm64) HOST="aarch64-linux-gnu";; armv7l) HOST="arm-linux-gnueabihf";; *) HOST="";; esac ;;
  Darwin) case "$arch" in arm64) HOST="arm64-apple-darwin";; x86_64) HOST="x86_64-apple-darwin";; *) HOST="";; esac ;;
  MINGW*|MSYS*|CYGWIN*) HOST="x86_64-mingw32" ;;
  *) HOST="" ;;
esac
[ -n "$HOST" ] || { echo "ERROR: unsupported host: $os $arch" >&2; exit 1; }

need() { command -v "$1" >/dev/null 2>&1; }

# ---- インデックスから「最新 platform の toolsDependencies」を解決 --------------
# 出力(ファイル): name<TAB>version<TAB>archiveFileName<TAB>url<TAB>sha256 (ホスト用が無い行は url が空)
# python3 → jq の順に試す(どちらかがあればよい)
resolve_py() {
  python3 - "$INDEX" "$HOST" > "$1" <<'PY'
import json, sys
idx, host = sys.argv[1], sys.argv[2]
d = json.load(open(idx, encoding="utf-8"))
def vkey(v):
    return [(0, int(p)) if p.isdigit() else (1, p) for p in v.replace("-", ".").split(".")]
tools = {}
for pkg in d["packages"]:
    for t in pkg.get("tools", []):
        tools[(t["name"], t["version"])] = t
best = None
for pkg in d["packages"]:
    for pl in pkg.get("platforms", []):
        if best is None or vkey(pl["version"]) > vkey(best["version"]):
            best = pl
for dep in best.get("toolsDependencies", []):
    t = tools.get((dep["name"], dep["version"]))
    sysd = next((s for s in (t["systems"] if t else []) if s["host"] == host), None)
    if sysd:
        print("\t".join([dep["name"], dep["version"], sysd["archiveFileName"], sysd["url"], sysd["checksum"].split(":", 1)[-1]]))
    else:
        print("\t".join([dep["name"], dep["version"], "", "", ""]))
PY
}
resolve_jq() {
  jq -r --arg host "$HOST" '
    ( [.packages[].platforms[]] | max_by(.version | split(".") | map(tonumber? // 0)) ) as $pl
    | ( [.packages[].tools[]] ) as $tools
    | $pl.toolsDependencies[]
    | . as $dep
    | ( [ $tools[] | select(.name==$dep.name and .version==$dep.version) | .systems[] | select(.host==$host) ] | first ) as $s
    | [$dep.name, $dep.version, ($s.archiveFileName // ""), ($s.url // ""), (($s.checksum // "") | sub("^SHA-256:";""))] | @tsv
  ' "$INDEX" > "$1"
}
resolve() {  # $1 = output file
  if need python3 && resolve_py "$1" 2>/dev/null && [ -s "$1" ]; then return 0; fi
  if need jq && resolve_jq "$1" 2>/dev/null && [ -s "$1" ]; then return 0; fi
  echo "ERROR: could not read ${INDEX} (python3 or jq is required)" >&2
  return 1
}

download() {  # url dest
  if need curl; then curl -fL --progress-bar -o "$2" "$1"
  elif need wget; then wget -q --show-progress -O "$2" "$1"
  else echo "ERROR: curl or wget is required" >&2; exit 1; fi
}
sha256() {
  if need sha256sum; then sha256sum "$1" | cut -d' ' -f1
  elif need shasum; then shasum -a 256 "$1" | cut -d' ' -f1
  else echo ""; fi
}

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

install_tool() {  # name version archive url sha256
  local name="$1" ver="$2" archive="$3" url="$4" want="$5"
  local target="${DEST}/${name}/${ver}"
  local probe="bin/${name}"; [ "$name" = "avr-gcc" ] && probe="bin/avr-gcc"
  if [ -z "$url" ]; then
    echo "[note] ${name} ${ver}: no archive for host ${HOST} in the index - skipped."
    return 0
  fi
  if [ -e "${target}/${probe}" ] && [ "$FORCE" -eq 0 ]; then
    echo "[skip] ${name} ${ver} is already installed at ${target}"
    return 0
  fi
  echo "[get ] ${archive}"
  download "$url" "${WORK}/${archive}"
  local have; have="$(sha256 "${WORK}/${archive}")"
  if [ -n "$want" ] && [ -n "$have" ]; then
    if [ "$want" != "$have" ]; then
      echo "ERROR: SHA-256 mismatch for ${archive}" >&2
      echo "  expected ${want}" >&2; echo "  got      ${have}" >&2; exit 1
    fi
    echo "[ok  ] SHA-256 verified"
  else
    echo "[warn] could not verify SHA-256; continuing"
  fi
  echo "[untar] ${archive}"
  rm -rf "${WORK}/x"; mkdir -p "${WORK}/x"
  tar -xzf "${WORK}/${archive}" -C "${WORK}/x"
  local top; top="$(find "${WORK}/x" -mindepth 1 -maxdepth 1 -type d | head -n1)"
  if [ -z "$top" ] || [ ! -e "${top}/${probe}" ]; then
    echo "ERROR: unexpected archive layout (no ${probe} under top directory)" >&2; exit 1
  fi
  rm -rf "$target"; mkdir -p "$(dirname "$target")"
  mv "$top" "$target"
  chmod -R u+rwX "$target"
  echo "[done] ${name} ${ver} -> ${target}"
}

echo "Index : ${INDEX}"
echo "Host  : ${HOST}"
resolve "${WORK}/tools.lst" || exit 1
INSTALLED=""
while IFS=$'\t' read -r name ver archive url sum; do
  [ -n "$name" ] || continue
  install_tool "$name" "$ver" "$archive" "$url" "$sum"
  INSTALLED="${INSTALLED} ${name}-${ver}"
done < "${WORK}/tools.lst"

# ---- platform.txt のバージョン固定と一致しているか確認 ----------------------
PT="${ROOT}/megaavr/platform.txt"
if [ -f "$PT" ]; then
  for tv in $INSTALLED; do
    grep -q "runtime.tools.${tv}.path" "$PT" || \
      echo "[warn] megaavr/platform.txt does not reference {runtime.tools.${tv}.path} - update the pin to match the index."
  done
fi

# ---- 旧方式の platform.local.txt を片付ける --------------------------------
if [ -f "${ROOT}/megaavr/platform.local.txt" ]; then
  rm -f "${ROOT}/megaavr/platform.local.txt"
  echo "[clean] removed obsolete megaavr/platform.local.txt (no longer needed)"
fi

echo
echo "Tools under ${DEST}:"
for t in "$DEST"/*/; do [ -d "$t" ] && ls -1 "$t" | sed "s|^|  $(basename "$t")/|"; done
echo
echo "Restart the Arduino IDE. The build/upload log should show tools under .../hardware/UkiUkiCore/tools/"
