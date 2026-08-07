#!/usr/bin/env bash
# ============================================================
#  build_wazamono.sh
#  WazamonoCore fork - build the USB CDC bootloader for the
#  Wazamono product boards. POSIX counterpart of build_wazamono.bat.
#
#  The clean-room bootloader source is NOT modified. The only per-board
#  difference is passed in at build time:
#
#    board             MCU         LED   pol       VREG  USB ident (VID:PID)
#    ----------------  ----------  ----  --------  ----  -------------------
#    Wazamono Tachi    avr64du32   PF3   act-LOW   0     0x1209:0x0005
#    Wazamono Tsurugi  avr64du32   PD6   act-HIGH  0     0x1209:0x0007
#    Wazamono Kunai    avr32du20   PD4   act-LOW   0     0x1209:0x0009
#
#    All boards feed VUSB from an external 3.3 V LDO (VREG=0, power
#    configuration 3s); no board enables the internal USB regulator.
#
#    - LED pin     : LED_PORT / LED_PIN
#    - LED polarity: LED_AH=1 (active-HIGH) | LED_AL=1 (active-LOW)
#                    Neither given => active-LOW; both given => LED_AH wins.
#    - USB identity: BOARD=TACHI | TSURUGI  (selects PID + product string)
#
#  --- Toolchain search order (first usable hit wins) -----------------
#    0) $GCC_BIN                          explicit override (set empty to
#                                         keep whatever is on PATH)
#    1) native Linux builds (WSL/Linux):
#         ~/wazamono-toolchain/build/prefix-native/bin
#         ~/avr-gcc/bin  or  ~/avr-gcc*/bin
#       (re-create with:  cd ~/wazamono-toolchain &&
#        bash scripts/build-avr-gcc.sh native
#        ...or extract avr-gcc-*-x86_64-pc-linux-gnu.tar.gz from the
#        wazamono-toolchain GitHub Release into ~/avr-gcc)
#    2) sketchbook tools, RELATIVE to this script (works from Git Bash
#       and WSL alike since the script lives on the Windows tree):
#         ../../../../../tools/avr-gcc/*-wazamono*/bin
#    3) Board Manager install:
#         <Users>/<name>/AppData/Local/Arduino15/packages/WazamonoCore/
#           tools/avr-gcc/*/bin   (via /c or /mnt/c)
#    4) legacy stock build:  /c/avr-gcc, /mnt/c/avr-gcc
#
#  NOTE (WSL): a directory qualifies only if it contains an executable
#  named exactly `avr-gcc` for this shell. Windows toolchains (avr-gcc.exe)
#  qualify under Git Bash/MSYS, which resolves the .exe suffix, but NOT
#  under WSL, where make cannot spawn `avr-gcc` from an .exe-only dir.
#  Under WSL use a native Linux toolchain (option 1).
#
#  Usage (from this directory):
#    ./build_wazamono.sh
# ============================================================
set -euo pipefail
cd "$(dirname "$0")"

# does $1/avr-gcc work for THIS shell?
usable_bin() {
  [ -d "$1" ] || return 1
  if [ -x "$1/avr-gcc" ]; then return 0; fi
  case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*) [ -f "$1/avr-gcc.exe" ] && return 0 ;;
  esac
  return 1
}

if [ "${GCC_BIN+set}" != "set" ]; then
  GCC_BIN=""
  candidates=()
  # 1) native builds
  candidates+=("$HOME/wazamono-toolchain/build/prefix-native/bin")
  for d in "$HOME"/avr-gcc*/bin "$HOME/avr-gcc/bin"; do candidates+=("$d"); done
  # 2) sketchbook tools relative to this script
  for d in ../../../../../tools/avr-gcc/*-wazamono*/bin; do candidates+=("$d"); done
  # 3) Board Manager install (Git Bash: /c, WSL: /mnt/c)
  for root in /c/Users /mnt/c/Users; do
    for d in "$root"/*/AppData/Local/Arduino15/packages/WazamonoCore/tools/avr-gcc/*/bin; do
      candidates+=("$d")
    done
  done

  for cand in "${candidates[@]}"; do
    if usable_bin "$cand"; then
      GCC_BIN="$(cd "$cand" && pwd)"
      break
    fi
  done
  if [ -z "$GCC_BIN" ]; then
    echo "ERROR: no usable avr-gcc found for this shell." >&2
    echo "  WSL/Linux: build or extract a native toolchain, e.g." >&2
    echo "    cd ~/wazamono-toolchain && bash scripts/build-avr-gcc.sh native" >&2
    echo "  Git Bash : install WazamonoCore via the Board Manager." >&2
    echo "  Or set GCC_BIN=/path/to/toolchain/bin explicitly." >&2
    exit 1
  fi
fi
if [ -n "${GCC_BIN}" ]; then
  case ":$PATH:" in
    *":$GCC_BIN:"*) ;;
    *) PATH="$GCC_BIN:$PATH" ;;
  esac
  echo "Using avr-gcc from: $GCC_BIN"
fi

MAKE="${MAKE:-make}"
TOOLROOT="${TOOLROOT:-}"

build() {            # $1=class  $2=mcu  $3=LEDport  $4=LEDpin  $5=board  $6=LED polarity (AH | AL)  $7=VREG (0 | 1)
  local polflag=""
  if [ "${6:-}" = "AH" ]; then polflag="LED_AH=1"; fi
  if [ "${6:-}" = "AL" ]; then polflag="LED_AL=1"; fi
  local vreg="${7:-0}"   # omitted -> 0 (safe: VREG off, external VUSB)
  echo ""
  echo "------ building $2  ->  usbcdcboot_$1.hex  (LED $3 $4, board $5, pol ${6:-}, VREG $vreg) ------"
  rm -f src/*.o "usbcdcboot_$1".{elf,hex,lst,map} 2>/dev/null || true
  $MAKE ${TOOLROOT:+TOOLROOT="$TOOLROOT"} MCU="$2" TARGET="usbcdcboot_$1" LED_PORT="$3" LED_PIN="$4" BOARD="$5" VREG="$vreg" $polflag all
  $MAKE ${TOOLROOT:+TOOLROOT="$TOOLROOT"} MCU="$2" TARGET="usbcdcboot_$1" BOARD="$5" VREG="$vreg" $polflag size
}

#     class             mcu        LEDport LEDpin board     LEDpol(AH|AL) VREG
build wazamonotachi   avr64du32   PORTF   3      TACHI     AL            0
build wazamonotsurugi avr64du32   PORTD   6      TSURUGI   AH            0
build wazamonokunai   avr32du20   PORTD   4      KUNAI     AL            0

echo ""
echo "=== collecting hex files into ../hex/ ==="
mkdir -p ../hex
mv -f usbcdcboot_*.hex ../hex/
ls -1 ../hex/usbcdcboot_wazamono*.hex
