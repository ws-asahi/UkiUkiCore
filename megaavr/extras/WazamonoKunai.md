# Wazamono Kunai (苦無)

**English** | [日本語](WazamonoKunai.ja.md)

**Seeeduino XIAO compatible — AVR32DU20 / USB-C**

Wazamono Kunai is a redesign of the ultra-compact Seeeduino XIAO form factor around the `AVR32DU20` AVR.
It is less powerful than the SAMD21 in the Seeeduino XIAO, but the AVR microcontroller brings switchable 5 V / 3.3 V operation and high-drive GPIO.
  
This page is the documentation for Wazamono Kunai. For an overview of the core as a whole, see the [README](../../README.md).  
**Status: prototype.** Pin definitions and the bootloader may change. The final BOM and schematic are in preparation.  

---

## Overview

| Item | Detail |
|------|--------|
| MCU | AVR32DU20 (20-pin) |
| Form factor | Seeeduino XIAO compatible |
| USB | USB-C (USB 2.0 Full-Speed, built into the MCU) |
| Clock | 24 MHz internal oscillator |
| Power | USB 5 V / VIN input 4–6 V (operating voltage selectable between 5 V and 3.3 V with JP1) |
| Upload | USB CDC bootloader (STK500v1) |

---

## Board specifications (AVR32DU20)

| Item | Value |
|------|-------|
| Flash | 32 KB (28 KB for sketches / 4 KB USB bootloader) |
| SRAM | 4 KB |
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
| CCL (LUT) | 3 |
| Event system | 4 channels |
| Analog comparator (AC) | 1 |

<sub>Figures are from the AVR16/32DU family datasheet (DS40002576).</sub>

---

## Comparison with the SAMD21 (Seeeduino XIAO)

The Seeeduino XIAO that Wazamono Kunai replaces uses the **ATSAMD21G18**.  
The AVR32DU20 is an **8-bit AVRxt core** and does not match the SAMD21 in compute performance or memory,  
but it differentiates itself with **native 5 V operation**, **high output current**, and the ease of use that comes with AVR peripherals.  
It can also **switch between 5 V and 3.3 V operation on the same board**.  

| Item | Wazamono Kunai (AVR32DU20) | Seeeduino XIAO (SAMD21G18) |
|------|----------------------------|----------------------------|
| Core | 8-bit AVRxt | 32-bit ARM Cortex-M0+ |
| Max. clock | 24 MHz | 48 MHz |
| Operating voltage | 5 V / 3.3 V | 3.3 V only |
| Flash | 32 KB | 256 KB |
| SRAM | 4 KB | 32 KB |
| EEPROM | 256 B | None (Flash emulation) |
| ADC | 10-bit | 12-bit |
| DAC | None | 10-bit × 1 |
| USB | Full-Speed device (built in) | Full-Speed device (built in) |
| USART | 2 | SERCOM × 6 (shared) |
| SPI | 2 (one host only) | SERCOM × 6 (shared) |
| I2C | 1 | SERCOM × 6 (shared) |
| External interrupts | All pins | Almost all pins |
| CCL (LUT) | 3 | None |
| Event system | 4 ch | None |
| Analog comparator (AC) | 1 | None |

---

### Main performance advantages

- **AVR / Arduino-AVR ecosystem** — The wealth of libraries and examples for AVR microcontrollers work as-is or with minor changes.
- **New-generation peripherals** — CCL (3 logic blocks), AC (1 analog comparator), the event system (4 channels), and more allow hardware signal processing without the CPU.
- **Drive capability per pin** — The robust AVR I/O can source/sink in the 20 mA class; the Seeeduino XIAO manages about 7 mA.
- **Additional UART** — Two hardware UARTs are available.
- **RS-422/485** — RS-485 communication is possible using the USART (an external transceiver chip is required).
- **Multiple operating voltages** — Thanks to the AVRxt core, 5 V or 3.3 V can be selected.

### Points to note

- **The SAMD21 has more memory and compute performance** (several times faster in simple arithmetic, 8× the memory).
- **The ADC is 10-bit** (the SAMD21 has a 12-bit ADC), and some pins have no ADC.
- **PWM is 8-bit on up to 7 pins, and there is no DAC** (the SAMD21 has PWM on every pin plus a 10-bit DAC on D0).
- **No dedicated LED_BUILTIN** (dropped due to the pin budget). The TX / RX LEDs can be driven as user LEDs.

---

## Data storage areas

