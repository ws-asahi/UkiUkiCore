/* pins_arduino.h - Variant definition for the Wazamono Tsurugi (AVR64DU32)
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * DxCore is (C) Spence Konde 2021-2022, open source (LGPL 2.1, see LICENSE.md),
 * based on existing Arduino cores. This variant (C) Workshop Asahi 2026.
 *
 * Board   : Wazamono Tsurugi (Arduino Uno R3 form factor, AVR64DU32, USB-C)
 * MCU     : AVR64DU32  (32-pin TQFP/VQFN)
 * Clock   : external 24 MHz crystal on PA0/PA1 (XTALHF1/XTALHF2)
 *           -> boards.txt must select an external-HF-crystal clock option, so
 *              CLOCK_SOURCE & 0x03 == 1 and PA0/PA1 are removed from the GPIO map.
 *           -> USB CLK_USB (48 MHz) is still produced by OSCHF + PLL48M and is
 *              auto-tuned to the USB SOF, independently of this crystal.
 *
 *  ===== Pin numbering: Arduino Uno R3 compatible (NONCANONICAL) =====
 *   D#   MCU   Uno R3 role / notes                          A#,  AIN
 *   D0   PA5   RX  (Serial0 / USART0 RX, ALT1)              A6,  AIN25
 *   D1   PA4   TX  (Serial0 / USART0 TX, ALT1)              A7,  AIN24
 *   D2   PA6   (USART0 XCK, ALT1)                           A8,  AIN26
 *   D3   PA7   ~AC0 OUT | EVOUTA | (USART0 XDIR, ALT1)      A9,  AIN27
 *   D4   PF4   ~PWM(TCB0, ALT1)                             A11, AIN20
 *   D5   PD0   ~PWM(TCA0 WO0) | CCL                         A13, AIN0
 *   D6   PD1   ~PWM(TCA0 WO1) | CCL                         A14, AIN1
 *   D7   PF5   (general I/O)                                A12, AIN21
 *   D8   PC3   (general I/O, VUSB power domain)             A10, AIN31
 *   D9   PD2   ~PWM(TCA0 WO2) | CCL | AC0 AINP0 | EVOUTD    A15, AIN2
 *   D10  PD3   ~PWM(TCA0 WO3) | CCL | AC0 AINN0 | (SS)      A16, AIN3
 *   D11  PD4   ~PWM(TCA0 WO4) | SPI MOSI                    A17, AIN4
 *   D12  PD5   ~PWM(TCA0 WO5) | SPI MISO                    A18, AIN5
 *   D13  PD6   SPI SCK | LED_BUILTIN                        A19, AIN6
 *   D14  PF0   A0 | CCL                                     A0,  AIN16
 *   D15  PF1   A1 | CCL                                     A1,  AIN17
 *   D16  PF2   A2 | CCL | EVOUTF                            A2,  AIN18
 *   D17  PF3   A3 | CCL                                     A3,  AIN19
 *   D18  PA2   A4 | SDA (I2C)                               A4,  AIN22
 *   D19  PA3   A5 | SCL (I2C)                               A5,  AIN23
 *   --- not exposed as a numbered Dn (appended so the arrays are complete) ---
 *        PD7   AREF (VREFA)                         index 20  (reserved)
 *        PA0   XTALHF1 (24 MHz crystal)             index 21  (NOT_A_PIN)
 *        PA1   XTALHF2 (24 MHz crystal)             index 22  (NOT_A_PIN)
 *        PF6   RESET                                index 23
 *        PF7   UPDI                                 index 24  (== PIN_PF7, highest)
 *
 *  ===== Peripheral routing (set by this variant + boards.txt) =====
 *   TCA0  -> PORTD (WO0..WO5 = PD0,PD1,PD2,PD3,PD4,PD5 = D5,D6,D9,D10,D11,D12)
 *   TCB0  -> ALT1 (PF4 = D4)  : ~PWM on D4
 *   millis-> TCB1 : boards.txt MUST pass -DMILLIS_USE_TIMERB1 so TCB0 is free for D4 PWM
 *   SPI0  -> ALT4 (PD4 MOSI / PD5 MISO / PD6 SCK = D11/D12/D13). Board is SPI host;
 *           default CS exposed as PIN_SPI_SS = PD3 (D10, Uno convention). The ALT4
 *           hardware SS (PD7) is repurposed as AREF, so software chip-select is used.
 *   TWI0  -> default (PA2 SDA / PA3 SCL) = D18/D19 = A4/A5 (Uno I2C convention).
 *   USART0-> Serial0, ALT1 (PA4 TX / PA5 RX) = D1/D0. (The Uno R3 D0/D1 UART.)
 *   USART1-> exists on the AVR DU but has NO usable pin position on Tsurugi
 *           (DU USART1 is only PD6/PD7, which are SPI SCK / AREF here). Inert.
 *   Serial-> native USB CDC (USBSerial), Leonardo/Micro convention.
 *
 *   NOTE on names: the Uno R3 D0/D1 UART is wired to USART0 on this board, so the
 *   hardware UART is "Serial0" (not "Serial1" as on the Tachi). "Serial" is the
 *   native USB CDC (serial monitor), so day-to-day Serial.print() behaves exactly
 *   like a classic Uno; use "Serial0" only to talk to an external device on D0/D1.
 */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>
