# WazamonoCore

**English** | [日本語](README.ja.md)

**The Arduino core for the Wazamono (業物) series**
A board support package (Arduino core) for the "Wazamono" family of Arduino-compatible boards built on the new-generation AVR DU series, which has native USB.

![platform](https://img.shields.io/badge/platform-AVR%20DU-blue)
![license](https://img.shields.io/badge/license-LGPL--2.1-green)
![version](https://img.shields.io/badge/core-v0.0.7-orange)
![based on](https://img.shields.io/badge/based%20on-DxCore-lightgrey)

  
The Wazamono series is a family of boards that aims to replace the classic Arduino-compatible form factors with a **new-generation AVR that has USB built in**.  
There is no separate USB-to-serial converter; the microcontroller itself connects directly to the PC.  
WazamonoCore is the dedicated core for developing these boards in the Arduino IDE.  
It is based on [DxCore](https://github.com/SpenceKonde/DxCore), **restructured to keep only what the Wazamono series needs**.  
 
⚠️ **This is a development release (0.0.7 series).** The API, board definitions, and bootloader may change without notice.  
 

---

## Supported boards

| Board | MCU | Form factor | Status |
|-------|-----|-------------|--------|
| [**Wazamono Tachi (太刀)**](megaavr/extras/WazamonoTachi.md) | AVR64DU32 | Pro Micro compatible / USB-C | 🔧 Prototype |
| [**Wazamono Tsurugi (剣)**](megaavr/extras/WazamonoTsurugi.md) | AVR64DU32 | Uno R3 compatible / USB-C | 🔧 Prototype |
| [**Wazamono Kunai (苦無)**](megaavr/extras/WazamonoKunai.md) | AVR32DU20 | XIAO compatible / USB-C | 🔧 Prototype |

>  
> This core ships variants for **Tachi**, **Tsurugi**, and **Kunai**.  
>  

---

## The heart of the series - AVR DU

Every board in the Wazamono series uses the "**AVR DU**", a new-generation AVR with USB built in.  
Its defining feature is that it talks to the PC directly, with no USB-to-serial converter chip.  
Tachi and Tsurugi carry the **AVR64DU32**; the compact Kunai carries the **AVR32DU20**.  

| Item | AVR64DU32 | AVR32DU20 |
|------|-----------|-----------|
| Flash | 64 KB | 32 KB |
| SRAM | 8 KB | 4 KB |
| EEPROM | 256 B | 256 B |
| USERROW | 512 B | 512 B |
| Operating clock | 24 MHz | 24 MHz |
| USB | USB 2.0 Full-Speed | USB 2.0 Full-Speed |
| ADC | 10-bit 170 ksps, 21 ch | 10-bit 170 ksps, 11 ch |
| USART | 2 | 2 |
| SPI | 2 (one can act as client) | 2 (one can act as client) |
| I2C | 1 | 1 |
| CCL (LUT) | 4 | 3 |
| Event system | 6 ch | 4 ch |
| Analog comparator (AC) | 1 | 1 |

<sub>Figures are taken from datasheets DS40002548A (AVR64DU28/32) and DS40002576 (AVR16/32DU family).</sub> 

---

## Features

- **Better baseline performance** - 1.5× the clock, faster instruction execution.
- **Revised non-volatile memory** - New storage areas such as USERROW and Flash are available (EEPROM, on the other hand, shrinks to 256 B).
- **Native USB** - No extra USB-to-serial chip. `Serial` is the USB virtual serial port as-is.
- **USB bootloader** - Sketches are uploaded over USB-CDC (STK500v1).
- **HID / MIDI** - USB HID (keyboard, mouse, etc.) and USB-MIDI are supported.
- **High compatibility** - Because the MCU belongs to the same family, code for the Uno R3 and Pro Micro runs mostly unchanged.
- **Strong pin drivers** - Each pin keeps the 20 mA drive capability, so peripherals that cannot be driven by the Uno R4's 8 mA still work.
- **Analog input on many pins** - Every digital I/O pin can read analog values (Kunai lacks it on a few pins).
- **UPDI** - The running MCU can be accessed with a UPDI debugger or similar tool.
- **Native avr-gcc** - Unlike DxCore, the latest avr-gcc compiler is used (and will be updated going forward).
- **64-bit floating point** - Larger program size, but 64-bit floating point is available (implemented as `long double`).
- **Power management built mainly on Japanese parts** - The power-control ICs are unified on Torex products.
- **Ideal diode on the USB power path** - When running on USB bus power, the board supplies 5 V DC with almost no voltage loss.

---

## Installation

### Via Board Manager (recommended)

1. In the Arduino IDE, add the following to **File > Preferences > Additional boards manager URLs**:

   ```
   https://wazamono.ws-asahi.net/package_wazamono_index.json
   ```

2. Open **Tools > Board > Boards Manager**, search for "**Wazamono**", and install
   **Wazamono Boards (AVR DU series)**.

3. Along with the core itself, the dedicated toolchain (avr-gcc 15.2.0 / avrdude 8.1)
   is downloaded and configured automatically. No further setup is required.

See [Installation.md](Installation.md) for detailed steps and manual installation (for developers).

---

### Requirements

- Arduino IDE 1.8.13 or later, or 2.x
- Reflashing the bootloader requires a UPDI programmer (PICkit 4/5, Atmel-ICE, jtag2updi, etc.).

>  
> On Linux, always use the Arduino IDE distributed by [arduino.cc](https://www.arduino.cc).  
> The versions in distribution package managers are modified and do not work correctly.  
>  

---

## Quick start

1. Choose your board under **Tools > Board > WazamonoCore**
2. Connect with a USB cable and upload

**Blink:**
```cpp
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}
void loop() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(500);
}
```

**USB serial:**
```cpp
void setup() {
  Serial.begin(115200);
}
void loop() {
  Serial.println(millis());
  delay(1000);
}
```

---

## Uploading (via the bootloader)

1. Connect the board over USB.
2. Upload the sketch from the Arduino IDE. A **1200 bps touch** is performed at the start of the upload and the board enters the bootloader automatically.
3. If it does not enter the bootloader automatically, **double-tap the reset button**.

---

## License and credits

>  
> WazamonoCore is a **product-specific fork** derived from [DxCore](https://github.com/SpenceKonde/DxCore) (© Spence Konde, LGPL 2.1).  
> This core is also distributed under **LGPL 2.1**.  
>  

- Base core: **DxCore** - © Spence Konde 2021–2022, and the respective Arduino cores
- Wazamono customizations, USB stack, and board definitions: © Workshop Asahi 2026
- "Wazamono (業物)", "Tachi (太刀)", "Tsurugi (剣)", and "Kunai (苦無)" are product names of Workshop Asahi

>  
> See [LICENSE.md](LICENSE.md) for the full license text.  
> Some files and libraries may be provided under a different license; where so, this is stated at the top of the file.  
>  
