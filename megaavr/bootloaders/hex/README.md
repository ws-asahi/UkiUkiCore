# Here live the bootloader hex files

## .lst and .elf files

Additionally, there is a tar'ed BZip'ed copy of all the assembly listings and elf files produced compiling all of these bootloaders, these are in listings.tar.bz2 and elves.tar.bz2.

## Build logs

Build logs are now included of both normal output and error output.

## Files

| File | Board | LED (DFU) | USB (VID:PID) |
|------|-------|-----------|---------------|
| `usbcdcboot_ukiukiduino.hex` | UkiUkiduino (Uno R3 form) | WS2812D-F5 on PA0, RGB order | 0x1209:0x000B |
| `usbcdcboot_ukiukiduinopromicro.hex` | UkiUkiduino ProMicro | XL-5050RGBC-WS2812B on PF4, GRB order | 0x1209:0x000D |

Both are produced by `../usbcdcboot/build_ukiukiduino.sh` (or `.bat`); the PIDs are
pid.codes test-range placeholders until the official assignment is made.