#include "timers.h"

/* Informational pinout tags. DU_32PIN_PINOUT keeps DU-32 feature assumptions in
 * the core; WAZAMONO_TSURUGI_PINOUT identifies this board. NONCANONICAL_PIN_NUMBERS
 * tells the core to derive (port,bit) from the tables below instead of assuming
 * pin number == port order (our numbering does not follow port order). */
#define DU_32PIN_PINOUT
#define WAZAMONO_TSURUGI_PINOUT
#define NONCANONICAL_PIN_NUMBERS

/* ---- Digital pin number for each MCU pin (Uno R3 layout, D0..D19 contiguous) ---- */
#define PIN_PA5 (0)   // D0  RX  (USART0 RX, ALT1)
#define PIN_PA4 (1)   // D1  TX  (USART0 TX, ALT1)
#define PIN_PA6 (2)   // D2  (USART0 XCK)
#define PIN_PA7 (3)   // D3  AC0 OUT / EVOUTA
#define PIN_PF4 (4)   // D4  TCB0 PWM (ALT1)
#define PIN_PD0 (5)   // D5  TCA0 WO0
#define PIN_PD1 (6)   // D6  TCA0 WO1
#define PIN_PF5 (7)   // D7  general I/O
#define PIN_PC3 (8)   // D8  general I/O (VUSB domain)
#define PIN_PD2 (9)   // D9  TCA0 WO2
#define PIN_PD3 (10)  // D10 TCA0 WO3 / SS (Uno convention)
#define PIN_PD4 (11)  // D11 TCA0 WO4 / SPI MOSI
#define PIN_PD5 (12)  // D12 TCA0 WO5 / SPI MISO
#define PIN_PD6 (13)  // D13 SPI SCK / LED_BUILTIN
#define PIN_PF0 (14)  // D14 A0
#define PIN_PF1 (15)  // D15 A1
#define PIN_PF2 (16)  // D16 A2
#define PIN_PF3 (17)  // D17 A3
#define PIN_PA2 (18)  // D18 A4 / SDA
#define PIN_PA3 (19)  // D19 A5 / SCL
#define PIN_PD7 (20)  // AREF (VREFA) - reserved, no Dn alias
#define PIN_PA0 (21)  // XTALHF1 (crystal)
#define PIN_PA1 (22)  // XTALHF2 (crystal)
#define PIN_PF6 (23)  // RESET
#define PIN_PF7 (24)  // UPDI  (highest index -> sets NUM_DIGITAL_PINS = 25)

/* ---- Counts ---- */
#define PINS_COUNT                     (25)  // length of the pin tables (incl. reserved)
#define NUM_ANALOG_INPUTS              (31)  // highest ADC channel in use is AIN31 (PC3)
// NUM_DIGITAL_PINS / NUM_TOTAL_PINS  -> auto = PIN_PF7 + 1 = 25
// NUM_INTERNALLY_USED_PINS           -> auto = 2 (external crystal: PA0, PA1)

#if !defined(LED_BUILTIN)
  #define LED_BUILTIN                  (PIN_PD6)   // D13, on-board LED (Uno convention)
#endif

#ifdef CORE_ATTACH_OLD
  #define EXTERNAL_NUM_INTERRUPTS      (48)
#endif

