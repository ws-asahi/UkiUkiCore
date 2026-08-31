# ClockOut — Main clock output (CLKOUT) library

A library that **outputs CLK_PER (the peripheral/CPU clock) on the CLKOUT pin** of a Wazamono board. As with `Serial`, `begin()` starts the output and `end()` stops it.

```cpp
#include <ClockOut.h>

void setup() {
  ClockOut.begin();                        // Start outputting CLK_PER
  Serial.println(ClockOut.frequency());    // 24000000 (when running at 24 MHz)
}
```

## CLKOUT pin

CLKOUT is **fixed to PA7** and has no alternate position.

| Board | CLKOUT pin |
|-------|------------|
| Wazamono Tachi | **D8** |
| Wazamono Tsurugi | **D2** |
| Wazamono Kunai | **D1** |

## Typical uses

- **Clocking external ICs** — Distribute the system clock to a codec / ADC / logic IC and omit a second crystal or oscillator
- **Synchronizing with another MCU** — Feed it into the other device's external clock input (XTALHF1) for synchronous operation
- **Measuring the actual clock** — Check the system clock with a scope or frequency counter. This is especially useful on **Kunai, which has no crystal**, since it auto-tunes OSCHF against the USB frame signal and CLKOUT is a practical way to confirm that the tuning is working
- **Production test** — A test point for comparing the internal oscillator's accuracy against a reference

## API

| Method | Description |
|--------|-------------|
| `bool begin()` | Start CLKOUT output. If PA7 is taken by another peripheral, **changes nothing and returns `false`** |
| `void end()` | Stop the output. Returns PA7 to input |
| `bool isRunning()` | Whether output is actually active. Reads the CLKOUT bit in the register directly, so **it also detects the automatic hardware stop caused by CFD (Clock Failure Detection)** |
| `uint32_t frequency()` | Output frequency [Hz] (= CLK_PER = `F_CPU`) |
| `uint8_t pin()` | Arduino pin number of CLKOUT (Tachi = 8 / Tsurugi = 2 / Kunai = 1) |

## Why there is no divider argument

What CLKOUT produces is **CLK_PER itself**. The only divider in this path is the CLK_MAIN prescaler (`CLKCTRL.MCLKCTRLB`), and it **changes the CPU clock at the same time** (`millis()`, USB, and UART baud rates would all shift).

The library therefore provides no divider argument. **To output an arbitrary frequency on a pin, use TCA/TCB PWM** (`analogWrite()` / `tone()`) **or the CCL** instead of CLKOUT.

## PA7 conflicts

Several functions converge on PA7.

| Function | Detection | Notes |
|----------|-----------|-------|
| AC0 comparator output | **`begin()` refuses** | Checks OUTEN in `AC0.CTRLA` |
| Event output EVOUTA | **`begin()` refuses** | Checks the EVSYS user assignment and PORTMUX position |
| SPI SS (**Kunai only**) | **`begin()` refuses** | Refuses if SPI0 is enabled (including while `SPISlave` is in use) |
| USART0 XDIR (ALT1 position) | Not detected | The DIR line for RS-485. Manage the exclusion yourself when using it |
| Ordinary GPIO use by the sketch | Cannot be detected | Get the pin number with `pin()` and manage it yourself |

## EMI and current consumption

A continuous 24 MHz square wave is a **strong EMI source**, and the pin driver's current consumption increases. The following is recommended in products.

- Keep the trace as short as possible. Long traces, or leaving the output running on a header, are unfavorable from a VCCI compliance standpoint
- Turn it on only while needed with `begin()` / `end()` (see the `ClockOut_OnDemand` example)
- Do not enable it when unused (the reset default is disabled)

## Implementation notes (DS40002548A chapter 12)

- The output signal is **CLK_PER** (12.2.2 Signal Description: `CLKOUT — Digital output — CLK_PER output`)
- Control is bit 7 (CLKOUT) of `CLKCTRL.MCLKCTRLA`. This register is **CCP protected**, so the value to write is loaded into a register beforehand, then the CCP key is written, and interrupts are disabled to stay within the protection window (a read-modify-write does not complete inside the window)
- When **CFD (Clock Failure Detection)** overrides the main clock's CLKSEL, **the hardware automatically clears the CLKOUT bit**. `isRunning()` reflects this
- The datasheet does not state whether the CLKOUT override also sets the pin direction, so `begin()` calls `pinMode(OUTPUT)` explicitly (harmless either way)

## Examples

- **ClockOut_Basic** — Start output, print the frequency and pin number, monitor for a CFD stop
- **ClockOut_OnDemand** — EMI-conscious use: output only while an external device needs it