The AVR32DU20 has several non-volatile memory areas for different purposes.    
However, they fall well short of the SAMD21 in capacity and endurance.  
This board is not suited to storing large amounts of data.  

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
| Kunai (5.0 V) | AVR32DU20 | 24.0 | (not tested) | (not tested) |
| Kunai (3.3 V) | AVR32DU20 | 24.0 | (not tested) | (not tested) |
| Seeeduino XIAO | SAMD21G18 | 48.0 | 46547.108 | 51.73 |

---

## Pin mapping

The numbering matches the Seeeduino XIAO.  
Unlike the SAMD21, however, some GPIOs lack PWM or ADC.  
No PWM: D1, D2, D3, D8  
No ADC: D6, D7  
Due to hardware constraints there is also no independent LED_BUILTIN.  

| Pin | MCU | Alias | ADC ch | Main functions |
|-----|-----|-------|--------|----------------|
| D0 | PC3 | A0 | AIN31 | ~PWM (TCB1 + LUT1) |
| D1 | PA7 | A1 | AIN27 | **SS** (SPI) / **OUT** (AnalogComp) / EVOUTA / CLKOUT |
| D2 | PD6 | A2 | AIN6 | **TX** (Serial2) / **P** (AnalogComp) |
| D3 | PD7 | A3 | AIN7 | **RX** (Serial2) / **AREF** / **N** (AnalogComp) / EVOUTD |
| D4 | PA2 | A4 | AIN22 | **I2C SDA** / ~PWM (TCA0 WO2) / **XCK** (Serial1) / **CLK** (SPI1) / **IN2** (CustomLogic) |
| D5 | PA3 | A5 | AIN23 | **I2C SCL** / ~PWM (TCA0 WO3) / **XDIR** (Serial1) / **OUT** (CustomLogic) |
| D6 | PA0 | — | — | **TX** (Serial1) / ~PWM (TCA0 WO0) / **MISO** (SPI1) / **IN0** (CustomLogic) |
| D7 | PA1 | — | — | **RX** (Serial1) / ~PWM (TCA0 WO1) / **MOSI** (SPI1) / **IN1** (CustomLogic) |
| D8 | PA6 | A8 | AIN26 | **SCK** (SPI) |
| D9 | PA5 | A9 | AIN25 | ~PWM (TCA0 WO5) / **MISO** (SPI) |
| D10 | PA4 | A10 | AIN24 | ~PWM (TCA0 WO4) / **MOSI** (SPI)  |
| D11 | PD4 | A11 | AIN4 | **LED_BUILTIN** / **LED_BUILTIN_TX** (driven by USB-CDC activity) |
| D12 | PD5 | A12 | AIN5 | **LED_BUILTIN_RX** (driven by USB-CDC activity) |

> 
> **PWM on D0 comes from TCB1** (routed through CCL LUT1), the only TCB PWM output on Kunai.  
> When TCB1 is taken by `tone()` or similar, PWM on D0 is suspended.  
>
> **D3 can be used as AREF.**  
> While it serves as AREF it cannot be used for anything else.  
>
> D11 and D12 have no physical pin.  
> D13 is not implemented due to the pin budget.  
> 

---

### Serial ports

| Object | Hardware | Pins | Notes |
|--------|----------|------|-------|
| `Serial` | USB CDC | USB-C | Serial monitor (virtual COM) |
| `Serial1` | USART0 | D7 (RX) / D6 (TX) | Seeeduino XIAO compatible hardware UART |
| `Serial2` | USART1 | D3 (RX) / D2 (TX) | Additional UART |

> 
> Serial1 can be combined with XCK (D4) / XDIR (D5) for **RS-485 direction control or SPI host mode**.  
>  
> `Serial` is defined as an alias of `USBSerial` and uses USB-CDC.  
> 

---

### SPI

| Object | SPI | SPI1 |
| Signal | Pin (client capable) | Pin (host only) |
|--------|------|------|
| MOSI | D10 | D7 |
| MISO | D9 | D6 |
| SCK | D8 | D4 |
| SS | D1 | None |

>  
> **Client (receiver) operation:** Because the hardware SS (D1) is on a real pin,  
> the bundled **SPISlave library** (ESP8266-compatible API) lets the board act as an SPI client.  
> While it does, D1 becomes the SS input and is mutually exclusive with the AC0 output, EVOUTA, and CLKOUT (ClockOut).
>  
> See [libraries/SPISlave](../libraries/SPISlave/README.md) for details.  
>  

### I2C (Wire)

| Signal | Pin |
|--------|-----|
| SDA | D4 |
| SCL | D5 |

> 
> Located on D4/D5, the same as the Seeeduino XIAO.  
> A plain `Wire.begin()` works as-is.  
> 

### PWM (`analogWrite()`)

