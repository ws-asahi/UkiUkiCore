/* SPISlave.h - SPI client ("slave") mode for Wazamono boards.
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * (C) Workshop Asahi 2026.  DxCore is (C) Spence Konde, LGPL 2.1 (see License.md).
 *
 * API format follows the SPISlave library bundled with the ESP8266 Arduino
 * core (esp8266/Arduino, libraries/SPISlave) so sketches written for it port
 * over with minimal change:
 *
 *     SPISlave.onData([](uint8_t *data, size_t len) { ... });   // received
 *     SPISlave.onDataSent([]() { ... });                        // reply read
 *     SPISlave.begin();
 *     SPISlave.setData("Hello Master!");                        // next reply
 *
 * Differences from the ESP8266 original (hardware dictates them):
 *   - Transactions are variable-length. The ESP8266 hardware works in fixed
 *     32-byte packets; the AVR DU SPI is a plain byte stream, so a
 *     "transaction" here is everything the host clocks while SS is low, and
 *     onData() fires when SS returns high. len is the actual byte count
 *     (clamped to SPISLAVE_BUFFER_SIZE).
 *   - setStatus()/onStatus()/onStatusSent() do not exist: the ESP8266 status
 *     register is specific to its SPI block.
 *   - begin() takes an optional SPI mode number 0..3 (default 0), which must
 *     match the host's SPISettings dataMode.
 *
 * Hardware (DS40002548A chapter 26):
 *   - Client mode: MASTER = 0. MOSI/SCK/SS become inputs; MISO must be set
 *     as an output by the user (Table 26-1) - begin() does this. While SS is
 *     high the hardware releases MISO, so multiple clients can share the bus.
 *   - Buffer mode with BUFWR = 1: bytes written while SS is high are copied
 *     straight into the shift register (26.3.2.2.1), so the first byte of
 *     the reply is ready before the host starts clocking.
 *   - Reply bytes beyond setData()'s length are sent as 0x00, matching the
 *     ESP8266 library's zero-fill behaviour.
 *
 * Supported boards (SS = the hardware SS pin of the board's SPI0 position,
 * PIN_SPI_SS_HARDWARE - not necessarily PIN_SPI_SS, which is the host-mode CS):
 *   Tachi    SS = PD7 = D18 (A0)     SPI0 ALT4   MOSI D16 / MISO D14 / SCK D15
 *   Tsurugi  SS = PD7 = D20 = AREF   SPI0 ALT4   MOSI D11 / MISO D12 / SCK D13
 *   UkiUkiduino  (same as Tsurugi)   SPI0 ALT4   MOSI D11 / MISO D12 / SCK D13
 *   Kunai    SS = PA7 = D1           SPI0 DEFAULT MOSI D10 / MISO D9  / SCK D8
 * Tsurugi / UkiUkiduino: the AREF header pin IS the SS input while SPISlave is active, so
 * it is exclusive with an external analog reference (analogReference(EXTERNAL)),
 * with GPIO/analog use of D20/A20, and with Serial2 (whose RX is PD7 and whose
 * TX PD6 is SCK). begin() enables the pull-up on it; nothing else about the
 * pin is changed, so remove any shield reference voltage from AREF first.
 *
 * Callbacks run in interrupt context: keep them short, and declare data they
 * share with loop() as volatile. Using this library and SPI.h (host mode) on
 * the same sketch is possible, but SPI.begin() and SPISlave.begin() must not
 * be active at the same time - both program the same SPI0 peripheral.
 */

#ifndef SPISLAVE_H
#define SPISLAVE_H

#include <Arduino.h>

#if !defined(ARDUINO_AVR_TACHI) && !defined(ARDUINO_AVR_TSURUGI) && !defined(ARDUINO_AVR_KUNAI) && !defined(ARDUINO_AVR_UKIUKIDUINO)
  #error "SPISlave supports the Wazamono Tachi, Tsurugi, Kunai and the UkiUkiduino only."
#endif

/* The SS input of client mode is the SPI0 position's own SS pin. Variants
 * export it as PIN_SPI_SS_HARDWARE; PIN_SPI_SS may instead be the host-mode
 * software CS (Tsurugi: D10), which the hardware does not look at. */
#if defined(PIN_SPI_SS_HARDWARE)
  #define SPISLAVE_SS_PIN (PIN_SPI_SS_HARDWARE)
#else
  #define SPISLAVE_SS_PIN (PIN_SPI_SS)
#endif

/* Receive/transmit buffer size in bytes (one transaction each way). The
 * default matches the ESP8266 library's 32-byte packets; override by
 * defining SPISLAVE_BUFFER_SIZE before including this header (max 255). */
#if !defined(SPISLAVE_BUFFER_SIZE)
  #define SPISLAVE_BUFFER_SIZE (32)
#endif

typedef void (*SpiSlaveDataHandler)(uint8_t *data, size_t len);
typedef void (*SpiSlaveSentHandler)(void);

class SPISlaveClass {
public:
  /* Start client mode. dataMode is the SPI mode number 0..3 (CPOL/CPHA) and
   * must match the host; bit order is MSB first (set the host accordingly). */
  void begin(uint8_t dataMode = 0);
  void end(void);

  /* Stage the reply the host will clock out during the NEXT transaction(s).
   * The data is copied (up to SPISLAVE_BUFFER_SIZE bytes); the same reply is
   * re-sent from its first byte on every new transaction until replaced. */
  void setData(uint8_t *data, size_t len);
  void setData(const char *data) {
    setData((uint8_t *)data, strlen(data));
  }

  /* Called (from interrupt context) when a transaction ends - SS returned
   * high - and at least one byte was received. data/len are valid only for
   * the duration of the callback: copy what you need. */
  void onData(SpiSlaveDataHandler cb);

  /* Called (from interrupt context) when a transaction ends in which the
   * host clocked out the entire reply staged with setData(). */
  void onDataSent(SpiSlaveSentHandler cb);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SPISLAVE)
extern SPISlaveClass SPISlave;
#endif

#endif /* SPISLAVE_H */
