/* pins_arduino.h - Variant definition for the Wazamono Kunai (AVR32DU20)
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * DxCore is (C) Spence Konde 2021-2022, open source (LGPL 2.1, see LICENSE.md),
 * based on existing Arduino cores. This variant (C) Workshop Asahi 2026.
 *
 * Board   : Wazamono Kunai (Seeeduino XIAO form factor, AVR32DU20, USB-C)
 * MCU     : AVR32DU20  (20-pin VQFN/SSOP)
 * Clock   : internal 24 MHz OSCHF, crystal-less (boards.txt: clocksource = 0).
 *           USB CLK_USB (48 MHz) = OSCHF + PLL48M, auto-tuned to the USB SOF.
 *           PA0/PA1 therefore stay plain GPIO (D6/D7 = the Serial1 UART).
 *
 *  ===== Pin numbering: Seeeduino XIAO compatible (NONCANONICAL) =====
 *   D#   MCU   XIAO role / notes                             A#,  AIN
 *   D0   PC3   A0 | ~PWM(TCB1 via CCL LUT1)                  A0,  AIN31
 *   D1   PA7   A1 | SPI SS | AC0 OUT | EVOUTA | CLKOUT       A1,  AIN27
 *   D2   PD6   A2 | Serial2 TX (USART1 ALT2) | LUT2-OUT alt  A2,  AIN6
 *   D3   PD7   A3 | Serial2 RX (USART1 ALT2) | EVOUTD        A3,  AIN7
 *   D4   PA2   A4 | SDA (TWI0 default) | ~PWM(TCA0 WO2)      A4,  AIN22
 *   D5   PA3   A5 | SCL (TWI0 default) | ~PWM(TCA0 WO3)      A5,  AIN23
 *   D6   PA0   TX (Serial1 / USART0 default) | ~PWM(WO0)     -    (no ADC)
 *   D7   PA1   RX (Serial1 / USART0 default) | ~PWM(WO1)     -    (no ADC)
 *   D8   PA6   SPI SCK                                       A8,  AIN26
 *   D9   PA5   SPI MISO | ~PWM(TCA0 WO5)                     A9,  AIN25
 *   D10  PA4   SPI MOSI | ~PWM(TCA0 WO4)                     A10, AIN24
 *   D11  PD4   LED_BUILTIN | USB-CDC TX activity LED         A11, AIN4
 *   D12  PD5   USB-CDC RX activity LED                       A12, AIN5
 *   --- not exposed as a numbered Dn (appended so the arrays are complete) ---
 *        PF6   RESET                                index 15
 *        PF7   UPDI                                 index 16  (== PIN_PF7, highest)
 *   (D13/D14 do not exist as pins: indices 13/14 are gaps, so XIAO sketches
 *    that blink the user LED "13" become harmless no-ops. The Seeeduino XIAO
 *    puts its TX LED on 11 and RX LED on 12; rev.3 follows that numbering.)
 *
 *  ===== Peripheral routing (set by this variant + boards.txt) =====
 *   TCA0  -> PORTA (WO0..WO5 = PA0..PA5 = D6,D7,D4,D5,D10,D9) : TCA0_PINS below
 *   TCB1  -> D0 PWM *through CCL LUT1*: D0 is PC3 = LUT1-OUT (default position).
 *           TCB1 runs in 8-bit PWM mode and its internal WO signal feeds LUT1
 *           (INSEL1 = TCB, DS40002548A 30.2.2.1); the LUT passes it to the pin.
 *           analogWrite(D0, x) does all of this transparently (core
 *           wazamono_lutpwm.c). TCB1's own WO pin position is parked on ALT1
 *           (TCB1_PINS): ALT1 = PF5, which does not exist on the 20-pin DU -
 *           the parking spot is entirely harmless here.
 *   millis-> TCB0 : boards.txt passes -DMILLIS_USE_TIMERB0, so TCB1 is free.
 *   tone()-> TCB1 : Tone.cpp auto-selects TCB1 when millis is on TCB0. tone()
 *           and D0 PWM share TCB1: calling tone() suspends D0 PWM only.
 *           Likewise, taking LUT1 over (direct CCL register use) suspends D0
 *           PWM: analogWrite(D0) then falls back to plain digital output.
 *   SPI0  -> DEFAULT (PA4 MOSI / PA5 MISO / PA6 SCK / PA7 SS = D10/D9/D8/D1).
 *           Board is SPI host; run with Client Select Disable (SSD=1) so D1's
 *           level cannot flip host -> client mode. D1 doubles as software CS.
 *   TWI0  -> default (PA2 SDA / PA3 SCL) = D4/D5 = A4/A5 (XIAO I2C position).
 *   USART0-> user-facing "Serial1", DEFAULT position (PA0 TX / PA1 RX = D6/D7,
 *           the XIAO UART position). The object is Serial0; UART0.cpp emits the
 *           Serial1 alias (WAZAMONO_SERIAL1_IS_USART0).
 *   USART1-> user-facing "Serial2", ALT2 (PD6 TX / PD7 RX = D2/D3)
 *           (WAZAMONO_SERIAL2_IS_USART1; the only USART1 position on the DU).
 *   CCL   -> LUT0 is the CustomLogic user unit (IN0/IN1/IN2 = D6/D7/D4,
 *           OUT = D5, alt OUT = D8). LUT1 belongs to D0 PWM (above). LUT2's
 *           alternate output pin is D2 (PD6); its input pins do not exist on
 *           the DU-20. LUT3 has no pins here.
 *   LED   -> PD4 (D11) is LED_BUILTIN and doubles as the USB-CDC TX activity
 *           LED; PD5 (D12) is the USB-CDC RX activity LED (XIAO numbering:
 *           TX = 11, RX = 12; weak hooks in usb_cdc.c, overridden in
 *           wazamono_kunai_init.cpp). Both LEDs are active-LOW (matching the
 *           bootloader's PD4 blink).
 *   Serial-> native USB CDC (USBSerial), Leonardo/Micro convention.
 */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>
