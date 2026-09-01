/* SPISlave.cpp - SPI client ("slave") mode for Wazamono boards.
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * (C) Workshop Asahi 2026.  DxCore is (C) Spence Konde, LGPL 2.1 (see License.md).
 *
 * Clean-room implementation from DS40002548A chapter 26 (SPI) only. See
 * SPISlave.h for the API description and the ESP8266 compatibility notes.
 *
 * Operation:
 *   - SPI0 runs in Client mode (MASTER = 0) with Buffer mode + BUFWR = 1.
 *   - The reply is fed one byte per received byte from the RXC interrupt:
 *     each completed byte exchange frees exactly one transmit slot, so one
 *     write per RXCIF can never hit the single-slot transmit buffer while
 *     it is full (26.3.2.2.1: a second write while full would overwrite).
 *     Before a transaction (SS high) two bytes are pre-staged: the first
 *     goes straight to the shift register (BUFWR = 1), the second waits in
 *     the transmit buffer.
 *   - Transaction framing comes from the SS pin itself: a rising edge on
 *     SPISLAVE_SS_PIN (attachInterrupt) marks the end, fires the callbacks, and
 *     re-stages the reply for the next transaction. The SPI hardware has no
 *     "deselected" interrupt of its own (SSIF only flags a Host-mode demotion,
 *     26.5.6), which is why the port interrupt is used.
 */

#include "SPISlave.h"

/* ---- state ------------------------------------------------------------- */
static volatile uint8_t  s_rx[SPISLAVE_BUFFER_SIZE];
static uint8_t           s_tx[SPISLAVE_BUFFER_SIZE];
static volatile uint8_t  s_rx_len = 0;   /* bytes stored this transaction    */
static volatile uint8_t  s_tx_len = 0;   /* staged reply length              */
static volatile uint8_t  s_tx_pos = 0;   /* reply bytes handed to hardware   */
static volatile SpiSlaveDataHandler s_data_cb      = nullptr;
static volatile SpiSlaveSentHandler s_data_sent_cb = nullptr;
static volatile bool     s_running = false;

/* Pre-stage the first (up to) two reply bytes while SS is high: the first
 * write lands in the shift register (BUFWR = 1), the second in the transmit
 * buffer. With no reply staged, zeros are pre-staged (zero-fill).
 *
 * The transmit path has to be flushed first. With BUFWR = 1 a write to DATA
 * is silently dropped while DREIF is low - DS40002548B Figure 26-4 spells
 * this out: "the Transmit Data Buffer register is not updated since the
 * DREIF is low", and the value is lost. So whatever is already staged cannot
 * be overwritten by simply writing again, and a prime that follows an
 * earlier prime (begin() then setData(), or a host that clocked fewer bytes
 * than were staged) would leave the stale bytes in place - they come out as
 * the first bytes of the next transaction. Cycling ENABLE empties the shift
 * register and the transmit buffer. Only ever called with SS high, so no
 * transfer is disturbed. */
static void _spislave_reset_fifo(void) {
  /* Drain the receive path and clear the flags. BUFOVF is not cleared by
   * writing a one to it while the receive FIFO still holds the overflow -
   * DS40002548B 26.5.5 lists reading DATA first among the ways to clear it -
   * and disabling the peripheral does not clear it either. The FIFO is the
   * Receive Data register plus the Receive Data Buffer, so three reads cover
   * it with the shift register. */
  (void)SPI0.DATA;
  (void)SPI0.DATA;
  (void)SPI0.DATA;
  SPI0.INTFLAGS = SPI_RXCIF_bm | SPI_TXCIF_bm | SPI_SSIF_bm | SPI_BUFOVF_bm;
}

static void _spislave_prime(void) {
  uint8_t ctrla = SPI0.CTRLA;
  SPI0.CTRLA    = ctrla & ~SPI_ENABLE_bm;
  SPI0.CTRLA    = ctrla;
  _spislave_reset_fifo();
  SPI0.DATA = (0 < s_tx_len) ? s_tx[0] : 0;
  SPI0.DATA = (1 < s_tx_len) ? s_tx[1] : 0;
  s_tx_pos = 2;
}

/* SS rising edge: the host ended the transaction. */
static void _spislave_ss_rising(void) {
  /* The port interrupt may run before the SPI interrupt has drained the last
   * byte(s) of the transaction - collect them first (receive only: writing
   * DATA now, with SS already high, would corrupt the upcoming pre-stage). */
  while (SPI0.INTFLAGS & SPI_RXCIF_bm) {
    uint8_t b = SPI0.DATA;
    if (s_rx_len < SPISLAVE_BUFFER_SIZE) {
      s_rx[s_rx_len++] = b;
    }
    s_tx_pos++;                       /* that byte also consumed a reply byte */
  }
  uint8_t n = s_rx_len;
  /* "Reply fully read": the transaction consumed every staged byte. s_tx_pos
   * counts bytes handed to the hardware (pre-staging included), so require
   * that the host actually clocked at least s_tx_len bytes as well. */
  SpiSlaveSentHandler sent_cb = s_data_sent_cb;
  SpiSlaveDataHandler data_cb = s_data_cb;
  if (sent_cb && s_tx_len && (s_tx_pos >= s_tx_len) && (n >= s_tx_len)) {
    sent_cb();
  }
  if (data_cb && n) {
    data_cb((uint8_t *)s_rx, (size_t)n);
  }
  s_rx_len = 0;
  /* Drop whatever is left of the old reply position and re-stage from the
   * start, so the (possibly new) reply is ready for the next transaction. */
  uint8_t flags = SPI0.INTFLAGS;          /* clear stale RXCIF/BUFOVF...     */
  if (flags & SPI_BUFOVF_bm) {
    SPI0.INTFLAGS = SPI_BUFOVF_bm;
  }
  _spislave_prime();
}

