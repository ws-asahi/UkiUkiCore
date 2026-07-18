/* Example 4: Quick bit of fun with split mode
 * https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/TakingOverTCA0.md
 *
 * A quick example of how cool split mode can be - You can get two different PWM frequencies out of the same timer.
 * Split mode only has one mode - both halves of the timer independently count down.
 * Here, we've made it even more interesting by using two frequencies almost identical to each other.... they will
 * "beat" against each other with a frequency of 1.43 Hz (366 Hz / 256). You should be able to observe that with a
 * bicolor LED (and appropriate resistor) between the two pins. These have two LEDs with opposite polarity, typically
 * a red and a green, connected between two pins... the question is - what will it look like? How will it be different
 * from a single color LED? Make predictions and then test them. When I (Spence) did this, I was wrong.
 */

#if defined(MILLIS_USE_TIMERA0)
  #error "This sketch takes over TCA0, don't use for millis here."
#endif

/* Split-mode PWM comes out on TCA0 WO2 and WO3, so the physical pins follow
 * each board's PWM port mux. */
#if defined(WAZAMONO_BOARD_TACHI)
  #define DEMO_WO2_PIN 7                          // D7 (PF2)
  #define DEMO_WO3_PIN 8                          // D8 (PF3)
  #define DEMO_TCA_MUX PORTMUX_TCA0_PORTF_gc
#elif defined(WAZAMONO_BOARD_TSURUGI)
  #define DEMO_WO2_PIN 9                          // D9 (PD2)
  #define DEMO_WO3_PIN 10                         // D10 (PD3)
  #define DEMO_TCA_MUX PORTMUX_TCA0_PORTD_gc
#elif defined(WAZAMONO_BOARD_KUNAI)
  #define DEMO_WO2_PIN 3                          // D3 (PA2)
  #define DEMO_WO3_PIN 2                          // D2 (PA3)
  #define DEMO_TCA_MUX PORTMUX_TCA0_PORTA_gc
#else
  #error "This example supports Wazamono boards only."
#endif


void setup() {
  // We will be outputting PWM on PD2 amd PD3
  // No need to enable split mode - core has already done that for us.
  pinMode(DEMO_WO2_PIN, OUTPUT); // TCA0 WO2
  pinMode(DEMO_WO3_PIN, OUTPUT); // TCA0 WO3
  PORTMUX.TCAROUTEA = (PORTMUX.TCAROUTEA & ~(PORTMUX_TCA0_gm)) | DEMO_TCA_MUX;
  TCA0.SPLIT.CTRLB = TCA_SPLIT_LCMP2EN_bm | TCA_SPLIT_HCMP0EN_bm; // PWM on WO2, WO3
  TCA0.SPLIT.LPER = 0xFF; // Count all the way down from 255 on WO0/WO1/WO2
  TCA0.SPLIT.HPER = 0xFE; // Count down from only 254 on WO3/WO4/WO5
  TCA0.SPLIT.LCMP2 = 128; // 50% duty cycle
  TCA0.SPLIT.HCMP0 = 127; // 50% duty cycle
  TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV256_gc | TCA_SPLIT_ENABLE_bm; // enable the timer with prescaler of 256 - slow it down so the phases shift more slowly, but not so slow it would flicker...
}

void loop() {
  // nothing to do here but enjoy your PWM.
}