/* ---- Explicit maps (NONCANONICAL numbering: arithmetic shortcuts cannot be used) ---- */
#define digitalPinToAnalogInput(p)  ( \
    (p) == PIN_PD0 ?  0 : (p) == PIN_PD1 ?  1 : (p) == PIN_PD2 ?  2 : (p) == PIN_PD3 ?  3 : \
    (p) == PIN_PD4 ?  4 : (p) == PIN_PD5 ?  5 : (p) == PIN_PD6 ?  6 : \
    (p) == PIN_PF0 ? 16 : (p) == PIN_PF1 ? 17 : (p) == PIN_PF2 ? 18 : (p) == PIN_PF3 ? 19 : \
    (p) == PIN_PF4 ? 20 : (p) == PIN_PF5 ? 21 : \
    (p) == PIN_PA2 ? 22 : (p) == PIN_PA3 ? 23 : (p) == PIN_PA4 ? 24 : (p) == PIN_PA5 ? 25 : \
    (p) == PIN_PA6 ? 26 : (p) == PIN_PA7 ? 27 : (p) == PIN_PC3 ? 31 : NOT_A_PIN )

#define analogChannelToDigitalPin(p)  ( \
    (p) ==  0 ? PIN_PD0 : (p) ==  1 ? PIN_PD1 : (p) ==  2 ? PIN_PD2 : (p) ==  3 ? PIN_PD3 : \
    (p) ==  4 ? PIN_PD4 : (p) ==  5 ? PIN_PD5 : (p) ==  6 ? PIN_PD6 : \
    (p) == 16 ? PIN_PF0 : (p) == 17 ? PIN_PF1 : (p) == 18 ? PIN_PF2 : (p) == 19 ? PIN_PF3 : \
    (p) == 20 ? PIN_PF4 : (p) == 21 ? PIN_PF5 : \
    (p) == 22 ? PIN_PA2 : (p) == 23 ? PIN_PA3 : (p) == 24 ? PIN_PA4 : (p) == 25 ? PIN_PA5 : \
    (p) == 26 ? PIN_PA6 : (p) == 27 ? PIN_PA7 : (p) == 31 ? PIN_PC3 : NOT_A_PIN )

#define analogInputToDigitalPin(p)        analogChannelToDigitalPin((p) & 0x7F)
#define digitalOrAnalogPinToDigital(p)    (((p) & 0x80) ? analogChannelToDigitalPin((p) & 0x7f) : (((p) <= NUM_DIGITAL_PINS) ? (p) : NOT_A_PIN))
#define portToPinZero(port)               ((port) == PA ? PIN_PA0 : ((port) == PC ? PIN_PC3 : ((port) == PD ? PIN_PD0 : ((port) == PF ? PIN_PF0 : NOT_A_PIN))))

/* ---- PWM ----
 * millis lives on TCB1 (boards.txt -DMILLIS_USE_TIMERB1), leaving TCB0 (PF4/D4)
 * available for PWM. TCA0 is routed to PORTD, so its WO0..WO5 land on
 * PD0,PD1,PD2,PD3,PD4,PD5 = D5,D6,D9,D10,D11,D12 (NOT contiguous in pin order). */
#if defined(MILLIS_USE_TIMERB1)
  #define digitalPinHasPWMTCB(p) ((p) == PIN_PF4)      /* TCB0 free -> D4 PWM */
#elif defined(MILLIS_USE_TIMERB0)
  #define digitalPinHasPWMTCB(p) (0)                   /* TCB0 is millis; TCB1 WO unused here */
#else
  #define digitalPinHasPWMTCB(p) ((p) == PIN_PF4)
#endif
#define digitalPinHasPWMTCA(p) ( \
    (p) == PIN_PD0 || (p) == PIN_PD1 || (p) == PIN_PD2 || \
    (p) == PIN_PD3 || (p) == PIN_PD4 || (p) == PIN_PD5 )

/* TCA0 routed to PORTD: WO0..WO5 = PD0..PD5 = D5,D6,D9,D10,D11,D12. */
#define TCA0_PINS                       (PORTMUX_TCA0_PORTD_gc)
#define TCB0_PINS                       (0x01)   // TCB0 WO -> ALT1 (PF4 = D4)
#define TCB1_PINS                       (0x00)   // TCB1 WO default (PA3); TCB1 = millis, WO unused

#define PIN_TCA0_WO0_INIT               (PIN_PD0)
#define PIN_TCB0_WO_INIT                (PIN_PF4)
#define PIN_TCB1_WO_INIT                (PIN_PA3)

