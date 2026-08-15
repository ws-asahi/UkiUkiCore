/* wazamono_lutpwm.c - analogWrite() PWM on a CCL LUT output pin, carried by TCB1
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * (C) Workshop Asahi 2026.  DxCore is (C) Spence Konde, LGPL 2.1 (see License.md).
 *
 * See wazamono_lutpwm.h for the design description. Hardware references:
 * DS40002548A 30.2.2.1 (CCL input selection: INSEL value 0x0A = TCBn, where
 * INSEL1 selects TCB1 WO), 30.3.1.1 (LUT configuration registers are
 * enable-protected: configure only while the LUT is disabled), 30.5.10/11
 * (LUTnCTRLB/C layout: INSEL1[3:0] in CTRLB[7:4], INSEL0[3:0] in CTRLB[3:0],
 * INSEL2[3:0] in CTRLC[3:0]), 17.3.2 (CCLROUTEA: one position bit per LUT).
 */

#include "Arduino.h"
#include "wazamono_lutpwm.h"

#if defined(WAZAMONO_TCB1_LUTPWM_ENABLED)

/* Register block of the target LUT: CTRLA, CTRLB, CTRLC, TRUTH - 4 bytes/LUT. */
#define _LUTPWM_BASE   ((volatile uint8_t *)(&CCL.LUT0CTRLA) + (WAZAMONO_TCB1_LUTPWM_LUT << 2))
#define _LUTPWM_CTRLA  (_LUTPWM_BASE[0])
#define _LUTPWM_CTRLB  (_LUTPWM_BASE[1])
#define _LUTPWM_CTRLC  (_LUTPWM_BASE[2])
#define _LUTPWM_TRUTH  (_LUTPWM_BASE[3])

/* This module's exact configuration - doubling as the ownership signature.
 * No other WazamonoCore component ever selects INSEL1 = TCB (CustomLogic has
 * no TCB input source), so a LUT wearing these bytes is ours.
 *   CTRLA: OUTEN | ENABLE (clock source CLK_PER, no filter/edge detector -
 *          the LUT is a purely combinational pass-through, so it responds
 *          asynchronously and the clock is unused).
 *   CTRLB: INSEL1 = TCB1 WO (0x0A), INSEL0 = MASK.
 *   CTRLC: INSEL2 = MASK.
 *   TRUTH: 0xCC = OUT follows IN1 whatever IN0/IN2 read as.               */
#define _LUTPWM_CFG_CTRLA  (CCL_OUTEN_bm | CCL_ENABLE_bm)
#define _LUTPWM_CFG_CTRLB  (0x0A << 4)  /* INSEL1 = TCBn -> TCB1 WO (DS40002548A 30.2.2.1) */
#define _LUTPWM_CFG_CTRLC  (0x00)
#define _LUTPWM_CFG_TRUTH  (0xCC)

#define _LUTPWM_ROUTE_bm   (1 << WAZAMONO_TCB1_LUTPWM_LUT)

static uint8_t _lutpwm_is_ours(void) {
  if (_LUTPWM_CTRLA != _LUTPWM_CFG_CTRLA) return 0;
  if (_LUTPWM_CTRLB != _LUTPWM_CFG_CTRLB) return 0;
  if (_LUTPWM_CTRLC != _LUTPWM_CFG_CTRLC) return 0;
  if (_LUTPWM_TRUTH != _LUTPWM_CFG_TRUTH) return 0;
  uint8_t route = (PORTMUX.CCLROUTEA & _LUTPWM_ROUTE_bm) ? 1 : 0;
  return (route == (WAZAMONO_TCB1_LUTPWM_LUT_ALT)) ? 1 : 0;
}

uint8_t wazamono_tcb1_lutpwm_engage(void) {
  if (_LUTPWM_CTRLA & CCL_ENABLE_bm) {
    /* LUT already enabled: either it is ours from an earlier analogWrite()
     * (nothing to do), or someone else configured it (refuse - the caller
     * falls back to digital output; we never clobber a foreign LUT). */
    return _lutpwm_is_ours();
  }
  /* LUT disabled -> take it. The enable-protected registers are writable now
   * (DS40002548A 30.3.1.1). Route first, then configure, then enable.       */
  #if (WAZAMONO_TCB1_LUTPWM_LUT_ALT)
    PORTMUX.CCLROUTEA |=  _LUTPWM_ROUTE_bm;
  #else
    PORTMUX.CCLROUTEA &= (uint8_t)~_LUTPWM_ROUTE_bm;
  #endif
  _LUTPWM_CTRLB = _LUTPWM_CFG_CTRLB;
  _LUTPWM_CTRLC = _LUTPWM_CFG_CTRLC;
  _LUTPWM_TRUTH = _LUTPWM_CFG_TRUTH;
  _LUTPWM_CTRLA = _LUTPWM_CFG_CTRLA;
  /* Global CCL enable is shared by all LUTs: set it, never clear it here
   * (other LUTs - CustomLogic units - may be running). */
  CCL.CTRLA |= CCL_ENABLE_bm;
  return 1;
}

void wazamono_tcb1_lutpwm_disengage(void) {
  if (_lutpwm_is_ours()) {
    _LUTPWM_CTRLA = 0;  /* disable + release OUTEN; pin returns to the PORT */
  }
}

#endif /* WAZAMONO_TCB1_LUTPWM_ENABLED */

#if defined(WAZAMONO_TCB1_PWMMUX_ENABLED)

/* Same LUT configuration bytes as the single-route block above (and the same
 * ownership-signature idea). INSEL1 = TCB selects TCB1's WO for input 1 on
 * every LUT (DS40002548A 30.2.2.1: the TCB input source of CCL input n is
 * TCBn - input 1 = TCB1 - so the byte is identical for LUT0 and LUT1; both
 * routes were verified on silicon at 44% duty, CCMPEN = 0). */
