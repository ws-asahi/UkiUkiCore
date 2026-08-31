# CustomLogic — Hardware logic gate library for Wazamono

A WazamonoCore-specific library that makes the CCL (Configurable Custom Logic) built into the
AVR DU series as easy to use as Serial or SD. From the moment you call `begin()`, the logic gate
keeps running in hardware **without using the CPU at all**.

```cpp
#include <CustomLogic.h>

void setup() {
  CustomLogic.begin(AND);                  // OUT = IN0 AND IN1
  // CustomLogic.begin(OR, OR);            // 3-input OR: (IN0 OR IN1) OR IN2
  // CustomLogic.begin(AND, OR);           // (IN0 AND IN1) OR IN2
  // CustomLogic.begin(NOP, OR);           // IN1 OR IN2 (IN0 unused)
  // CustomLogic.begin(NOP);               // Buffer of IN0 (OUT = IN0)
  // CustomLogic.beginTruthTable(0x96, 3); // Arbitrary truth table (3-input XOR)
}

void loop() {
  // The gate keeps running even if you write nothing here
}
```

## Units and pins (fixed in hardware)

| | IN0 | IN1 | IN2 | OUT | OUT (alternate) |
|---|---|---|---|---|---|
| **CustomLogic** (Tachi) | D5 (PD0) | D6 (PD1) | D9 (PD2) | D10 (PD3) | D15 (PD6) |
| **CustomLogic** (Tsurugi) | D5 (PD0) | D6 (PD1) | D9 (PD2) | D10 (PD3) | D13 (PD6) |
| **CustomLogic** (Kunai) | D6 (PA0) | D7 (PA1) | D4 (PA2) | D5 (PA3) | D8 (PA6) |
| **CustomLogic1** (Tachi) | A1 (PF0) | A2 (PF1) | A3 (PF2) | D17 (PF3) | — |
| **CustomLogic1** (Tsurugi) | A0 (PF0) | A1 (PF1) | A2 (PF2) | A3 (PF3) | — |

- Kunai has a single unit only (there is no CustomLogic1)
- Input pins are pulled up automatically, so you can experiment by wiring a button straight to GND
  (a driven logic signal overrides the pull-up, so it can be connected directly as well)

## API

| Method | Description |
|---|---|
| `begin(logic1)` | 2-input gate: OUT = IN0 (logic1) IN1. `LogicType` is one of `AND/OR/XOR/NAND/NOR/XNOR/NOT/NOP` (NOT = inverter of IN0, NOP = buffer of IN0; both are single-input) |
| `begin(logic1, logic2)` | 3-input logic: OUT = (IN0 logic1 IN1) logic2 IN2. Example: `begin(AND, OR)` = A·B+C. **NOP can drop one side**: `begin(NOP, x)` = IN1 (x) IN2 with IN0 unused, `begin(x, NOP)` = IN2 unused (same as `begin(x)`). NOT cannot be combined; (NOP, NOP) is invalid |
| `beginTruthTable(table, n)` | Specify the truth table directly. Bit i is "the output when the input pattern equals the number i" (IN2 = bit 2, IN1 = bit 1, IN0 = bit 0) |
| `setInputIN0(src)` / `setInputIN1(src)` / `setInputIN2(src)` | Change the **source** of that input (default `LOGIC_PIN`). `setInput(n, src)` does the same |
| `setOutput(pin)` | Change the **output pin** (output only to that pin) |
| `addOutput(pin)` | **Add** an output destination (drive several pins at once) |
| `disableOutput()` | No output pin (interrupts and feeding other units only) |
| `read()` | Current state of the output pin |
| `attachInterrupt(fn, mode)` | Call a function on output change (`RISING/FALLING/CHANGE`) |
| `detachInterrupt()` | Remove the interrupt |
| `end()` | Stop and release the pins |

## Input sources (`LogicInput`)

Inputs do not have to come from pins. These are **on-chip connections**, so they consume no pins, no wiring, and no CPU time.

| Constant | Meaning |
|---|---|
| `LOGIC_PIN` | The unit's INn pin (**default**) |
| `LOGIC_ANALOG_COMP` | **The AnalogComp result** (AC0 output). `AnalogComp.begin()` is all that is needed; `enableOutput()` is not required |
| `LOGIC_OWN_OUTPUT` | **The unit's own output**. Lets you build latches and oscillators (CustomLogic only) |
| `LOGIC_OTHER_UNIT` | **The other unit's output**. Lets you build two-stage logic (Tachi/Tsurugi only) |
| `LOGIC_EVENT_A` / `LOGIC_EVENT_B` | Via an event connection. **Any pin** can become an input (wired with the EventSystem library) |

