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
#    board            MCU         LED   note
#    ---------------  ----------  ----  ----------------------------
#    Wazamono Tachi   avr64du32   PD5   RX LED / LED_BUILTIN, act-LOW
#
#  main.c drives the LED active-LOW; the Wazamono RX/TX LEDs are also
#  active-LOW (Pro Micro convention), so only the pin moves.
#
#  Usage (from this directory):
#    ./build_wazamono.sh                       # toolchain on $PATH
#    TOOLROOT=../../../../tools ./build_wazamono.sh   # DxCore toolchain
# ============================================================
set -euo pipefail
cd "$(dirname "$0")"

MAKE="${MAKE:-make}"
TOOLROOT="${TOOLROOT:-}"

build() {            # $1=class  $2=mcu  $3=LEDport  $4=LEDpin
  echo ""
  echo "------ building $2  ->  usbcdcboot_$1.hex  (LED $3 $4) ------"
  rm -f src/*.o "usbcdcboot_$1".{elf,hex,lst,map} 2>/dev/null || true
  $MAKE ${TOOLROOT:+TOOLROOT="$TOOLROOT"} MCU="$2" TARGET="usbcdcboot_$1" LED_PORT="$3" LED_PIN="$4" all
  $MAKE ${TOOLROOT:+TOOLROOT="$TOOLROOT"} MCU="$2" TARGET="usbcdcboot_$1" size
}

#     class           mcu         LEDport LEDpin
build wazamonotachi   avr64du32   PORTD   5
# build wazamonotsurugi avr64du32   PORTD   6   # (enable when Tsurugi is finalized)

echo ""
echo "=== collecting hex files into ../hex/ ==="
mkdir -p ../hex
mv -f usbcdcboot_*.hex ../hex/
ls -1 ../hex/usbcdcboot_wazamono*.hex
