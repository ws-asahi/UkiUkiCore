# Libraries bundled with WazamonoCore

WazamonoCore bundles every library used with the Wazamono boards (Tachi, Tsurugi, Kunai — all AVR DU series). The DU-specific peripheral libraries follow a common design: like `Serial`, they are **predefined objects**, they use **pins that are fixed per board**, addressed by **Arduino pin number**, and there is no need to open the I/O headers. `AnalogComp`, `CustomLogic`, and `EventSystem` are built to be combined with each other. Libraries for peripherals that do not exist on the DU series (TCD, ZCD, OPAMP, PTC, MVIO) are not included.

## Wazamono-specific

### Flash
[Flash readme](../libraries/Flash/README.md) Writes to program flash from the running sketch. On Wazamono boards this goes through a small stub (`spm z+; ret`) that the USB CDC bootloader keeps at the end of its 4 KB boot section. `Flash.checkWritable()` validates the bootloader before writing, and any write below the 4 KB boundary (the bootloader itself) is refused. The erase unit is a 512-byte flash page. Use **high addresses first** so you do not collide with your own code. Verified on hardware with the FlashDemo and FlashWriteTest examples.

### USERSIG
[USERSIG readme](../libraries/USERSIG/README.md) The USERROW (user signature area) is **512 bytes** of non-volatile memory at 0x1200 that survives a chip erase and normal sketch uploads — the ideal place for serial numbers, calibration values, and board configuration. The API is EEPROM-compatible (`read`/`write`/`update`/`get`/`put`). Because the USERROW can only be erased as a whole, writes that need to turn a bit back to 1 are buffered in RAM (512 bytes) and applied by `flush()`. `write()` returns 1 when the write went straight through and 0 when it is waiting for `flush()`. Endurance is finite — do not write in a loop. The UsersigTest example checks all 512 bytes on hardware.

### DxCore
[DxCore readme](../libraries/DxCore/README.md) Helper wrappers around chip configuration (also home to the PWMTest example). Part of the API touches peripherals that do not exist on the DU (MVIO, OPAMP); it is kept for compatibility and will be slimmed down before the board package is published.

## Peripheral libraries
The DU's analog comparator, custom logic, and event system are provided as three small libraries in the same style that can be interconnected.

### AnalogComp
[AnalogComp readme](../libraries/AnalogComp/README.md) Exposes the on-chip analog comparator (AC0) as a single predefined object: compare two voltages, compare against the internal reference (`begin(INTERNAL2V5)` etc., with fine level control), hysteresis, reading the result, driving the AC output pin, and `attachInterrupt()`-style callbacks on output change. The comparison result can be fed directly to `CustomLogic` (`LOGIC_ANALOG_COMP`) or `EventSystem` (`EVENT_ANALOG_COMP`) **using neither a pin nor the CPU**.

### CustomLogic
[CustomLogic readme](../libraries/CustomLogic/README.md) Exposes the CCL (Configurable Custom Logic) lookup tables as predefined units with fixed pins: pick a gate (`AND`/`OR`/`XOR`/`NAND`/`NOR`/`XNOR`/`NOT`/`NOP`) or any 3-input truth table, and the result runs in hardware with **zero CPU time**. Inputs can come from the unit's pins, from the analog comparator, from its own output (to build latches), from the other unit, or from any pin via `EventSystem` (`setInputINn()`). The result can be driven to **several destinations at once** — the dedicated OUT pin, the alternate pin, and the board's event output pins (`setOutput()`/`addOutput()`). `attachInterrupt()`-style callbacks on output change are available too.

### EventSystem
[EventSystem readme](../libraries/EventSystem/README.md) The chip's internal "wiring". Six predefined connections (`EventSystem` through `EventSystem5`) each carry one source — an Arduino pin, the AnalogComp result, a CustomLogic output, or a software `trigger()` pulse — to any number of destinations (the board's fixed event output pins, CustomLogic event inputs). `EventSystem.connect(8, 2);` is all it takes. Pin sources are limited to **two at a time from the same port** (a hardware property). Timer/USART/SPI event features are not provided, to avoid conflicts.

## USB class libraries
Wazamono boards are native USB devices. In addition to the CDC serial port (`Serial`), two bundled class libraries let them act as HID devices or MIDI instruments. Both are Wazamono forks whose AVR DU support has been submitted upstream (see the bundling notes in each readme for provenance).

### HID-Project
[HID-Project readme](../libraries/HID-Project/Readme.md) NicoHood's extended HID library (bundled from the ws-asahi/HID fork, MIT): BootKeyboard/BootMouse, Keyboard, Mouse, AbsoluteMouse, Consumer (media keys), System, Gamepad, RawHID. 13 examples included.

### MIDIUSB
[MIDIUSB readme](../libraries/MIDIUSB/README.adoc) The official Arduino MIDIUSB library (bundled from the ws-asahi/MIDIUSB fork, LGPL 2.1): the board enumerates as a USB-MIDI instrument and can send and receive MIDI event packets. 5 examples included.

### HID
The low-level PluggableUSB HID transport (`HID_`) on which HID-Project is built. Not used directly from sketches.

## Standard Arduino libraries

### EEPROM
[EEPROM readme](../libraries/EEPROM/README.md) The standard API for the DU's **256-byte** built-in EEPROM. Erase is byte-granular (unlike the USERROW, no buffering tricks are needed). Beware of libraries that assume the EEPROM internals of other architectures.

### SPI
[SPI readme](../libraries/SPI/README.md) The standard SPI master API on SPI0. Each Wazamono board fixes its SPI pin assignment to match the silkscreen (Tsurugi: the Uno's D11–D13 positions), so there is no need to call `swap()` in sketches (and you should not).

### Wire
[Wire readme](../libraries/Wire/README.md) TWI master/slave (dual mode supported). In addition to the full standard API it supports general-call reception, a second address, and an address mask. **The internal pull-ups are not enabled automatically** — call `Wire.usePullups()` if your bus has no pull-up resistors (fitting real resistors is the proper solution).

### SD
[SD readme](../libraries/SD/README.adoc) The standard Arduino SD card library (FAT16/FAT32 over SPI). 7 examples included.

### SoftwareSerial
Inherited unchanged from the official megaavr core, but **best avoided**: every board has a USB CDC port (`Serial`) and fixed-pin hardware USARTs. SoftwareSerial monopolizes the interrupt on the pins it uses and burns CPU time bit-banging.

## General hardware

### Servo
[Servo reference](https://www.arduino.cc/reference/en/libraries/servo/) The improved reimplementation from the megaTinyCore/DxCore lineage: it does not depend on the TCA0 prescaler, so **changing the PWM frequency does not break servos**, and ISR timing is improved. If the Library Manager version of Servo takes precedence over the bundled one, switch to `#include <Servo_DxCore.h>` — the API is identical.

### tinyNeoPixel
[tinyNeoPixel documentation](tinyNeoPixel.md) WS2812-style (NeoPixel) control in two flavors: `tinyNeoPixel` (Adafruit-compatible, dynamic buffer) and `tinyNeoPixel_Static` (you declare the frame buffer yourself, so RAM usage shows up in the compile output and malloc is not used). The show() timing is written for the AVRxt instruction timing of these parts and holds at every supported clock.
