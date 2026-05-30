/*
 * avrdu_cdc_bl/src/nvm.h
 * --------------------------------------------------------------------
 *  Clean-room implementation.  Reference: AVR64DU32 datasheet
 *  section 11 (NVMCTRL).  License: LGPL 2.1.
 */
#ifndef AVRDU_BL_NVM_H
#define AVRDU_BL_NVM_H

#include <stdint.h>

/* AVR64DU32 flash organization (datasheet section 11.2):
 *   - Page size: 512 bytes
 *   - 64 KB total, 128 pages
 *   - BOOT, APPCODE, APPDATA regions divided by BOOTSIZE/CODESIZE fuses
 *
 * Self-write rules (datasheet 11.3.7):
 *   - Code in BOOT can write to APPCODE / APPDATA only
 *   - Code in APPCODE can write to APPDATA only (and only if APPCODE
 *     is unlocked)
 *   - BOOT is always locked against self-write
 *
 * The functions below assume they are called from BOOT and that the
 * target byte_addr lies in APPCODE (>= 0x1000 with BOOTSIZE=0x08).
 */
#define NVM_FLASH_PAGE_SIZE 512u

/* Erase the flash page containing byte_addr, then write `nbytes`
 * bytes from `data` starting at byte_addr.  `nbytes` is typically a
 * multiple of 2 (word writes); if it is odd, the trailing byte is
 * written as a byte and then padded with 0xFF.                       */
void nvm_write_page(uint32_t byte_addr,
                    const uint8_t *data,
                    uint16_t nbytes);

/* Read one byte from flash.  Use mapped flash or LPM as appropriate.
 * The bootloader only needs this for READ_PAGE responses, which the
 * arduino programmer uses for verify passes.                         */
uint8_t nvm_read_byte(uint32_t byte_addr);

#endif  /* AVRDU_BL_NVM_H */