#include "timers.h"

/* Board identification: ARDUINO_AVR_KUNAI is THE macro that identifies this
 * board, following the Arduino convention (ARDUINO_AVR_PROMICRO, ARDUINO_UNOWIFIR4).
 * The Arduino build system defines it from boards.txt (build.board=AVR_KUNAI
 * -> -DARDUINO_AVR_KUNAI); the fallback below covers builds outside the IDE
 * (e.g. the bootloader tree, which passes it explicitly). It is used by the
 * core, the bundled libraries, the examples and the bootloader sources alike.
 * MCU identification comes from the compiler (-mmcu): __AVR_AVR32DU20__,
 * plus the core's family tags __AVR_DU__ / _AVR_FAMILY / _AVR_PINCOUNT
 * (core_devices.h).
 * DU_20PIN_PINOUT keeps DU-20 feature assumptions in the core;
 * NONCANONICAL_PIN_NUMBERS tells the core to derive (port,bit) from the tables
 * below instead of assuming pin number == port order. */
#ifndef ARDUINO_AVR_KUNAI
  #define ARDUINO_AVR_KUNAI 1
#endif
#define DU_20PIN_PINOUT
#define NONCANONICAL_PIN_NUMBERS

/* ---- Digital pin number for each MCU pin (XIAO layout, D0..D10 + D13/D14) ---- */
#define PIN_PC3 (0)   // D0  A0 / ~PWM(TCB1 via CCL LUT1)
#define PIN_PA7 (1)   // D1  A1 / SPI SS / AC0 OUT / EVOUTA / CLKOUT
#define PIN_PD6 (2)   // D2  A2 / Serial2 TX (USART1 ALT2) / LUT2-OUT (alt)
#define PIN_PD7 (3)   // D3  A3 / Serial2 RX (USART1 ALT2) / EVOUTD
#define PIN_PA2 (4)   // D4  A4 / SDA (TWI0 default) / TCA0 WO2 / USART0 XCK
#define PIN_PA3 (5)   // D5  A5 / SCL (TWI0 default) / TCA0 WO3 / LUT0-OUT / USART0 XDIR
#define PIN_PA0 (6)   // D6  TX (Serial1 = USART0 default) / TCA0 WO0 / LUT0-IN0
#define PIN_PA1 (7)   // D7  RX (Serial1 = USART0 default) / TCA0 WO1 / LUT0-IN1
#define PIN_PA6 (8)   // D8  SPI SCK / LUT0-OUT (alt)
#define PIN_PA5 (9)   // D9  SPI MISO / TCA0 WO5
#define PIN_PA4 (10)  // D10 SPI MOSI / TCA0 WO4
#define PIN_PD4 (11)  // D11 LED_BUILTIN / USB-CDC TX activity LED
#define PIN_PD5 (12)  // D12 USB-CDC RX activity LED
#define PIN_PF6 (15)  // RESET
#define PIN_PF7 (16)  // UPDI  (highest index -> sets NUM_DIGITAL_PINS = 17)

