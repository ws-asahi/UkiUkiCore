/* pins_arduino.h - Variant definition for the Wazamono Tachi (AVR64DU32)
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * DxCore is (C) Spence Konde 2021-2022, open source (LGPL 2.1, see LICENSE.md),
 * based on existing Arduino cores. This variant (C) Workshop Asahi 2026.
 *
 * Board   : Wazamono Tachi  (Pro Micro form factor, AVR64DU32, USB-C, Grove)
 * MCU     : AVR64DU32  (32-pin TQFP/VQFN)
 * Clock   : external 24 MHz crystal on PA0/PA1 (XTALHF1/XTALHF2)
 *           -> boards.txt must select an external-HF-crystal clock option, so
 *              CLOCK_SOURCE & 0x03 == 1 and PA0/PA1 are removed from the GPIO map.
 *           -> USB CLK_USB (48 MHz) is still produced by OSCHF + PLL48M and is
 *              auto-tuned to the USB SOF, independently of this crystal.
 *
 *  ===== Pin numbering: Pro Micro compatible (NONCANONICAL) =====
 *   D#   MCU   Pro Micro role / notes
 *   D0   PD7   RX  (Serial1 / USART1 RX, ALT2)      A30, AIN7
 *   D1   PD6   TX  (Serial1 / USART1 TX, ALT2)      A31, AIN6
 *   D2   PA2   SDA (Grove I2C) | Serial0 TX (ALT2)  A32, AIN22
 *   D3   PA3   SCL (Grove I2C) | Serial0 RX (ALT2) | ~PWM(TCB1)  A33, AIN23
 *   D4   PA7   ~AC0 OUT | (hardware SPI SS)         A6,  AIN27
 *   D5   PF0   ~PWM(TCA0 WO0)                       A34, AIN16
 *   D6   PF1   ~PWM(TCA0 WO1)                       A7,  AIN17
 *   D7   PF2   ~PWM(TCA0 WO2)                       A35, AIN18
 *   D8   PF3   ~PWM(TCA0 WO3)                       A8,  AIN19
 *   D9   PF4   ~PWM(TCA0 WO4)                       A9,  AIN20
 *   D10  PF5   ~PWM(TCA0 WO5)                       A10, AIN21
 *   D11..D13   (do not exist - gap, like the 32U4 Pro Micro)
 *   D14  PA5   MISO                                 A36, AIN25
 *   D15  PA6   SCK                                  A37, AIN26
 *   D16  PA4   MOSI                                 A38, AIN24
 *   D17  PD5   RX LED + LED_BUILTIN (on-board, no header)   AIN5
 *   D18  PD0   A0                                   AIN0
 *   D19  PD1   A1                                   AIN1
 *   D20  PD2   A2                                   AIN2
 *   D21  PD3   A3                                   AIN3
 *   D22..D29   (do not exist - gap)
 *   D30  PD4   TX LED (on-board, no header)         AIN4
 *   --- not on the Pro Micro header (appended so the arrays are complete) ---
 *        PC3   VBUS detect (VUSB power domain)      AIN31   index 31
 *        PA0   XTALHF1 (24 MHz crystal)             index 32  (NOT_A_PIN)
 *        PA1   XTALHF2 (24 MHz crystal)             index 33  (NOT_A_PIN)
 *        PF6   RESET                                index 34
 *        PF7   UPDI                                 index 35  (== PIN_PF7, highest)
 *
 *  ===== Peripheral routing (set by this variant + boards.txt) =====
 *   TCA0  -> PORTF (WO0..WO5 = PF0..PF5 = D5..D10)   : TCA0_PINS below
 *   TCB1  -> PA3 (D3) default position               : ~PWM on D3
 *   millis-> TCB0  : boards.txt MUST pass -DMILLIS_USE_TIMERB0 so TCB1 is free for D3 PWM
 *   SPI0  -> default (PA4/PA5/PA6/PA7). Hardware SS (PA7) shared with D4/AC0 OUT;
 *           board is SPI host, so chip-selects are user GPIO (auto-SS not used).
 *   TWI0  -> default (PA2 SDA / PA3 SCL) = the Grove connector.
 *   USART1-> Serial1, ALT2 (PD6 TX / PD7 RX) = D1/D0. (Only usable USART1 position.)
 *   USART0-> Serial2, ALT2 (PA2 TX / PA3 RX) = D2/D3.  <-- shares pins with Grove I2C,
 *           so Serial2 and Wire are mutually exclusive (use one or the other).
 *   Serial-> native USB CDC (USBSerial), Leonardo/Micro convention.
 *
 *   NOTE on names: in DxCore USART0 is "Serial0" and USART1 is "Serial1".
 *   USART1 on the Pro Micro header pins D0/D1 is therefore Serial1 (matching the
 *   classic Pro Micro). The extra UART on D2/D3 (USART0) is exposed to users as
 *   Serial2 (ascending board convention); Serial0 remains as its internal alias.
 */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>
