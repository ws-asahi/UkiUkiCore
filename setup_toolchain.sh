#!/usr/bin/env bash
# ============================================================================
#  setup_toolchain.sh  -  UkiUkiCore 手動インストール用ツールチェーン配置スクリプト
#
#  wazamono-toolchain のリリースから avr-gcc / avrdude をダウンロードし、
#  このリポジトリ直下の tools/ に Arduino IDE が認識する形で配置します:
#
#      <sketchbook>/hardware/UkiUkiCore/
#          megaavr/                        <- コア(platform.txt など)
#          tools/avr-gcc/15.2.0-wazamono2/ <- bin/avr-gcc ...
#          tools/avrdude/8.1-wazamono2/    <- bin/avrdude, etc/avrdude.conf
#
#  Arduino IDE 2 / arduino-cli は hardware/<VENDOR>/tools/<name>/<version>/ を
#  {runtime.tools.<name>-<version>.path} として登録するため、コンパイル・
#  スケッチ書き込み・ブートローダ書き込みの全てでこのツールが使われます。
#  (platform.local.txt は不要です。あれば削除します。)
#
#  使い方:  ./setup_toolchain.sh [--force] [--dest DIR]
#      --force     既に配置済みでも削除して入れ直す
#      --dest DIR  配置先(既定: このスクリプトのある場所の tools/)
#
#  対応ホスト:
#      Linux x86_64          avr-gcc + avrdude
#      Linux aarch64         avrdude のみ(avr-gcc は未提供)
#      macOS arm64 / x86_64  avrdude のみ(avr-gcc は未提供)
#      Windows               setup_toolchain.bat を使ってください
#  必要なコマンド: curl(または wget), tar, sha256sum(または shasum)
# ============================================================================
set -euo pipefail

# ---- バージョン(docs/package_ukiuki_index.json と揃えること) ---------------
TAG="tools-15.2.0-wazamono2"
GCC_VER="15.2.0-wazamono2"
DUDE_VER="8.1-wazamono2"
BASE_URL="https://github.com/ws-asahi/wazamono-toolchain/releases/download/${TAG}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEST="${SCRIPT_DIR}/tools"
FORCE=0

while [ $# -gt 0 ]; do
  case "$1" in
    --force) FORCE=1 ;;
    --dest)  shift; DEST="$1" ;;
    -h|--help) sed -n '2,29p' "$0"; exit 0 ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
  shift
done

# ---- ホスト判定 -------------------------------------------------------------
os="$(uname -s)"; arch="$(uname -m)"
GCC_HOST=""; DUDE_HOST=""
case "$os" in
  Linux)
    case "$arch" in
      x86_64)         GCC_HOST="x86_64-pc-linux-gnu"; DUDE_HOST="x86_64-pc-linux-gnu" ;;
      aarch64|arm64)  DUDE_HOST="aarch64-linux-gnu" ;;
    esac ;;
  Darwin)
    case "$arch" in
      arm64)          DUDE_HOST="arm64-apple-darwin" ;;
      x86_64)         DUDE_HOST="x86_64-apple-darwin" ;;
    esac ;;
  MINGW*|MSYS*|CYGWIN*)
    GCC_HOST="x86_64-mingw32"; DUDE_HOST="x86_64-mingw32" ;;
esac
if [ -z "$DUDE_HOST" ]; then
  echo "ERROR: unsupported host: $os $arch" >&2; exit 1
fi

# ---- ヘルパ -----------------------------------------------------------------
need() { command -v "$1" >/dev/null 2>&1; }
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

echo "Fetching checksums.txt ..."
download "${BASE_URL}/checksums.txt" "${WORK}/checksums.txt"

# install <name> <version> <archive> <check-file>
install_tool() {
  local name="$1" ver="$2" archive="$3" probe="$4"
  local target="${DEST}/${name}/${ver}"
  if [ -e "${target}/${probe}" ] && [ "$FORCE" -eq 0 ]; then
    echo "[skip] ${name} ${ver} is already installed at ${target}"
    return 0
  fi
  echo "[get ] ${archive}"
  download "${BASE_URL}/${archive}" "${WORK}/${archive}"

  local want have
  want="$(grep " ${archive}\$" "${WORK}/checksums.txt" | cut -d' ' -f1 || true)"
  have="$(sha256 "${WORK}/${archive}")"
  if [ -n "$want" ] && [ -n "$have" ]; then
    if [ "$want" != "$have" ]; then
      echo "ERROR: SHA-256 mismatch for ${archive}" >&2
      echo "  expected ${want}" >&2; echo "  got      ${have}" >&2; exit 1
    fi
    echo "[ok  ] SHA-256 verified"
  else
    echo "[warn] could not verify SHA-256 (no checksum tool or entry); continuing"
  fi

  echo "[untar] ${archive}"
  rm -rf "${WORK}/x"; mkdir -p "${WORK}/x"
  tar -xzf "${WORK}/${archive}" -C "${WORK}/x"
  # アーカイブ直下は <name>-<version>/ の 1 ディレクトリ
  local top
  top="$(find "${WORK}/x" -mindepth 1 -maxdepth 1 -type d | head -n1)"
  if [ -z "$top" ] || [ ! -e "${top}/${probe}" ]; then
    echo "ERROR: unexpected archive layout (no ${probe} under top directory)" >&2; exit 1
  fi
  rm -rf "$target"; mkdir -p "$(dirname "$target")"
  mv "$top" "$target"
  chmod -R u+rwX "$target"
  echo "[done] ${name} ${ver} -> ${target}"
}

install_tool avrdude "$DUDE_VER" "avrdude-${DUDE_VER}-${DUDE_HOST}.tar.gz" "bin/avrdude"
if [ -n "$GCC_HOST" ]; then
  install_tool avr-gcc "$GCC_VER" "avr-gcc-${GCC_VER}-${GCC_HOST}.tar.gz" "bin/avr-gcc"
else
  echo "[note] avr-gcc is not provided for ${os} ${arch} in ${TAG}; only avrdude was installed."
  echo "       Build with a Boards Manager install, or place an avr-gcc 15.x build at"
  echo "       ${DEST}/avr-gcc/${GCC_VER}/ (bin/avr-gcc must exist)."
fi

# ---- 旧方式の platform.local.txt を片付ける --------------------------------
local_txt="${SCRIPT_DIR}/megaavr/platform.local.txt"
if [ -f "$local_txt" ]; then
  rm -f "$local_txt"
  echo "[clean] removed obsolete megaavr/platform.local.txt (no longer needed)"
fi

echo
echo "Installed under: ${DEST}"
ls -1 "${DEST}" 2>/dev/null | sed 's/^/  /'
echo
echo "Restart the Arduino IDE. The build/upload log should show tools under"
echo "  .../hardware/UkiUkiCore/tools/avr-gcc/${GCC_VER}/  and  .../tools/avrdude/${DUDE_VER}/"
