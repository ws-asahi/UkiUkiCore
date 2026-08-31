# Wazamono Tsurugi (剣)

**English** | [日本語](WazamonoTsurugi.ja.md)

**Arduino Uno R3 compatible — AVR64DU32 / USB-C**

Wazamono Tsurugi is a redesign of the Arduino Uno R3 form factor around the `AVR64DU32`, an AVR with native USB.  
Where the Uno R3 needed a separate USB-to-serial chip, Tsurugi's microcontroller connects directly to the PC over USB-C.  
Its USB functionality is close to that of the Leonardo, but due to hardware constraints the dedicated SPI header pins are not independent from GPIO.  
It also carries a **DC jack input (up to 24 V) with a synchronous buck converter**, intended for industrial use.  
 
This page is the documentation for Wazamono Tsurugi. For an overview of the core as a whole, see the [README](../../README.md).  
**Status: prototype.** Pin definitions and the bootloader may change.  
 

---

## Overview

| Item | Detail |
|------|--------|
| MCU | AVR64DU32 |
| Form factor | Uno R3 compatible |
| USB | USB-C (USB 2.0 Full-Speed, built into the MCU) |
| Clock | 24 MHz internal oscillator |
| Power | USB 5 V / DC jack 7–24 V DC |
| Upload | USB CDC bootloader (STK500v1) |

---

## Board specifications (AVR64DU32)

| Item | Value |
|------|-------|
| Flash | 64 KB (60 KB for sketches / 4 KB USB bootloader) |
| SRAM | 8 KB |
| EEPROM | 256 B |
| USERROW | 512 B |
| Max. operating frequency | 24 MHz |
| USB | USB 2.0 Full-Speed device |
| USB endpoints | IN 16 / OUT 16 (32 total) |
| ADC | 10-bit 170 ksps × 1 |
| Timers | 16-bit TCA0 × 1 / 16-bit TCB × 2 |
| USART | 2 |
| SPI | 2 |
| I2C | 1 |
| External interrupts | All pins |
| CCL (LUT) | 4 |
| Event system | 6 channels |
| Analog comparator (AC) | 1 |

<sub>Figures are from datasheet DS40002548A (AVR64DU32). Sketch Flash size and SRAM size are the values configured in boards.txt.</sub>

---

## Comparison with the ATmega328P (Arduino Uno R3) and ATmega32U4 (Arduino Leonardo)

The Arduino Uno R3 that Wazamono Tsurugi replaces uses the **ATmega328P** (no native USB / 8-bit MCU).  
The **ATmega32U4** (native USB / 8-bit MCU), which is close in performance, is included for comparison as well.  
The AVR64DU32 is a new-generation **AVRxt core** with built-in USB and stronger clock, memory, and peripherals across the board.  

| Item | Wazamono Tsurugi (AVR64DU32) | Arduino Uno R3 (ATmega328P) | Arduino Leonardo (ATmega32U4) |
|------|------------------------------|------------------------------|------------------------------|
| Core | AVRxt (improved instruction timing) | Classic AVR | Classic AVR |
| Max. clock | 24 MHz | 16 MHz | 16 MHz |
| USB | Built into the MCU (no converter chip) | None (separate USB chip on the board) | Built into the MCU (no converter chip) |
| Flash | 64 KB | 32 KB | 32 KB |
| SRAM | 8 KB | 2 KB | 2.5 KB |
| EEPROM | 256 B | 1 KB | 1 KB |
| USERROW | 512 B | - | - |
| ADC | 10-bit 170 ksps, 21 ch | 10-bit 9.6 ksps, 6 ch | 10-bit 9.6 ksps, 12 ch |
| Timers | 16-bit × 3 (TCA0 + TCB × 2) | 16-bit × 1 + 8-bit × 2 | 16-bit × 2 + 8-bit × 1 + 10-bit × 1 |
| USART | 2 | 1 | 1 |
| SPI | 2 (one can act as client) | 1 (no client mode) | 1 (no client mode) |
| I2C | 1 | 1 | 1 |
| External interrupts | All pins | 2 | 4 | 
| CCL (LUT) | 4 | None | None |
| Event system | 6 ch | None | None |
| Analog comparator (AC) | 1 | None | None |

