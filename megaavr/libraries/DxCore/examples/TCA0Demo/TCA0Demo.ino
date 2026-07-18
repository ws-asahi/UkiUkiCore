/* Example 1: 16-bit PWM in single mode, dual slope with interrupt.
 * https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/TakingOverTCA0.md
 *
 * The whole "ISR adjusting duty cycle" like that probably isn't something you'd actually
 * want to do - but it demonstrates how to configure an ISR on TCA0, which is the point.
 */

#if defined(MILLIS_USE_TIMERA0)
  #error "This sketch takes over TCA0, don't use for millis here."
#endif

unsigned int DutyCycle = 0;
/* PWM comes out on TCA0 WO1, so the physical pin follows each board's PWM port mux. */
#if defined(WAZAMONO_BOARD_TACHI)
  uint8_t OutputPin = 6;                          // D6 (PF1, TCA0 WO1)
  #define DEMO_TCA_MUX PORTMUX_TCA0_PORTF_gc
#elif defined(WAZAMONO_BOARD_TSURUGI)
  uint8_t OutputPin = 6;                          // D6 (PD1, TCA0 WO1)
  #define DEMO_TCA_MUX PORTMUX_TCA0_PORTD_gc
#elif defined(WAZAMONO_BOARD_KUNAI)
  uint8_t OutputPin = 5;                          // D5 (PA1, TCA0 WO1; shared with SCL)
  #define DEMO_TCA_MUX PORTMUX_TCA0_PORTA_gc
#else
  #error "This example supports Wazamono boards only."
#endif


void setup() {
  pinMode(OutputPin, OUTPUT);
  takeOverTCA0(); // this replaces disabling and resettng the timer, required previously.
  PORTMUX.TCAROUTEA   = (PORTMUX.TCAROUTEA & ~(PORTMUX_TCA0_gm)) | DEMO_TCA_MUX; // route WO to this board's PWM port
  TCA0.SINGLE.CTRLB   = (TCA_SINGLE_CMP1EN_bm | TCA_SINGLE_WGMODE_DSBOTTOM_gc); // Dual slope PWM mode OVF interrupt at BOTTOM, PWM on WO1.
  TCA0.SINGLE.PER     = 0xFFFF;               // Count all the way up to 0xFFFF.
  //                                             At 20MHz, this gives ~152Hz PWM with no prescaling.
  TCA0.SINGLE.CMP1    = DutyCycle;            // 0 - 65535
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;    // enable overflow interrupt
  TCA0.SINGLE.CTRLA   = TCA_SINGLE_ENABLE_bm; // enable the timer with no prescaler
}

void loop() { // Not even going to do anything in here
}

ISR(TCA0_OVF_vect) {    // on overflow, we will increment TCA0.CMP0, this will happen after every full cycle - a little over 7 minutes.
  TCA0.SINGLE.CMP1      = DutyCycle++; // Because we are in Dual Slope Bottom mode, OVF fires at BOTTOM, at end, not TOP, in middle of the pulse.
  TCA0.SINGLE.INTFLAGS  = TCA_SINGLE_OVF_bm; // Always remember to clear the interrupt flags, otherwise the interrupt will fire continually!
}
