# SPISlave — SPI client (receiver) library

A library that lets a Wazamono board operate as an **SPI client (traditionally called a slave)**, receiving transmissions from a host and returning responses.

The API follows the same format as the **SPISlave library bundled with the ESP8266 Arduino core**, so sketches written for the ESP8266 can be ported with minimal changes.

## Supported boards

| Board | SS pin | SPI position |
|-------|--------|--------------|
| Wazamono Tachi | A0 / D18 (PD7) | SPI0 ALT4 (MOSI = D16 / MISO = D14 / SCK = D15) |
| Wazamono Tsurugi | **AREF** / D20 (PD7) | SPI0 ALT4 (MOSI = D11 / MISO = D12 / SCK = D13) |
| Wazamono Kunai | D1 (PA7) | SPI0 default (MOSI = D10 / MISO = D9 / SCK = D8) |

SS is the **hardware SS pin** of the board's SPI0 position (`PIN_SPI_SS_HARDWARE` in the variant). It may differ from `PIN_SPI_SS` used in host mode (a software CS on D10 for Tsurugi).

> **Note for Tsurugi:** The hardware SS is the **AREF header pin**. While SPISlave is active, AREF becomes the SS input and is therefore **mutually exclusive** with the external reference (`analogReference(EXTERNAL)` / AREF supplied from a shield), D20/A20 as GPIO/analog, and Serial2 (RX = PD7, TX = PD6 = SCK). `begin()` enables the pull-up on AREF. Connect the host's CS line to the AREF pin.

## Usage

```cpp
#include <SPISlave.h>

void setup() {
  // Received from the host (one transaction, delivered when SS returns High)
  SPISlave.onData([](uint8_t *data, size_t len) {
    // data/len are valid only inside the callback; copy what you need
    SPISlave.setData("Hello Master!");   // Prepare the response the host reads next
  });

  // The host has read the setData() response to the end
  SPISlave.onDataSent([]() { });

  SPISlave.begin();          // SPI mode 0 (default), MSB first
  SPISlave.setData("boot");  // Response to the first read
}
```

On the host side, use the ordinary SPI library: pull CS Low → send/receive → pull CS High (see the `SPISlave_Host` example).

## API

| Method | Description |
|--------|-------------|
| `begin(dataMode = 0)` | Start client mode. `dataMode` is the SPI mode number 0–3 (match the host's `SPISettings`). Bit order is fixed to MSB first |
| `end()` | Stop (returns MISO to input) |
| `setData(uint8_t *data, size_t len)` / `setData(const char *)` | Set the response the host will read in the next transaction (copied internally, up to `SPISLAVE_BUFFER_SIZE` bytes). The same response is sent from the beginning on every transaction until the next `setData()` |
| `onData(cb)` | `void cb(uint8_t *data, size_t len)`. Delivers all bytes received in the transaction **when SS returns High** |
| `onDataSent(cb)` | `void cb()`. Called at the end of a transaction in which the host read the response to the end |

- Callbacks run in **interrupt context**. Keep them short, and declare variables shared with loop() as `volatile`.
- Bytes read beyond the length set by `setData()` are padded with **0x00** (same behavior as the ESP8266 version).
- The buffer length defaults to 32 bytes. It can be changed by defining e.g. `#define SPISLAVE_BUFFER_SIZE 64` before `#include <SPISlave.h>` (maximum 255).

## Differences from the ESP8266 version

| Item | ESP8266 | This library |
|------|---------|--------------|
| Transaction length | Fixed 32 bytes (hardware limitation) | **Variable length** (all bytes while SS is Low; `len` is the actual count received) |
| `setStatus()` / `onStatus()` / `onStatusSent()` | Present (ESP8266-specific status register) | **Not present** |
| `begin()` argument | None | SPI mode number 0–3 (default 0) |

## Implementation notes (DS40002548A chapter 26)

- Client mode (MASTER = 0) + buffer mode (BUFEN = 1, **BUFWR = 1**). A DATA write while SS is High goes straight to the shift register (26.3.2.2.1), so the first byte of the response is already loaded before the transaction starts
- Subsequent response bytes are topped up "one byte per byte received" from the RXC interrupt. The transmit buffer is single-stage, and this discipline prevents overwrite loss
- Transaction boundaries are detected on the **rising edge of the SS pin** (`attachInterrupt`). The SPI's own SSIF is only for detecting demotion from host mode (26.5.6) and cannot be used for this purpose
- As in Table 26-1, the only pin whose direction must be set in client mode is MISO (done by `begin()`). While SS is High, the hardware releases MISO automatically, so several clients can share the bus
- The host-side SPI library (`SPI.h`) can be used in the same sketch, but `SPI.begin()` and `SPISlave.begin()` both configure SPI0, so they **cannot be active at the same time**

## Limitations and notes

- Because an interrupt runs for every byte, keep the host's SCK modest (start around 1 MHz). With fast clocks and no gap between consecutive bytes, `BUFOVF` (receive two-stage buffer overflow) can drop data. Verify the upper limit on real hardware
- When calling `setData()` from `onData` (request → response), have the host send and read in **separate transactions** with a short wait (about 1 ms) between them (see the example)