---

## Comparison with the RA4M1 (Arduino Uno R4)

Compared with the Renesas RA4M1 in the Uno R4, the AVR64DU32 falls short in most respects.  
There are, however, a few areas where the AVR64DU32 comes out ahead.  

| Item | Wazamono Tsurugi (AVR64DU32) | Arduino Uno R4 (RA4M1) |
|------|------------------------------|------------------------------|
| Core | 8-bit AVRxt | 32-bit Arm Cortex-M4 (with FPU) |
| Max. clock | 24 MHz | 48 MHz |
| Operating voltage | 1.8–5.5 V | 1.6–5.5 V |
| USB | Built into the MCU (no converter chip) | Built into the MCU (no converter chip) |
| Flash | 64 KB | 256 KB |
| SRAM | 8 KB | 32 KB |
| EEPROM | 256 B | None (8 KB data flash, emulated by the core) |
| ADC | 10-bit 170 ksps, 21 ch | 14-bit, 18 ch (64-pin package) |
| DAC | None | 12-bit × 1 |
| Timers | 16-bit × 3 (TCA0 + TCB × 2) | 32-bit × 2 + 16-bit × 6 (GPT) + 16-bit × 2 (AGT) |
| USART | 2 | 4 (SCI) |
| SPI | 2 (one can act as client) | 2 (both can act as client) + simple SPI on 4 SCI |
| I2C | 1 | 2 + simple I2C (master only) on 4 SCI |
| External interrupts | **All pins** | 12 IRQ lines (64-pin package, multiplexed onto pins) |
| CCL (LUT) | **4** | None |
| Event system | 6 ch | Yes (ELC) |
| Analog comparator (AC) | 1 | 2 (ACMPLP) |
| GPIO drive capability | **20 mA** | 4–8 mA (20 mA on P408/P409 only) |

<sub>RA4M1 figures are from the Renesas RA4M1 Group Datasheet (R01DS0355) for the 64-pin R7FA4M1AB3CFM used on the Uno R4.</sub>

---

### Main performance advantages

- **USB built into the MCU** — The Uno R3 carried a separate USB-to-serial chip; Tsurugi does not need one.  
  `Serial` is the USB CDC virtual serial port as-is, and the board can also act as USB HID (keyboard/mouse) or USB-MIDI.  