/* ---- Counts ---- */
#define PINS_COUNT                     (17)  // length of the pin tables (incl. reserved; 13/14 are gaps)
#define NUM_ANALOG_INPUTS              (31)  // highest ADC channel in use is AIN31 (PC3)
// NUM_DIGITAL_PINS / NUM_TOTAL_PINS  -> auto = PIN_PF7 + 1 = 17

#if !defined(LED_BUILTIN)
  #define LED_BUILTIN                  (PIN_PD4)   // D11, on-board LED (active-LOW)
#endif
#define LED_BUILTIN_TX                 (PIN_PD4)   // D11 doubles as USB-CDC TX activity LED
#define LED_BUILTIN_RX                 (PIN_PD5)   // D12 USB-CDC RX activity LED (active-LOW)

/* ---- Event output pins: FIXED by the board's pin-configuration table ----
 * One pin per event output, no alternatives. Libraries (CustomLogic, and the
 * EventSystem library in its Wazamono form) route event outputs to these only.
 *   EVOUTA -> PA7 = D1   (PORTMUX ALT1; PA2/D4 stays I2C SDA)
 *   EVOUTD -> PD7 = D3   (PORTMUX ALT1; default PD2 does not exist on DU-20)
 *   EVOUTF -> none       (PF2 absent; PF7 alternate is the UPDI pin) */
#define WAZAMONO_EVOUTA_PIN            (PIN_PA7)
#define WAZAMONO_EVOUTA_ALT            (1)
#define WAZAMONO_EVOUTD_PIN            (PIN_PD7)
#define WAZAMONO_EVOUTD_ALT            (1)

#ifdef CORE_ATTACH_OLD
  #define EXTERNAL_NUM_INTERRUPTS      (48)
#endif

/* ---- Explicit maps (NONCANONICAL numbering: arithmetic shortcuts cannot be used) ----
 * ADC channels per DS40002576A: PA0/PA1 have no ADC input on the DU-20. */
#define digitalPinToAnalogInput(p)  ( \
    (p) == PIN_PC3 ? 31 : (p) == PIN_PA7 ? 27 : (p) == PIN_PD6 ?  6 : (p) == PIN_PD7 ?  7 : \
    (p) == PIN_PA2 ? 22 : (p) == PIN_PA3 ? 23 : (p) == PIN_PA6 ? 26 : (p) == PIN_PA5 ? 25 : \
    (p) == PIN_PA4 ? 24 : (p) == PIN_PD4 ?  4 : (p) == PIN_PD5 ?  5 : NOT_A_PIN )

#define analogChannelToDigitalPin(p)  ( \
    (p) == 31 ? PIN_PC3 : (p) == 27 ? PIN_PA7 : (p) ==  6 ? PIN_PD6 : (p) ==  7 ? PIN_PD7 : \
    (p) == 22 ? PIN_PA2 : (p) == 23 ? PIN_PA3 : (p) == 26 ? PIN_PA6 : (p) == 25 ? PIN_PA5 : \
    (p) == 24 ? PIN_PA4 : (p) ==  4 ? PIN_PD4 : (p) ==  5 ? PIN_PD5 : NOT_A_PIN )

