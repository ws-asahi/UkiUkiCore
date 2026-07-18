/* Example 3: High speed 8-bit PWM
 * https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/TakingOverTCA0.md
 *
 * This demonstrates high speed PWM with split mode disabled. The maximum frequency of PWM
 * possible with a full 8 bits of resolution is CLK_PER/256: at the Wazamono boards' 24 MHz,
 * that is 93.75 kHz. The next highest frequency for which perfect 8-bit resolution is
 * possible is half of that. Higher frequencies require lower resolution (see the previous
 * example for one approach, which can also be used for intermediate frequencies) - though
 * if the frequency is constant, varying your input between 0 and the period instead of
 * using map() is desirable, as map may not be smooth. As a further aside, if 93.75 kHz is
 * suitable, there is no need to disable split mode (unless other features are required,
 * like event inputs or buffering).
 */

#if defined(MILLIS_USE_TIMERA0)
  #error "This sketch takes over TCA0, don't use for millis here."
#endif

/* PWM comes out on TCA0 WO2, so the physical pin follows each board's PWM port mux. */
#if defined(WAZAMONO_BOARD_TACHI)
  #define DEMO_OUT_PIN 7                          // D7 (PF2, TCA0 WO2)
  #define DEMO_TCA_MUX PORTMUX_TCA0_PORTF_gc
#elif defined(WAZAMONO_BOARD_TSURUGI)
  #define DEMO_OUT_PIN 9                          // D9 (PD2, TCA0 WO2)
  #define DEMO_TCA_MUX PORTMUX_TCA0_PORTD_gc
#elif defined(WAZAMONO_BOARD_KUNAI)
  #define DEMO_OUT_PIN 3                          // D3 (PA2, TCA0 WO2)
  #define DEMO_TCA_MUX PORTMUX_TCA0_PORTA_gc
#else
  #error "This example supports Wazamono boards only."
#endif


void setup() {
  // We will be outputting PWM on PA2
  pinMode(DEMO_OUT_PIN, OUTPUT);
  takeOverTCA0();

  PORTMUX.TCAROUTEA = (PORTMUX.TCAROUTEA & ~(PORTMUX_TCA0_gm)) | DEMO_TCA_MUX;
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP2EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc); // Single slope PWM mode, PWM on WO2
  TCA0.SINGLE.PER = 0x00FF; // Count all the way up to 0x00FF (255) - 8-bit PWM
  // At 20MHz, this gives ~78.125kHz PWM
  TCA0.SINGLE.CMP2 = 0;
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; // enable the timer with no prescaler
}

void loop() { // Lets generate some output just to prove it works
  static byte pass = 0;
  static unsigned int duty = 255;
  TCA0.SINGLE.CMP2 = duty--; // step down the duty cycle each iteration through loop;
  delay(100);  // so we can see the duty cycle changing over time on the scope/with an LED
  if (!duty) {
    if (pass == 0) {
      // After the first pass, lets go up to 100kHz
      pass = 1;
      duty = 199;
      TCA0.SINGLE.PER = 199;
    } else if (pass == 1) {
      // and now the requested 62 kHz (actually 62.11kHz)
      pass = 2;
      duty = 322;
      TCA0.SINGLE.PER = 322;
    } else { // and back to the beginning.
      pass = 0;
      duty = 255;
      TCA0.SINGLE.PER = 255;
    }
  }
}
