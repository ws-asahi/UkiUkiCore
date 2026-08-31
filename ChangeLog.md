# Changelog

The change history of WazamonoCore. WazamonoCore is the Arduino core for the Wazamono series, derived from [DxCore](https://github.com/SpenceKonde/DxCore) (1.6 series).

---

## v0.0.6 — Pro Micro compatibility validation on Tachi and the fixes it produced / Tsurugi pin map rev.3

Compatibility with the Pro Micro (ATmega32U4) was validated end to end on Wazamono Tachi rev.4 hardware (3.3 V and 5 V units), and the core bugs found along the way were fixed. This release also includes support for Tsurugi hardware rev.C and the switch of SerialUPDI to avrdude 8.1.

### Fixes affecting all boards

Bugs found during validation that affect all three Wazamono boards (and other DxCore-derived targets).

- **`analogWrite(pin, 0)` / `analogWrite(pin, 255)` did not make the pin an output** (`wiring_pwm.c`): The Arduino AVR core calls `pinMode(pin, OUTPUT)` before the 0/255 branches and explicitly states that analog output pins do not need `pinMode()`, but this core returned from the end-point branches without setting the direction. On a pin without a prior `pinMode()`, `analogWrite(pin, 0)` left it Hi-Z and `analogWrite(pin, 255)` merely enabled the pull-up without driving. The normal PWM path goes through `_setOutput()` at the end, so only the two end-point values were affected — but those are exactly the places where `pinMode()` is easy to omit ("start a fade from 0", "write ON/OFF with `analogWrite()`").
- **`HardwareSerial::getPin()` always returned `NOT_A_PIN`** (`UART.cpp`): The argument order of `_getPin()` did not match its declaration, so the requested pin index went into `mux_count` and the mux count into `pin`. The `pin > 3` guard then rejected every index on parts whose USART0 has five or more mux options (including the AVR DU). Ordinary UART use never calls `getPin()`, so nothing showed, but `mspiBegin()` / `syncBegin()` use it solely to set the XCK pin's direction and inversion, so **USART SPI host mode and synchronous mode silently did nothing**.
- **TCB PWM did not resume after `noTone()`** (`timers.h`): `disableTimer()` in `Tone.cpp` contains code to return the TCB to PWM mode, but it was wrapped in `ENABLE_TCB_PWM`, a constant defined nowhere in the core. Since `init_TCBs()` puts every TCB other than the millis one into 8-bit PWM mode, this core does use TCBs for PWM, and with the restore code compiled out, **one `tone()` killed `analogWrite()` on the affected pin until reset**. On Tachi millis uses TCB0, so `tone()` takes TCB1 and the casualty is D3.

### AVR DU fixes

- **Internal ADC channels turned into real I/O pins** (`wiring_analog.c`): `analogRead()` / `_analogReadEnh()` masked the channel with `0x3F`. MUXPOS on the AVR DU is 7 bits, and the internal channels are at 0x40 (GND) / 0x42 (TEMPSENSE) / 0x44 (VDDDIV10), a different layout from tinyAVR/Dx's 0x30–0x33. After masking, `ADC_GROUND` → AIN0, `ADC_TEMPERATURE` → AIN2, `ADC_VDDDIV10` → AIN4, so **an unrelated I/O pin was read without any error** (on hardware, VDD read as 1.4 V and the internal temperature as 415 °C). The invalid-channel check also assumed 0x30–0x33, so it was changed to accept only the correct DU values.
- **`analogClockSpeed()` returned garbage** (`wiring_analog.c`): An `int16_t` PROGMEM table was read with `pgm_read_byte_near()` (low byte only), and the comparison and return value dereferenced a code-space address in data space. At 24 MHz it returned −9227 kHz instead of the correct 2000 kHz, and an explicit ADC clock request selected the wrong prescaler.

### Tachi

- **Added Pro Micro (32U4) compatibility macros**: `TX_RX_LED_INIT` / `TXLED0` / `TXLED1` / `RXLED0` / `RXLED1` (TX = PC3/D30, RX = PF3/D17, both active LOW), `SerialUSB`, and `A4` (22) / `A5` (23) / `A11` (29). A4/A5/A11 are defined by SparkFun's promicro variant for unconnected pins; they were undefined in this core, so sketches failed to compile. The same numbers are assigned, but on Tachi they are all gaps in the pin table, so they compile and become safe no-ops at run time (they never land on a different real pin).
- `SERIAL_PORT_HARDWARE_OPEN` changed from `Serial2` **to `Serial1`**. This matches SparkFun's value, and by definition Serial1 on D0/D1 is the legitimate "first free port" (Serial2 is exclusive with SPI SCK/SS).

### USB CDC

- **Added the Leonardo `Serial_` CDC accessors** (`USBSerial.h`): `dtr()` / `rts()` / `baud()` / `stopbits()` / `paritytype()` / `numbits()` / `readBreak()` and the two enums, implemented with the same names, meaning, and raw values as ArduinoCore-avr. The state was already held by `usb_cdc.c`; the missing accessors were added and the wValue of `SEND_BREAK`, previously answered with a ZLP and discarded, is now latched. The classic USB-to-serial bridge sketch that picks up the host's baud rate with `Serial.baud()` and passes it to `Serial1.begin()` did not even compile before.

### SPISlave

- **The first two bytes of a primed response could not be overwritten**: With BUFWR = 1, DATA writes while DREIF is low are silently dropped (DS40002548B Figure 26-4), so `setData()` could not replace the zeros loaded by `begin()`, and the first transaction was preceded by two zero bytes. ENABLE is now cycled before priming to flush the transmit path.
- **A lingering BUFOVF jammed the receive path**: When the host clocks faster than the interrupt handler can keep up, the client overflows, and BUFOVF is not cleared until the receive FIFO is drained — even across `end()` / `begin()`. In that state RXCIF never sets, the SPI interrupt stops refilling the response, the shift register echoes MOSI one byte late, and the SS rising-edge handler reports empty transactions. **One failure at high speed left the board unable to communicate at any speed until reset.** `begin()` / `end()` / the priming path now read DATA three times to empty the FIFO.
- Fixed the comment in `SPISlave.h` (Tachi's SS is PD7 = D18 = A0; it still said D4 from an older revision).

### Non-canonical pin numbering

- **`_getPin()` derived RX/XDIR as pin number + 1** (`UART.cpp`): That is the correct relationship for PORT bit positions, but the function returns Arduino pin numbers, so it only holds when the variant numbers pins in port bit order. Tachi has TX = PA4 = D1 / RX = PA5 = D0, so `getPin(1)` returned D2 (PA2 = Grove SDA). When `NONCANONICAL_PIN_NUMBERS` is defined, the bit position is advanced within the same port and mapped back. Canonically numbered variants behave as before.

### Behavior confirmed by validation (no change)

Items confirmed on Tachi rev.4 hardware and judged not to need a fix. See WazamonoTachi.md for details.

- The 36-entry pin map, analog aliases, and no-op behavior of reserved pins are as designed
- PWM on D3/D5/D6/D9/D10 is the same set as the Pro Micro (D14/D16 added, exclusive with SPI). All 7 channels are unified at 1470.6 Hz (the 32U4 mixes 490/980 Hz)
- Despite having no crystal, the clock measured within ±0.1% (n = 2, both 5 V and 3.3 V). Serial1's overall baud error is at most 0.08%, more than 25× inside the ±2% tolerance for 8N1. Auto-tune is effectively idle because the raw accuracy already beats its 0.4% step resolution
- The ADC is natively 10-bit and `analogRead()` returns 0–1023 (same as the 32U4). The four internal references agree with each other within 0.3%. No differential ADC (the 32U4 has one) / extendable to 13 bits by accumulation
- External interrupts work with RISING/FALLING/CHANGE/LOW on all 18 header pins (the 32U4 has only 5, with LOW limited to INT0–3). Latency is about 5–7 µs, with a linear 0.33 µs/bit difference by PORT bit position (r = 0.995, lower bits faster)
- Communication between two identical units using USART0 SPI host mode and SPI0 client mode works in all four modes. The stable ceiling is 1 MHz (limited by interrupt contention on the same CPU, not the SPI hardware)

### Known incompatibilities (by design, will not be fixed)

- The PWM pin that `tone()` steals differs (Timer1 → D9/D10 on the 32U4; TCB1 → **D3** on Tachi)
- `digitalPinToInterrupt()` is the identity mapping and never returns `NOT_AN_INTERRUPT`. Passing a gap pin is rejected on the `attachInterrupt()` side and becomes a no-op, but sketches that branch on `NOT_AN_INTERRUPT` will not take that branch
- EEPROM 1 KB → **256 B** (DS40002548B Table 8-3; fixed at 256 B on every AVR DU part). SRAM 2.5 KB → 8 KB, Flash 32 KB → 64 KB
- The `analogReference()` options differ (the 32U4's `INTERNAL` / `INTERNAL1V1` / `INTERNAL2V56` are **deliberately undefined**. Silently mapping them to a nearby value would quietly corrupt measurements, so a compile error that gets noticed was chosen instead)
- A0 (PD7) doubles as VREFA, so using the external reference takes away A0 / SPI SS / Serial2 RX at the same time
- `LED_BUILTIN` = 17 (13 on the 32U4; unconnected pins in both cases), `SS` = 18 (17 = RX LED on the 32U4), the numeric values of `A6`–`A10` are the pins' own numbers (the 32U4 uses the Leonardo duplicate numbers 24–28; the pins referred to are the same)

### Bootloader

- Tachi's bootloader hex is now generated **for avr64du32 / LED = PF3 / VREG = 0**. All compatibility validation in this release was done on rev.4 hardware flashed with this hex. The v0.0.5 note that "`usbcdcboot_wazamonotachi.hex` is still built for avr64du28/PA0" is resolved (Tachi briefly considered the AVR64DU28 but returned to the AVR64DU32 for board-area reasons).

### Tools

- **SerialUPDI now works without Python**: The three `SerialUPDI - *` programmer entries were switched from `prog.py` (pymcuprog) to avrdude 8.1's `-c serialupdi`. This fixes the manual / git installation case where `megaavr/tools/python3/` does not exist and Burn Bootloader failed with `executable file not found in %PATH%`.
- As a side effect, there is no longer a dependency on `bootloader.serupdifuse5`, which the upstream DxCore AVR DU section never defined. Fuse programming is unified on the `avrdudefuse5` path, the same as PICkit / nEDBG / Atmel-ICE.
- `megaavr/tools/prog.py` and `megaavr/tools/libs/` remain in the tree but are no longer referenced by any recipe (`bootloader.pymcuprogstring` is likewise unused).

### Tsurugi pin map rev.3 (contains breaking changes)

Changes for Tsurugi hardware rev.C. Tachi / Kunai are unaffected.

- Pin map rev.3: **D4 = PC3 (was D7) / D7 = PF4 (was D4) swapped**. D4 becomes the default position of CCL LUT1-OUT. A10/A13 follow the D numbers.
- **AREF (PD7) exposed as D20/A20**: When the external reference is not in use, it is available as GPIO / SPI0 hardware SS (ALT4) / Serial2 RX (a structural feature of modern AVRs; impossible on the Uno R3).
- **Serial2 added**: Fixed to USART1 ALT2 (TX = D13/PD6, RX = D20/PD7). Shared with D13 (SCK) and AREF, so it cannot be used while SPI or the external reference is in use.
- **Exclusive TCB1 PWM**: The `analogWrite()` output is selected from D3 (LUT0 ALT) / D4 (LUT1 default) / D7 (TCB1 WO direct). The pin most recently written takes the output (default D3). The `WAZAMONO_TCB1_PWMMUX` mechanism was added to the core (the single-path `WAZAMONO_TCB1_LUTPWM_*` on Tachi/Kunai is unchanged).
- **D7/D8 swapped (following the final board rev.C)**: D7 = PF5 (AIN21, the direct TCB1 WO PWM output) / D8 = PF4 (AIN20, no PWM). A13/A14 follow the D numbers.
- **Crystal removed**: The clock selection menu was removed from boards.txt and the clock fixed at the internal OSCHF 24 MHz (same policy as the Uno R4). USB's 48 MHz continues to come from PLL48M + SOF auto-tuning.
- **USB-CDC TX/RX LEDs**: PA0 (TX) / PA1 (RX), freed by removing the crystal, now drive activity LEDs (active LOW, 100 ms one-shot, Pro Micro convention). Directly controllable with `digitalWrite()` as **D30 (TX) / D31 (RX)** (D21–D29 are no-op gaps). In addition to `PIN_LED_TX` / `PIN_LED_RX` / `LED_BUILTIN_TX` / `LED_BUILTIN_RX`, the Leonardo-compatible `TXLED1/TXLED0/RXLED1/RXLED0` macros are defined.

### Documentation

- WazamonoTsurugi.md updated to pin map rev.3 (D4/D7, D20/AREF, Serial2, exclusive PWM, clock section, LED table, and in the power section the EN divider values 100k/100k and the start-up voltage of about 5.0 V).

---

## v0.0.5 — Pin map rev.4 (Tachi returns to the AVR64DU32)

Changes for Tachi hardware rev.4. **Tachi requires burn-bootloader to be redone** (the MCU returns from avr64du28 to avr64du32). Tsurugi / Kunai are unchanged.

### Tachi (breaking changes)

- MCU returned from **AVR64DU28 → AVR64DU32 (TQFP-32)**. The TQFP package was needed for board-area reasons, and the feature loss of the 28-pin part did not justify its cost saving (about ¥20). The crystal-less design (internal OSCHF fixed at 24 MHz) continues.
- Pin map rev.4: D4 = **PF4** (A6), D7 = **PA6** (A17/XCK), D8 = **PA7** (A8/XDIR/AC0 output/EVOUTA/CLKOUT), D17 = **PF3** (A21), D18 = PD7 (A0/SPI SS/Serial2 RX/VREFA), D19 = PF0 (A1), D20 = **PF1** (A2), D21 = **PF2** (A3/EVOUTF), **D30 = PC3 (A34) restored**. Gaps are D11–D13 and D22–D29.
- LEDs returned to the Pro Micro convention of two (both active LOW): **RX LED = D17 (PF3) = `LED_BUILTIN` = `LED_BUILTIN_RX`**, **TX LED = D30 (PC3) = `LED_BUILTIN_TX`**. No dedicated user LED (the old D13).
- **PA0 and PA1 are unconnected on the board; PF5 is hidden (reserved)**: They are hidden indices (NOT_A_PORT/NOT_A_PIN) and become no-ops if operated by mistake. AIN21 (PF5) is unavailable and the `AIN21` macro is not defined.
- EVOUTF (PF2 = D21) restored. LUT3 is fully usable (IN0/IN1/IN2 = A1/A2/A3, OUT = D17). LUT1-OUT is D30 (TX LED).
- Analog aliases: A0–A3 = D18–D21, A6–A10 Pro Micro compatible, A11 remains a deliberate gap, A12–A17 = D0–D7, A18–A21 = D14–D17, A34 = D30.

### Bootloader

- Build matrix updated: Tachi = `avr64du32` / LED **PF3**, active LOW / VREG = 0.
- **Note: `bootloaders/hex/usbcdcboot_wazamonotachi.hex` is still built for avr64du28/PA0.** Run `build_wazamono.(sh|bat)` with wazamono-toolchain to regenerate it, commit, and then burn-bootloader the rev.4 board. The Tsurugi / Kunai hex files remain valid.

### Documentation

- The pin table in WazamonoTachi.md updated to rev.4 (fixed the AVR64DU28 label in the specification heading, removed a duplicated empty I2C table). The board menu name in the README changed to "Wazamono Tachi (Pro Micro)".
- The Tachi rows in the library READMEs (EventSystem / CustomLogic / ClockOut) and example sketches (CompToPin / PinToPin / MultipleOutputs / TCA0Demo1–4) updated to rev.4 values (EVOUTA = D8 etc.).

---

## v0.0.4 — Pin map rev.3 (Tachi on the AVR64DU28 / unified VUSB supply)

Changes for hardware rev.3. **All three boards require burn-bootloader to be redone** (mandatory for Tachi because the MCU changes, and for Tsurugi because the VREG setting changes).

### Tachi (breaking changes)

- MCU changed from **AVR64DU32 (32-pin) → AVR64DU28 (28-pin)** (cost reduction). `build.mcu=avr64du28` in `boards.txt`.
- **External crystal removed**. The clock is fixed at the internal OSCHF 24 MHz and the Clock Speed menu removed (same configuration as Kunai). PA0/PA1 freed as GPIO.
- Pin map rev.3: D4 = PF1 (A6), D7 = PA1 (no ADC), D8 = PC3 (A8), D14–D16 = SPI (unchanged), D17 = PA0, D18 = PD7 (**A0**/SPI SS/Serial2 RX/VREFA), D19 = PF0 (A1), D20 = PA6 (A2/XCK), D21 = PA7 (A3/XDIR/EVOUTA/CLKOUT). **D11–D13 are gaps** (the dedicated LED on the old D13 is gone), and the old D30 (TX LED) is gone too.
- **A single LED on D17 (PA0)**: `LED_BUILTIN` = `LED_BUILTIN_RX`, active LOW, doubling as the USB-CDC receive activity indicator. `LED_BUILTIN_TX` is undefined (referencing it is a compile error).
- Analog aliases: A0–A3 = D18–D21, A6–A10 remain Pro Micro compatible, **A11 deliberately undefined** (to prevent confusion with the Leonardo's A11 = D12), A12–A16 = D0–D5, A17 a reserved gap, A18–A20 = D14–D16.
- EVOUTF removed (PF2 does not exist). Event outputs are EVOUTA (D21) / EVOUTD (D9).
- Serial2 RX moved from D4 → **D18** (TX stays on D15). The SPI SS label also moved from D4 → **D18** (same PD7).

### Tsurugi

- No pin map changes.
- **VUSB (the USB transceiver's 3.3 V) is now supplied by the on-board external 3.3 V LDO (NJM2881F33) instead of the internal USB regulator**, unifying the circuit with Tachi / Kunai (power configuration 3s). `-DUSB_VREG_INTERNAL` removed from `boards.txt`, and the bootloader is built with VREG = 0.

### Kunai

- On-board LED numbering and roles changed to follow the XIAO: **D11 = PD4 = LED_BUILTIN + TX activity LED / D12 = PD5 = RX activity LED** (renumbered from the old D13/D14, and the TX/RX roles swapped). Analog aliases A13/A14 → A11/A12.
- Indices 13/14 become gaps, so `digitalWrite(13, ...)` aimed at the XIAO's user LED "13" is a harmless no-op.

### Bootloader

- Build matrix updated: Tachi = `avr64du28` / LED **PA0**, active LOW / VREG = 0; Tsurugi = VREG = 0 (changed); Kunai = unchanged.
- **Note: the hex files in `bootloaders/hex/` have not been regenerated as of this release.** Run `build_wazamono.(sh|bat)` with wazamono-toolchain (avr-gcc 15.2.0-wazamono1) to update the hex files before burn-bootloader. The old `usbcdcboot_wazamonotachi.hex` targets avr64du32/PC3 and cannot be used on rev.3 boards.

### Documentation

- WazamonoTachi.md fully revised for rev.3 (power: RAW input + Torex XC6702D501 / XC8110 / XC6503D331; crystal removed from the BOM).
- Per-board pin tables in the library READMEs (EventSystem / CustomLogic / ClockOut / SPISlave) and example sketches (CompToPin / PinToPin / MultipleOutputs / TCA0Demo1–4 / ServoMaxTest) corrected to match the current variants (including rev.1-era values that had survived on all three boards).

---

## v0.0.3 — Pro Micro compatible watchdog support

Watchdog code written for the SparkFun Pro Micro (ATmega32U4) now builds and runs unmodified.

### Added

- `cores/dxcore/wdt_compat.h` added to the core and included automatically from `Arduino.h`. Classic AVR code using `#include <avr/wdt.h>` works as-is.
  - `wdt_enable(WDTO_*)` … Correctly converts the classic `WDTO_*` values (15 ms to 8 s) to the PERIOD encoding of the AVR DU's `WDT.CTRLA` (WDTO_2S → 2.0 s etc.). The avr-libc version does not convert, resulting in wrong periods or undefined values.
  - `wdt_disable()` … Writes `WDT_PERIOD_OFF_gc` to stop the WDT.
  - `wdt_reset()` … The `WDR` instruction (common to classic and modern; the avr-libc version is used as-is).
  - `WDTO_15MS` through `WDTO_8S` defined with the classic values (0–9).
- Per datasheet DS40002548A §21.3.6 ("do not write `WDT.CTRLA` while SYNCBUSY = 1"), SYNCBUSY is awaited before writing. Consecutive `wdt_enable()`/`wdt_disable()` calls are applied reliably (avoids the "second write is ignored" problem of naive implementations).

### Notes

- `ResetWithWDT()` and friends in the existing `DxCore` library remain usable.
- This compatibility layer applies to the whole core (both Tachi and Tsurugi).
- `MCUSR` (the classic AVR reset flags) is out of scope. Code containing `MCUSR = 0;` needs separate reset-flag compatibility.

---

## v0.0.2 — Wazamono Tsurugi (beta) added

Software support for **Wazamono Tsurugi**, the Arduino Uno R3 successor, has been added.

### Added

- **Wazamono Tsurugi** variant (`variants/WazamonoTsurugi`). AVR64DU32, Uno R3 compatible pinout (D0–D13, A0–A5, AREF).
  - Numbering follows the Uno R3 standard (D0–D19 contiguous). `LED_BUILTIN` = D13 (PD6).
  - Serial: `Serial` = USB CDC, **`Serial0`** = D0/D1 hardware UART (USART0 ALT1). USART1 exists on the chip but has no usable pins on Tsurugi.
  - SPI (D11/D12/D13 = SPI0 ALT4, SS = D10) and I2C (A4/A5 = TWI0) placed per Uno R3 convention.
  - PWM: TCA0 → PORTD (D5, D6, D9, D10, D11, D12), TCB0 ALT1 → D4. **millis on TCB1**, freeing TCB0 for D4 PWM.
- **Wazamono Tsurugi** added to `boards.txt` (VID/PID application `0x1209:0x0008` / bootloader `0x1209:0x0007`).
- Board document `extras/WazamonoTsurugi.md` (including a comparison with the ATmega328P / Uno R3) added.

### Known limitations

- Tsurugi's final BOM and schematic, and its dedicated bootloader hex (`usbcdcboot_wazamonotsurugi.hex`), are in preparation.
- The VID/PID uses the pid.codes test range for development. It must be replaced with an official VID/PID before product shipment.

---

## v0.0.1 — Initial release

The first release, restructured from DxCore as a core dedicated to the Wazamono series.

### Core structure

- Derived from DxCore (1.6 series). Definitions for MCUs and board families outside the Wazamono series (DA / DB / DD / EA / EB and generic DU boards, etc.) removed.
- `boards.txt` reduced to a single board, **Wazamono Tachi (AVR64DU32)**. Menus fixed for the product (chip selection and the various option menus removed, defaults fixed).
- Only the "Clock Speed" menu kept. Selectable between 24 MHz external crystal (default) and the internal oscillator (24/20/16 MHz).

### Supported boards

- **Wazamono Tachi (太刀)** — Pro Micro successor, AVR64DU32, USB-C. Variant `WazamonoTachi` added.
  - Serial: `Serial` = USB CDC, `Serial1` = USART1 (D0/D1, fixed ALT2), `Serial2` = USART0 (D2/D3, fixed ALT2, exclusive with I2C).
  - SPI (PA4/PA5/PA6/PA7), I2C (PA2/PA3), PWM (TCA0 → PORTF D5–D10, TCB1 D3) assigned.
  - `millis()` / `micros()` use TCB0, freeing TCB1 (D3) and TCA0 (D5–D10) for PWM.
  - `LED_BUILTIN` = D17 (PD5, RX LED).

### USB

- A clean-room USB stack based solely on the USB 2.0 specification and the datasheet (DS40002548A).
- USB CDC virtual serial, USB HID, and USB-MIDI supported.
- USB CDC bootloader (STK500v1, 1200 bps touch). A double-tap of the reset button also enters the bootloader.

### Pin definitions

- In the Wazamono Tachi variant (`pins_arduino.h`), the pin assignment of each peripheral is fixed on the variant side. No `swap()` calls are needed in sketches.
- Analog inputs **A0–A3** assigned to PD3 / PD2 / PD1 / PD0 (D18–D21) to match the board silkscreen.

### Known limitations and plans

- **Wazamono Tsurugi (剣)** (Arduino Uno R3 successor) is in circuit design. A variant will be added as soon as it is supported.
- Installation from the Board Manager (JSON URL) not yet supported (manual installation only).
- The VID/PID uses the pid.codes test range for development (`0x1209:0x0006` / `0x1209:0x0005`). It must be replaced with an official VID/PID before product shipment.
