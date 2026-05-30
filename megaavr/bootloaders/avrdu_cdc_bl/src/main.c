/*
 * avrdu_cdc_bl/src/main.c
 * --------------------------------------------------------------------
 *  Clean-room implementation.  References:
 *    - AVR64DU32 datasheet (Microchip DS40002676A or later)
 *    - Atmel AVR064 - STK500 Communication Protocol App Note
 *    - USB 2.0 specification, USB CDC PSTN 1.20 specification
 *    - Microchip ATPACK device headers (avr/io.h)
 *  No source from Optiboot, LUFA, TinyUSB, V-USB or any other
 *  bootloader / USB project was consulted while writing this file.
 *
 *  Role:
 *    Entry point of the AVRDU CDC bootloader.  Decides whether to
 *    remain in the bootloader for an avrdude session or jump to the
 *    application at 0x1000, and runs the main poll loop while
 *    resident.
 *
 *  License: LGPL 2.1 (matches the host DxCore repository).
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <stdbool.h>

#include "usb_min.h"
#include "cdc_min.h"
#include "stk500.h"
#include "nvm.h"

/* ----- Coordination with the runtime (mirrored in runtime/usb_cdc.h) ----- */
#ifndef AVRDU_BL_MAGIC_ADDR
#define AVRDU_BL_MAGIC_ADDR  0x7FFE  /* last 2 bytes of 8 KB SRAM */
#endif
#define AVRDU_BL_MAGIC_STAY  0xB007
#define AVRDU_BL_MAGIC_NONE  0x0000

/* ----- Application location ----- */
#define APP_RESET_VECTOR_BYTE  0x1000   /* must match BOOTEND in fuse */
#define APP_RESET_VECTOR_WORD  (APP_RESET_VECTOR_BYTE >> 1)

/* ----- WDT timeout we ask the silicon to apply when leaving prog mode ----- */
#define WDT_PERIOD_EXIT_gc  WDT_PERIOD_8CLK_gc  /* ~8 ms */

/* --------------------------------------------------------------------
 *  Vector-table relocation
 *
 *  The AVR DU has IVSEL (in CPUINT.CTRLA) which makes the CPU read
 *  interrupt vectors from the BOOT section instead of APPCODE.  The
 *  bootloader does not actually use any interrupts (we poll), but
 *  setting IVSEL is still required so that the reset vector at 0x0000
 *  takes us into BOOT, not into APPCODE.  This is normally the silicon
 *  default after a reset when running from BOOT, but we make it
 *  explicit so we don't depend on undocumented startup behaviour.
 *  Conversely, before jumping to the app we clear IVSEL so that the
 *  app's own interrupt handlers in APPCODE are reachable.
 * -------------------------------------------------------------------- */
static inline void vectors_to_boot(void) {
    CCP = CCP_IOREG_gc;
    CPUINT.CTRLA |= CPUINT_IVSEL_bm;
}

static inline void vectors_to_app(void) {
    CCP = CCP_IOREG_gc;
    CPUINT.CTRLA &= (uint8_t)~CPUINT_IVSEL_bm;
}

/* --------------------------------------------------------------------
 *  Clock setup - AVR DU needs OSCHF running at one of the
 *  USB-compatible frequencies (12/16/20/24 MHz) and the USB peripheral
 *  selects its own 48 MHz internal oscillator automatically when
 *  enabled.  We pick 24 MHz here for the core.
 *  See datasheet 11.5 (CLKCTRL) and 28.3.4 (USB clocking).
 * -------------------------------------------------------------------- */
static void clocks_init(void) {
    /* Read-modify-write OSCHFCTRLA: keep the AUTOTUNE field's reset
     * value (will be promoted to SOF tracking by usb_min_init), and
     * set FRQSEL to 0x9 = 24 MHz.  Bits [5:2] hold FRQSEL. */
    uint8_t oschf = CLKCTRL.OSCHFCTRLA;
    oschf = (oschf & ~(0x0F << 2)) | CLKCTRL_FRQSEL_24M_gc;
    _PROTECTED_WRITE(CLKCTRL.OSCHFCTRLA, oschf);

    /* Disable main clock prescaler so the CPU runs at the full
     * OSCHF rate (24 MHz). */
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0x00);

    /* Wait until OSCHF is stable. */
    while (!(CLKCTRL.MCLKSTATUS & CLKCTRL_OSCHFS_bm)) { /* spin */ }
}

