# EventSystem — On-chip wiring library (Wazamono-specific)

The **event system** is "wiring" provided inside the chip. It carries signals from one place to another
**without using the CPU at all**. It keeps running whatever the sketch is doing, even while the CPU is asleep.

This library provides **six connections** out of the box:

```
EventSystem, EventSystem1, EventSystem2, EventSystem3, EventSystem4, EventSystem5
```

Each connection carries "**one source → any number of destinations**".

```cpp
#include <EventSystem.h>

void setup() {
  EventSystem.connect(8, 2);   // The state of pin D8 appears on D2 as-is. That's it.
}
```

## Sources (what is carried)

| Value | Meaning |
|---|---|
| Arduino pin number | The level of that pin (**up to two at a time from the same port** — a hardware limitation) |
| `EVENT_ANALOG_COMP` | **The AnalogComp result**. `AnalogComp.begin()` is all that is needed |
| `EVENT_CUSTOM_LOGIC` | **The CustomLogic output** |
| `EVENT_CUSTOM_LOGIC1` | The CustomLogic1 output (Tachi/Tsurugi only) |
| `EVENT_SOFTWARE` | Carries nothing by itself. Sends a **one-clock pulse** the moment `trigger()` is called |

## Destinations (where it is delivered)

**To a pin** — The available pins are fixed per board (as in the pinout tables):

| | EVOUTA | EVOUTD | EVOUTF |
|---|---|---|---|
| **Tachi** | D8 | D9 | A3 |
| **Tsurugi** | D2 | D9 | A2 |
| **Kunai** | D1 | D3 | — |

**To a CustomLogic input** — `EVENT_TO_LOGIC_A` / `EVENT_TO_LOGIC_B`
(for CustomLogic1: `EVENT_TO_LOGIC1_A` / `_B`; Tachi/Tsurugi only).
The logic side receives it with `CustomLogic.setInputINn(LOGIC_EVENT_A)`.

```cpp
// "Voltage above 2.5 V AND button" — no wiring, no CPU
AnalogComp.begin(INTERNAL2V5);
EventSystem.connect(EVENT_ANALOG_COMP, EVENT_TO_LOGIC_A);
CustomLogic.setInputIN0(LOGIC_EVENT_A);
CustomLogic.begin(AND);

// Use any pin (D8) as CustomLogic's IN0
EventSystem1.connect(8, EVENT_TO_LOGIC_B);
```

## API

| Method | Description |
|---|---|
| `connect(source, destination)` | Make a connection. Calling it again replaces the source (destinations are kept) |
| `addDestination(destination)` | **Add** a destination (deliver one connection to several places at once) |
| `disconnect(destination)` | Remove only that destination |
| `trigger()` | Send a **one-clock pulse** on this connection (mainly for `EVENT_SOFTWARE`) |
| `connected()` | Whether connected |
| `end()` | Tear the connection down and release everything it used |

Every `connect`/`addDestination` returns success as a `bool`. It returns `false` when:
the destination is not one of that board's fixed pins, the pin source would be the third from the same port,
the destination is in use by another connection, this connection's channel is in use by another feature
(e.g. `CustomLogic.addOutput()`), or a `*_LOGIC1` value is specified on Kunai.

## Characteristics and notes

- **Pins are never taken silently** — A pin is touched only when you specify it as a source or destination.
  Source pins get a pull-up (a button to GND just works; a driven signal overrides the pull-up).
- **Coexists safely with CustomLogic** — `CustomLogic.setOutput()/addOutput()` uses the same six channels,
  but neither side ever takes over "a channel configured by the other" (CustomLogic allocates from the high end,
  this library uses only its own fixed number, so in practice they do not collide).
- **The `trigger()` pulse is one clock (about 42 ns at 24 MHz)** — Not visible on an LED wired directly.
  Use it for "edge-driven" things such as a CustomLogic latch (SoftwareTrigger example) or a counter.
- The event features of TCA/TCB/RTC/USART/SPI are **not provided by this library**, to avoid conflicts with other features.

## Examples

- **PinToPin** — A button to an event output pin (the minimal one-liner)
- **CompToPin** — AnalogComp threshold result to a pin (an indicator with no CPU)
- **PinToLogic** — Any pin as a CustomLogic input (a real example of not sacrificing I2C on Kunai)
- **SoftwareTrigger** — Set a CustomLogic latch with `trigger()`

## Provenance

This library is a **new implementation** for WazamonoCore. It is based solely on the AVR64DU28/32 datasheet
(DS40002548A) §16 EVSYS, §17 PORTMUX, and §18 PORT, and contains no code derived from existing
implementations, including DxCore's Event library.