#define digitalPinHasPWM(p)             (digitalPinHasPWMTCB(p) || digitalPinHasPWMTCA(p))

/* ---- SPI (host; chip-selects are user GPIO). Tsurugi routes SPI to ALT4 (PORTD) ---- */
#define SPI_INTERFACES_COUNT            (1)
#define SPI_MUX                         (PORTMUX_SPI0_ALT4_gc)    // PD4/PD5/PD6
#define SPI_MUX_PINSWAP_4               (PORTMUX_SPI0_ALT4_gc)
#define SPI_MUX_PINSWAP_NONE            (PORTMUX_SPI0_NONE_gc)
#define PIN_SPI_MOSI                    (PIN_PD4)   // D11
#define PIN_SPI_MISO                    (PIN_PD5)   // D12
#define PIN_SPI_SCK                     (PIN_PD6)   // D13
#define PIN_SPI_SS                      (PIN_PD3)   // D10 (Uno convention; software CS, host mode)
#define PIN_SPI_MOSI_PINSWAP_4          (PIN_PD4)
#define PIN_SPI_MISO_PINSWAP_4          (PIN_PD5)
#define PIN_SPI_SCK_PINSWAP_4           (PIN_PD6)
#define PIN_SPI_SS_PINSWAP_4            (PIN_PD3)

/* ---- TWI 0 (I2C on A4/A5, Uno convention) ---- */
#define PIN_WIRE_SDA                    (PIN_PA2)   // D18 / A4
#define PIN_WIRE_SCL                    (PIN_PA3)   // D19 / A5
#define PIN_WIRE_SDA_PINSWAP_1          (NOT_A_PIN)
#define PIN_WIRE_SCL_PINSWAP_1          (NOT_A_PIN)
#define PIN_WIRE_SDA_PINSWAP_3          (NOT_A_PIN)
#define PIN_WIRE_SCL_PINSWAP_3          (NOT_A_PIN)

/* ---- USART0 -> "Serial0" = the Uno R3 D0/D1 UART. Default position ALT1 (PA4/PA5) ---- */
#define HWSERIAL0_MUX                   (0x00 /* PORTMUX_USART0_DEFAULT_gc - PA0/PA1 = crystal */)
#define HWSERIAL0_MUX_PINSWAP_1         (0x01 /* PORTMUX_USART0_ALT1_gc - PA4..PA7 */)
#define HWSERIAL0_MUX_PINSWAP_2         (0x02 /* PORTMUX_USART0_ALT2_gc - PA2/PA3 (= I2C) */)
#define HWSERIAL0_MUX_PINSWAP_3         (0x03 /* PORTMUX_USART0_ALT3_gc - PD4..PD7 (= SPI/AREF) */)
#define HWSERIAL0_MUX_PINSWAP_NONE      (0x05)
#define HWSERIAL0_MUX_DEFAULT          (1)   /* Tsurugi default: USART0 ALT1 (PA4/PA5). DEFAULT(PA0/PA1) is the crystal. */
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

/* ---- USART1 -> "Serial1": instantiated by the core, but NO usable pin position on
 *   Tsurugi (DU USART1 is only PD6/PD7 (ALT2) = SPI SCK / AREF here). Left inert. ---- */
#define HWSERIAL1_MUX                   (0x00 /* PORTMUX_USART1_DEFAULT_gc - no pins */)
#define HWSERIAL1_MUX_PINSWAP_1         (0x01 << 3 /* ALT1 absent on DU - placeholder */)
#define HWSERIAL1_MUX_PINSWAP_2         (0x02 << 3 /* PORTMUX_USART1_ALT2_gc - PD6/PD7 (occupied) */)
#define HWSERIAL1_MUX_PINSWAP_NONE      (0x03 << 3)
#define HWSERIAL1_MUX_DEFAULT          (0)
#define PIN_HWSERIAL1_TX                (NOT_A_PIN)
#define PIN_HWSERIAL1_RX                (NOT_A_PIN)
#define PIN_HWSERIAL1_XCK               (NOT_A_PIN)
#define PIN_HWSERIAL1_XDIR              (NOT_A_PIN)
#define PIN_HWSERIAL1_TX_PINSWAP_1      (NOT_A_PIN)
#define PIN_HWSERIAL1_RX_PINSWAP_1      (NOT_A_PIN)
#define PIN_HWSERIAL1_XCK_PINSWAP_1     (NOT_A_PIN)
#define PIN_HWSERIAL1_XDIR_PINSWAP_1    (NOT_A_PIN)
#define PIN_HWSERIAL1_TX_PINSWAP_2      (NOT_A_PIN)
#define PIN_HWSERIAL1_RX_PINSWAP_2      (NOT_A_PIN)
#define PIN_HWSERIAL1_XCK_PINSWAP_2     (NOT_A_PIN)
#define PIN_HWSERIAL1_XDIR_PINSWAP_2    (NOT_A_PIN)

