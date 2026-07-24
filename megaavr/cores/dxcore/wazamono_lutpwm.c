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
