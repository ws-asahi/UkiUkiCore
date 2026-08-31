# Wazamono Tachi (太刀)

**English** | [日本語](WazamonoTachi.ja.md)

**Pro Micro compatible — AVR64DU32 / USB-C**

Wazamono Tachi is a redesign of the SparkFun Pro Micro form factor around the `AVR64DU32`, an AVR with native USB.  
There is no USB-to-serial converter chip; the microcontroller itself connects directly to the PC over USB-C.  
  
This page is the documentation for Wazamono Tachi. For an overview of the core as a whole, see the [README](../../README.md).  
**Status: prototype.** Pin definitions and the bootloader may change. The final BOM and schematic are in preparation.  

---

## Overview

| Item | Detail |
|------|--------|
| MCU | AVR64DU32 |
| Form factor | SparkFun Pro Micro compatible |
| USB | USB-C (USB 2.0 Full-Speed, built into the MCU) |
| Clock | 24 MHz internal oscillator |
| Power | USB 5 V / VIN input 6.5–12 V (operating voltage selectable between 5 V and 3.3 V with JP1) |
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
| Timers | TCA0 × 1 / TCB × 2 |
| USART | 2 |
| SPI | 2 |
| I2C | 1 |
| External interrupts | All pins |
| CCL (LUT) | 4 |
| Event system | 6 channels |
| Analog comparator (AC) | 1 |

<sub>Memory specifications and endurance figures are from datasheet DS40002548A (§8 Memories / §11 NVMCTRL / Electrical Characteristics).</sub>

---

## Comparison with the ATmega32U4 (Pro Micro)

The Pro Micro that Wazamono Tachi replaces uses the **ATmega32U4**.  
The Pro Micro is also a USB-native AVR, but the AVR64DU32 is a new-generation **AVRxt core**  
with substantially stronger clock, memory, and peripherals.  
It can also **switch between 5 V and 3.3 V operation on the same board**.   

| Item | Wazamono Tachi (AVR64DU32) | Pro Micro (ATmega32U4) |
|------|----------------------------|------------------------|
| Core | AVRxt (improved instruction timing) | Classic AVR |
| Max. clock | 24 MHz (over the full 1.8–5.5 V range) | 16 MHz (≥ 4.5 V) / typically 8 MHz at 3.3 V |
| Operating voltage | 5 V / 3.3 V (no component change) | 5 V / 3.3 V (component change required) |
| Flash | 64 KB | 32 KB |
| SRAM | 8 KB | 2.5 KB |
| EEPROM | 256 B | 1 KB |
| USERROW | 512 B | - |
| ADC | 10-bit 170 ksps, 12 ch | 10-bit 9.6 ksps, 12 ch |
| Timers | 16-bit × 1 + 8-bit × 2 | 16-bit × 2 + 8-bit × 1 + 10-bit × 1 |
| USART | 2 | 1 |
| SPI | 2 (one can act as client) | 1 (no client mode) |
| I2C | 1 | 1 |
| External interrupts | All pins | 4 | 
| CCL (LUT) | 4 | None |
| Event system | 6 ch | None |
| Analog comparator (AC) | 1 | None |

---

### Main performance advantages

- **Clock and processing speed** — 24 MHz operation (vs. 16 MHz on the ATmega32U4), and  
  the AVRxt core has improved timing on some instructions, giving roughly 12% higher benchmark scores at the same clock.  
- **Memory** — 2× the Flash (64 KB), about 3.2× the SRAM (8 KB). More headroom for large buffers, USB composite devices, and library-heavy sketches.  
- **New-generation peripherals** — CCL (4 logic blocks) and the event system (6 channels) allow hardware signal processing without the CPU.  
- **Analog input** — ADC channels increase to 21, so every GPIO supports analog input.  
- **Drive capability per pin** — The robust AVR I/O can source/sink in the 20 mA class.  
- **Additional UART** — Two hardware UARTs are available.  
- **RS-422/485** — RS-485 communication is possible using the USART (an external transceiver chip is required).  
- **Multiple operating voltages** — Thanks to the AVRxt core, 5 V or 3.3 V can be selected **without changing any peripheral components**.  

---

### Points to note

- **The ATmega32U4 has more EEPROM** (1 KB vs. 256 B).
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
| Tachi (5.0 V) | AVR64DU32 | 24.0 | 28931.556 | 14.18 |
| Tachi (3.3 V) | AVR64DU32 | 24.0 | 28931.850 | 14.18 |
| Pro Micro (5.0 V) | ATmega32U4 | 16.0 | 17208.020 | - (did not run) |
| Pro Micro (3.3 V) | ATmega32U4 | 8.0 | (not tested) | - (did not run) |

---

## Pin mapping

The numbering matches the SparkFun Pro Micro.  
Every external pin is connected to the ADC.  
Basic pin numbers are identical to the Pro Micro; pins that originally lacked an ADC channel have been given new analog pin numbers.  