/* ---- Arduino analog aliases. Uno R3 header = A0..A5; the remaining ADC-capable
 *      pins are also reachable as A6..A19 (one per digital pin). ---- */
#define PIN_A0   (PIN_PF0)   // D14
#define PIN_A1   (PIN_PF1)   // D15
#define PIN_A2   (PIN_PF2)   // D16
#define PIN_A3   (PIN_PF3)   // D17
#define PIN_A4   (PIN_PA2)   // D18  SDA
#define PIN_A5   (PIN_PA3)   // D19  SCL
#define PIN_A6   (PIN_PA5)   // D0
#define PIN_A7   (PIN_PA4)   // D1
#define PIN_A8   (PIN_PA6)   // D2
#define PIN_A9   (PIN_PA7)   // D3
#define PIN_A10  (PIN_PC3)   // D8
#define PIN_A11  (PIN_PF4)   // D4
#define PIN_A12  (PIN_PF5)   // D7
#define PIN_A13  (PIN_PD0)   // D5
#define PIN_A14  (PIN_PD1)   // D6
#define PIN_A15  (PIN_PD2)   // D9
#define PIN_A16  (PIN_PD3)   // D10
#define PIN_A17  (PIN_PD4)   // D11
#define PIN_A18  (PIN_PD5)   // D12
#define PIN_A19  (PIN_PD6)   // D13

/* --- Uno R4 style number-prefixed digital pin aliases (header pins D0..D19) ---
 * D-number == Arduino digital pin number. Internal-only pins (PD7 AREF, PA0/PA1
 * crystal, PF6 RESET, PF7 UPDI) are intentionally NOT exposed as Dn. */
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
#undef D13
#undef D14
#undef D15
#undef D16
#undef D17
#undef D18
#undef D19
static const uint8_t D0  = PIN_PA5;  // RX
static const uint8_t D1  = PIN_PA4;  // TX
static const uint8_t D2  = PIN_PA6;
static const uint8_t D3  = PIN_PA7;  // AC0 OUT
static const uint8_t D4  = PIN_PF4;  // TCB0 PWM
static const uint8_t D5  = PIN_PD0;
static const uint8_t D6  = PIN_PD1;
static const uint8_t D7  = PIN_PF5;
static const uint8_t D8  = PIN_PC3;
static const uint8_t D9  = PIN_PD2;
static const uint8_t D10 = PIN_PD3;  // SS (Uno convention)
static const uint8_t D11 = PIN_PD4;  // MOSI
static const uint8_t D12 = PIN_PD5;  // MISO
static const uint8_t D13 = PIN_PD6;  // SCK / LED_BUILTIN
static const uint8_t D14 = PIN_PF0;  // A0
static const uint8_t D15 = PIN_PF1;  // A1
static const uint8_t D16 = PIN_PF2;  // A2
static const uint8_t D17 = PIN_PF3;  // A3
static const uint8_t D18 = PIN_PA2;  // A4 / SDA
static const uint8_t D19 = PIN_PA3;  // A5 / SCL

static const uint8_t A0   = PIN_A0;
static const uint8_t A1   = PIN_A1;
static const uint8_t A2   = PIN_A2;
static const uint8_t A3   = PIN_A3;
static const uint8_t A4   = PIN_A4;
static const uint8_t A5   = PIN_A5;
static const uint8_t A6   = PIN_A6;
static const uint8_t A7   = PIN_A7;
static const uint8_t A8   = PIN_A8;
static const uint8_t A9   = PIN_A9;
static const uint8_t A10  = PIN_A10;
static const uint8_t A11  = PIN_A11;
static const uint8_t A12  = PIN_A12;
static const uint8_t A13  = PIN_A13;
static const uint8_t A14  = PIN_A14;
static const uint8_t A15  = PIN_A15;
static const uint8_t A16  = PIN_A16;
static const uint8_t A17  = PIN_A17;
static const uint8_t A18  = PIN_A18;
static const uint8_t A19  = PIN_A19;