#define analogInputToDigitalPin(p)        analogChannelToDigitalPin((p) & 0x7F)
#define digitalOrAnalogPinToDigital(p)    (((p) & 0x80) ? analogChannelToDigitalPin((p) & 0x7f) : (((p) <= NUM_DIGITAL_PINS) ? (p) : NOT_A_PIN))
#define portToPinZero(port)               ((port) == PA ? PIN_PA0 : ((port) == PC ? PIN_PC3 : ((port) == PD ? PIN_PD4 : NOT_A_PIN)))

/* ---- PWM ----
 * millis lives on TCB0 (boards.txt -DMILLIS_USE_TIMERB0), leaving TCB1 free for
 * D0 PWM and for tone(). D0 (PC3) is not a TCB WO pin: it is LUT1's output, and
 * analogWrite(D0) delivers TCB1's 8-bit PWM waveform through LUT1 (see the
 * WAZAMONO_TCB1_LUTPWM_* block below and cores/dxcore/wazamono_lutpwm.h).
 * tone(): Tone.cpp routes tone to TCB1 whenever millis is on TCB0, so tone
 * shares TCB1 with D0 PWM - calling tone() suspends D0 PWM only.
 * TCA0 -> PORTA: WO0..WO5 = PA0..PA5 = D6,D7,D4,D5,D10,D9. */
#if defined(MILLIS_USE_TIMERB1)
  #define digitalPinHasPWMTCB(p) (0)                   /* TCB1 is millis; no TCB PWM */
#elif defined(MILLIS_USE_TIMERB0)
  #define digitalPinHasPWMTCB(p) ((p) == PIN_PC3)      /* TCB0=millis -> TCB1 free -> D0 PWM (via LUT1) */
#else
  #define digitalPinHasPWMTCB(p) ((p) == PIN_PC3)
#endif
#define digitalPinHasPWMTCA(p) ( \
    (p) == PIN_PA0 || (p) == PIN_PA1 || (p) == PIN_PA2 || \
    (p) == PIN_PA3 || (p) == PIN_PA4 || (p) == PIN_PA5 )

/* TCA0 routed to PORTA: WO0..WO5 = PA0..PA5 = D6,D7,D4,D5,D10,D9. */
#define TCA0_PINS                       (PORTMUX_TCA0_PORTA_gc)
#define TCB0_PINS                       (0x00)   // TCB0 = millis; WO unused (default PA2 pos)
#define TCB1_PINS                       (0x02)   // PORTMUX.TCBROUTEA bit 1: TCB1 WO parked on ALT1.
                                                 // ALT1 = PF5, which does not exist on the 20-pin DU:
                                                 // the D0 waveform travels through LUT1, and the WO
                                                 // parking spot is harmless. (DS40002548A 17.3.5)

#define PIN_TCA0_WO0_INIT               (PIN_PA0)
#define PIN_TCB0_WO_INIT                (PIN_PA2)   // TCB0 = millis; not enabled for PWM
#define PIN_TCB1_WO_INIT                (NOT_A_PIN) // parked on ALT1/PF5 (absent on DU-20); PWM leaves via LUT1/D0

/* ---- D0 PWM: TCB1 through CCL LUT1 (core mechanism, wazamono_lutpwm.{h,c}) ----
 * analogWrite(D0, x) makes TCB1's PWM8 waveform reach D0 = PC3 = LUT1-OUT:
 *   TCB1 WO (internal) -> LUT1 INSEL1 = TCB -> TRUTH 0xCC -> PC3.
 * The digital_pin_to_timer[] entry for D0 is TIMERB1, which sends analogWrite()
 * down the standard TCB path; the macros below tell the core hook which LUT
 * carries the waveform. Conflicts disable D0 analogWrite automatically:
 * tone()/TCB1 reconfiguration (CNTMODE check) and LUT1 use by other code
 * (signature check in wazamono_lutpwm.c). LUT1 is not offered by the
 * CustomLogic library on the Kunai (its user unit is LUT0), so in practice
 * only direct CCL register use competes for it.
 * CCMPEN stays 0: the CCL taps the internal WO signal, which was measured on
 * silicon to run regardless of that bit (see wazamono_lutpwm.h). The exact
 * LUT1 -> PC3 path used here was part of that measurement. */
