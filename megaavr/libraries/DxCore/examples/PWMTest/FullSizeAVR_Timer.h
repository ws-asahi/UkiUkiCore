/* Timer/pin tables for the PWMTest diagnostic - WazamonoCore (AVR DU-series).
 *
 * The DU-series has three PWM-capable timers: TCA0, TCB0 and TCB1.
 * The tables below list, for each PORTMUX option, which physical pin carries
 * each waveform output - with per-board adjustments for pins that are not
 * bonded out (Kunai) or otherwise unavailable (crystal, VBUS detect).
 */

// *INDENT-OFF*

#define _tca0 0x00
#define _tcb0 0x10
#define _tcb1 0x11

const PROGMEM_MAPPED uint8_t MyTimers[] = {
  _tca0,
  _tcb0,
  _tcb1,
  255
};

/* TCA0 waveform outputs WO0-WO5 for each TCAROUTEA value (one 6-entry row per
 * port, PORTA..PORTG, so the index is 6*mux + channel). The DU routes TCA0
 * only to PORTA, PORTC (WO3 = PC3 only), PORTD and PORTF; the other rows are
 * padding so that indexing matches the PORTMUX register values.
 * PC3 is deliberately excluded: it is the VBUS divider input on Tachi/Kunai
 * on Tachi/Kunai, so PWM must not be driven there (on Tsurugi PC3 = D7, a
 * plain GPIO, but TCA0 is muxed to PORTD so it is not a PWM pin anyway).
 */
const PROGMEM_MAPPED uint8_t TCA0pinsets[] = {
  /* PORTA (mux 0) */
  #if defined(WAZAMONO_BOARD_KUNAI)
    PIN_PA0,   PIN_PA1,    /* D4, D5 - internal oscillator, pins are free */
  #else
    NOT_A_PIN, NOT_A_PIN,  /* PA0/PA1 carry the 24 MHz crystal on Tachi/Tsurugi */
  #endif
  PIN_PA2,   PIN_PA3,   PIN_PA4,   PIN_PA5,
  /* PORTB (mux 1) - no PORTB on the DU-series */
  NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN,
  /* PORTC (mux 2) - only WO3 = PC3 exists, and it is reserved on all boards */
  NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN,
  /* PORTD (mux 3) */
  #if defined(WAZAMONO_BOARD_KUNAI)
    NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, /* PD0-PD3 not bonded out on Kunai */
  #else
    PIN_PD0,   PIN_PD1,   PIN_PD2,   PIN_PD3,
  #endif
  PIN_PD4,   PIN_PD5,
  /* PORTE (mux 4) - no PORTE on the DU-series */
  NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN,
  /* PORTF (mux 5) */
  #if defined(WAZAMONO_BOARD_KUNAI)
    NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, /* PF0-PF5 not bonded out on Kunai */
  #else
    PIN_PF0,   PIN_PF1,   PIN_PF2,   PIN_PF3,   PIN_PF4,   PIN_PF5,
  #endif
  /* PORTG (mux 6) - no PORTG on the DU-series */
  NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN, NOT_A_PIN
};

/* TCBn waveform output pins: one (default, alternate) pair per TCB, indexed
 * as (timernum << 1) + mux. TCB0: PA2 / PF4, TCB1: PA3 / PF5.
 */
const PROGMEM_MAPPED uint8_t TCBpinsets[] = {
  PIN_PA2,                /* TCB0 default WO (Tachi: D2 / Tsurugi: D18 / Kunai: D3) */
  #if defined(WAZAMONO_BOARD_KUNAI)
    NOT_A_PIN,            /* TCB0 alt WO = PF4, not bonded out on Kunai */
  #else
    PIN_PF4,              /* TCB0 alt WO (Tachi: D9 / Tsurugi: D4) */
  #endif
  PIN_PA3,                /* TCB1 default WO (Tachi: D3 / Tsurugi: D19 / Kunai: D2) */
  #if defined(WAZAMONO_BOARD_KUNAI)
    NOT_A_PIN,            /* TCB1 alt WO = PF5, not bonded out on Kunai */
  #else
    PIN_PF5,              /* TCB1 alt WO (Tachi: D10 / Tsurugi: D3) */
  #endif
};