/* Direct ADC channel identifiers (ADC_CH() sets the 0x80 "this is a channel" flag). */
#define AIN0   ADC_CH(0)
#define AIN1   ADC_CH(1)
#define AIN2   ADC_CH(2)
#define AIN3   ADC_CH(3)
#define AIN4   ADC_CH(4)
#define AIN5   ADC_CH(5)
#define AIN6   ADC_CH(6)
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

/* ---- Pin arrays (ARDUINO_MAIN). Indexed by digital pin number (0..24). ---- */
#ifdef ARDUINO_MAIN
  const uint8_t digital_pin_to_port[] = {
    PA,         //  0 PA5  RX/USART0 RX
    PA,         //  1 PA4  TX/USART0 TX
    PA,         //  2 PA6  USART0 XCK
    PA,         //  3 PA7  AC0 OUT/EVOUTA
    PF,         //  4 PF4  TCB0 PWM
    PD,         //  5 PD0  TCA0 WO0
    PD,         //  6 PD1  TCA0 WO1
    PF,         //  7 PF5
    PC,         //  8 PC3
    PD,         //  9 PD2  TCA0 WO2
    PD,         // 10 PD3  TCA0 WO3 / SS
    PD,         // 11 PD4  TCA0 WO4 / MOSI
    PD,         // 12 PD5  TCA0 WO5 / MISO
    PD,         // 13 PD6  SCK / LED_BUILTIN
    PF,         // 14 PF0  A0
    PF,         // 15 PF1  A1
    PF,         // 16 PF2  A2
    PF,         // 17 PF3  A3
    PA,         // 18 PA2  A4 / SDA
    PA,         // 19 PA3  A5 / SCL
    PD,         // 20 PD7  AREF
    PA,         // 21 PA0  XTALHF1
    PA,         // 22 PA1  XTALHF2
    PF,         // 23 PF6  RESET
    PF          // 24 PF7  UPDI
  };

  const uint8_t digital_pin_to_bit_position[] = {
    PIN5_bp,   //  0 PA5
    PIN4_bp,   //  1 PA4
    PIN6_bp,   //  2 PA6
    PIN7_bp,   //  3 PA7
    PIN4_bp,   //  4 PF4
    PIN0_bp,   //  5 PD0
    PIN1_bp,   //  6 PD1
    PIN5_bp,   //  7 PF5
    PIN3_bp,   //  8 PC3
    PIN2_bp,   //  9 PD2
    PIN3_bp,   // 10 PD3
    PIN4_bp,   // 11 PD4
    PIN5_bp,   // 12 PD5
    PIN6_bp,   // 13 PD6
    PIN0_bp,   // 14 PF0
    PIN1_bp,   // 15 PF1
    PIN2_bp,   // 16 PF2
    PIN3_bp,   // 17 PF3
    PIN2_bp,   // 18 PA2
    PIN3_bp,   // 19 PA3
    PIN7_bp,   // 20 PD7  AREF
    #if ((CLOCK_SOURCE & 0x03) == 0) // internal clock -> PA0 is a usable GPIO
      PIN0_bp, // 21 PA0
    #else                            // external crystal/clock -> PA0 = XTALHF1
      NOT_A_PIN,
    #endif
    #if ((CLOCK_SOURCE & 0x03) == 1) // external crystal also takes PA1
      NOT_A_PIN,
    #else
      PIN1_bp, // 22 PA1
    #endif
    PIN6_bp,   // 23 PF6 RESET
    PIN7_bp    // 24 PF7 UPDI
  };

  const uint8_t digital_pin_to_bit_mask[] = {
    PIN5_bm,   //  0 PA5
    PIN4_bm,   //  1 PA4
    PIN6_bm,   //  2 PA6
    PIN7_bm,   //  3 PA7
    PIN4_bm,   //  4 PF4
    PIN0_bm,   //  5 PD0
    PIN1_bm,   //  6 PD1
    PIN5_bm,   //  7 PF5
    PIN3_bm,   //  8 PC3
    PIN2_bm,   //  9 PD2
    PIN3_bm,   // 10 PD3
    PIN4_bm,   // 11 PD4
    PIN5_bm,   // 12 PD5
    PIN6_bm,   // 13 PD6
    PIN0_bm,   // 14 PF0
    PIN1_bm,   // 15 PF1
    PIN2_bm,   // 16 PF2
    PIN3_bm,   // 17 PF3
    PIN2_bm,   // 18 PA2
    PIN3_bm,   // 19 PA3
    PIN7_bm,   // 20 PD7  AREF
    #if ((CLOCK_SOURCE & 0x03) == 0)
      PIN0_bm, // 21 PA0
    #else
      NOT_A_PIN,
    #endif
    #if ((CLOCK_SOURCE & 0x03) == 1)
      NOT_A_PIN,
    #else
      PIN1_bm, // 22 PA1
    #endif
    PIN6_bm,   // 23 PF6 RESET
    PIN7_bm    // 24 PF7 UPDI
  };

  /* TCA0 PWM is resolved dynamically from PORTMUX, so TCA0 pins are NOT_ON_TIMER
   * here. Only the TCB output used for PWM is listed (PF4 = TCB0). TCB1 = millis. */
  const uint8_t digital_pin_to_timer[] = {
    NOT_ON_TIMER, //  0 PA5
    NOT_ON_TIMER, //  1 PA4
    NOT_ON_TIMER, //  2 PA6
    NOT_ON_TIMER, //  3 PA7
    TIMERB0,      //  4 PF4  (TCB0 - D4 PWM, ALT1)
    NOT_ON_TIMER, //  5 PD0  (TCA0 WO0, dynamic)
    NOT_ON_TIMER, //  6 PD1  (TCA0 WO1, dynamic)
    NOT_ON_TIMER, //  7 PF5
    NOT_ON_TIMER, //  8 PC3
    NOT_ON_TIMER, //  9 PD2  (TCA0 WO2, dynamic)
    NOT_ON_TIMER, // 10 PD3  (TCA0 WO3, dynamic)
    NOT_ON_TIMER, // 11 PD4  (TCA0 WO4, dynamic)
    NOT_ON_TIMER, // 12 PD5  (TCA0 WO5, dynamic)
    NOT_ON_TIMER, // 13 PD6
    NOT_ON_TIMER, // 14 PF0
    NOT_ON_TIMER, // 15 PF1
    NOT_ON_TIMER, // 16 PF2
    NOT_ON_TIMER, // 17 PF3
    NOT_ON_TIMER, // 18 PA2
    NOT_ON_TIMER, // 19 PA3
    NOT_ON_TIMER, // 20 PD7  AREF
    NOT_ON_TIMER, // 21 PA0
    NOT_ON_TIMER, // 22 PA1
    NOT_ON_TIMER, // 23 PF6 RESET
    NOT_ON_TIMER  // 24 PF7 UPDI
  };