#define WAZAMONO_TCB1_LUTPWM_PIN        (PIN_PC3)   /* D0 */
#define WAZAMONO_TCB1_LUTPWM_LUT        (1)         /* LUT1 */
#define WAZAMONO_TCB1_LUTPWM_LUT_ALT    (0)         /* CCLROUTEA: LUT1-OUT default = PC3 */
#define WAZAMONO_TCB1_LUTPWM_CCMPEN     (0)         /* verified on silicon; leave at 0 */

#define digitalPinHasPWM(p)             (digitalPinHasPWMTCB(p) || digitalPinHasPWMTCA(p))

/* ---- SPI (host; chip-selects are user GPIO). Kunai uses the DEFAULT position ---- */
#define SPI_INTERFACES_COUNT            (1)
#define SPI_MUX                         (PORTMUX_SPI0_DEFAULT_gc)  // PA4/PA5/PA6
#define SPI_MUX_PINSWAP_NONE            (PORTMUX_SPI0_NONE_gc)
#define PIN_SPI_MOSI                    (PIN_PA4)   // D10
#define PIN_SPI_MISO                    (PIN_PA5)   // D9
#define PIN_SPI_SCK                     (PIN_PA6)   // D8
#define PIN_SPI_SS                      (PIN_PA7)   // D1 (hardware SS position; host mode + SSD=1, software CS)
#define PIN_SPI_SS_HARDWARE             (PIN_PA7)   // the DEFAULT-position SS pin itself (client mode / SPISlave)

/* ---- TWI 0 (I2C on A4/A5 = D4/D5, XIAO convention; TWI0 DEFAULT position) ---- */
#define PIN_WIRE_SDA                    (PIN_PA2)   // D4 / A4
#define PIN_WIRE_SCL                    (PIN_PA3)   // D5 / A5
#define PIN_WIRE_SDA_PINSWAP_1          (NOT_A_PIN)
#define PIN_WIRE_SCL_PINSWAP_1          (NOT_A_PIN)
#define PIN_WIRE_SDA_PINSWAP_3          (NOT_A_PIN)
#define PIN_WIRE_SCL_PINSWAP_3          (NOT_A_PIN)

/* ---- USART0 -> user-facing "Serial1" = the XIAO D6/D7 UART. DEFAULT position
 * (PA0 TX / PA1 RX; the chip is crystal-less, so PA0/PA1 are free for it).
 * The object is Serial0; UART0.cpp emits the Serial1 alias. ---- */
#ifndef WAZAMONO_SERIAL1_IS_USART0
  #define WAZAMONO_SERIAL1_IS_USART0
#endif
#define HWSERIAL0_MUX                   (0x00 /* PORTMUX_USART0_DEFAULT_gc - PA0/PA1 = D6/D7 */)
#define HWSERIAL0_MUX_PINSWAP_1         (0x01 /* PORTMUX_USART0_ALT1_gc - PA4..PA7 (= SPI) */)
#define HWSERIAL0_MUX_PINSWAP_2         (0x02 /* PORTMUX_USART0_ALT2_gc - PA2/PA3 (= I2C) */)
#define HWSERIAL0_MUX_PINSWAP_3         (0x03 /* PORTMUX_USART0_ALT3_gc - PD4..PD7 (= LEDs/Serial2) */)
#define HWSERIAL0_MUX_PINSWAP_NONE      (0x05)
#define HWSERIAL0_MUX_DEFAULT          (0)   /* Kunai default: USART0 DEFAULT (PA0/PA1 = D6/D7). */
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

/* ---- USART1 -> user-facing "Serial2" on D2/D3 (ALT2 = PD6/PD7, the only
 * USART1 pin position on the DU). The object is Serial1; UART1.cpp renames it
 * to Serial2 (WAZAMONO_SERIAL2_IS_USART1). ---- */