#include "timers.h"

/* Informational pinout tags. DU_32PIN_PINOUT keeps DU-32 feature assumptions in
 * the core; WAZAMONO_TACHI_PINOUT identifies this board. NONCANONICAL_PIN_NUMBERS
 * tells the core to derive (port,bit) from the tables below instead of assuming
 * pin number == port order (our numbering does not follow port order). */
#define DU_32PIN_PINOUT
#define WAZAMONO_TACHI_PINOUT
#define NONCANONICAL_PIN_NUMBERS

         /*##  ### #   #  ###
          #   #  #  ##  # #
          ####   #  # # #  ###
          #      #  #  ##     #
          #     ### #   #  # */
/* Digital pin number for each MCU pin (Pro Micro layout). */
#define PIN_PD7 (0)   // D0  RX  (USART1 RX)
#define PIN_PD6 (1)   // D1  TX  (USART1 TX)
#define PIN_PA2 (2)   // D2  SDA / Serial0 TX
#define PIN_PA3 (3)   // D3  SCL / Serial0 RX / TCB1 PWM
#define PIN_PA7 (4)   // D4  AC0 OUT / SPI SS
#define PIN_PF0 (5)   // D5  TCA0 WO0
#define PIN_PF1 (6)   // D6  TCA0 WO1
#define PIN_PF2 (7)   // D7  TCA0 WO2
#define PIN_PF3 (8)   // D8  TCA0 WO3
#define PIN_PF4 (9)   // D9  TCA0 WO4
#define PIN_PF5 (10)  // D10 TCA0 WO5
//  no  D11..D13               (gap)
#define PIN_PA5 (14)  // D14 MISO
#define PIN_PA6 (15)  // D15 SCK
#define PIN_PA4 (16)  // D16 MOSI
#define PIN_PD5 (17)  // D17 RX LED + LED_BUILTIN
#define PIN_PD0 (18)  // D18 A0
#define PIN_PD1 (19)  // D19 A1
#define PIN_PD2 (20)  // D20 A2
#define PIN_PD3 (21)  // D21 A3
//  no  D22..D29               (gap)
#define PIN_PD4 (30)  // D30 TX LED
#define PIN_PC3 (31)  // VBUS detect (not on header)
#define PIN_PA0 (32)  // XTALHF1 (crystal)
#define PIN_PA1 (33)  // XTALHF2 (crystal)
#define PIN_PF6 (34)  // RESET
#define PIN_PF7 (35)  // UPDI  (highest index -> sets NUM_DIGITAL_PINS = 36)

         /*##   ##   ###  ###  ###  ###
          #   # #  # #      #  #    #
          ####  ####  ###   #  #     ###
          #   # #  #     #  #  #        #
          ####  #  # ####  ###  ###  # */
#define PINS_COUNT                     (36)  // length of the pin tables (incl. gaps/reserved)
#define NUM_ANALOG_INPUTS              (31)  // highest ADC channel in use is AIN31 (PC3)
// NUM_DIGITAL_PINS / NUM_TOTAL_PINS  -> auto = PIN_PF7 + 1 = 36
// NUM_INTERNALLY_USED_PINS           -> auto = 2 (external crystal: PA0, PA1)

#if !defined(LED_BUILTIN)
  #define LED_BUILTIN                  (PIN_PD5)   // D17, RX LED (active-LOW on board)
#endif
#define LED_BUILTIN_RX                 (PIN_PD5)   // D17 RX LED  (Pro Micro convention)
#define LED_BUILTIN_TX                 (PIN_PD4)   // D30 TX LED  (Pro Micro convention)

#ifdef CORE_ATTACH_OLD
  #define EXTERNAL_NUM_INTERRUPTS      (48)
#endif

         /*   #  ###   ### ####   ###   ###
          ## ## #   # #    #   # #   # #
          # # # ##### #    ####  #   #  ###
          #   # #   # #    # #   #   #     #
          #   # #   #  ### #  #   ###   ## */
