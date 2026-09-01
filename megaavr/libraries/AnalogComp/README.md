# AnalogComp — Analog comparator library for Wazamono

A WazamonoCore-specific library that makes the analog comparator (AC0) built into the AVR DU series
as easy to use as Serial or SD. The instance `AnalogComp` is predefined, so you can start with
just `begin()`.

```cpp
#include <AnalogComp.h>

void setup() {
  AnalogComp.begin();                    // Compare the + pin with the − pin
  // AnalogComp.begin(INTERNAL2V5);      // Compare the + pin with the internal 2.5 V reference
  // AnalogComp.begin(VDD, 128);         // Compare the + pin with 1/2 of VDD
}

void loop() {
  if (AnalogComp.read()) {     // true when + > −
    ...
  }
}
```

## Input/output pins (fixed in hardware)

| | + input | − input | Output (`enableOutput()`) |
|---|---|---|---|
| **Tachi** | D9 (PD2) | D10 (PD3) | D8 (PA7) |
| **Tsurugi** | D9 (PD2) | D10 (PD3) | D2 (PA7) |
| **Kunai** | D2 (PD6) | D3 (PD7) | D1 (PA7) |

`setInputs(plus, minus)` switches to other supported pins
(+: PD2/PD6, −: PD3/PD0/PD7, whichever exist on the board).

## API

| Method | Description |
|---|---|
| `begin()` | Start comparing the + pin with the − pin |
| `begin(ref)` / `begin(ref, level)` | Start comparing the + pin with a reference voltage. `ref` takes the same constants as `analogReference()` (`INTERNAL1V024/2V048/2V5/4V096`, `VDD`, `EXTERNAL`). Threshold = Vref × level ÷ 256 (level defaults to 255 ≈ Vref itself) |
| `read()` | Comparison result. `true` when + > − |
| `setThreshold(ref, level)` | Change the reference voltage and threshold (switches the − input to the reference side) |
| `setInputs(plus, minus)` | Switch the input pins (`false` for unsupported pins) |
| `setHysteresis(level)` | `AC_HYST_NONE/SMALL/MEDIUM/LARGE` (about 0/10/25/50 mV) |
| `enableOutput(invert)` / `disableOutput()` | Drive the comparison result on PA7 |
| `attachInterrupt(fn, mode)` | Call a function on `RISING/FALLING/CHANGE` |
| `detachInterrupt()` | Remove the interrupt |
| `end()` | Stop and release the pins |

## Examples

- **ReadState** — Print the pin-vs-pin comparison result to serial
- **Threshold** — Show the comparison against the internal 2.5 V reference on an LED
- **OutputPin** — Drive the comparison result on PA7 in hardware (no sketch involvement)
- **EdgeInterrupt** — Interrupt on threshold crossing

## Provenance

This library is an independent implementation based solely on the AVR64DU28/32 datasheet (DS40002548A)
and the official Microchip device headers. No code from existing comparator libraries, including the
Comparator library of DxCore/megaTinyCore, has been used or consulted.

## Notes

- Keep input voltages within GND to VDD.
- With `EXTERNAL`, supply the reference voltage on the VREFA pin (PD7).
- For noisy signals, combining with `setHysteresis()` is recommended (especially when using interrupts).
- To preserve the accuracy of the internal reference, keep the threshold at least 0.5 V below VDD.