#ifndef WAZAMONO_SERIAL2_IS_USART1
  #define WAZAMONO_SERIAL2_IS_USART1
#endif
#define HWSERIAL1_MUX                   (0x00 /* PORTMUX_USART1_DEFAULT_gc - no pins */)
#define HWSERIAL1_MUX_PINSWAP_1         (0x01 << 3 /* ALT1 absent on DU - placeholder */)
#define HWSERIAL1_MUX_PINSWAP_2         (0x02 << 3 /* PORTMUX_USART1_ALT2_gc - PD6/PD7 = D2/D3 */)
#define HWSERIAL1_MUX_PINSWAP_NONE      (0x03 << 3)
#define HWSERIAL1_MUX_DEFAULT          (2)   /* Kunai default: USART1 ALT2 (PD6/PD7 = D2/D3) */
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

/* ---- Arduino analog aliases. XIAO header = A0..A10 (A6/A7 skipped: PA0/PA1
 * have no ADC); the LED pins are also reachable as A11/A12. ---- */
#define PIN_A0   (PIN_PC3)   // D0
#define PIN_A1   (PIN_PA7)   // D1
#define PIN_A2   (PIN_PD6)   // D2
#define PIN_A3   (PIN_PD7)   // D3
#define PIN_A4   (PIN_PA2)   // D4   SDA
#define PIN_A5   (PIN_PA3)   // D5   SCL
#define PIN_A8   (PIN_PA6)   // D8
#define PIN_A9   (PIN_PA5)   // D9
#define PIN_A10  (PIN_PA4)   // D10
#define PIN_A11  (PIN_PD4)   // D11  LED_BUILTIN
#define PIN_A12  (PIN_PD5)   // D12

/* --- XIAO style number-prefixed digital pin aliases (D0..D12 contiguous) ---
 * D-number == Arduino digital pin number. Internal-only pins (PF6 RESET,
 * PF7 UPDI) are intentionally NOT exposed as Dn. D13/D14 do not exist. */
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
#undef D11
#undef D12
static const uint8_t D0  = PIN_PC3;  // A0 / ~PWM(TCB1 via CCL LUT1)
static const uint8_t D1  = PIN_PA7;  // A1 / SS
static const uint8_t D2  = PIN_PD6;  // A2 / Serial2 TX
static const uint8_t D3  = PIN_PD7;  // A3 / Serial2 RX
static const uint8_t D4  = PIN_PA2;  // A4 / SDA
static const uint8_t D5  = PIN_PA3;  // A5 / SCL
static const uint8_t D6  = PIN_PA0;  // TX (Serial1)
static const uint8_t D7  = PIN_PA1;  // RX (Serial1)
static const uint8_t D8  = PIN_PA6;  // SCK
static const uint8_t D9  = PIN_PA5;  // MISO
static const uint8_t D10 = PIN_PA4;  // MOSI
static const uint8_t D11 = PIN_PD4;  // LED_BUILTIN / USB-CDC TX activity LED
static const uint8_t D12 = PIN_PD5;  // USB-CDC RX activity LED

static const uint8_t A0   = PIN_A0;
static const uint8_t A1   = PIN_A1;
static const uint8_t A2   = PIN_A2;
static const uint8_t A3   = PIN_A3;
static const uint8_t A4   = PIN_A4;
static const uint8_t A5   = PIN_A5;
static const uint8_t A8   = PIN_A8;
static const uint8_t A9   = PIN_A9;
static const uint8_t A10  = PIN_A10;
static const uint8_t A11  = PIN_A11;
static const uint8_t A12  = PIN_A12;

/* Direct ADC channel identifiers (ADC_CH() sets the 0x80 "this is a channel" flag). */
#define AIN4   ADC_CH(4)
#define AIN5   ADC_CH(5)
#define AIN6   ADC_CH(6)
#define AIN7   ADC_CH(7)
#define AIN22  ADC_CH(22)
#define AIN23  ADC_CH(23)
#define AIN24  ADC_CH(24)
#define AIN25  ADC_CH(25)
#define AIN26  ADC_CH(26)
#define AIN27  ADC_CH(27)
#define AIN31  ADC_CH(31)