/* Explicit maps (NONCANONICAL numbering: arithmetic shortcuts cannot be used). */
#define digitalPinToAnalogInput(p)  ( \
    (p) == PIN_PD0 ?  0 : (p) == PIN_PD1 ?  1 : (p) == PIN_PD2 ?  2 : (p) == PIN_PD3 ?  3 : \
    (p) == PIN_PD4 ?  4 : (p) == PIN_PD5 ?  5 : (p) == PIN_PD6 ?  6 : (p) == PIN_PD7 ?  7 : \
    (p) == PIN_PF0 ? 16 : (p) == PIN_PF1 ? 17 : (p) == PIN_PF2 ? 18 : (p) == PIN_PF3 ? 19 : \
    (p) == PIN_PF4 ? 20 : (p) == PIN_PF5 ? 21 : \
    (p) == PIN_PA2 ? 22 : (p) == PIN_PA3 ? 23 : (p) == PIN_PA4 ? 24 : (p) == PIN_PA5 ? 25 : \
    (p) == PIN_PA6 ? 26 : (p) == PIN_PA7 ? 27 : (p) == PIN_PC3 ? 31 : NOT_A_PIN )

#define analogChannelToDigitalPin(p)  ( \
    (p) ==  0 ? PIN_PD0 : (p) ==  1 ? PIN_PD1 : (p) ==  2 ? PIN_PD2 : (p) ==  3 ? PIN_PD3 : \
    (p) ==  4 ? PIN_PD4 : (p) ==  5 ? PIN_PD5 : (p) ==  6 ? PIN_PD6 : (p) ==  7 ? PIN_PD7 : \
    (p) == 16 ? PIN_PF0 : (p) == 17 ? PIN_PF1 : (p) == 18 ? PIN_PF2 : (p) == 19 ? PIN_PF3 : \
    (p) == 20 ? PIN_PF4 : (p) == 21 ? PIN_PF5 : \
    (p) == 22 ? PIN_PA2 : (p) == 23 ? PIN_PA3 : (p) == 24 ? PIN_PA4 : (p) == 25 ? PIN_PA5 : \
    (p) == 26 ? PIN_PA6 : (p) == 27 ? PIN_PA7 : (p) == 31 ? PIN_PC3 : NOT_A_PIN )

#define analogInputToDigitalPin(p)        analogChannelToDigitalPin((p) & 0x7F)
#define digitalOrAnalogPinToDigital(p)    (((p) & 0x80) ? analogChannelToDigitalPin((p) & 0x7f) : (((p) <= NUM_DIGITAL_PINS) ? (p) : NOT_A_PIN))
#define portToPinZero(port)               ((port) == PA ? PIN_PA0 : ((port) == PC ? PIN_PC3 : ((port) == PD ? PIN_PD0 : ((port) == PF ? PIN_PF0 : NOT_A_PIN))))

/* PWM ---------------------------------------------------------------------- */
/* millis lives on TCB0 (boards.txt -DMILLIS_USE_TIMERB0), leaving TCB1 (PA3/D3)
 * available for PWM. The macro tracks whichever TCB is free. */
#if defined(MILLIS_USE_TIMERB0)
  #define digitalPinHasPWMTCB(p) (((p) == PIN_PA3))
#elif defined(MILLIS_USE_TIMERB1)
  #define digitalPinHasPWMTCB(p) (((p) == PIN_PA2))
#else
  #define digitalPinHasPWMTCB(p) (((p) == PIN_PA2) || ((p) == PIN_PA3))
#endif

/* TCA0 is routed to PORTF, so WO0..WO5 land on PF0..PF5 = D5..D10 (contiguous). */
#define TCA0_PINS                       (PORTMUX_TCA0_PORTF_gc)
#define TCB0_PINS                       (0x00)   // TCB0 WO on PA2 (default)
#define TCB1_PINS                       (0x00)   // TCB1 WO on PA3 (default)

#define PIN_TCA0_WO0_INIT               (PIN_PF0)
#define PIN_TCB0_WO_INIT                (PIN_PA2)
#define PIN_TCB1_WO_INIT                (PIN_PA3)

#define digitalPinHasPWM(p)             (digitalPinHasPWMTCB(p) || ((p) >= PIN_PF0 && (p) <= PIN_PF5))

         /*##   ###  ####  ##### #   # #   # #   #
          #   # #   # #   #   #   ## ## #   #  # #
          ####  #   # ####    #   # # # #   #   #
          #     #   # #  #    #   #   # #   #  # #
          #      ###  #   #   #   #   #  ###  #   */
#define SPI_INTERFACES_COUNT            (1)

