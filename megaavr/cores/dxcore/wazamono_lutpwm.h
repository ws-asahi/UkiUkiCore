/* wazamono_lutpwm.h - analogWrite() PWM on a CCL LUT output pin, carried by TCB1
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * (C) Workshop Asahi 2026.  DxCore is (C) Spence Konde, LGPL 2.1 (see License.md).
 *
 * Some Wazamono boards have a pin whose silkscreen position calls for PWM,
 * but which is neither a TCA0 waveform pin nor a TCB WO pin. When that pin
 * IS a CCL LUT output, the otherwise-idle TCB1 can deliver real hardware PWM
 * there (this is genuine PWM, not software emulation):
 *
 *     TCB1 (8-bit PWM mode)  --- internal "TCB1 WO" signal --->
 *     LUT input 1 (INSEL1 = TCB, DS40002548A 30.2.2.1, value 0x0A) --->
 *     LUT truth table 0xCC (OUT follows IN1) ---> LUTn-OUT pin
 *
 * Boards opt in by defining, in pins_arduino.h:
 *
 *   WAZAMONO_TCB1_LUTPWM_PIN      the Arduino pin number of the LUT-OUT pin.
 *                                 The variant must also set this pin's
 *                                 digital_pin_to_timer[] entry to TIMERB1,
 *                                 which routes analogWrite()/turnOffPWM()
 *                                 through the standard TCB path.
 *   WAZAMONO_TCB1_LUTPWM_LUT      LUT index (0..3) whose OUT pin it is.
 *   WAZAMONO_TCB1_LUTPWM_LUT_ALT  CCLROUTEA bit value for that LUT
 *                                 (0 = default OUT pin, 1 = alternate).
 *   WAZAMONO_TCB1_LUTPWM_CCMPEN   0 or 1. Whether the TCB path may also set
 *                                 TCB1's CCMPEN. See the note below.
 *   TCB1_PINS                     should park TCB1's own WO pin position on
 *                                 the least harmful PORTMUX position (0x02 =
 *                                 ALT1 = PF5; absent on 20-pin parts).
 *
 * Conflict handling ("someone else is using TCB1 or the LUT"):
 *   - TCB1 side: the standard analogWrite() TCB path only acts while TCB1 is
 *     in 8-bit PWM mode. tone() (which uses TCB1 on these boards) or any user
 *     reconfiguration changes CNTMODE, so analogWrite() automatically falls
 *     back to plain digital output until TCB1 is back in PWM8 mode.
 *   - LUT side: wazamono_tcb1_lutpwm_engage() takes the LUT only when it is
 *     disabled, and recognizes its own exact register signature. If the LUT
 *     is enabled with any other configuration (CustomLogic or direct register
 *     use), engage() refuses and analogWrite() falls back to digital output.
 *     Ownership is re-checked on every call, so a LUT taken over mid-flight
 *     is never clobbered, and one released later is re-acquired transparently.
 *
 * NOTE on WAZAMONO_TCB1_LUTPWM_CCMPEN - VERIFIED ON SILICON, keep it 0.
 * The CCMPEN bit description (DS40002548A 24.5.2) scopes the bit to the *pin*:
 * it makes the waveform available on the corresponding WO pin, overriding the
 * port output value regardless of the pin's direction. Table 24-2 ("CCMPEN = 0
 * -> No output") left it open whether the internal "TCB1 WO" signal the CCL
 * taps runs as well, so it was measured on an AVR64DU32 Curiosity Nano:
 *
 *   TCB1 PWM8 (CCMPL = 0xFF, CCMPH = 0x80) -> LUT INSEL1 = TCB -> OUT pin
 *     LUT0 ALT1 -> PA6 (Tsurugi D3):  waveform present with CCMPEN = 0
 *     LUT1      -> PC3 (Kunai   D0):  waveform present with CCMPEN = 0
 *   Both pins behaved identically with CCMPEN = 1, confirming the bit has no
 *   bearing on the internal signal. (The same run also showed the CCL output
 *   enable overriding the port direction: the OUT pin is driven without the
 *   sketch setting it as an output.)
 *
 * The macro is kept as a per-variant escape hatch, but no Wazamono board needs
 * it: setting it to 1 would only add the waveform on TCB1's parked WO pin.
 */

#ifndef WAZAMONO_LUTPWM_H
#define WAZAMONO_LUTPWM_H

#include <avr/io.h>

#if defined(WAZAMONO_TCB1_LUTPWM_PIN) && defined(TCB1) && !defined(MILLIS_USE_TIMERB1)
  #if !defined(WAZAMONO_TCB1_LUTPWM_LUT) || !defined(WAZAMONO_TCB1_LUTPWM_LUT_ALT)
    #error "WAZAMONO_TCB1_LUTPWM_PIN requires WAZAMONO_TCB1_LUTPWM_LUT and _LUT_ALT."
  #endif
  #if !defined(WAZAMONO_TCB1_LUTPWM_CCMPEN)
    #define WAZAMONO_TCB1_LUTPWM_CCMPEN (0)
  #endif
  #define WAZAMONO_TCB1_LUTPWM_ENABLED 1

  #ifdef __cplusplus
  extern "C" {
  #endif

  /* Make sure the LUT carries TCB1's waveform to the pin. Returns 1 when the
   * LUT is ours (already or newly configured), 0 when it is in use by someone
   * else - the caller then falls back to plain digital output. */
  uint8_t wazamono_tcb1_lutpwm_engage(void);

  /* Stop driving the pin from the LUT (turnOffPWM/digitalWrite). Only touches
   * the LUT if it still carries this module's own configuration. */
  void wazamono_tcb1_lutpwm_disengage(void);

  #ifdef __cplusplus
  }
  #endif