/* ---- Pin arrays (ARDUINO_MAIN). Indexed by digital pin number (0..16).
 * Indices 13 and 14 are gaps (NOT_A_PIN / NOT_A_PORT): the XIAO's "13" user
 * LED has no counterpart here, so digitalWrite(13, ...) is a harmless no-op. ---- */
#ifdef ARDUINO_MAIN
  const uint8_t digital_pin_to_port[] = {
    PC,         //  0 PC3  D0  A0 / TCB1 PWM via CCL LUT1
    PA,         //  1 PA7  D1  A1 / SS / AC0 OUT / EVOUTA
    PD,         //  2 PD6  D2  A2 / Serial2 TX
    PD,         //  3 PD7  D3  A3 / Serial2 RX / EVOUTD
    PA,         //  4 PA2  D4  A4 / SDA / TCA0 WO2
    PA,         //  5 PA3  D5  A5 / SCL / TCA0 WO3
    PA,         //  6 PA0  D6  TX / TCA0 WO0
    PA,         //  7 PA1  D7  RX / TCA0 WO1
    PA,         //  8 PA6  D8  SCK
    PA,         //  9 PA5  D9  MISO / TCA0 WO5
    PA,         // 10 PA4  D10 MOSI / TCA0 WO4
    PD,         // 11 PD4  D11 LED_BUILTIN / CDC TX LED
    PD,         // 12 PD5  D12 CDC RX LED
    NOT_A_PORT, // 13 (gap - no D13; XIAO user-LED writes become no-ops)
    NOT_A_PORT, // 14 (gap)
    PF,         // 15 PF6  RESET
    PF          // 16 PF7  UPDI
  };

  const uint8_t digital_pin_to_bit_position[] = {
    PIN3_bp,   //  0 PC3  D0
    PIN7_bp,   //  1 PA7  D1
    PIN6_bp,   //  2 PD6  D2
    PIN7_bp,   //  3 PD7  D3
    PIN2_bp,   //  4 PA2  D4
    PIN3_bp,   //  5 PA3  D5
    PIN0_bp,   //  6 PA0  D6
    PIN1_bp,   //  7 PA1  D7
    PIN6_bp,   //  8 PA6  D8
    PIN5_bp,   //  9 PA5  D9
    PIN4_bp,   // 10 PA4  D10
    PIN4_bp,   // 11 PD4  D11
    PIN5_bp,   // 12 PD5  D12
    NOT_A_PIN, // 13 (gap)
    NOT_A_PIN, // 14 (gap)
    PIN6_bp,   // 15 PF6  RESET
    PIN7_bp    // 16 PF7  UPDI
  };

  const uint8_t digital_pin_to_bit_mask[] = {
    PIN3_bm,   //  0 PC3  D0
    PIN7_bm,   //  1 PA7  D1
    PIN6_bm,   //  2 PD6  D2
    PIN7_bm,   //  3 PD7  D3
    PIN2_bm,   //  4 PA2  D4
    PIN3_bm,   //  5 PA3  D5
    PIN0_bm,   //  6 PA0  D6
    PIN1_bm,   //  7 PA1  D7
    PIN6_bm,   //  8 PA6  D8
    PIN5_bm,   //  9 PA5  D9
    PIN4_bm,   // 10 PA4  D10
    PIN4_bm,   // 11 PD4  D11
    PIN5_bm,   // 12 PD5  D12
    NOT_A_PIN, // 13 (gap)
    NOT_A_PIN, // 14 (gap)
    PIN6_bm,   // 15 PF6  RESET
    PIN7_bm    // 16 PF7  UPDI
  };

  /* TCA0 PWM is resolved dynamically from PORTMUX, so TCA0 pins are NOT_ON_TIMER
   * here. D0 (PC3) is marked TIMERB1: it carries TCB1's PWM through CCL LUT1
   * (see WAZAMONO_TCB1_LUTPWM_* above). millis = TCB0. */
  const uint8_t digital_pin_to_timer[] = {
    TIMERB1,      //  0 PC3  D0  (TCB1 PWM delivered via CCL LUT1)
    NOT_ON_TIMER, //  1 PA7  D1
    NOT_ON_TIMER, //  2 PD6  D2
    NOT_ON_TIMER, //  3 PD7  D3
    NOT_ON_TIMER, //  4 PA2  D4  (TCA0 WO2, dynamic; TCB0 WO = millis)
    NOT_ON_TIMER, //  5 PA3  D5  (TCA0 WO3, dynamic)
    NOT_ON_TIMER, //  6 PA0  D6  (TCA0 WO0, dynamic)
    NOT_ON_TIMER, //  7 PA1  D7  (TCA0 WO1, dynamic)
    NOT_ON_TIMER, //  8 PA6  D8
    NOT_ON_TIMER, //  9 PA5  D9  (TCA0 WO5, dynamic)
    NOT_ON_TIMER, // 10 PA4  D10 (TCA0 WO4, dynamic)
    NOT_ON_TIMER, // 11 PD4  D11
    NOT_ON_TIMER, // 12 PD5  D12
    NOT_ON_TIMER, // 13 (gap)
    NOT_ON_TIMER, // 14 (gap)
    NOT_ON_TIMER, // 15 PF6  RESET
    NOT_ON_TIMER  // 16 PF7  UPDI
  };
