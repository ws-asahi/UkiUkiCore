/*
 * wdt_compat.h - Pro Micro / classic-AVR <avr/wdt.h> compatibility for AVR DU
 * --------------------------------------------------------------------------
 *  WazamonoCore addition.  The Wazamono Tachi aims for high source-level
 *  compatibility with the SparkFun Pro Micro (ATmega32U4), so classic-AVR
 *  watchdog code should build and behave identically with no edits:
 *
 *      #include <avr/wdt.h>
 *      wdt_enable(WDTO_2S);   // 2-second time-out
 *      ...
 *      wdt_reset();           // pet the dog
 *      ...
 *      wdt_disable();
 *
 *  On the AVR DU (and every modern AVR with the new WDT peripheral) the
 *  watchdog is controlled through the lock/CCP-protected WDT.CTRLA register
 *  using WDT_PERIOD_*_gc time-out codes - a completely different encoding from
 *  the classic ATmega WDTCSR.  avr-libc's <avr/wdt.h> for these parts does NOT
 *  map the classic WDTO_* constants onto that encoding, so unmodified Pro
 *  Micro code either fails to build or selects the wrong time-out (for
 *  example, wdt_enable(WDTO_2S) would request 0.5 s).
 *
 *  This header is pulled in by Arduino.h.  It includes avr-libc's <avr/wdt.h>
 *  first - which sets that header's include guard and gives us a correct
 *  wdt_reset() - then re-defines wdt_enable()/wdt_disable() and the WDTO_*
 *  constants to reproduce the classic behaviour on the DU's WDT.  Because the
 *  sketch's own "#include <avr/wdt.h>" then hits the already-set include
 *  guard, these definitions win.
 *
 *  Reference: AVR64DU32 data sheet DS40002548A, section 21 (WDT):
 *    - PERIOD codes: 0x1 = 8CLK (7.8 ms) ... 0xB = 8KCLK (8 s)   [21.5.1]
 *    - WDT.CTRLA is CCP-protected (key IOREG)                    [21.3.7]
 *    - "Writing to WDT.CTRLA while SYNCBUSY = 1 is not allowed"  [21.3.6]
 *
 *  License: same as the rest of the core (LGPL 2.1).
 */
#ifndef WAZAMONO_WDT_COMPAT_H
#define WAZAMONO_WDT_COMPAT_H

/* Bring in avr-libc's <avr/wdt.h> first.  This sets that header's include
 * guard, so a later "#include <avr/wdt.h>" from the sketch becomes a no-op and
 * our re-definitions below remain in effect.  It also provides a correct
 * wdt_reset() (the WDR instruction is identical on classic and modern AVR). */
#include <avr/wdt.h>

/* Only modern-AVR parts (Dx / DU / megaAVR-0 / tinyAVR) have the new WDT
 * peripheral and need remapping.  On a classic AVR, avr-libc's <avr/wdt.h> is
 * already Pro Micro compatible, so leave everything untouched there. */
#if defined(WDT_PERIOD_OFF_gc)

#include <stdint.h>

/* Map a classic WDTO_* value (0..9, i.e. 15 ms .. 8 s) to the DU's WDT.CTRLA
 * PERIOD code.  The classic time-out ladder lines up exactly with the modern
 * ladder shifted by +2:
 *     WDTO_15MS (0) -> 16CLK (0x2) = 15.625 ms
 *     WDTO_30MS (1) -> 32CLK (0x3) = 31.25  ms
 *     ...
 *     WDTO_2S   (7) -> 2KCLK (0x9) =  2.0   s
 *     WDTO_8S   (9) -> 8KCLK (0xB) =  8.0   s
 * Anything above WDTO_8S is clamped to the 8 s maximum. */
__attribute__((__unused__))
static __inline__ uint8_t _wazamono_wdt_period_from_wdto(uint8_t wdto) {
    uint8_t period = (uint8_t)(wdto + 2u);
    if (period > WDT_PERIOD_8KCLK_gc) {
        period = WDT_PERIOD_8KCLK_gc;
    }
    return period;
}

/* Write WDT.CTRLA safely.  The data sheet forbids writing while a previous
 * change is still synchronising to the WDT clock domain (SYNCBUSY = 1), so
 * wait for that to clear first, then perform the CCP-protected write.  This
 * also makes back-to-back wdt_enable()/wdt_disable() calls reliable - the
 * naive "_PROTECTED_WRITE only" form can silently drop the second write. */
__attribute__((__unused__))
static __inline__ void _wazamono_wdt_write_ctrla(uint8_t ctrla) {
    while (WDT.STATUS & WDT_SYNCBUSY_bm) {
        /* busy-wait: clears within a few CLK_WDT cycles (~ms at 1.024 kHz) */
    }
    _PROTECTED_WRITE(WDT.CTRLA, ctrla);
}

/* ---- Replace avr-libc's versions with the classic-compatible ones ---- */
#undef wdt_enable
#undef wdt_disable
#define wdt_enable(timeout) \
    _wazamono_wdt_write_ctrla(_wazamono_wdt_period_from_wdto((uint8_t)(timeout)))
#define wdt_disable() \
    _wazamono_wdt_write_ctrla(WDT_PERIOD_OFF_gc)

/* wdt_reset() = the WDR instruction, identical on every AVR.  Keep avr-libc's
 * definition, but provide a fallback in case it was not defined for this part. */
#ifndef wdt_reset
#define wdt_reset() __asm__ __volatile__ ("wdr")
#endif

/* Classic WDTO_* time-out selectors (values 0..9), as on the ATmega32U4.
 * Force the classic values so wdt_enable()'s remap above is correct, even if
 * avr-libc happened to define these names with a different (modern) encoding. */
#undef WDTO_15MS
#undef WDTO_30MS
#undef WDTO_60MS
#undef WDTO_120MS
#undef WDTO_250MS
#undef WDTO_500MS
#undef WDTO_1S
#undef WDTO_2S
#undef WDTO_4S
#undef WDTO_8S
#define WDTO_15MS   0
#define WDTO_30MS   1
#define WDTO_60MS   2
#define WDTO_120MS  3
#define WDTO_250MS  4
#define WDTO_500MS  5
#define WDTO_1S     6
#define WDTO_2S     7
#define WDTO_4S     8
#define WDTO_8S     9

#endif /* WDT_PERIOD_OFF_gc */
#endif /* WAZAMONO_WDT_COMPAT_H */