// SPI 0  (host; chip-selects are user GPIO)
#define SPI_MUX                         (PORTMUX_SPI0_DEFAULT_gc)
#define SPI_MUX_PINSWAP_4               (PORTMUX_SPI0_ALT4_gc)
#define SPI_MUX_PINSWAP_NONE            (PORTMUX_SPI0_NONE_gc)
#define PIN_SPI_MOSI                    (PIN_PA4)   // D16
#define PIN_SPI_MISO                    (PIN_PA5)   // D14
#define PIN_SPI_SCK                     (PIN_PA6)   // D15
#define PIN_SPI_SS                      (PIN_PA7)   // D4 (hardware SS; shared with AC0 OUT)
#define PIN_SPI_MOSI_PINSWAP_4          (PIN_PD4)
#define PIN_SPI_MISO_PINSWAP_4          (PIN_PD5)
#define PIN_SPI_SCK_PINSWAP_4           (PIN_PD6)
#define PIN_SPI_SS_PINSWAP_4            (PIN_PD7)

// TWI 0  (Grove I2C)
#define PIN_WIRE_SDA                    (PIN_PA2)   // D2
#define PIN_WIRE_SCL                    (PIN_PA3)   // D3
#define PIN_WIRE_SDA_PINSWAP_1          (NOT_A_PIN)
#define PIN_WIRE_SCL_PINSWAP_1          (NOT_A_PIN)
#define PIN_WIRE_SDA_PINSWAP_3          (NOT_A_PIN) // ALT3 = PA0/PA1 = crystal, unavailable
#define PIN_WIRE_SCL_PINSWAP_3          (NOT_A_PIN)

// USART0 -> "Serial0", default position ALT2 (PA2/PA3 = D2/D3)
#define HWSERIAL0_MUX                   (0x00 /* PORTMUX_USART0_DEFAULT_gc */)
#define HWSERIAL0_MUX_PINSWAP_1         (0x01 /* PORTMUX_USART0_ALT1_gc */)
#define HWSERIAL0_MUX_PINSWAP_2         (0x02 /* PORTMUX_USART0_ALT2_gc */)
#define HWSERIAL0_MUX_PINSWAP_3         (0x03 /* PORTMUX_USART0_ALT3_gc */)
#define HWSERIAL0_MUX_PINSWAP_NONE      (0x05)
#define HWSERIAL0_MUX_DEFAULT          (2)   /* Tachi default: USART0 ALT2 (PA2/PA3). DEFAULT(PA0/PA1) is the crystal. */
#define PIN_HWSERIAL0_TX                (PIN_PA0)
#define PIN_HWSERIAL0_RX                (PIN_PA1)
#define PIN_HWSERIAL0_XCK               (PIN_PA2)
#define PIN_HWSERIAL0_XDIR              (PIN_PA3)
#define PIN_HWSERIAL0_TX_PINSWAP_1      (PIN_PA4)
#define PIN_HWSERIAL0_RX_PINSWAP_1      (PIN_PA5)
#define PIN_HWSERIAL0_XCK_PINSWAP_1     (PIN_PA6)
#define PIN_HWSERIAL0_XDIR_PINSWAP_1    (PIN_PA7)
#define PIN_HWSERIAL0_TX_PINSWAP_2      (PIN_PA2)
#define PIN_HWSERIAL0_RX_PINSWAP_2      (PIN_PA3)
#define PIN_HWSERIAL0_XCK_PINSWAP_2     (NOT_A_PIN)
#define PIN_HWSERIAL0_XDIR_PINSWAP_2    (NOT_A_PIN)
#define PIN_HWSERIAL0_TX_PINSWAP_3      (PIN_PD4)
#define PIN_HWSERIAL0_RX_PINSWAP_3      (PIN_PD5)
#define PIN_HWSERIAL0_XCK_PINSWAP_3     (PIN_PD6)
#define PIN_HWSERIAL0_XDIR_PINSWAP_3    (PIN_PD7)