#define _PWMMUX_CFG_CTRLA  (CCL_OUTEN_bm | CCL_ENABLE_bm)
#define _PWMMUX_CFG_CTRLB  (0x0A << 4)   /* INSEL1 = TCB -> TCB1 WO */
#define _PWMMUX_CFG_CTRLC  (0x00)
#define _PWMMUX_CFG_TRUTH  (0xCC)        /* OUT follows IN1 */

static volatile uint8_t s_pwmmux_active = NOT_A_PIN;

static volatile uint8_t *_pwmmux_lut_base(uint8_t lut) {
  return (volatile uint8_t *)(&CCL.LUT0CTRLA) + ((uint8_t)(lut) << 2);
}

static uint8_t _pwmmux_lut_is_ours(uint8_t lut, uint8_t alt) {
  volatile uint8_t *b = _pwmmux_lut_base(lut);
  if (b[0] != _PWMMUX_CFG_CTRLA) return 0;
  if (b[1] != _PWMMUX_CFG_CTRLB) return 0;
  if (b[2] != _PWMMUX_CFG_CTRLC) return 0;
  if (b[3] != _PWMMUX_CFG_TRUTH) return 0;
  uint8_t route = (PORTMUX.CCLROUTEA & (1 << lut)) ? 1 : 0;
  return (route == alt) ? 1 : 0;
}

static uint8_t _pwmmux_lut_engage(uint8_t lut, uint8_t alt) {
  volatile uint8_t *b = _pwmmux_lut_base(lut);
  if (b[0] & CCL_ENABLE_bm) {
    return _pwmmux_lut_is_ours(lut, alt);  /* ours: done; foreign: refuse */
  }
  if (alt) PORTMUX.CCLROUTEA |=  (uint8_t)(1 << lut);
  else     PORTMUX.CCLROUTEA &= (uint8_t)~(1 << lut);
  b[1] = _PWMMUX_CFG_CTRLB;
  b[2] = _PWMMUX_CFG_CTRLC;
  b[3] = _PWMMUX_CFG_TRUTH;
  b[0] = _PWMMUX_CFG_CTRLA;
  CCL.CTRLA |= CCL_ENABLE_bm;   /* shared enable: set, never cleared here */
  return 1;
}

static void _pwmmux_lut_disengage(uint8_t lut, uint8_t alt) {
  if (_pwmmux_lut_is_ours(lut, alt)) {
    _pwmmux_lut_base(lut)[0] = 0;   /* disable + drop OUTEN; pin -> PORT */
  }
}

/* Release every outlet except `keep_pin` (NOT_A_PIN = release all). */
static void _pwmmux_release_others(uint8_t keep_pin) {
  if (keep_pin != WAZAMONO_TCB1_PWM_LUT0_PIN) {
    _pwmmux_lut_disengage(WAZAMONO_TCB1_PWM_LUT0, WAZAMONO_TCB1_PWM_LUT0_ALT);
  }
  if (keep_pin != WAZAMONO_TCB1_PWM_LUT1_PIN) {
    _pwmmux_lut_disengage(WAZAMONO_TCB1_PWM_LUT1, WAZAMONO_TCB1_PWM_LUT1_ALT);
  }
  if (keep_pin != WAZAMONO_TCB1_PWM_WO_PIN) {
    TCB1.CTRLB &= (uint8_t)~TCB_CCMPEN_bm;   /* WO outlet off */
  }
}

uint8_t wazamono_tcb1_pwm_engage(uint8_t pin) {
  uint8_t okflag = 0;
  if (pin == WAZAMONO_TCB1_PWM_LUT0_PIN) {
    okflag = _pwmmux_lut_engage(WAZAMONO_TCB1_PWM_LUT0, WAZAMONO_TCB1_PWM_LUT0_ALT);
  } else if (pin == WAZAMONO_TCB1_PWM_LUT1_PIN) {
    okflag = _pwmmux_lut_engage(WAZAMONO_TCB1_PWM_LUT1, WAZAMONO_TCB1_PWM_LUT1_ALT);
  } else if (pin == WAZAMONO_TCB1_PWM_WO_PIN) {
    /* Outlet = TCB1's own WO pin. The PORTMUX position was parked by the
     * core's timer init (TCB1_PINS); CCMPEN opens the outlet. The caller
     * (analogWrite) has already verified CNTMODE == PWM8. */
    TCB1.CTRLB |= TCB_CCMPEN_bm;
    okflag = 1;
  }
  if (okflag) {
    _pwmmux_release_others(pin);   /* exclusive: last caller wins */
    s_pwmmux_active = pin;
  }
  return okflag;
}

void wazamono_tcb1_pwm_release(uint8_t pin) {
  if (pin == WAZAMONO_TCB1_PWM_LUT0_PIN) {
    _pwmmux_lut_disengage(WAZAMONO_TCB1_PWM_LUT0, WAZAMONO_TCB1_PWM_LUT0_ALT);
  } else if (pin == WAZAMONO_TCB1_PWM_LUT1_PIN) {
    _pwmmux_lut_disengage(WAZAMONO_TCB1_PWM_LUT1, WAZAMONO_TCB1_PWM_LUT1_ALT);
  } else if (pin == WAZAMONO_TCB1_PWM_WO_PIN) {
    TCB1.CTRLB &= (uint8_t)~TCB_CCMPEN_bm;
  } else {
    return;
  }
  if (s_pwmmux_active == pin) s_pwmmux_active = NOT_A_PIN;
}

uint8_t wazamono_tcb1_pwm_active_pin(void) {
  return s_pwmmux_active;
}

#endif /* WAZAMONO_TCB1_PWMMUX_ENABLED */
