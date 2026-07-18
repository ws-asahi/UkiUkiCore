#!/usr/bin/env bash
# ============================================================
#  build_ukiukiduino.sh
#  UkiUkiCore - build the USB CDC bootloader for the UkiUkiduino.
#  POSIX counterpart of build_ukiukiduino.bat.
#
#  The clean-room bootloader source is NOT modified. Board parameters
#  are passed in at build time:
#
#    board        MCU         LED   pol       VREG  USB ident (VID:PID)
#    -----------  ----------  ----  --------  ----  -----------------------------
#    UkiUkiduino  avr64du32   PA0   act-HIGH  1     0x1209:0x000B (test placeholder)
#
#    - LED pin     : LED_PORT / LED_PIN
#    - LED polarity: LED_AH=1 (active-HIGH) | LED_AL=1 (active-LOW)
#                    Neither given => active-LOW; both given => LED_AH wins.
#    - VREG        : 1 = internal VUSB regulator (the UkiUkiduino), 0 = external
#    - USB identity: fixed in src/usb_desc.h (replace the test PID with the
#      officially assigned pid.codes PID before release)
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
#    3) Board Manager install (UkiUkiCore first, then WazamonoCore -
#       both packages ship the same wazamono toolchain):
#         <Users>/<name>/AppData/Local/Arduino15/packages/{UkiUkiCore,
#           WazamonoCore}/tools/avr-gcc/*/bin   (via /c or /mnt/c)
#    4) legacy stock build:  /c/avr-gcc, /mnt/c/avr-gcc
#
#  Usage (from this directory):
#    ./build_ukiukiduino.sh
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
    for pkg in UkiUkiCore WazamonoCore; do
      for d in "$root"/*/AppData/Local/Arduino15/packages/$pkg/tools/avr-gcc/*/bin; do
        candidates+=("$d")
      done
    done
  done
  # 4) legacy stock builds
  for root in /c/avr-gcc /mnt/c/avr-gcc; do
    candidates+=("$root/bin")
    for d in "$root"/*-wazamono*/bin "$root"/avr-gcc-*/bin; do candidates+=("$d"); done
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
    echo "  Git Bash : install UkiUkiCore/WazamonoCore via the Board Manager." >&2
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

build() {            # $1=class  $2=mcu  $3=LEDport  $4=LEDpin  $5=LED polarity (AH | AL)  $6=VREG (0 | 1)
  local polflag=""
  if [ "${5:-}" = "AH" ]; then polflag="LED_AH=1"; fi
  if [ "${5:-}" = "AL" ]; then polflag="LED_AL=1"; fi
  local vreg="${6:-1}"   # omitted -> 1 (UkiUkiduino: internal VUSB regulator)
  echo ""
  echo "------ building $2  ->  usbcdcboot_$1.hex  (LED $3 $4, pol ${5:-}, VREG $vreg) ------"
  rm -f src/*.o "usbcdcboot_$1".{elf,hex,lst,map} 2>/dev/null || true
  $MAKE ${TOOLROOT:+TOOLROOT="$TOOLROOT"} MCU="$2" TARGET="usbcdcboot_$1" LED_PORT="$3" LED_PIN="$4" VREG="$vreg" $polflag all
  $MAKE ${TOOLROOT:+TOOLROOT="$TOOLROOT"} MCU="$2" TARGET="usbcdcboot_$1" VREG="$vreg" $polflag size
}

#     class        mcu        LEDport LEDpin LEDpol(AH|AL) VREG
build ukiukiduino avr64du32   PORTA   0      AH            1

echo ""
echo "=== collecting hex files into ../hex/ ==="
mkdir -p ../hex
mv -f usbcdcboot_*.hex ../hex/
ls -1 ../hex/usbcdcboot_ukiukiduino.hex