// USART1 -> "Serial1", only usable position is ALT2 (PD6/PD7 = D1/D0)
#define HWSERIAL1_MUX                   (0x00 /* PORTMUX_USART1_DEFAULT_gc - no pins */)
#define HWSERIAL1_MUX_PINSWAP_1         (0x01 << 3 /* ALT1 absent on DU - placeholder so the PINSWAP_2 row is built */)
#define HWSERIAL1_MUX_PINSWAP_2         (0x02 << 3 /* PORTMUX_USART1_ALT2_gc */)
#define HWSERIAL1_MUX_PINSWAP_NONE      (0x03 << 3)
#define PIN_HWSERIAL1_TX                (NOT_A_PIN)
#define PIN_HWSERIAL1_RX                (NOT_A_PIN)
#define PIN_HWSERIAL1_XCK               (NOT_A_PIN)
#define PIN_HWSERIAL1_XDIR              (NOT_A_PIN)
#define PIN_HWSERIAL1_TX_PINSWAP_1      (NOT_A_PIN)
#define PIN_HWSERIAL1_RX_PINSWAP_1      (NOT_A_PIN)
#define PIN_HWSERIAL1_XCK_PINSWAP_1     (NOT_A_PIN)
#define PIN_HWSERIAL1_XDIR_PINSWAP_1    (NOT_A_PIN)
#define PIN_HWSERIAL1_TX_PINSWAP_2      (PIN_PD6)
#define PIN_HWSERIAL1_RX_PINSWAP_2      (PIN_PD7)
#define PIN_HWSERIAL1_XCK_PINSWAP_2     (NOT_A_PIN)
#define PIN_HWSERIAL1_XDIR_PINSWAP_2    (NOT_A_PIN)
#define HWSERIAL1_MUX_DEFAULT          (2)   /* DU: USART1 ALT2 (PD6/PD7); USART1 has no usable DEFAULT position */

         /*##  #   #  ###  #     ###   ###      ####  ### #   #  ###
          #   # ##  # #   # #    #   # #         #   #  #  ##  # #
          ##### # # # ##### #    #   # #  ##     ####   #  # # #  ###
          #   # #  ## #   # #    #   # #   #     #      #  #  ##     #
          #   # #   # #   # ####  ###   ###      #     ### #   #  # */
/* Arduino analog aliases. Pro Micro keeps A0..A3 and A6..A10; A4/A5 do not exist
 * on the Pro Micro. The extra ADC-capable pins are exposed as A30..A38. */
#define PIN_A0   (PIN_PD0)
#define PIN_A1   (PIN_PD1)
#define PIN_A2   (PIN_PD2)
#define PIN_A3   (PIN_PD3)
#define PIN_A4   (NOT_A_PIN)
#define PIN_A5   (NOT_A_PIN)
#define PIN_A6   (PIN_PA7)   // D4
#define PIN_A7   (PIN_PF1)   // D6
#define PIN_A8   (PIN_PF3)   // D8
#define PIN_A9   (PIN_PF4)   // D9
#define PIN_A10  (PIN_PF5)   // D10
#define PIN_A30  (PIN_PD7)   // D0
#define PIN_A31  (PIN_PD6)   // D1
#define PIN_A32  (PIN_PA2)   // D2
#define PIN_A33  (PIN_PA3)   // D3
#define PIN_A34  (PIN_PF0)   // D5
#define PIN_A35  (PIN_PF2)   // D7
#define PIN_A36  (PIN_PA5)   // D14
#define PIN_A37  (PIN_PA6)   // D15
#define PIN_A38  (PIN_PA4)   // D16
/* PD5(D17, RX LED/LED_BUILTIN) and PD4(D30, TX LED) are on-board only (no header
 * pin per the Pro Micro pin table), so they get no analog alias. */

/* --- Uno R4 style number-prefixed digital pin aliases (header pins only) ---
 * D-number == Arduino digital pin number. Internal-only pins (PC3 VBUS-detect,
 * PA0/PA1 crystal, PF6 RESET, PF7 UPDI) are intentionally NOT exposed as Dn.
 * #undef guards clear any stray macro definitions, matching the Uno R4 pattern. */
#undef D0
#undef D1
#undef D2
#undef D3
#undef D4
#undef D5
#undef D6
#undef D7
#undef D8
#undef D9
#undef D10
#undef D14
#undef D15
#undef D16
#undef D17
#undef D18
#undef D19
#undef D20
#undef D21
#undef D30
static const uint8_t D0  = PIN_PD7;  // RX
static const uint8_t D1  = PIN_PD6;  // TX
static const uint8_t D2  = PIN_PA2;  // SDA
static const uint8_t D3  = PIN_PA3;  // SCL
static const uint8_t D4  = PIN_PA7;  // SPI SS / AC0 OUT
static const uint8_t D5  = PIN_PF0;
static const uint8_t D6  = PIN_PF1;
static const uint8_t D7  = PIN_PF2;
static const uint8_t D8  = PIN_PF3;
static const uint8_t D9  = PIN_PF4;
static const uint8_t D10 = PIN_PF5;
static const uint8_t D14 = PIN_PA5;  // MISO
static const uint8_t D15 = PIN_PA6;  // SCK
static const uint8_t D16 = PIN_PA4;  // MOSI
static const uint8_t D17 = PIN_PD5;  // RX LED / LED_BUILTIN (on-board only, no header pin)
static const uint8_t D18 = PIN_PD0;  // A0
static const uint8_t D19 = PIN_PD1;  // A1
static const uint8_t D20 = PIN_PD2;  // A2
static const uint8_t D21 = PIN_PD3;  // A3
static const uint8_t D30 = PIN_PD4;  // TX LED (on-board only, no header pin)

