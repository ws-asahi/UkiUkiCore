#!/usr/bin/env bash
# ============================================================
#  build_ukiukiduino.sh
#  UkiUkiCore - build the USB CDC bootloader for the UkiUkiduino.
#  POSIX counterpart of build_ukiukiduino.bat.
#
#  The clean-room bootloader source is NOT modified. Board parameters
#  are passed in at build time:
#
#    board        MCU         LED   pol       USB ident (VID:PID)
#    -----------  ----------  ----  --------  -----------------------------
#    UkiUkiduino  avr64du32   PA0   act-HIGH  0x1209:0x000B (test placeholder)
#
#    - LED pin     : LED_PORT / LED_PIN
#    - LED polarity: LED_AH=1 (active-HIGH) | LED_AL=1 (active-LOW)
#                    Neither given => active-LOW; both given => LED_AH wins.
#    - USB identity: fixed in src/usb_desc.h (replace the test PID with the
#      officially assigned pid.codes PID before release)
#
#  Toolchain: by default this uses the Windows avr-gcc you copied to
#  C:\avr-gcc (the same one the IDE uses), auto-translated to this shell's
#  view -- /c/avr-gcc under Git Bash/MSYS, /mnt/c/avr-gcc under WSL -- and
#  prepended to PATH.  make then runs avr-gcc(.exe) from PATH.
#
#  Override the toolchain, e.g. a native Linux build under WSL:
#    GCC_BIN=$HOME/avr-gcc-build/build/avr-gcc-15.2.0-x64-linux/bin ./build_ukiukiduino.sh
#  or skip auto-detect and use whatever avr-gcc is already on your PATH:
#    GCC_BIN= ./build_ukiukiduino.sh
#  (Under WSL prefer the Linux toolchain above; running the Windows .exe via
#   /mnt/c works only for source trees on a path the .exe can resolve.)
#
#  Usage (from this directory):
#    ./build_ukiukiduino.sh
# ============================================================
set -euo pipefail
cd "$(dirname "$0")"

# --- Toolchain: default to C:\avr-gcc, translated for this shell ----------
# Set GCC_BIN explicitly to override; set it to empty to keep your own PATH.
if [ "${GCC_BIN+set}" != "set" ]; then
  GCC_BIN=""
  for root in /c/avr-gcc /mnt/c/avr-gcc; do
    [ -d "$root" ] || continue
    cand="$(ls -d "$root"/avr-gcc-*/bin 2>/dev/null | sort | tail -n1)"
    if [ -z "$cand" ] && [ -d "$root/bin" ]; then cand="$root/bin"; fi
    if [ -n "$cand" ]; then GCC_BIN="$cand"; break; fi
  done
fi
if [ -n "${GCC_BIN}" ]; then
  case ":$PATH:" in
    *":$GCC_BIN:"*) ;;
    *) PATH="$GCC_BIN:$PATH" ;;
  esac
fi

MAKE="${MAKE:-make}"
TOOLROOT="${TOOLROOT:-}"

build() {            # $1=class  $2=mcu  $3=LEDport  $4=LEDpin  $5=LED polarity (AH | AL)
  local polflag=""
  if [ "${5:-}" = "AH" ]; then polflag="LED_AH=1"; fi
  if [ "${5:-}" = "AL" ]; then polflag="LED_AL=1"; fi
  echo ""
  echo "------ building $2  ->  usbcdcboot_$1.hex  (LED $3 $4, pol ${5:-}) ------"
  rm -f src/*.o "usbcdcboot_$1".{elf,hex,lst,map} 2>/dev/null || true
  $MAKE ${TOOLROOT:+TOOLROOT="$TOOLROOT"} MCU="$2" TARGET="usbcdcboot_$1" LED_PORT="$3" LED_PIN="$4" $polflag all
  $MAKE ${TOOLROOT:+TOOLROOT="$TOOLROOT"} MCU="$2" TARGET="usbcdcboot_$1" $polflag size
}

#     class        mcu        LEDport LEDpin LEDpol(AH|AL)
build ukiukiduino avr64du32   PORTA   0      AH

echo ""
echo "=== collecting hex files into ../hex/ ==="
mkdir -p ../hex
mv -f usbcdcboot_*.hex ../hex/
ls -1 ../hex/usbcdcboot_ukiukiduino.hex
