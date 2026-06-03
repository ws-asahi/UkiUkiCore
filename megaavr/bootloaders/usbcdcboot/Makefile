# usbcdcboot/Makefile
# --------------------------------------------------------------------
#   Build rules for the AVRDU CDC bootloader.
#   Produces usbcdcboot_64du32.hex, .elf, .lst, .map.
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
TARGET   := usbcdcboot_64du32
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

# --- Build rules ----------------------------------------------------
.PHONY: all clean size

all: $(TARGET).hex $(TARGET).lst

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
	$(SIZE) --mcu=$(MCU) -A $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .signature -R .user_signatures $< $@

$(TARGET).lst: $(TARGET).elf
	$(OBJDUMP) -h -S $< > $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

size: $(TARGET).elf
	@echo ""
	@echo "=== Bootloader size vs $(BOOTSIZE)-byte budget ==="
	@$(SIZE) --mcu=$(MCU) -A $< \
	  | awk '/^.text/  { t=$$2 } \
	         /^.data/  { d=$$2 } \
	         END { printf "  .text = %u\n  .data = %u (init image)\n  total = %u bytes\n  free  = %d bytes\n", \
	                       t, d, t+d, $(BOOTSIZE)-(t+d) }'

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).hex $(TARGET).lst $(TARGET).map