static const uint8_t A0   = PIN_A0;
static const uint8_t A1   = PIN_A1;
static const uint8_t A2   = PIN_A2;
static const uint8_t A3   = PIN_A3;
static const uint8_t A6   = PIN_A6;
static const uint8_t A7   = PIN_A7;
static const uint8_t A8   = PIN_A8;
static const uint8_t A9   = PIN_A9;
static const uint8_t A10  = PIN_A10;
static const uint8_t A30  = PIN_A30;
static const uint8_t A31  = PIN_A31;
static const uint8_t A32  = PIN_A32;
static const uint8_t A33  = PIN_A33;
static const uint8_t A34  = PIN_A34;
static const uint8_t A35  = PIN_A35;
static const uint8_t A36  = PIN_A36;
static const uint8_t A37  = PIN_A37;
static const uint8_t A38  = PIN_A38;

/* Direct ADC channel identifiers (ADC_CH() sets the 0x80 "this is a channel" flag). */
#define AIN0   ADC_CH(0)
#define AIN1   ADC_CH(1)
#define AIN2   ADC_CH(2)
#define AIN3   ADC_CH(3)
#define AIN4   ADC_CH(4)
#define AIN5   ADC_CH(5)
#define AIN6   ADC_CH(6)
#define AIN7   ADC_CH(7)
#define AIN16  ADC_CH(16)
#define AIN17  ADC_CH(17)
#define AIN18  ADC_CH(18)
#define AIN19  ADC_CH(19)
#define AIN20  ADC_CH(20)
#define AIN21  ADC_CH(21)
#define AIN22  ADC_CH(22)
#define AIN23  ADC_CH(23)
#define AIN24  ADC_CH(24)
#define AIN25  ADC_CH(25)
#define AIN26  ADC_CH(26)
#define AIN27  ADC_CH(27)
#define AIN31  ADC_CH(31)

         /*##  ### #   #      ###  ####  ####   ###  #   #  ###
          #   #  #  ##  #     #   # #   # #   # #   #  # #  #
          ####   #  # # #     ##### ####  ####  #####   #    ###
          #      #  #  ##     #   # #  #  #  #  #   #   #       #
          #     ### #   #     #   # #   # #   # #   #   #    # */
