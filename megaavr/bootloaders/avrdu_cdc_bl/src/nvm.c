/*
 * avrdu_cdc_bl/src/nvm.c
 * --------------------------------------------------------------------
 *  Clean-room implementation.  Reference: AVR64DU32 datasheet
 *  section 11 (NVMCTRL) and section 6.5 (CCP protection).  No code
 *  from Optiboot or any other AVR Dx self-program implementation was
 *  consulted.
 *
 *  License: LGPL 2.1.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>

#include "nvm.h"

/* --------------------------------------------------------------------
 *  Issue a CCP-protected NVMCTRL command.  Datasheet 11.5.1.1:
 *  certain NVMCTRL.CTRLA writes require the prior write of 0x9D to
 *  the CCP register within four clocks.
 * -------------------------------------------------------------------- */
static inline void nvm_cmd(uint8_t cmd) {
    while (NVMCTRL.STATUS & NVMCTRL_FLBUSY_bm) { /* spin */ }
    CCP = CCP_SPM_gc;             /* 0x9D */
    NVMCTRL.CTRLA = cmd;
}

/* --------------------------------------------------------------------
 *  Flash page erase.  Datasheet 11.3.3: write FLPER then perform a
 *  store to any address inside the target page.
 *
 *  AVR DU has the data flash mapped at the start of program memory,
 *  so a plain ST instruction (which `*p = ...` becomes) triggers the
 *  programming sequence as long as the FLMAP setting points at the
 *  right 32 KB window.  For 64 KB AVR64DU32 we explicitly select
 *  the right FLMAP section before the store.
 * -------------------------------------------------------------------- */
static void flash_page_erase(uint32_t byte_addr) {
    /* Select FLMAP section so that byte_addr is reachable via the
     * lower 32 KB data-space window.  Section 0 covers 0x0000..0x7FFF,
     * Section 1 covers 0x8000..0xFFFF. */
    uint8_t section = (byte_addr >= 0x8000UL) ? 1 : 0;
    NVMCTRL.CTRLB = (NVMCTRL.CTRLB & ~NVMCTRL_FLMAP_gm)
                    | (section << NVMCTRL_FLMAP_gp);

    /* Compute the data-space pointer corresponding to byte_addr. */
    volatile uint8_t *p =
        (volatile uint8_t *)((uint16_t)(byte_addr & 0x7FFFu));

    nvm_cmd(NVMCTRL_CMD_FLPER_gc);
    *p = 0xFF;                    /* dummy store triggers erase */
    while (NVMCTRL.STATUS & NVMCTRL_FLBUSY_bm) { /* spin */ }
    nvm_cmd(NVMCTRL_CMD_NOOP_gc);
}

/* --------------------------------------------------------------------
 *  Flash word-write.  Datasheet 11.3.4: FLWR command + word stores.
 *  We perform N two-byte stores; the silicon programs them as it
 *  goes.  Hardware enforces that no programming happens to BOOT.
 * -------------------------------------------------------------------- */
static void flash_page_write(uint32_t byte_addr,
                             const uint8_t *data,
                             uint16_t nbytes) {
    uint8_t section = (byte_addr >= 0x8000UL) ? 1 : 0;
    NVMCTRL.CTRLB = (NVMCTRL.CTRLB & ~NVMCTRL_FLMAP_gm)
                    | (section << NVMCTRL_FLMAP_gp);

    volatile uint16_t *p =
        (volatile uint16_t *)((uint16_t)(byte_addr & 0x7FFFu));

    nvm_cmd(NVMCTRL_CMD_FLWR_gc);
    uint16_t i = 0;
    while (i + 1 < nbytes) {
        uint16_t w = (uint16_t)data[i] | ((uint16_t)data[i+1] << 8);
        *p++ = w;
        i += 2;
        while (NVMCTRL.STATUS & NVMCTRL_FLBUSY_bm) { /* spin */ }
    }
    if (i < nbytes) {
        /* Tail byte: write paired with 0xFF padding so silicon
         * receives a full word. */
        uint16_t w = (uint16_t)data[i] | 0xFF00u;
        *p = w;
        while (NVMCTRL.STATUS & NVMCTRL_FLBUSY_bm) { /* spin */ }
    }
    nvm_cmd(NVMCTRL_CMD_NOOP_gc);
}

/* --------------------------------------------------------------------
 *  Public: erase target page and write `nbytes` bytes into it.
 *
 *  The caller already aligns byte_addr to a 512-byte boundary;
 *  STK500's LOAD_ADDRESS + PROG_PAGE sequence from avrdude does this
 *  naturally for the AVR-style ".hex" record stream.
 * -------------------------------------------------------------------- */
void nvm_write_page(uint32_t byte_addr,
                    const uint8_t *data,
                    uint16_t nbytes) {
    if (byte_addr < 0x1000UL) {
        /* Refuse to clobber the BOOT section.  Silicon already
         * enforces this, but bailing early avoids confusing
         * FBUSY hangs. */
        return;
    }
    if (nbytes == 0) return;
    if (nbytes > NVM_FLASH_PAGE_SIZE) nbytes = NVM_FLASH_PAGE_SIZE;

    flash_page_erase(byte_addr);
    flash_page_write(byte_addr, data, nbytes);
}

/* --------------------------------------------------------------------
 *  Public: read one byte from flash, via the FLMAP window.
 * -------------------------------------------------------------------- */
uint8_t nvm_read_byte(uint32_t byte_addr) {
    uint8_t section = (byte_addr >= 0x8000UL) ? 1 : 0;
    NVMCTRL.CTRLB = (NVMCTRL.CTRLB & ~NVMCTRL_FLMAP_gm)
                    | (section << NVMCTRL_FLMAP_gp);

    volatile const uint8_t *p =
        (volatile const uint8_t *)((uint16_t)(byte_addr & 0x7FFFu));
    return *p;
}