```cpp
// Voltage above 2.5 V AND button not pressed -> output HIGH (no CPU involved)
AnalogComp.begin(INTERNAL2V5);
CustomLogic.setInputIN0(LOGIC_ANALOG_COMP);
CustomLogic.begin(AND);

// Two stages: (IN0₁ OR IN1₁) AND IN1₀   (Tachi/Tsurugi)
CustomLogic1.begin(OR);
CustomLogic.setInputIN0(LOGIC_OTHER_UNIT);
CustomLogic.begin(AND);

// Any pin (D8) into IN0, via an event       (with the EventSystem library)
EventSystem.connect(8, EVENT_TO_LOGIC_A);
CustomLogic.setInputIN0(LOGIC_EVENT_A);
CustomLogic.begin(AND);
```

**An input that does not use a pin never touches that pin** (the pull-up is not set either).
For example, on Kunai, routing IN0/IN1 through events leaves the overlapping I2C pins (D4/D5)
free for I2C.

### Relationship between `LOGIC_OWN_OUTPUT` and `LOGIC_OTHER_UNIT`

The CCL feeds back the output of the **even-numbered** LUT of a pair. CustomLogic is the even one
(Tachi/Tsurugi = LUT2, Kunai = LUT0), so it can see its own output; CustomLogic1 is the odd one
(LUT3), so what it sees there is **CustomLogic's output**. That is exactly what `LOGIC_OTHER_UNIT`
means, so `setInput()` rejects (returns `false` for) `LOGIC_OWN_OUTPUT` on CustomLogic1 and
`LOGIC_OTHER_UNIT` on Kunai.

## Output destinations (`setOutput` / `addOutput`)

The result can go to pins other than the dedicated OUT pin. **The library configures the event system itself**,
so you do not need the EventSystem library for this.

| Output path | Description |
|---|---|
| Dedicated OUT pin | Default. Driven as soon as you call `begin()` |
| Alternate OUT pin | The "OUT (alternate)" column above. Not available on CustomLogic1 |
| **Event output pins (EVOUT)** | The pins in the table below. **Can be used at the same time as the dedicated OUT pin** |

**Event output pins (fixed per board — as in the pinout tables)**

| | EVOUTA | EVOUTD | EVOUTF |
|---|---|---|---|
| **Tachi** | D8 (PA7) | D9 (PD2) | A3 (PF2) |
| **Tsurugi** | D2 (PA7) | D9 (PD2) | A2 (PF2) |
| **Kunai** | D0 (PA7) | D7 (PD7) | — |

```cpp
CustomLogic.begin(AND);
CustomLogic.addOutput(8);      // Drive both the dedicated OUT pin and D8 (Tachi)
// CustomLogic.setOutput(2);   // Drive D2 only (dedicated OUT pin unused)
// CustomLogic.disableOutput();// Drive no pin (interrupts and chaining only)
```

At most **one dedicated OUT pin + the event output pins in the table above (one each)** can be driven simultaneously
(up to 4 on Tachi/Tsurugi, up to 3 on Kunai).

- Event channels are **allocated automatically from the high end (CH5 downward)**. EventSystem connections
  (fixed numbers counting up from CH0 for EventSystem) and channels configured by other means are never taken over.
- **Note on the alternate OUT pin**: on Tachi it is shared with **D15 (SPI SCK / Serial2 TX)**, on Tsurugi with
  **D13 (SPI SCK)**. Tsurugi's D13 drives the on-board LED through an op-amp, so driving D13 makes
  **the on-board LED display the logic result** (cannot be combined with SPI). Tsurugi's PD7 is AREF and is
  not available as an event output.

## Examples

- **TwoInputAND** — 2-input AND gate (two buttons + LED)
- **ThreeInputOR** — 3-input OR. Change the gate name to get other gates
- **TruthTable** — Direct truth table (3-input XOR = 0x96)
- **EdgeInterrupt** — Interrupt on gate output change
- **DualUnits** — Two units running at once (Tachi/Tsurugi)
- **AnalogCompInput** — Use the AnalogComp result as an input (voltage threshold AND button)
- **SetResetLatch** — SR latch using its own output as an input ("memory" in pure hardware)
- **MultipleOutputs** — Drive the same result to the dedicated OUT pin and an event output pin

## Notes

- **Pin sharing with AnalogComp (Tachi/Tsurugi)**: CustomLogic's IN2/OUT (PD2/PD3) are the same pins as
  AnalogComp's default inputs. However, when comparing against the **internal reference, as in
  `AnalogComp.begin(INTERNAL2V5)`, the negative input (PD3) is not used**, so there is no conflict
  (see the AnalogCompInput example). A 2-input gate does not touch the IN2 pin (PD2) either.
- **Sharing with I2C on Kunai**: Kunai's CustomLogic (PA0–PA3) overlaps the I2C pins.
  They can coexist if you use pin-less inputs such as `setInputIN0(LOGIC_EVENT_A)`.
- The gate responds directly in hardware (no filtering). Keep this in mind when counting
  bouncy mechanical contacts with interrupts.

## Provenance

This library is an independent implementation based solely on the AVR64DU28/32 datasheet (DS40002548A)
and the official Microchip device headers. No code from existing CCL libraries, including the Logic
library of DxCore/megaTinyCore, has been used or consulted.