| Pin | MCU | Alias | ADC ch | Main functions |
|-----|-----|-------|--------|----------------|
| D0 | PA5 | A11 | AIN25 | **RX** (Serial1) / **MOSI** (SPI1) |
| D1 | PA4 | A12 | AIN24 | **TX** (Serial1) / **MISO** (SPI1) |
| D2 | PA2 | A13 | AIN22 | **SDA** (I2C) |
| D3 | PA3 | A14 | AIN23 | ~PWM (TCB1 WO) / **SCL** (I2C) |
| D4 | PC3 | A6 | AIN31 | ~PWM (TCB1 + LUT1) |
| D5 | PD0 | A15 | AIN0 | ~PWM (TCA0 WO0) / **IN0** (CustomLogic) |
| D6 | PD1 | A7 | AIN1 | ~PWM (TCA0 WO1) / **IN1** (CustomLogic) |
| D7 | PA6 | A16 | AIN26 | ~PWM (TCB1 + LUT0) / **XCK** (Serial1) / **CLK** (SPI1) |
| D8 | PA7 | A8 | AIN27 | **XDIR** (Serial1) / **OUT** (AnalogComp) / EVOUTA / CLKOUT |
| D9 | PD2 | A9 | AIN2 | ~PWM (TCA0 WO2) / **IN2** (CustomLogic) / **P** (AnalogComp) |
| D10 | PD3 | A10 | AIN3 | ~PWM (TCA0 WO3) / **OUT** (CustomLogic) / **N** (AnalogComp) |
| D14 | PD5 | A17 | AIN5 | ~PWM (TCA0 WO5) / **MISO** (SPI) |
| D15 | PD6 | A18 | AIN6 | **SCK** (SPI) / **TX** (Serial2) |
| D16 | PD4 | A19 | AIN4 | ~PWM (TCA0 WO4) / SPI **MOSI** |
| D17 | PF3 | A20 | AIN19 | **LED_BUILTIN** (on-board user LED) / **OUT** (CustomLogic1) |
| D30 | PA0 | - | - | **LED_BUILTIN_TX** (driven by USB-CDC activity) |
| D31 | PA1 | - | - | **LED_BUILTIN_RX** (driven by USB-CDC activity) |
| A0 | PD7 | D18 | AIN7 | **SS** (SPI) / **RX** (Serial2) / **AREF** |
| A1 | PF0 | D19 | AIN16 | **IN0** (CustomLogic1) |
| A2 | PF1 | D20 | AIN17 | **IN1** (CustomLogic1) |
| A3 | PF2 | D21 | AIN18 | **IN2** (CustomLogic1) / EVOUTF |
| A4 | PF4 | D22 | AIN20 | Test pad TP1 (no header) |
| A5 | PF5 | D23 | AIN21 | Test pad TP2 (no header) |

> 
> **PWM on D3 / D4 / D7 is mutually exclusive.**  
> A single TCB1 waveform is routed to one of these pins, so the pin most recently written with `analogWrite()` becomes the output (default: D3).  
> Three different PWM signals cannot be produced at once; frequency and duty are shared by all three pins.  
> When TCB1 is taken by `tone()` or similar, PWM is disabled on all three.  
> 
> **A0 can be used as AREF or as the SPI SS (client side).**  
> These functions are mutually exclusive.  
> 
> D17 / D30 / D31 have no physical pin.
> A4 / A5 are not brought out to the header; they are only available as test pads on the back. 
> 

---

### Serial ports

| Object | Hardware | Pins | Notes |
|--------|----------|------|-------|
| `Serial` | USB CDC | USB-C | Serial monitor (virtual COM) |
| `Serial1` | USART0 | D0 (RX) / D1 (TX) | Pro Micro compatible hardware UART |
| `Serial2` | USART1 | A0 (RX) / D15 (TX) | Additional UART |

> 
> Serial1 can be combined with XCK (D7) / XDIR (D8) for **RS-485 direction control or SPI host mode**.  
>  
> `Serial` is defined as an alias of `USBSerial` and uses USB-CDC.  
> 

---

### SPI

| Object | SPI | SPI1 |
| Signal | Pin (client capable) | Pin (host only) |
|--------|------|------|
| MOSI | D16 | D0 |
| MISO | D14 | D1 |
| SCK | D15 | D7 |
| SS | A0 | None |

>  
> **Client (receiver) operation:** Because the hardware SS (A0) is on a real pin,  
> the bundled **SPISlave library** (ESP8266-compatible API) lets the board act as an SPI client.  
> While it does, A0 becomes the SS input and is mutually exclusive with the external reference (`analogReference(EXTERNAL)`) and Serial2. 
>  
> See [libraries/SPISlave](../libraries/SPISlave/README.md) for details.  
>  

---

### I2C (Wire)

| Signal | Pin |
|--------|-----|
| SDA | D2 |
| SCL | D3 |

> 
> Located on D2/D3, the same as the Pro Micro.  
> A plain `Wire.begin()` works as-is.  
> 

---

### PWM (`analogWrite()`)

- **D5 / D6 / D9 / D10 / D14 / D16** - TCA0
- **D3 / D4 / D7** - The 8-bit PWM waveform of TCB1, output directly or via a LUT (mutually exclusive)
- Compared with the Pro Micro, PWM has been added on D14 / D16.