- **Clock and processing speed** — 24 MHz operation (1.5× the Uno's 16 MHz), and
  the AVRxt core has improved timing on some instructions, giving roughly 12% higher benchmark scores at the same clock.  
- **Memory** — 2× the Flash (64 KB), 4× the SRAM (8 KB).  
- **New-generation peripherals** — CCL (4 logic blocks) and the event system (6 channels) allow hardware signal processing without the CPU.  
- **Analog input** — ADC channels increase to 21, so every GPIO supports analog input.  
- **Drive capability per pin** — The robust AVR I/O can source/sink in the 20 mA class.  
- **Additional UART** — Two hardware UARTs are available.  
- **RS-422/485** — RS-485 communication is possible using the USART (an external transceiver chip is required).  
- **Wide input voltage range** — Up to 24 V can be fed through the DC jack; the on-board buck converter generates 5 V.  

---

### Points to note

- **The ATmega328P / ATmega32U4 have more EEPROM** (1 KB vs. 256 B).
- Applications that store a lot of non-volatile data may need to revisit their storage strategy (User Row or Flash; see "Data storage areas" below).

---

## Data storage areas

The AVR64DU32 has several non-volatile memory areas for different purposes.  
EEPROM is smaller than on the ATmega (256 B), but new areas such as **USERROW (user row)** are available instead.  

| Area | Size | Erase unit | Endurance | On chip erase (re-upload) | Library |
|------|------|------------|-----------|---------------------------|---------|
| EEPROM | 256 B | Byte (1–32 B) | 100,000 cycles | Erased (can be preserved with the EESAVE fuse) | `EEPROM.h` |
| USERROW | 512 B | Whole 512 B page | 1,000 cycles | **Preserved** | `USERSIG.h` |
| Flash (APPDATA) | Free space in the sketch area | 512 B page | 1,000 cycles | Erased | `Flash.h` |
| SIGROW | Read-only | — | — | — | Contains the factory-programmed 16-byte unique serial number |

<sub>Memory specifications and endurance figures are from datasheet DS40002548A (§8 Memories / §11 NVMCTRL / Electrical Characteristics).</sub>

---

## Benchmarks

Benchmark results against other compatible boards.  

| Board | MCU | Clock (MHz) | Dhrystone 2.1 (avg. of 5) | CoreMark 1.0 Iterations/Sec (avg. of 5) |
|-------|-----|-------------|---------------------------|-----------------------------------------|
| Tsurugi | AVR64DU32 | 24.0 | (not tested)  | (not tested) |
| Arduino Uno R3 | ATmega328P | 16.0 | 17307.590 | - (could not be uploaded) |
| Arduino Leonardo | ATmega32U4 | 16.0 | 17207.268 | - (did not run) |
| Arduino Uno R4 | RA4M1 | 48.0 | 62303.986 | 81.78 |

---

## Pin mapping

The numbering matches the Arduino Uno R3.  
Every external pin is connected to the ADC.  
Basic pin numbers are identical to the Uno R3; pins that originally lacked an ADC channel have been given new analog pin numbers.  
The layout therefore differs considerably from the Leonardo.  

| Pin | MCU | Alias | ADC ch | Main functions |
|-----|-----|-------|--------|----------------|
| D0 | PA5 | A6 | AIN25 | **RX** (Serial1) / **MOSI** (SPI1) |
| D1 | PA4 | A7 | AIN24 | **TX** (Serial1) / **MISO** (SPI1) |
| D2 | PA7 | A8 | AIN27 | **XDIR** (Serial1) / **OUT** (AnalogComp) / EVOUTA / CLKOUT |
| D3 | PA6 | A9 | AIN26 | ~PWM (TCB1 + LUT0) / **XCK** (Serial1) / **CLK** (SPI1) |
| D4 | PC3 | A10 | AIN31 | ~PWM (TCB1 + LUT1) |
| D5 | PD0 | A11 | AIN0 | ~PWM (TCA0 WO0) / **IN0** (CustomLogic) |
| D6 | PD1 | A12 | AIN1 | ~PWM (TCA0 WO1) / **IN1** (CustomLogic) |
| D7 | PF5 | A13 | AIN21 | ~PWM (**TCB1 WO direct**, exclusive) |
| D8 | PF4 | A14 | AIN20 | - |
| D9 | PD2 | A15 | AIN2 | ~PWM (TCA0 WO2) / **IN2** (CustomLogic) / **P** (AnalogComp) / EVOUTD |
| D10 | PD3 | A16 | AIN3 | ~PWM (TCA0 WO3) / **OUT** (CustomLogic) / **N** (AnalogComp) |
| D11 | PD4 | A17 | AIN4 | ~PWM (TCA0 WO4) / **MOSI** (SPI) |
| D12 | PD5 | A18 | AIN5 | ~PWM (TCA0 WO5) / **MISO** (SPI) |
| D13 | PD6 | A19 | AIN6 | LED_BUILTIN / **SCK** (SPI) / **TX** (Serial2) |
| AREF | PD7 | D20 / A20 | AIN7 | AREF **(shared with GPIO)** / **SS** (SPI) / **RX** (Serial2) |
| D30 | PA0 | - | - | **LED_BUILTIN_TX** (driven by USB-CDC activity) |
| D31 | PA1 | - | - | **LED_BUILTIN_RX** (driven by USB-CDC activity) |
| A0 | PF0 | D14 | AIN16 | **IN0** (CustomLogic1) |
| A1 | PF1 | D15 | AIN17 | **IN1** (CustomLogic1) |
| A2 | PF2 | D16 | AIN18 | **IN2** (CustomLogic1) / EVOUTF |
| A3 | PF3 | D17 | AIN19 | **OUT** (CustomLogic1) |
| A4 | PA2 | D18 | AIN22 | **SDA** (I2C) |
| A5 | PA3 | D19 | AIN23 | **SCL** (I2C) |

>  
> **PWM on D3 / D4 / D7 is mutually exclusive.**  
> A single TCB1 waveform is routed to one of these pins, so the pin most recently written with `analogWrite()` becomes the output (default: D3).  
> Three different PWM signals cannot be produced at once; frequency and duty are shared by all three pins.  
> When TCB1 is taken by `tone()` or similar, PWM is disabled on all three.  
>  
> **AREF can be used as GPIO D20 / A20 or as the SPI SS (client side).**  
> These functions are mutually exclusive.  
>  
> D30 and D31 have no physical pin.  
>  

---

### Serial ports

| Object | Hardware | Pins | Notes |
|--------|----------|------|-------|
| `Serial` | USB CDC | USB-C | Serial monitor (virtual COM) |
| `Serial1` | USART0 | D0 (RX) / D1 (TX) | Uno R3 compatible hardware UART |
| `Serial2` | USART1 | AREF (RX) / D13 (TX) | Additional UART |

>  
> Serial1 can be combined with XCK (D3) / XDIR (D2) for **RS-485 direction control or SPI host mode**.  
>  
> `Serial` is defined as an alias of `USBSerial` and uses USB-CDC.  
>  

---

### SPI

| Object | SPI | SPI1 |
| Signal | Pin (client capable) | Pin (host only) |
|--------|------|------|
| MOSI | D11 | D0 |
| MISO | D12 | D1 |
| SCK | D13 | D3 |
| SS | AREF | None |

>  
> **Client (receiver) operation:** Because the hardware SS (AREF) is on a real pin,  
> the bundled **SPISlave library** (ESP8266-compatible API) lets the board act as an SPI client.  
> While it does, AREF becomes the SS input and is mutually exclusive with the external reference (`analogReference(EXTERNAL)`), GPIO D20/A20, and Serial2.  
>  
> See [libraries/SPISlave](../libraries/SPISlave/README.md) for details.  
>  
>  

---

### I2C (Wire)

| Signal | Pin |
|--------|-----|
| SDA | A4 |
| SCL | A5 |

>  
> Located on A4/A5, the same as the Uno R3. **This differs from the Leonardo.**  
> A plain `Wire.begin()` works as-is.  
>  

---

### PWM (`analogWrite()`)

- **D5 / D6 / D9 / D10 / D11 / D12** - TCA0
- **D3 / D4 / D7** - The 8-bit PWM waveform of TCB1, output directly or via a LUT (mutually exclusive)
- Compared with the Uno R3, PWM has been added on D12.

>  
> **Exclusive PWM:** While TCB1 is in use for something else, `analogWrite(D3)` (or D4, D7) gives up PWM and falls back to a plain HIGH/LOW output (threshold 127).  
> - `tone()` uses TCB1, so PWM on D3, D4, and D7 stops while it is running.  
> This corresponds to Timer2 on the Uno R3 (D3 and D11 stop while `tone()` is running).  
>  

---

### Analog input

- 10-bit ADC
- **A0–A5** on the Uno R3 header (= D14–D19)
- Every digital pin also has an ADC channel and can be referenced as A6–A20

> 
> There are many input channels but only one ADC, so calling `analogRead` on many channels in quick succession degrades stability (same behavior as the megaAVR).
> 

---

### Clock output (CLKOUT)

- The main clock (CLK_PER) can be output on **D2**. Useful for clocking external ICs, synchronizing with another MCU, or measuring the actual clock.
- The bundled **ClockOut library** turns it on and off with `ClockOut.begin()` / `ClockOut.end()` (see [libraries/ClockOut](../libraries/ClockOut/README.md)).

>  
> A continuous 24 MHz square wave is an EMI source; enable it only for as long as needed.  
> D2 is shared with the AC0 output and EVOUTA, so `begin()` returns `false` while either of those is in use.  
>  

---

## Clock

- Tsurugi has **no crystal**; the system clock is generated from the internal OSCHF (default **24 MHz**; see the options below).
- The 48 MHz USB clock (CLK_USB) is generated by the internal PLL48M and is automatically trimmed against USB SOF, so USB works to specification without a crystal.

>  
> When the USB host is disconnected, clock accuracy falls back to that of the internal oscillator alone.  
>  

---

### Clock speed options

Two settings are available under **Tools > Clock Speed** in the Arduino IDE.

| Menu | F_CPU | Typical use |
|------|-------|-------------|
| 24 MHz internal (default) | 24 MHz | Normal use |
| 16 MHz internal | 16 MHz | Timing compatibility with classic AVR (16 MHz), lower power |

>  
> PWM frequency and timing functions such as `delayMicroseconds()` follow F_CPU.  
> `millis()` / `micros()` work correctly with either option.  
>  

---

## Power

Tsurugi has **two power inputs**, either of which can supply the 5 V rail.

- **USB-C (5 V):** Protected against reverse current by an ideal diode; supplies 5 V without damaging the host.
- **DC jack (up to 24 V):** Fed through a 5.5/2.1 mm DC jack, protected against reverse polarity by a Schottky diode, then a **synchronous buck converter** generates 5 V DC at 600 mA.
- **3.3 V (for shield pins):** On-board LDO.

>  
> On paper the +5 V supply capability is below the Uno R3 (5 V, 1 A), but when powered from 9 V or more the Uno R3 / Leonardo lose capacity to heat.  
> Tsurugi's buck converter delivers a stable 600 mA at any input voltage, so above 9 V the situation reverses in Tsurugi's favor.  
> Around 7 V it stays below the Uno R3's capacity.  
>  

---

## LEDs and switches

| Part | Color | Connection | Purpose |
|------|-------|------------|---------|
| Power LED | Green | Power rail | Power indicator |
| LED_BUILTIN | White | D13 (active-HIGH) | User LED (untouched by the core) |
| LED_BUILTIN_TX | Red | D30 (active-LOW) | USB-CDC transmit activity |
| LED_BUILTIN_RX | Red | D31 (active-LOW) | USB-CDC receive activity |
| Reset | RESET | Tactile switch |

>  
> The RX / TX LEDs flash with a ~100 ms pulse on CDC traffic and coexist with `digitalWrite(D30, ...)` from the sketch.  
>  
> The `TXLED1`/`TXLED0`/`RXLED1`/`RXLED0` macros from Pro Micro sketches (1 = on, 0 = off) are also defined.  
> As on the 32U4 core, the ~100 ms activity pulse from the variant overrides the pin during USB-CDC traffic.  
>  
> LED_BUILTIN (D13) is shared with SPI SCK, so, as on the Uno R3, the LED flickers with SCK traffic while SPI is in use.  
>  

---

## Uploading

1. Connect the board over USB.
2. Upload the sketch from the Arduino IDE. A **1200 bps touch** is performed at the start of the upload and the board enters the USB CDC bootloader automatically.
3. If it does not enter the bootloader automatically, **double-tap the reset button**.

For the first flash, or to rewrite the USB bootloader, connect a UPDI programmer (PICkit 4/5, Atmel-ICE, jtag2updi, etc.) to the UPDI pin.

<sub>The development VID/PID is from the pid.codes test range (application `0x1209:0x0008` / bootloader `0x1209:0x0007`).</sub>

---

## Board / MCU identification macros

| Macro | Purpose |
|-------|---------|
| `ARDUINO_AVR_TSURUGI` | Board identification |
| `__AVR_AVR64DU32__` | MCU identification |
| `__AVR_DU__` | Product group `"DU"` identification |

---

## Software compatibility (Arduino Uno R3)

- Tsurugi aims to minimize the effort of porting from the Uno R3 / Leonardo.  
- The instruction set is essentially the same as the classic megaAVR.  
- Compared with the Uno R3, `Serial` is pure USB-CDC.  
  D0 / D1 are available as `Serial1` (as on the Leonardo).  

>  
> The register layout has changed substantially, so porting code that manipulates registers directly is considerably harder.  
>  

---

## Main components

>  
> The final BOM and schematic for Tsurugi are in preparation and will be added to this page once fixed.  
>  

---

## Official documentation

- AVR64DU32 product page: <https://www.microchip.com/en-us/product/AVR64DU32>
- Datasheet: DS40002548B (AVR64DU32)
