# avrdu_cdc_bl/Makefile
# --------------------------------------------------------------------
#   Build rules for the AVRDU CDC bootloader.
#   Produces usbcdcboot_64du.hex, .elf, .lst, .map (one hex per build
#   class; see build_all_du.bat for the 16du/16du14/32du/32du14/64du set).
#
#   Usage from inside this directory:
#
#       make TOOLROOT=../../../../tools           # use DxCore's toolchain
#       make clean
#       make all                                  # default target
#       make size                                 # report .text size vs 4 KB budget
#
#   The DxCore toolchain ships avr-gcc 7.3.0 / atmel3.6.1 / azduino8 under
#   tools/avr-gcc/<ver>/bin/avr-gcc.  If TOOLROOT is not set we look for it
#   on $PATH.  License: LGPL 2.1.
# --------------------------------------------------------------------

MCU      := avr64du32
F_CPU    := 24000000UL
TARGET   := usbcdcboot_64du
BOOT_BASE := 0x0000
BOOTSIZE  := 4096           # bytes reserved for the bootloader (BOOTSIZE fuse * 512)

# --- Toolchain ------------------------------------------------------
ifeq ($(strip $(TOOLROOT)),)
CC := avr-gcc
OBJCOPY := avr-objcopy
OBJDUMP := avr-objdump
SIZE    := avr-size
else
AVR_GCC_DIR := $(firstword $(wildcard $(TOOLROOT)/avr-gcc/*))
CC      := $(AVR_GCC_DIR)/bin/avr-gcc
OBJCOPY := $(AVR_GCC_DIR)/bin/avr-objcopy
OBJDUMP := $(AVR_GCC_DIR)/bin/avr-objdump
SIZE    := $(AVR_GCC_DIR)/bin/avr-size
endif

# --- Sources --------------------------------------------------------
SRCS := src/main.c \
        src/usb_min.c \
        src/usb_desc.c \
        src/cdc_min.c \
        src/stk500.c \
        src/nvm.c

OBJS := $(SRCS:.c=.o)

# --- Compile / link flags -------------------------------------------
CFLAGS := -mmcu=$(MCU) \
          -DF_CPU=$(F_CPU) \
          -DBOOTLOADER_BUILD=1 \
          \
          -Os \
          -ffunction-sections -fdata-sections \
          -fno-common \
          -mrelax \
          -Wall -Wextra -Wstrict-prototypes \
          -std=gnu11 \
          -Isrc

# --- LED pin override (per-build) ------------------------------------
# main.c defaults the DFU LED to PA7 (present on 20/28/32-pin DU).  The
# 14-pin DU has no PA7, so its build classes pass LED_PORT/LED_PIN to move
# the LED to PD6, e.g.:  make ... LED_PORT=PORTD LED_PIN=6
# (Kept as plain tokens -- no spaces/parens -- so Windows cmd passes them
# through cleanly.)
ifneq ($(strip $(LED_PORT)),)
CFLAGS += -DBL_LED_PORT=$(LED_PORT)
endif
ifneq ($(strip $(LED_PIN)),)
CFLAGS += -DBL_LED_PIN=$(LED_PIN)
endif

# --- Board selection (per-build) ------------------------------------
# Selects the per-board USB bootloader identity.  VID is shared (0x1209);
# the PID and the USB product string differ per board (see src/usb_desc.h
# and src/usb_desc.c).  Pass the board tag, e.g.:
#   make ... BOARD=TSURUGI      (unset / BOARD=TACHI = the default Tachi identity)
ifneq ($(strip $(BOARD)),)
CFLAGS += -DWAZAMONO_BOARD_$(BOARD)
endif

# --- LED polarity (per-build) ---------------------------------------
# Two explicit options select the DFU LED drive polarity:
#   make ... LED_AH=1   -> active-HIGH (DxCore / Arduino "D13" convention)
#   make ... LED_AL=1   -> active-LOW  (Pro Micro RX/TX LED convention)
# If neither is given the bootloader falls back to active-LOW; if both are
# given, LED_AH wins (resolved in src/main.c).
ifneq ($(strip $(LED_AH)),)
CFLAGS += -DLED_AH
endif
ifneq ($(strip $(LED_AL)),)
CFLAGS += -DLED_AL
endif

# The toolchain's specs-avr64du32 file handles the avrxmega2 paths
# automatically when -mmcu=avr64du32 is set, so we don't need any
# explicit -B options here.

LDFLAGS := -mmcu=$(MCU) \
           -Wl,--gc-sections \
           -Wl,--section-start=.text=$(BOOT_BASE) \
           -Wl,-Map,$(TARGET).map \
           -mrelax
# NOTE: we intentionally do NOT pass -nostartfiles.  The standard
# crt provides _start which calls main, and is what makes
# --gc-sections see main as reachable.  Crt overhead is ~50 bytes,
# more than compensated by the section-level dead-code elimination.

# Optional: a custom linker script could replace --section-start above
# if we wanted finer control over .data init image placement.  For a
# 4 KB bootloader the default link with .text=0 is sufficient.

# --- rodata placement (per-MCU) --------------------------------------
# GCC >= 14 places .rodata in the FLMAP-mapped flash window by default
# on 64 KB+ parts (avrxmega2: avr64du28/32).  That is wrong for THIS
# program:
#  (1) the bootloader must be fully contained in the 4 KB BOOT section,
#      but windowed .rodata lands at flash 0x8000 - inside the app area,
#      where any app image larger than 28 KB would overwrite it;
#  (2) nvm.c retargets NVMCTRL.CTRLB.FLMAP while erasing/writing app
#      pages, so window-resident .rodata (USB descriptors!) would be
#      unreadable mid-upload while the interrupt-driven USB still runs.
# -mrodata-in-ram restores the classic layout: .rodata is copied to RAM
# by __do_copy_data (~0.3 KB of 8 KB RAM) and never depends on FLMAP.
# Fully-mapped avrxmega3 parts (avr16/32du*) reject the option and do
# not need it: their .rodata stays inside the contiguous 4 KB image.
ifneq (,$(findstring avr64,$(MCU)))
CFLAGS  += -mrodata-in-ram
LDFLAGS += -mrodata-in-ram
endif

# --- Build rules ----------------------------------------------------
.PHONY: all clean size

all: $(TARGET).hex $(TARGET).lst

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
	$(SIZE) $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .signature -R .user_signatures $< $@

$(TARGET).lst: $(TARGET).elf
	$(OBJDUMP) -h -S $< > $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

size: $(TARGET).elf
	@echo ""
	@echo "=== Bootloader size (budget: $(BOOTSIZE) bytes flash for .text + .data) ==="
	$(SIZE) -A $<

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).hex $(TARGET).lst $(TARGET).map