- **D4, D5, D6, D7, D9, D10** - TCA0
- **D0** - The 8-bit PWM waveform of TCB1, output via CCL LUT1

>  
> **TCB1 PWM:** While TCB1 is in use for something else, `analogWrite(D0)` gives up PWM and falls back to a plain HIGH/LOW output (threshold 127).  
> - `tone()` uses TCB1, so PWM on D0 stops while it is running.  
>  
> The Seeeduino XIAO can output PWM on every GPIO; Kunai has some restrictions on PWM output.
>  

### Analog input

- 10-bit ADC
- Pads **A0–A10** (A6/A7 do not exist)

> Due to hardware constraints, D6 / D7 have no analog input.

---

### Clock output (CLKOUT)

- The main clock (CLK_PER) can be output on **D1**. Useful for clocking external ICs, synchronizing with another MCU, or measuring the actual clock.
- The bundled **ClockOut library** turns it on and off with `ClockOut.begin()` / `ClockOut.end()` (see [libraries/ClockOut](../libraries/ClockOut/README.md)).

>  
> A continuous 24 MHz square wave is an EMI source; enable it only for as long as needed.  
> D1 is shared with the AC0 output and EVOUTA, so `begin()` returns `false` while either of those is in use.  
>  

---

## Clock

- Kunai has **no crystal**; the system clock is generated from the internal OSCHF (default **24 MHz**; see the options below).
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
- **VIN input (5 V):** The combination of the input voltage and the JP1 solder setting determines the operating voltage.
- **Voltage selection:** Jumper pad **JP1** selects VCC as 5 V or 3.3 V.  

> 
> To select the voltage, bridge the JP1 pad on the back to the desired voltage side with solder.  
> 
> Using XIAO-specific accessories with the 5 V setting is very likely to damage them.  
> Take care with the combination of the JP1 setting and the attached peripherals.  
> 

---

## LEDs and switches

| Part | Color | Connection | Purpose |
|------|-------|------------|---------|
| Power LED | Green | Power rail | Power indicator |
| LED_BUILTIN_TX | Red | D11 (active-LOW) | USB-CDC transmit activity |
| LED_BUILTIN_RX | Red | D12 (active-LOW) | USB-CDC receive activity |
| LED_BUILTIN | - | D11 (active-LOW) | No dedicated LED; mapped to the TX LED for compatibility |
| Reset | RESET | Tactile switch |

> 
> The RX / TX LEDs flash with a ~100 ms pulse on CDC traffic and coexist with `digitalWrite(D11, ...)` from the sketch.  
>  
> The `TXLED1`/`TXLED0`/`RXLED1`/`RXLED0` macros from Pro Micro sketches (1 = on, 0 = off) are also defined.  
> The ~100 ms activity pulse from the variant overrides the pin during USB-CDC traffic.  
>  
> Due to hardware constraints, LED_BUILTIN (D13) is not implemented.  
> For compatibility, LED_BUILTIN is mapped to D11.
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
| `ARDUINO_AVR_KUNAI` | Board identification |
| `__AVR_AVR32DU20__` | MCU identification |
| `__AVR_DU__` | Product group `"DU"` identification |

---

## Software / hardware compatibility (Seeeduino XIAO)

- Kunai uses a new 8-bit AVR microcontroller, so there are many situations where it is not compatible with the SAMD21.
- Development in the Arduino IDE works in general, but anything that touches registers directly loses compatibility.
- On the other hand, it is highly compatible with the classic megaAVR and with the new AVRs, including Tachi / Tsurugi.

The following differ from the XIAO.  

- **No LED_BUILTIN**: There is no independent LED; the TX LED serves in its place.
- **No SWCLK / SWDIO**: The pads behind the USB connector are not SWCLK / SWDIO but UPDI and HV RESET (pads for UPDI v2).
- **5 V operation possible**: When running at 5 V, the XIAO's 3.3 V supply pin carries 5 V, which can destroy peripherals rated for 3.3 V.
- **PWM on up to 7 pins**: D1, D2, D3, and D8 have no PWM. D0 uses TCB1 (shared with `tone()`).
- **No ADC on some pins**: A6 and A7 have no ADC.
- **Different variable sizes**: `int` is 2 bytes instead of 4. `double` is 4 bytes instead of 8 (`long double` is 8 bytes, matching). 

---

## Main components

> The final BOM and schematic for Kunai are in preparation and will be added to this page once fixed.

---

## Official documentation

- AVR32DU20 product page: <https://www.microchip.com/en-us/product/AVR32DU20>
- Datasheet: DS40002576B (AVR16/32DU family)