#ifdef ARDUINO_MAIN
  // Indexed by digital pin number (0..35). Gaps and USB pins are NOT_A_PORT.
  const uint8_t digital_pin_to_port[] = {
    PD,         //  0 PD7  RX/USART1 RX
    PD,         //  1 PD6  TX/USART1 TX
    PA,         //  2 PA2  SDA/Serial0 TX
    PA,         //  3 PA3  SCL/Serial0 RX/TCB1
    PA,         //  4 PA7  AC0 OUT/SS
    PF,         //  5 PF0  TCA0 WO0
    PF,         //  6 PF1  TCA0 WO1
    PF,         //  7 PF2  TCA0 WO2
    PF,         //  8 PF3  TCA0 WO3
    PF,         //  9 PF4  TCA0 WO4
    PF,         // 10 PF5  TCA0 WO5
    NOT_A_PORT, // 11 (gap)
    NOT_A_PORT, // 12 (gap)
    NOT_A_PORT, // 13 (gap)
    PA,         // 14 PA5  MISO
    PA,         // 15 PA6  SCK
    PA,         // 16 PA4  MOSI
    PD,         // 17 PD5  RX LED / LED_BUILTIN
    PD,         // 18 PD0  A0
    PD,         // 19 PD1  A1
    PD,         // 20 PD2  A2
    PD,         // 21 PD3  A3
    NOT_A_PORT, // 22 (gap)
    NOT_A_PORT, // 23 (gap)
    NOT_A_PORT, // 24 (gap)
    NOT_A_PORT, // 25 (gap)
    NOT_A_PORT, // 26 (gap)
    NOT_A_PORT, // 27 (gap)
    NOT_A_PORT, // 28 (gap)
    NOT_A_PORT, // 29 (gap)
    PD,         // 30 PD4  TX LED
    PC,         // 31 PC3  VBUS detect
    PA,         // 32 PA0  XTALHF1
    PA,         // 33 PA1  XTALHF2
    PF,         // 34 PF6  RESET
    PF          // 35 PF7  UPDI
  };

  /* Bit position within the port (for PINnCTRL access). */
  const uint8_t digital_pin_to_bit_position[] = {
    PIN7_bp,   //  0 PD7
    PIN6_bp,   //  1 PD6
    PIN2_bp,   //  2 PA2
    PIN3_bp,   //  3 PA3
    PIN7_bp,   //  4 PA7
    PIN0_bp,   //  5 PF0
    PIN1_bp,   //  6 PF1
    PIN2_bp,   //  7 PF2
    PIN3_bp,   //  8 PF3
    PIN4_bp,   //  9 PF4
    PIN5_bp,   // 10 PF5
    NOT_A_PIN, // 11 (gap)
    NOT_A_PIN, // 12 (gap)
    NOT_A_PIN, // 13 (gap)
    PIN5_bp,   // 14 PA5
    PIN6_bp,   // 15 PA6
    PIN4_bp,   // 16 PA4
    PIN5_bp,   // 17 PD5
    PIN0_bp,   // 18 PD0
    PIN1_bp,   // 19 PD1
    PIN2_bp,   // 20 PD2
    PIN3_bp,   // 21 PD3
    NOT_A_PIN, // 22 (gap)
    NOT_A_PIN, // 23 (gap)
    NOT_A_PIN, // 24 (gap)
    NOT_A_PIN, // 25 (gap)
    NOT_A_PIN, // 26 (gap)
    NOT_A_PIN, // 27 (gap)
    NOT_A_PIN, // 28 (gap)
    NOT_A_PIN, // 29 (gap)
    PIN4_bp,   // 30 PD4
    PIN3_bp,   // 31 PC3
    #if ((CLOCK_SOURCE & 0x03) == 0) // internal clock -> PA0 is a usable GPIO
      PIN0_bp, // 32 PA0
    #else                            // external crystal/clock -> PA0 = XTALHF1
      NOT_A_PIN,
    #endif
    #if ((CLOCK_SOURCE & 0x03) == 1) // external crystal also takes PA1
      NOT_A_PIN,
    #else
      PIN1_bp, // 33 PA1
    #endif
    PIN6_bp,   // 34 PF6 RESET
    PIN7_bp    // 35 PF7 UPDI
  };

  const uint8_t digital_pin_to_bit_mask[] = {
    PIN7_bm,   //  0 PD7
    PIN6_bm,   //  1 PD6
    PIN2_bm,   //  2 PA2
    PIN3_bm,   //  3 PA3
    PIN7_bm,   //  4 PA7
    PIN0_bm,   //  5 PF0
    PIN1_bm,   //  6 PF1
    PIN2_bm,   //  7 PF2
    PIN3_bm,   //  8 PF3
    PIN4_bm,   //  9 PF4
    PIN5_bm,   // 10 PF5
    NOT_A_PIN, // 11 (gap)
    NOT_A_PIN, // 12 (gap)
    NOT_A_PIN, // 13 (gap)
    PIN5_bm,   // 14 PA5
    PIN6_bm,   // 15 PA6
    PIN4_bm,   // 16 PA4
    PIN5_bm,   // 17 PD5
    PIN0_bm,   // 18 PD0
    PIN1_bm,   // 19 PD1
    PIN2_bm,   // 20 PD2
    PIN3_bm,   // 21 PD3
    NOT_A_PIN, // 22 (gap)
    NOT_A_PIN, // 23 (gap)
    NOT_A_PIN, // 24 (gap)
    NOT_A_PIN, // 25 (gap)
    NOT_A_PIN, // 26 (gap)
    NOT_A_PIN, // 27 (gap)
    NOT_A_PIN, // 28 (gap)
    NOT_A_PIN, // 29 (gap)
    PIN4_bm,   // 30 PD4
    PIN3_bm,   // 31 PC3
    #if ((CLOCK_SOURCE & 0x03) == 0)
      PIN0_bm, // 32 PA0
    #else
      NOT_A_PIN,
    #endif
    #if ((CLOCK_SOURCE & 0x03) == 1)
      NOT_A_PIN,
    #else
      PIN1_bm, // 33 PA1
    #endif
    PIN6_bm,   // 34 PF6 RESET
    PIN7_bm    // 35 PF7 UPDI
  };

  /* TCA0 PWM is resolved dynamically from PORTMUX, so TCA0 pins are NOT_ON_TIMER
   * here. Only the TCB outputs are listed (PA2=TCB0, PA3=TCB1). */
  const uint8_t digital_pin_to_timer[] = {
    NOT_ON_TIMER, //  0 PD7
    NOT_ON_TIMER, //  1 PD6
    TIMERB0,      //  2 PA2  (TCB0 - used for millis on this board)
    TIMERB1,      //  3 PA3  (TCB1 - D3 PWM)
    NOT_ON_TIMER, //  4 PA7
    NOT_ON_TIMER, //  5 PF0  (TCA0 WO0, dynamic)
    NOT_ON_TIMER, //  6 PF1  (TCA0 WO1, dynamic)
    NOT_ON_TIMER, //  7 PF2  (TCA0 WO2, dynamic)
    NOT_ON_TIMER, //  8 PF3  (TCA0 WO3, dynamic)
    NOT_ON_TIMER, //  9 PF4  (TCA0 WO4, dynamic)
    NOT_ON_TIMER, // 10 PF5  (TCA0 WO5, dynamic)
    NOT_ON_TIMER, // 11 (gap)
    NOT_ON_TIMER, // 12 (gap)
    NOT_ON_TIMER, // 13 (gap)
    NOT_ON_TIMER, // 14 PA5
    NOT_ON_TIMER, // 15 PA6
    NOT_ON_TIMER, // 16 PA4
    NOT_ON_TIMER, // 17 PD5
    NOT_ON_TIMER, // 18 PD0
    NOT_ON_TIMER, // 19 PD1
    NOT_ON_TIMER, // 20 PD2
    NOT_ON_TIMER, // 21 PD3
    NOT_ON_TIMER, // 22 (gap)
    NOT_ON_TIMER, // 23 (gap)
    NOT_ON_TIMER, // 24 (gap)
    NOT_ON_TIMER, // 25 (gap)
    NOT_ON_TIMER, // 26 (gap)
    NOT_ON_TIMER, // 27 (gap)
    NOT_ON_TIMER, // 28 (gap)
    NOT_ON_TIMER, // 29 (gap)
    NOT_ON_TIMER, // 30 PD4
    NOT_ON_TIMER, // 31 PC3
    NOT_ON_TIMER, // 32 PA0
    NOT_ON_TIMER, // 33 PA1
    NOT_ON_TIMER, // 34 PF6 RESET
    NOT_ON_TIMER  // 35 PF7 UPDI
  };