> 
> **Exclusive PWM:** While TCB1 is in use for something else, `analogWrite(D3)` (or D4, D7) gives up PWM and falls back to a plain HIGH/LOW output (threshold 127).  
> `tone()` uses TCB1, so PWM on D3, D4, and D7 stops while it is running.  
> This corresponds to Timer3 on the Pro Micro (D5 stops while `tone()` is running).  
> 

---

### Analog input

- **A0–A3** as printed on the silkscreen (= D18–D21)
- **A4 / A5** (= D22 / D23) are test pads only
- Every digital pin also has an ADC channel and can be referenced as A6–A20 (contiguous, no gap at A11), except D30 / D31

---

### Clock output (CLKOUT)

- The main clock (CLK_PER) can be output on **D8**. Useful for clocking external ICs, synchronizing with another MCU, or measuring the actual clock.
- The bundled **ClockOut library** turns it on and off with `ClockOut.begin()` / `ClockOut.end()` (see [libraries/ClockOut](../libraries/ClockOut/README.md)).

>  
> A continuous 24 MHz square wave is an EMI source; enable it only for as long as needed.  
> D8 is shared with the AC0 output and EVOUTA, so `begin()` returns `false` while either of those is in use.  
>  

---

## Clock


- Tachi has **no crystal**; the system clock is generated from the internal OSCHF (default **24 MHz**; see the options below).
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

- **USB-C (5 V):** Protected against reverse current by an ideal diode, so the host is not damaged even when an external supply is connected at the same time.
- **VIN input (6.5–12 V):** An on-board high-voltage LDO generates 5 V.  
  Allowing for dropout, the **practical minimum input is about 6.5 V** and the **absolute maximum is 36 V**, but  
  the **recommended range is 6.5–12 V** on thermal grounds.  
- **Output capability:** The 5 V rail is rated **460 mA peak / 300 mA continuous**. The 3.3 V rail is roughly the same.  
  For reference, the LDO on the Pro Micro is rated 500 mA peak / 200 mA continuous.  
- **Voltage selection:** Jumper pad **JP1** selects VCC as 5 V or 3.3 V.  
  The AVR64DU32 runs at 24 MHz across the full 3.3–5 V range.  
- The Pro Micro (ATmega32U4) is forced to 8 MHz at 3.3 V; Tachi has no such restriction.

> 
> To select the voltage, bridge the JP1 pad on the back to the desired voltage side with solder.  
> 
> Tachi has no RAW pin; it is replaced by VIN.
> Because USB bus power and VCC leak back through the LDO, a voltage appears on VIN even when nothing is connected to it,
> but do not draw significant current from the VIN pin.
> 

---

## LEDs and switches

| Part | Color | Connection | Purpose |
|------|-------|------------|---------|
| Power LED | Green | Power rail | Power indicator |
| LED_BUILTIN | White | D17 (active-LOW) | User LED (untouched by the core) |
| LED_BUILTIN_TX | Red | D30 (active-LOW) | USB-CDC transmit activity |
| LED_BUILTIN_RX | Red | D31 (active-LOW) | USB-CDC receive activity |
| Reset | RESET | Tactile switch |

> 
> The RX / TX LEDs flash with a ~100 ms pulse on CDC transmit/receive and coexist with `digitalWrite(D30, ...)` / `digitalWrite(D31, ...)` from the sketch.  
>  
> The `TXLED1`/`TXLED0`/`RXLED1`/`RXLED0` macros from Pro Micro sketches (1 = on, 0 = off) are also defined.  
> As on the 32U4 core, the ~100 ms activity pulse from the variant overrides the pin during USB-CDC traffic.  
> 

---

## Uploading

1. Connect the board over USB.
2. Upload the sketch from the Arduino IDE. A **1200 bps touch** is performed at the start of the upload and the board enters the USB CDC bootloader automatically.
3. If it does not enter the bootloader automatically, **double-tap the reset pad with a jumper wire**.

For the first flash, or to rewrite the USB bootloader, connect a UPDI programmer (PICkit 4/5, Atmel-ICE, jtag2updi, etc.) to the UPDI pad.

<sub>The development VID/PID is from the pid.codes test range (application `0x1209:0x0006` / bootloader `0x1209:0x0005`).</sub>

---

## Board / MCU identification macros

| Macro | Purpose |
|-------|---------|
| `ARDUINO_AVR_TACHI` | Board identification |
| `__AVR_AVR64DU32__` | MCU identification |
| `__AVR_DU__` | Product group `"DU"` identification |

---

## Software compatibility (Pro Micro)

- Tachi aims to minimize the effort of porting from the Pro Micro.
- The instruction set is essentially the same as the classic megaAVR.

> 
> The register layout has changed substantially, so porting code that manipulates registers directly is considerably harder.
> 

---

## Main components

> 
> The final BOM and schematic for Tachi are in preparation and will be added to this page once fixed.
> 

---

## Official documentation

- AVR64DU32 product page: <https://www.microchip.com/en-us/product/AVR64DU32>
- Datasheet: DS40002548B (AVR64DU32)