#endif

/* =================================================================
 *  USB identity   (AVR DU = USB-native part, treated like the 32U4)
 * =================================================================
 *  USBCON enables Arduino's HID / Keyboard / Mouse / etc. on this board.
 *  NOTE: the native USB-CDC descriptor's VID/PID/product string are taken
 *  from cores/dxcore/usb_descriptors.{h,c} (which does NOT include this
 *  file), so the values below are informational only - keep them in sync.
 *  Effective app identity: 0x1209:0x0008, product "Wazamono Tsurugi".
 *  Obtain a real product VID/PID before release (pid.codes = dev only).
 */
#ifndef USBCON
  #define USBCON
#endif
#ifndef USB_VID
  #define USB_VID                0x1209
#endif
#ifndef USB_PID
  #define USB_PID                0x0008
#endif
#ifndef USB_MANUFACTURER
  #define USB_MANUFACTURER       "Workshop Asahi"
#endif
#ifndef USB_PRODUCT
  #define USB_PRODUCT            "Wazamono Tsurugi"
#endif

/* =================================================================
 *  Serial -> native USB CDC   (Leonardo/Micro convention)
 * =================================================================
 *  Serial  = USBSerial (on-chip USB CDC)              <- primary / USB serial monitor
 *  Serial0 = USART0   (D0/D1, ALT1 - the Uno R3 hardware UART)
 *  Serial1 = USART1   (exists in silicon, but no usable pins on Tsurugi - inert)
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
    #define SERIAL_PORT_HARDWARE    Serial0     /* Uno R3 D0/D1 hardware UART (USART0) */
  #endif
#endif

#endif