/* --------------------------------------------------------------------
 *  Heuristic: is the application slot blank?
 *
 *  Blank flash reads as 0xFFFF.  If the first instruction of the app
 *  (the RJMP / JMP at 0x1000) is 0xFFFF, there is nothing to jump to
 *  and we must stay in the bootloader.  This protects users who fuse
 *  the bootloader before any sketch has been uploaded.
 * -------------------------------------------------------------------- */
static bool app_appears_invalid(void) {
    uint16_t *p = (uint16_t *)APP_RESET_VECTOR_BYTE;
    return (*p == 0xFFFF);
}

/* --------------------------------------------------------------------
 *  Jump to application.  We deliberately route through an inline
 *  function so the compiler emits a far-call epilogue (eicall / ijmp
 *  with EIND for 64 KB parts) rather than a relative jump that
 *  cannot reach 0x1000 from BOOT.
 * -------------------------------------------------------------------- */
__attribute__((noreturn))
static void jump_to_app(void) {
    vectors_to_app();

    /* Reset any peripheral we touched (USB at minimum) so the app
     * starts from a known state.  Issue a software reset via RSTCTRL.
     * SWRR bit 0 (SWRST_bm) - datasheet 12.5.2. */
    _PROTECTED_WRITE(RSTCTRL.SWRR, RSTCTRL_SWRST_bm);

    /* Should never reach this. */
    while (1) { }
}

/* --------------------------------------------------------------------
 *  Trigger WDT, used after avrdude sends LEAVE_PROGMODE.
 * -------------------------------------------------------------------- */
__attribute__((noreturn))
void bl_exit_via_wdt(void) {
    /* Clear the stay-in-bootloader flag so the next boot proceeds
     * straight to the app. */
    *(volatile uint16_t *)AVRDU_BL_MAGIC_ADDR = AVRDU_BL_MAGIC_NONE;

    /* Detach USB so the host sees us go away. */
    usb_min_detach();

    _PROTECTED_WRITE(WDT.CTRLA, WDT_PERIOD_EXIT_gc);
    while (1) { }
}

/* ====================================================================
 *  Entry point
 * ==================================================================== */
int main(void) {
    /* 1. Read & clear reset cause. */
    uint8_t  rstfr = RSTCTRL.RSTFR;
    RSTCTRL.RSTFR  = 0xFF;

    /* 2. Read & clear the stay-in-bootloader magic. */
    volatile uint16_t *magic_p = (volatile uint16_t *)AVRDU_BL_MAGIC_ADDR;
    uint16_t magic = *magic_p;
    *magic_p = AVRDU_BL_MAGIC_NONE;

    /* 3. Make sure the CPU is reading vectors from BOOT.  Required for
     *    correct ISR dispatch even though we currently poll. */
    vectors_to_boot();

    /* 4. Decide.  Order matters: explicit request wins, then external
     *    reset (user pressed the button), then blank-app guard.       */
    bool stay_in_bl = false;

    if (magic == AVRDU_BL_MAGIC_STAY) {
        stay_in_bl = true;
    } else if (rstfr & RSTCTRL_EXTRF_bm) {
        stay_in_bl = true;
    } else if (app_appears_invalid()) {
        stay_in_bl = true;
    }

    if (!stay_in_bl) {
        jump_to_app();
    }

    /* 5. Stay in the bootloader.  Bring up the clock tree and USB. */
    clocks_init();
    usb_min_init();
    cdc_min_init();
    stk500_init();
    usb_min_attach();

    /* 6. Polling loop.  All three modules cooperate via the bulk
     *    OUT/IN pipes managed by usb_min.  stk500_poll() consumes
     *    one byte from CDC RX, runs the state machine, and pushes
     *    responses back to CDC TX. */
    for (;;) {
        usb_min_poll();
        cdc_min_poll();
        stk500_poll();
    }
}
