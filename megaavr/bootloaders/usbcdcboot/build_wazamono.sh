#!/usr/bin/env bash
# ============================================================
#  build_wazamono.sh
#  WazamonoCore fork - build the USB CDC bootloader for the
#  Wazamono product boards. POSIX counterpart of build_wazamono.bat.
#
#  The clean-room bootloader source is NOT modified. The only board
#  difference from the generic 64du build is the DFU-status LED pin,
#  passed in at build time:
#
#    board             MCU         LED   pol     USB ident (VID:PID)
#    ----------------  ----------  ----  ------  -------------------
#    Wazamono Tachi    avr64du32   PD5   act-LO  0x1209:0x0005
#    Wazamono Tsurugi  avr64du32   PD6   act-HI  0x1209:0x0007
#
#  Per-board build differences are passed in at build time; the
#  clean-room source is NOT edited per board:
#    - LED pin     : LED_PORT / LED_PIN
#    - LED polarity: LED_AH=1 (active-HIGH) | LED_AL=1 (active-LOW)
#                    Neither given => active-LOW; both given => LED_AH wins.
#    - USB identity: BOARD=TACHI | TSURUGI  (selects PID + product string)
#
#  Each board passes its polarity explicitly (Tachi=AL, Tsurugi=AH), so the
#  source-level fallback default can change later without affecting either
#  board.  Active-HIGH = DxCore / Arduino "D13"; active-LOW = Pro Micro.
#
#  Usage (from this directory):
#    ./build_wazamono.sh                       # toolchain on $PATH
#    TOOLROOT=../../../../tools ./build_wazamono.sh   # DxCore toolchain
# ============================================================
set -euo pipefail
cd "$(dirname "$0")"

MAKE="${MAKE:-make}"
TOOLROOT="${TOOLROOT:-}"

build() {            # $1=class  $2=mcu  $3=LEDport  $4=LEDpin  $5=board  $6=LED polarity (AH | AL)
  local polflag=""
  if [ "${6:-}" = "AH" ]; then polflag="LED_AH=1"; fi
  if [ "${6:-}" = "AL" ]; then polflag="LED_AL=1"; fi
  echo ""
  echo "------ building $2  ->  usbcdcboot_$1.hex  (LED $3 $4, board $5, pol ${6:-}) ------"
  rm -f src/*.o "usbcdcboot_$1".{elf,hex,lst,map} 2>/dev/null || true
  $MAKE ${TOOLROOT:+TOOLROOT="$TOOLROOT"} MCU="$2" TARGET="usbcdcboot_$1" LED_PORT="$3" LED_PIN="$4" BOARD="$5" $polflag all
  $MAKE ${TOOLROOT:+TOOLROOT="$TOOLROOT"} MCU="$2" TARGET="usbcdcboot_$1" BOARD="$5" $polflag size
}

#     class             mcu        LEDport LEDpin board     LEDpol(AH|AL)
build wazamonotachi   avr64du32   PORTD   5      TACHI     AL
build wazamonotsurugi avr64du32   PORTD   6      TSURUGI   AH

echo ""
echo "=== collecting hex files into ../hex/ ==="
mkdir -p ../hex
mv -f usbcdcboot_*.hex ../hex/
ls -1 ../hex/usbcdcboot_wazamono*.hex