#endif

/* =================================================================
 *  USB identity   (AVR DU = USB-native part, treated like the 32U4)
 * =================================================================
 *  USBCON enables Arduino's HID / Keyboard / Mouse / etc. on this board.
 *  NOTE: the native USB-CDC descriptor's VID/PID/product string are taken
 *  from cores/dxcore/usb_descriptors.{h,c} (which does NOT include this
 *  file), so the values below are informational only - keep them in sync.
 *  Effective app identity: 0x1209:0x0006, product "Wazamono Tachi".
 *  Obtain a real product VID/PID before release (pid.codes = dev only).
 */
#ifndef USBCON
  #define USBCON
#endif
#ifndef USB_VID
  #define USB_VID                0x1209
#endif
#ifndef USB_PID
  #define USB_PID                0x0006
#endif
#ifndef USB_MANUFACTURER
  #define USB_MANUFACTURER       "Workshop Asahi"
#endif
#ifndef USB_PRODUCT
  #define USB_PRODUCT            "Wazamono Tachi"
#endif

/* =================================================================
 *  Serial -> native USB CDC   (Leonardo/Micro convention)
 * =================================================================
 *  Serial  = USBSerial (on-chip USB CDC)              <- primary / USB serial monitor
 *  Serial1 = USART1   (D0/D1, ALT2 - the Pro Micro hardware UART)
 *  Serial2 = USART0   (D2/D3, ALT2 - extra UART; shares pins with Wire/Grove I2C,
 *                      so Serial2 and Wire are mutually exclusive)
 *  Serial0 is the DxCore-internal name for USART0; the user-facing name is Serial2.
 *  Define HAVE_NO_USB_SERIAL_REDIRECT (from boards.txt) to keep Serial==USART0.
 */

/* Expose USART0 as Serial2 (ascending board convention). The core's Serial0
 * object is kept intact for upstream mergeability; this is only a user-facing
 * alias. Serial2 (USART2) does not exist on the AVR DU, so the name is free. */
#ifndef Serial2
  #define Serial2 Serial0
#endif

#if defined(USB0) && !defined(HAVE_NO_USB_SERIAL_REDIRECT)
  #ifndef Serial
    #define Serial                  USBSerial   /* Serial = native USB CDC   */
  #endif
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR     Serial
  #endif
  #ifndef SERIAL_PORT_USBVIRTUAL
    #define SERIAL_PORT_USBVIRTUAL  Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE    Serial1     /* Pro Micro hardware UART on D0/D1 */
  #endif
  #ifndef SERIAL_PORT_HARDWARE_OPEN
    #define SERIAL_PORT_HARDWARE_OPEN  Serial2  /* extra UART on D2/D3 (USART0) */
  #endif
#endif

#endif