/* One interrupt vector serves all SPI flags. Only RXC is enabled: every
 * received byte is stored, and exactly one reply byte is fed back. */
ISR(SPI0_INT_vect) {
  while (SPI0.INTFLAGS & SPI_RXCIF_bm) {
    uint8_t b = SPI0.DATA;                          /* read clears RXCIF     */
    if (s_rx_len < SPISLAVE_BUFFER_SIZE) {
      s_rx[s_rx_len++] = b;
    }
    SPI0.DATA = (s_tx_pos < s_tx_len) ? s_tx[s_tx_pos] : 0;  /* zero-fill    */
    s_tx_pos++;
  }
  if (SPI0.INTFLAGS & SPI_BUFOVF_bm) {
    SPI0.INTFLAGS = SPI_BUFOVF_bm;                  /* bytes lost; carry on  */
  }
}

/* ---- class ------------------------------------------------------------- */
void SPISlaveClass::begin(uint8_t dataMode) {
  if (s_running) {
    end();
  }
  /* Pin positions: the variant's fixed SPI location (Tachi/Tsurugi ALT4,
   * Kunai DEFAULT). Only the SPI0 field of SPIROUTEA is touched. */
  PORTMUX.SPIROUTEA = (PORTMUX.SPIROUTEA & ~PORTMUX_SPI0_gm) | SPI_MUX;

  /* Table 26-1 (Client mode): MISO is the one pin whose direction the user
   * must set; MOSI/SCK/SS are inputs. While SS is high the hardware releases
   * MISO. A pull-up on SS keeps the client deselected if the host side is
   * absent or its CS line is not yet driven. SS is the position's own SS
   * pin (SPISLAVE_SS_PIN) - on Tsurugi that is the AREF header pin. */
  pinMode(PIN_SPI_MISO, OUTPUT);
  pinMode(PIN_SPI_MOSI, INPUT);
  pinMode(PIN_SPI_SCK,  INPUT);
  pinMode(SPISLAVE_SS_PIN,   INPUT_PULLUP);

  s_rx_len = 0;
  s_tx_len = 0;

  /* Buffer mode, BUFWR = 1 (pre-staging while SS is high goes straight to
   * the shift register), mode bits as requested. MASTER stays 0 = Client.
   * SSD is a Host-mode-only bit and is left 0. */
  SPI0.CTRLA   = 0;
  SPI0.CTRLB   = SPI_BUFEN_bm | SPI_BUFWR_bm | (dataMode & SPI_MODE_gm);
  /* A previous session may have ended in overflow (a host clocking faster
   * than the interrupt can be serviced). BUFOVF survives both end() and a
   * fresh begin() unless the receive FIFO is read out, and while it is set
   * the receive path stays jammed: no RXCIF, so neither the SPI interrupt
   * nor the transaction ends up delivering anything, and the shift register
   * just echoes MOSI back a byte late. Clear it properly before enabling. */
  _spislave_reset_fifo();
  SPI0.INTCTRL = SPI_RXCIE_bm;
  SPI0.CTRLA   = SPI_ENABLE_bm;                     /* MASTER = 0 -> Client  */

  _spislave_prime();

  /* Transaction framing: SS returning high ends the exchange. */
  attachInterrupt(SPISLAVE_SS_PIN, _spislave_ss_rising, RISING);
  s_running = true;
}

void SPISlaveClass::end(void) {
  if (!s_running) {
    return;
  }
  detachInterrupt(SPISLAVE_SS_PIN);
  SPI0.INTCTRL = 0;
  SPI0.CTRLA   = 0;
  SPI0.CTRLB   = 0;
  _spislave_reset_fifo();          /* leave nothing behind for the next begin() */
  pinMode(PIN_SPI_MISO, INPUT);
  s_running = false;
}

void SPISlaveClass::setData(uint8_t *data, size_t len) {
  if (len > SPISLAVE_BUFFER_SIZE) {
    len = SPISLAVE_BUFFER_SIZE;
  }
  uint8_t oldSREG = SREG;
  cli();
  memcpy(s_tx, data, len);
  s_tx_len = (uint8_t)len;
  /* If we are between transactions, restage now so the new reply's first
   * bytes replace the previously primed ones in the hardware (BUFWR = 1
   * writes while SS is high overwrite the shift register, 26.3.2.2.1).
   * Mid-transaction, the new reply takes effect from the next SS cycle. */
  if (s_running && (digitalRead(SPISLAVE_SS_PIN) == HIGH)) {
    s_tx_pos = 0;
    _spislave_prime();
  }
  SREG = oldSREG;
}

void SPISlaveClass::onData(SpiSlaveDataHandler cb)     { s_data_cb = cb; }
void SPISlaveClass::onDataSent(SpiSlaveSentHandler cb) { s_data_sent_cb = cb; }

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SPISLAVE)
SPISlaveClass SPISlave;
#endif
