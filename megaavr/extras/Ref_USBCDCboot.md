# The Wazamono USB CDC bootloader (usbcdcboot)

Every Wazamono board has native USB, so no serial adapter is ever needed:
plug the board into the computer and upload. This page is the user-facing
reference for the bootloader; developer documentation (design, build
instructions, provenance) lives with the sources in
[`megaavr/bootloaders/usbcdcboot/`](../bootloaders/usbcdcboot/).

## What it is

A small (4 KB boot section) USB CDC-ACM bootloader speaking the same
STK500v1 protocol as classic Arduino bootloaders, so avrdude uploads work
the usual way. It is a clean-room implementation written from the USB 2.0
specification and the DU datasheet (DS40002548A) - see `PROVENANCE.md`
next to the sources.

While the bootloader is active it enumerates as a CDC serial port with the
bootloader VID/PID; a running sketch enumerates as the application CDC.
(The IDs currently in the board definitions are pid.codes test IDs and
will be replaced before commercial release.) The board definitions
register both, so "Get Board Info" recognizes the device in either state.

## Burning it

"Burn Bootloader" with a UPDI programmer writes the per-board hex
(`usbcdcboot_wazamonotachi.hex` / `usbcdcboot_wazamonotsurugi.hex` /
`usbcdcboot_wazamonokunai.hex` in `megaavr/bootloaders/hex/`) and the
fuses (including `BOOTSIZE = 8`, i.e. a 4 KB boot section). After that,
no programmer is needed for day-to-day work. Uploading via "Upload Using
Programmer" (UPDI) erases the chip, bootloader included - re-burn it if
you want USB uploads back.

## Entry conditions

On every reset the bootloader decides between staying resident (USB
active, waiting for an upload) and jumping to the application:

* **1200 bps touch** - when the host opens the application's CDC port at
  1200 baud and drops DTR (which is what avrdude does at the start of an
  upload), the running sketch's USB stack writes a magic word, detaches
  from USB, and triggers a watchdog reset. The bootloader sees the magic
  word and stays. This is the normal, hands-free upload path.
* **Reset button** - an external reset (RESET pin, `EXTRF`) enters the
  bootloader. Useful when the sketch has crashed or its USB stack is not
  functional.
* **Empty application** - if the application reset vector reads as blank
  flash (`0xFFFF`), the bootloader stays, so a freshly bootloaded board
  is immediately uploadable.

Any other reset cause (power-on, brown-out, software reset, watchdog
reset without the magic word) starts the application directly, so a
deployed device does not sit in the bootloader after a power blip.

There is no timeout in the stay state: once entered, the bootloader waits
until an upload arrives or the board is reset.

## LED

While (and only while) the bootloader is resident, it drives the board's
LED (active LOW). Since this is a USB bootloader, the LED never collides
with a UART pin position.

## Writing to the flash from the app

The last page of the boot section contains an app-callable SPM stub, and
the last two bytes hold a bootloader version word. The Flash library uses
this automatically (`USING_AVRDU_CDC_BOOTLOADER` is defined on every
Wazamono board): `Flash.writeBytes()` and friends work from application
code, with the boot section itself protected.

## Practical notes

* No serial adapter, no autoreset circuit, no DTR capacitor: the USB
  cable is the whole story.
* Upload speed is not a menu option - USB CDC ignores the baud rate.
* The bootloader does not run on any UART, so `Serial0` and `Serial1`
  remain fully available to the sketch.
* Sketches start at 0x1000 (4 KB in); the board definitions account for
  this automatically.