#endif

/* =====================================================================
 * Multi-route variant (WAZAMONO_TCB1_PWMMUX): ONE TCB1 waveform, up to
 * THREE selectable outlets, exclusively routed (Tachi, Tsurugi, Kunai):
 *   - a LUT-OUT pin "A"    (WAZAMONO_TCB1_PWM_LUT0_PIN / _LUT0 / _LUT0_ALT)
 *   - a LUT-OUT pin "B"    (WAZAMONO_TCB1_PWM_LUT1_PIN / _LUT1 / _LUT1_ALT)
 *   - TCB1's own WO pin    (WAZAMONO_TCB1_PWM_WO_PIN; PORTMUX position is
 *                           set by TCB1_PINS as usual; outlet = CCMPEN).
 *                           OPTIONAL: leave it undefined (or define it as
 *                           NOT_A_PIN) on boards whose TCB1 WO positions are
 *                           all taken or absent - Kunai: default = PA3 (SCL),
 *                           ALT1 = PF5 (not bonded out on the 20-pin part).
 *   The "_LUT0"/"_LUT1" in the macro names label the two LUT outlets A and
 *   B; the LUT index actually used is the value of _LUT0 / _LUT1 (Kunai
 *   routes outlet A through LUT1 and outlet B through LUT2).
 * analogWrite() on any of the outlet pins claims the route (last caller
 * wins); the previously active outlet is released first, so at most one
 * pin ever carries the waveform. All outlet pins share TCB1's frequency
 * and duty. Foreign LUT configurations are never clobbered: engaging a
 * LUT outlet fails (returns 0 -> caller falls back to digital output)
 * if the LUT is enabled with a configuration other than ours.
 * The single-route block above and this block are mutually exclusive
 * per variant. */
#if defined(WAZAMONO_TCB1_PWMMUX) && defined(TCB1) && !defined(MILLIS_USE_TIMERB1)
  #if defined(WAZAMONO_TCB1_LUTPWM_ENABLED)
    #error "WAZAMONO_TCB1_PWMMUX and WAZAMONO_TCB1_LUTPWM_PIN are mutually exclusive."
  #endif
  #if !defined(WAZAMONO_TCB1_PWM_LUT0_PIN) || !defined(WAZAMONO_TCB1_PWM_LUT1_PIN)
    #error "WAZAMONO_TCB1_PWMMUX requires _LUT0_PIN and _LUT1_PIN."
  #endif
  #if !defined(WAZAMONO_TCB1_PWM_LUT0) || !defined(WAZAMONO_TCB1_PWM_LUT0_ALT) || \
      !defined(WAZAMONO_TCB1_PWM_LUT1) || !defined(WAZAMONO_TCB1_PWM_LUT1_ALT)
    #error "WAZAMONO_TCB1_PWMMUX requires _LUT0/_LUT0_ALT and _LUT1/_LUT1_ALT."
  #endif
  #if !defined(WAZAMONO_TCB1_PWM_WO_PIN)
    #define WAZAMONO_TCB1_PWM_WO_PIN (0xFF)   /* NOT_A_PIN: no direct WO outlet */
  #endif
  /* 1 when the board has a usable TCB1 WO outlet (Tachi D3, Tsurugi D7);
   * 0 when only the two LUT outlets exist (Kunai). Evaluated by the
   * preprocessor, so the WO code paths vanish entirely on Kunai. */
  #if (WAZAMONO_TCB1_PWM_WO_PIN) != 0xFF
    #define WAZAMONO_TCB1_PWM_HAS_WO 1
  #else
    #define WAZAMONO_TCB1_PWM_HAS_WO 0
  #endif
  #define WAZAMONO_TCB1_PWMMUX_ENABLED 1
  #if WAZAMONO_TCB1_PWM_HAS_WO
    #define WAZAMONO_TCB1_PWM_IS_ROUTED_PIN(p) \
        ((p) == WAZAMONO_TCB1_PWM_LUT0_PIN || \
         (p) == WAZAMONO_TCB1_PWM_LUT1_PIN || \
         (p) == WAZAMONO_TCB1_PWM_WO_PIN)
  #else
    #define WAZAMONO_TCB1_PWM_IS_ROUTED_PIN(p) \
        ((p) == WAZAMONO_TCB1_PWM_LUT0_PIN || \
         (p) == WAZAMONO_TCB1_PWM_LUT1_PIN)
  #endif

  #ifdef __cplusplus
  extern "C" {
  #endif

  /* Claim the route for `pin` (one of the outlet pins). Releases whichever
   * outlet was active before. Returns 1 on success, 0 when a needed LUT is
   * owned by someone else (caller falls back to plain digital output). */
  uint8_t wazamono_tcb1_pwm_engage(uint8_t pin);

  /* Release the route if - and only if - `pin` is its current outlet
   * (turnOffPWM / digitalWrite on that pin). */
  void wazamono_tcb1_pwm_release(uint8_t pin);

  /* The pin currently holding the route, or NOT_A_PIN when none does. */
  uint8_t wazamono_tcb1_pwm_active_pin(void);

  #ifdef __cplusplus
  }
  #endif
#endif

#endif /* WAZAMONO_LUTPWM_H */