#endif

/* =================================================================
 *  USB identity   (AVR DU = USB-native part, treated like the 32U4)
 * =================================================================
 *  USBCON enables Arduino's HID / Keyboard / Mouse / etc. on this board.
 *  NOTE: the native USB-CDC descriptor's VID/PID/product string are taken
 *  from cores/dxcore/usb_descriptors.{h,c} (which does NOT include this
 *  file), so the values below are informational only - keep them in sync.
 *  Effective app identity: 0x1209:0x000A, product "Wazamono Kunai".
 *  Obtain a real product VID/PID before release (pid.codes = dev only).
 */
#ifndef USBCON
  #define USBCON
#endif
#ifndef USB_VID
  #define USB_VID                0x1209
#endif
#ifndef USB_PID
  #define USB_PID                0x000A
#endif
#ifndef USB_MANUFACTURER
  #define USB_MANUFACTURER       "Workshop Asahi"
#endif
#ifndef USB_PRODUCT
  #define USB_PRODUCT            "Wazamono Kunai"
#endif

/* =================================================================
 *  Serial -> native USB CDC   (Leonardo/Micro convention)
 * =================================================================
 *  Serial  = USBSerial (on-chip USB CDC)              <- primary / USB serial monitor
 *  Serial1 = USART0   (D6 TX / D7 RX - the XIAO UART position; Uno R4-style
 *                      naming. Alias of Serial0 - both names reach the port.)
 *  Serial2 = USART1   (D2 TX / D3 RX, ALT2 - the DU's only USART1 position.)
 *  Define HAVE_NO_USB_SERIAL_REDIRECT (from boards.txt) to keep Serial==USART0.
 */
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
    #define SERIAL_PORT_HARDWARE    Serial1     /* XIAO D6/D7 hardware UART (USART0) */
  #endif
  #ifndef SERIAL_PORT_HARDWARE_OPEN
    #define SERIAL_PORT_HARDWARE_OPEN  Serial2  /* free hardware UART on D2/D3 (USART1) */
  #endif
#endif

/* ---- analogReference(): EXTERNAL disabled on Kunai ----
 * PD7 (the VREFA pin) is repurposed as D3 (Serial2 RX / EVOUTD), so there is
 * no external reference input. Undefining EXTERNAL turns analogReference(
 * EXTERNAL) into a compile error, steering sketches to the internal references
 * (1.024/2.048/2.5/4.096 V) or VDD. (Included from Arduino.h AFTER EXTERNAL
 * is defined.) */
#ifdef EXTERNAL
  #undef EXTERNAL
#endif

#endif
