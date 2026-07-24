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
 * NOTE on WAZAMONO_TCB1_LUTPWM_CCMPEN: the CCMPEN bit description
 * (DS40002548A 24.5.2) scopes the bit to the *pin*: it makes the waveform
 * output available on the corresponding WO pin, overriding the port output
 * value regardless of the pin's direction. The CCL taps the internal
 * "TCB1 WO" signal, which this module expects to run regardless of CCMPEN.
 * However, Table 24-2 ("CCMPEN = 0 -> No output") is ambiguous about the
 * internal signal, so this expectation MUST be verified on hardware once.
 * If the LUT turns out to need CCMPEN = 1, set WAZAMONO_TCB1_LUTPWM_CCMPEN
 * to 1 in the variant - at the cost that the parked WO pin (PF5), where it
 * physically exists, is then driven with the waveform whenever this PWM is
 * active (CCMPEN overrides the pin regardless of direction).
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

#endif /* WAZAMONO_LUTPWM_H */
