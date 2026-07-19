/* pins_arduino.h - Variant definition for the UkiUkiduino (AVR64DU32)
 * ---------------------------------------------------------------------------
 * Part of UkiUkiCore (a product-specific fork of WazamonoCore / SpenceKonde's
 * DxCore). DxCore is (C) Spence Konde 2021-2022, open source (LGPL 2.1, see
 * LICENSE.md), based on existing Arduino cores. This variant (C) Workshop
 * Asahi 2026.
 *
 * Board   : UkiUkiduino (Arduino Uno R3 form factor, AVR64DU32-I/PT, USB-C)
 * MCU     : AVR64DU32  (32-pin TQFP)
 * Clock   : CRYSTAL-LESS. Internal OSCHF fixed at 24 MHz (boards.txt sets
 *           build.clocksource=0, no Clock menu). PA0/PA1 are therefore plain
 *           GPIO on this board and are mapped to D7 / D20.
 *           USB CLK_USB (48 MHz) is produced by OSCHF + PLL48M and is
 *           auto-tuned to the USB SOF, independently of the system clock.
 *
 *  ===== Pin numbering: Arduino Uno R3 compatible (NONCANONICAL) =====
 *   D#   MCU   Uno R3 role / notes                          A#,  AIN
 *   D0   PA5   RX  (Serial0 / USART0 RX, ALT1)              A6,  AIN25
 *   D1   PA4   TX  (Serial0 / USART0 TX, ALT1)              A7,  AIN24
 *   D2   PA6   (USART0 XCK, ALT1)                           A8,  AIN26
 *   D3   PF5   ~PWM(TCB1, ALT1)                             A9,  AIN21
 *   D4   PF4   (general I/O; no PWM - TCB0 is millis)       A10, AIN20
 *   D5   PD0   ~PWM(TCA0 WO0) | CCL                         A11, AIN0
 *   D6   PD1   ~PWM(TCA0 WO1) | CCL                         A12, AIN1
 *   D7   PC3   AC0 AINP4 | CCL LUT1-OUT                   A13, AIN31
 *   D8   PA7   (general I/O, native 5V)                     A14, AIN27
 *   D9   PD2   ~PWM(TCA0 WO2) | CCL | AC0 AINP0 | EVOUTD    A15, AIN2
 *   D10  PD3   ~PWM(TCA0 WO3) | CCL | AC0 AINN0 | (SS)      A16, AIN3
 *   D11  PD4   ~PWM(TCA0 WO4) | SPI MOSI                    A17, AIN4
 *   D12  PD5   ~PWM(TCA0 WO5) | SPI MISO                    A18, AIN5
 *   D13  PD6   SPI SCK | LED_BUILTIN (mirrored onto PC3)    A19, AIN6
 *   D14  PF0   A0 | CCL                                     A0,  AIN16
 *   D15  PF1   A1 | CCL                                     A1,  AIN17
 *   D16  PF2   A2 | CCL | EVOUTF                            A2,  AIN18
 *   D17  PF3   A3 | CCL                                     A3,  AIN19
 *   D18  PA2   A4 | SDA (I2C)                               A4,  AIN22
 *   D19  PA3   A5 | SCL (I2C)                               A5,  AIN23
 *   D20  PA1   BTN_BUILTIN (on-board button, external 1k pull-down,
 *              pressed = HIGH; no header pin)
 *   --- not exposed as a numbered Dn (appended so the arrays are complete) ---
 *        PA0   BUILTIN_LED driver (software mirror of D13 writes; no ADC)
 *                                                           index 21
 *        PD7   AREF header pin = VREFA (external analog reference; AIN7)
 *                                                           index 22
 *        PF6   RESET                                        index 23
 *        PF7   UPDI (Power header pin 1)                    index 24  (== PIN_PF7, highest)
 *
 *  ===== Peripheral routing (set by this variant + boards.txt) =====
 *   TCA0  -> PORTD (WO0..WO5 = PD0,PD1,PD2,PD3,PD4,PD5 = D5,D6,D9,D10,D11,D12)
 *   TCB1  -> ALT1 (PF5 = D3)  : ~PWM on D3 (shared with tone(), see below)
 *   millis-> TCB0 : boards.txt passes -DMILLIS_USE_TIMERB0, so TCB1 is free for D3.
 *   tone()-> TCB1 : Tone.cpp auto-selects TCB1 when millis is on TCB0. tone() and
 *           D3 PWM share TCB1, so calling tone() suspends D3 PWM only; D11 PWM
 *           (TCA0) and millis (TCB0) keep running (cf. Uno R3 Timer2 = D3+D11).
 *   SPI0  -> ALT4 (PD4 MOSI / PD5 MISO / PD6 SCK = D11/D12/D13). Board is SPI host;
 *           default CS exposed as PIN_SPI_SS = PD3 (D10, Uno convention). The ALT4
 *           hardware SS is PD7 (= AREF); run SPI host with Client Select Disable
 *           (SPI0.CTRLB.SSD = 1) so the AREF level cannot flip host -> client mode.
 *   TWI0  -> default (PA2 SDA / PA3 SCL) = D18/D19 = A4/A5 (Uno I2C convention).
 *   USART0-> "Serial1" (alias of Serial0), ALT1 (PA4 TX / PA5 RX) = D1/D0.
 *           (The Uno R3 D0/D1 UART.)
 *           The DEFAULT mux position (PA0/PA1) is the LED driver / BTN_BUILTIN
 *           on this board - do NOT Serial1.swap(0); ALT1 stays the variant
 *           default so D0/D1 behave like the classic Uno UART.
 *   USART1-> exists on the AVR DU but has NO usable pin position on UkiUkiduino
 *           (DU USART1 is only PD6/PD7, which are SPI SCK / AREF here). Its
 *           object is omitted and "Serial1" is a linker alias of Serial0
 *           (WAZAMONO_SERIAL1_IS_USART0, set in boards.txt; see below).
 *   AREF  -> PD7 = VREFA is wired to the Uno R3 AREF header pin, so
 *           analogReference(EXTERNAL) IS supported on this board.
 *   LED   -> on-board LED is a WS2812D-F5 addressable RGB LED, data-in on PA0.
 *           digitalWrite()/digitalWriteFast() to D13 (PD6) are SOFTWARE-mirrored:
 *           the core reads the resulting PD6 OUT bit and sends the matching
 *           WS2812 frame on PA0 (HIGH = lit in the current color - yellow by
 *           default, LOW = off), so the stock Blink sketch works unmodified.
 *           setLEDColor()/setLEDBrightness() (below) change the lit color.
 *           Only software writes are mirrored - SPI (SCK) traffic does NOT
 *           blink the LED (a deliberate difference from the Uno R3), and
 *           direct register writes to PORTD are not mirrored. Each mirrored
 *           write costs ~340 us (280 us WS2812 latch guard + 31 us frame).
 *   BUTTON-> BTN_BUILTIN = D20 (PA1). External 1 kOhm pull-down on the board;
 *           pressed = HIGH. Use pinMode(BTN_BUILTIN, INPUT) - no pullup needed.
 *   Serial-> native USB CDC (USBSerial), Leonardo/Micro convention.
 *
 *   NOTE on names: the Uno R3 D0/D1 UART is wired to USART0 on this board. The
 *   USART1 object is suppressed and "Serial1" is aliased to USART0, so users
 *   reach the D0/D1 hardware UART as "Serial1" (Uno-family convention;
 *   "Serial0" refers to the same object). "Serial" is the native USB CDC
 *   (serial monitor), so day-to-day Serial.print() behaves exactly like a
 *   classic Uno; use "Serial1" only to talk to an external device on D0/D1.
 */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>
#include "timers.h"

/* Informational pinout tags. DU_32PIN_PINOUT keeps DU-32 feature assumptions in
 * the core; UKIUKIDUINO_PINOUT identifies this board. NONCANONICAL_PIN_NUMBERS
 * tells the core to derive (port,bit) from the tables below instead of assuming
 * pin number == port order (our numbering does not follow port order). */
#define DU_32PIN_PINOUT
#define UKIUKIDUINO_PINOUT
/* Board tag for the bundled Wazamono-family libraries (AnalogComp,
 * CustomLogic, EventSystem): selects the UkiUkiduino pin mapping, which for
 * these peripherals matches the Wazamono Tsurugi (PD/PF/PA7/PC3 layout). */
#define WAZAMONO_BOARD_UKIUKIDUINO
#define NONCANONICAL_PIN_NUMBERS

/* ---- Digital pin number for each MCU pin (Uno R3 layout, D0..D20 contiguous) ---- */
#define PIN_PA5 (0)   // D0  RX  (USART0 RX, ALT1)
#define PIN_PA4 (1)   // D1  TX  (USART0 TX, ALT1)
#define PIN_PA6 (2)   // D2  (USART0 XCK)
#define PIN_PF5 (3)   // D3  ~PWM(TCB1, ALT1)
#define PIN_PF4 (4)   // D4  general I/O (no PWM: TCB0 = millis)
#define PIN_PD0 (5)   // D5  TCA0 WO0
#define PIN_PD1 (6)   // D6  TCA0 WO1
#define PIN_PC3 (7)   // D7  A13/AIN31, AC0 AINP4, CCL LUT1-OUT
#define PIN_PA7 (8)   // D8  general I/O (native 5V)
#define PIN_PD2 (9)   // D9  TCA0 WO2
#define PIN_PD3 (10)  // D10 TCA0 WO3 / SS (Uno convention, SSD=1)
#define PIN_PD4 (11)  // D11 TCA0 WO4 / SPI MOSI
#define PIN_PD5 (12)  // D12 TCA0 WO5 / SPI MISO
#define PIN_PD6 (13)  // D13 SPI SCK / LED_BUILTIN (mirrored to PC3 via CCL)
#define PIN_PF0 (14)  // D14 A0
#define PIN_PF1 (15)  // D15 A1
#define PIN_PF2 (16)  // D16 A2
#define PIN_PF3 (17)  // D17 A3
#define PIN_PA2 (18)  // D18 A4 / SDA
#define PIN_PA3 (19)  // D19 A5 / SCL
#define PIN_PA1 (20)  // D20 BTN_BUILTIN (on-board button; no ADC channel)
#define PIN_PA0 (21)  // WS2812D-F5 LED data-in (driven by the D13 software mirror); no ADC; no Dn alias
#define PIN_PD7 (22)  // AREF header pin = VREFA (external reference; AIN7); no Dn alias
#define PIN_PF6 (23)  // RESET
#define PIN_PF7 (24)  // UPDI (Power header pin 1; highest index -> NUM_DIGITAL_PINS = 25)

/* ---- Counts ---- */
#define PINS_COUNT                     (25)  // length of the pin tables (incl. reserved)
#define NUM_ANALOG_INPUTS              (31)  // highest ADC channel in use is AIN31 (PC3)
// NUM_DIGITAL_PINS / NUM_TOTAL_PINS  -> auto = PIN_PF7 + 1 = 25
// NUM_INTERNALLY_USED_PINS           -> auto = 0 (internal clock; no crystal pins)

#if !defined(LED_BUILTIN)
  #define LED_BUILTIN                  (PIN_PD6)   // D13, on-board LED (Uno convention)
#endif
#if !defined(BTN_BUILTIN)
  #define BTN_BUILTIN                  (PIN_PA1)   // D20, on-board button (pull-down, pressed = HIGH)
#endif

/* ---- LED_BUILTIN software mirror (see wiring_digital.c) ----
 * digitalWrite()/digitalWriteFast() on D13 (PD6) also call the hook below,
 * which reads the RESULTING PD6 OUT bit (so HIGH/LOW/CHANGE all mirror
 * correctly) and sends the matching WS2812 frame on PA0: HIGH = lit in the
 * current color (default: yellow), LOW = off. Implemented in
 * ukiukiduino_led.cpp. SPI SCK traffic and direct register writes are
 * intentionally NOT mirrored.
 * TIMING: each mirrored write busy-waits the WS2812 frame-latch time
 * (RES > 280 us, WS2812D-F5 datasheet) and then streams the 24-bit frame
 * with interrupts briefly disabled (~31 us). digitalWrite(13) therefore
 * takes ~340 us - fine for Blink-style use, but do not bit-bang fast
 * protocols on D13 (it is the SPI SCK pin anyway; hardware SPI is not
 * affected because SCK traffic is not mirrored). */
#define LED_BUILTIN_MIRROR
#define LED_MIRROR_SRC_PIN             (PIN_PD6)   /* D13 */

#ifdef __cplusplus
extern "C" void __led_builtin_mirror_hook(void);
#else
void __led_builtin_mirror_hook(void);
#endif

#ifdef __cplusplus
/* ---- On-board LED color API (WS2812D-F5 on PA0) ----
 * The LED still follows D13 like a classic Uno: digitalWrite(LED_BUILTIN,
 * HIGH/LOW) = lit/off. These functions change WHAT "lit" looks like:
 *
 *   setLEDColor(r, g, b);        8-bit RGB components
 *   setLEDColor(Yellow);         named color (see LEDColorName)
 *   setLEDBrightness(level);     0-255 overall brightness (default 40)
 *
 * If the LED is currently lit (D13 OUT is HIGH) the change is applied
 * immediately; otherwise it takes effect at the next digitalWrite(13, HIGH).
 * Power-on defaults: Yellow, brightness 40 (the Uno "L"-LED look). */
enum LEDColorName : uint8_t {
  Red, Green, Blue, Yellow, Orange, Cyan, Magenta, Purple, Pink, White
};
void setLEDColor(uint8_t r, uint8_t g, uint8_t b);
void setLEDColor(LEDColorName color);
void setLEDBrightness(uint8_t brightness);
#endif

#ifdef CORE_ATTACH_OLD
  #define EXTERNAL_NUM_INTERRUPTS      (48)
#endif

/* ---- Explicit maps (NONCANONICAL numbering: arithmetic shortcuts cannot be used) ----
 * PA0 (LED driver) and PA1 (D20) have no ADC channel on the AVR DU and are
 * therefore absent from these maps (-> NOT_A_PIN). D7 (PC3) maps to AIN31 (A13).
 * PD7 (AREF) keeps its AIN7 mapping so the pin can still be sampled when it is
 * not used as the external reference. */
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

/* ---- PWM ----
 * millis lives on TCB0 (boards.txt -DMILLIS_USE_TIMERB0), leaving TCB1 free for
 * D3 PWM (TCB1 ALT1 = PF5) and for tone(): Tone.cpp routes tone to TCB1 whenever
 * millis is on TCB0, so tone shares TCB1 with D3 PWM - calling tone() suspends D3
 * PWM only, while D11 PWM on TCA0 keeps running (cf. Uno R3 Timer2 = D3+D11).
 * TCA0 -> PORTD: WO0..WO5 = PD0,PD1,PD2,PD3,PD4,PD5 = D5,D6,D9,D10,D11,D12. */
#if defined(MILLIS_USE_TIMERB1)
  #define digitalPinHasPWMTCB(p) (0)                   /* TCB1 is millis; no TCB PWM */
#elif defined(MILLIS_USE_TIMERB0)
  #define digitalPinHasPWMTCB(p) ((p) == PIN_PF5)      /* TCB0=millis -> TCB1 free -> D3 PWM */
#else
  #define digitalPinHasPWMTCB(p) ((p) == PIN_PF5)
#endif
#define digitalPinHasPWMTCA(p) ( \
    (p) == PIN_PD0 || (p) == PIN_PD1 || (p) == PIN_PD2 || \
    (p) == PIN_PD3 || (p) == PIN_PD4 || (p) == PIN_PD5 )

/* TCA0 routed to PORTD: WO0..WO5 = PD0..PD5 = D5,D6,D9,D10,D11,D12. */
#define TCA0_PINS                       (PORTMUX_TCA0_PORTD_gc)
#define TCB0_PINS                       (0x00)   // TCB0 = millis; WO unused (default PA2 pos)
#define TCB1_PINS                       (0x01)   // TCB1 WO -> ALT1 (PF5 = D3)

#define PIN_TCA0_WO0_INIT               (PIN_PD0)
#define PIN_TCB0_WO_INIT                (PIN_PA2)   // TCB0 = millis; not enabled for PWM
#define PIN_TCB1_WO_INIT                (PIN_PF5)   // D3 PWM (TCB1 ALT1)

#define digitalPinHasPWM(p)             (digitalPinHasPWMTCB(p) || digitalPinHasPWMTCA(p))

/* ---- SPI (host; chip-selects are user GPIO). UkiUkiduino routes SPI to ALT4 (PORTD) ---- */
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

/* ---- USART0 -> "Serial0" = the Uno R3 D0/D1 UART. Default position ALT1 (PA4/PA5).
 *   The DEFAULT mux (swap 0) lands on PA0/PA1 = D7/D20; it stays selectable via
 *   Serial0.swap(0) for advanced use, but the variant default is ALT1 (D0/D1). ---- */
#define HWSERIAL0_MUX                   (0x00 /* PORTMUX_USART0_DEFAULT_gc - PA0/PA1 = D7/D20 */)
#define HWSERIAL0_MUX_PINSWAP_1         (0x01 /* PORTMUX_USART0_ALT1_gc - PA4..PA7 */)
#define HWSERIAL0_MUX_PINSWAP_2         (0x02 /* PORTMUX_USART0_ALT2_gc - PA2/PA3 (= I2C) */)
#define HWSERIAL0_MUX_PINSWAP_3         (0x03 /* PORTMUX_USART0_ALT3_gc - PD4..PD7 (= SPI/AREF) */)
#define HWSERIAL0_MUX_PINSWAP_NONE      (0x05)
#define HWSERIAL0_MUX_DEFAULT          (1)   /* UkiUkiduino default: USART0 ALT1 (PA4/PA5) = D1/D0. */
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

/* ---- USART1: NO usable pin position on UkiUkiduino (DU USART1 is only PD6/PD7
 *   (ALT2) = SPI SCK / AREF here). boards.txt therefore builds with
 *   -DWAZAMONO_SERIAL1_IS_USART0 (mechanism inherited from WazamonoCore):
 *   UART1.cpp skips the USART1 object entirely and UART0.cpp emits
 *       extern HardwareSerial Serial1 __attribute__((alias("Serial0")));
 *   so "Serial1" IS the USART0 object (a true linker alias, not a macro) and
 *   the Uno R3 D0/D1 hardware UART is reachable as "Serial1" (Uno-family
 *   convention). "Serial0" remains a valid name for the same object. ---- */
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
 *      pins are also reachable as A6..A19 (D7 = PC3 = AIN31 = A13). ---- */
#define PIN_A0   (PIN_PF0)   // D14
#define PIN_A1   (PIN_PF1)   // D15
#define PIN_A2   (PIN_PF2)   // D16
#define PIN_A3   (PIN_PF3)   // D17
#define PIN_A4   (PIN_PA2)   // D18  SDA
#define PIN_A5   (PIN_PA3)   // D19  SCL
#define PIN_A6   (PIN_PA5)   // D0
#define PIN_A7   (PIN_PA4)   // D1
#define PIN_A8   (PIN_PA6)   // D2
#define PIN_A9   (PIN_PF5)   // D3
#define PIN_A10  (PIN_PF4)   // D4
#define PIN_A11  (PIN_PD0)   // D5
#define PIN_A12  (PIN_PD1)   // D6
#define PIN_A13  (PIN_PC3)   // D7
#define PIN_A14  (PIN_PA7)   // D8
#define PIN_A15  (PIN_PD2)   // D9
#define PIN_A16  (PIN_PD3)   // D10
#define PIN_A17  (PIN_PD4)   // D11
#define PIN_A18  (PIN_PD5)   // D12
#define PIN_A19  (PIN_PD6)   // D13

/* --- Uno R4 style number-prefixed digital pin aliases (D0..D20) ---
 * D-number == Arduino digital pin number. Internal-only pins (PC3 LED driver,
 * PD7 AREF, PF6 RESET, PF7 UPDI) are intentionally NOT exposed as Dn. */
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
#undef D20
static const uint8_t D0  = PIN_PA5;  // RX
static const uint8_t D1  = PIN_PA4;  // TX
static const uint8_t D2  = PIN_PA6;
static const uint8_t D3  = PIN_PF5;  // ~PWM(TCB1)
static const uint8_t D4  = PIN_PF4;
static const uint8_t D5  = PIN_PD0;
static const uint8_t D6  = PIN_PD1;
static const uint8_t D7  = PIN_PC3;  // A13/AIN31
static const uint8_t D8  = PIN_PA7;  // native 5V
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
static const uint8_t D20 = PIN_PA1;  // BTN_BUILTIN

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

/* ---- Pin arrays (ARDUINO_MAIN). Indexed by digital pin number (0..24). ----
 * Clock is fixed to the internal OSCHF (boards.txt build.clocksource=0), so
 * PA0 (D7) and PA1 (D20) are unconditionally plain GPIO - no CLOCK_SOURCE
 * conditionals are needed here. */
#ifdef ARDUINO_MAIN
  const uint8_t digital_pin_to_port[] = {
    PA,         //  0 PA5  D0  RX/USART0 RX
    PA,         //  1 PA4  D1  TX/USART0 TX
    PA,         //  2 PA6  D2  USART0 XCK
    PF,         //  3 PF5  D3  TCB1 PWM (ALT1)
    PF,         //  4 PF4  D4
    PD,         //  5 PD0  D5  TCA0 WO0
    PD,         //  6 PD1  D6  TCA0 WO1
    PC,         //  7 PC3  D7  (A13/AIN31, AC0 AINP4, LUT1-OUT)
    PA,         //  8 PA7  D8  (native 5V)
    PD,         //  9 PD2  D9  TCA0 WO2
    PD,         // 10 PD3  D10 TCA0 WO3 / SS
    PD,         // 11 PD4  D11 TCA0 WO4 / MOSI
    PD,         // 12 PD5  D12 TCA0 WO5 / MISO
    PD,         // 13 PD6  D13 SCK / LED_BUILTIN
    PF,         // 14 PF0  A0
    PF,         // 15 PF1  A1
    PF,         // 16 PF2  A2
    PF,         // 17 PF3  A3
    PA,         // 18 PA2  A4 / SDA
    PA,         // 19 PA3  A5 / SCL
    PA,         // 20 PA1  D20 BTN_BUILTIN
    PA,         // 21 PA0  BUILTIN_LED driver (software mirror)
    PD,         // 22 PD7  AREF (VREFA)
    PF,         // 23 PF6  RESET
    PF          // 24 PF7  UPDI
  };

  const uint8_t digital_pin_to_bit_position[] = {
    PIN5_bp,   //  0 PA5  D0
    PIN4_bp,   //  1 PA4  D1
    PIN6_bp,   //  2 PA6  D2
    PIN5_bp,   //  3 PF5  D3
    PIN4_bp,   //  4 PF4  D4
    PIN0_bp,   //  5 PD0  D5
    PIN1_bp,   //  6 PD1  D6
    PIN3_bp,   //  7 PC3  D7
    PIN7_bp,   //  8 PA7  D8
    PIN2_bp,   //  9 PD2  D9
    PIN3_bp,   // 10 PD3  D10
    PIN4_bp,   // 11 PD4  D11
    PIN5_bp,   // 12 PD5  D12
    PIN6_bp,   // 13 PD6  D13
    PIN0_bp,   // 14 PF0  A0
    PIN1_bp,   // 15 PF1  A1
    PIN2_bp,   // 16 PF2  A2
    PIN3_bp,   // 17 PF3  A3
    PIN2_bp,   // 18 PA2  A4
    PIN3_bp,   // 19 PA3  A5
    PIN1_bp,   // 20 PA1  D20 BTN_BUILTIN
    PIN0_bp,   // 21 PA0  LED
    PIN7_bp,   // 22 PD7  AREF
    PIN6_bp,   // 23 PF6  RESET
    PIN7_bp    // 24 PF7  UPDI
  };

  const uint8_t digital_pin_to_bit_mask[] = {
    PIN5_bm,   //  0 PA5  D0
    PIN4_bm,   //  1 PA4  D1
    PIN6_bm,   //  2 PA6  D2
    PIN5_bm,   //  3 PF5  D3
    PIN4_bm,   //  4 PF4  D4
    PIN0_bm,   //  5 PD0  D5
    PIN1_bm,   //  6 PD1  D6
    PIN3_bm,   //  7 PC3  D7
    PIN7_bm,   //  8 PA7  D8
    PIN2_bm,   //  9 PD2  D9
    PIN3_bm,   // 10 PD3  D10
    PIN4_bm,   // 11 PD4  D11
    PIN5_bm,   // 12 PD5  D12
    PIN6_bm,   // 13 PD6  D13
    PIN0_bm,   // 14 PF0  A0
    PIN1_bm,   // 15 PF1  A1
    PIN2_bm,   // 16 PF2  A2
    PIN3_bm,   // 17 PF3  A3
    PIN2_bm,   // 18 PA2  A4
    PIN3_bm,   // 19 PA3  A5
    PIN1_bm,   // 20 PA1  D20 BTN_BUILTIN
    PIN0_bm,   // 21 PA0  LED
    PIN7_bm,   // 22 PD7  AREF
    PIN6_bm,   // 23 PF6  RESET
    PIN7_bm    // 24 PF7  UPDI
  };

  /* TCA0 PWM is resolved dynamically from PORTMUX, so TCA0 pins are NOT_ON_TIMER
   * here. Only the TCB output used for PWM is listed (PF5 = TCB1, D3). TCB0 = millis. */
  const uint8_t digital_pin_to_timer[] = {
    NOT_ON_TIMER, //  0 PA5  D0
    NOT_ON_TIMER, //  1 PA4  D1
    NOT_ON_TIMER, //  2 PA6  D2
    TIMERB1,      //  3 PF5  D3  (TCB1 - D3 PWM, ALT1)
    NOT_ON_TIMER, //  4 PF4  D4  (no PWM: TCB0 = millis)
    NOT_ON_TIMER, //  5 PD0  D5  (TCA0 WO0, dynamic)
    NOT_ON_TIMER, //  6 PD1  D6  (TCA0 WO1, dynamic)
    NOT_ON_TIMER, //  7 PC3  D7
    NOT_ON_TIMER, //  8 PA7  D8
    NOT_ON_TIMER, //  9 PD2  D9  (TCA0 WO2, dynamic)
    NOT_ON_TIMER, // 10 PD3  D10 (TCA0 WO3, dynamic)
    NOT_ON_TIMER, // 11 PD4  D11 (TCA0 WO4, dynamic)
    NOT_ON_TIMER, // 12 PD5  D12 (TCA0 WO5, dynamic)
    NOT_ON_TIMER, // 13 PD6  D13
    NOT_ON_TIMER, // 14 PF0  A0
    NOT_ON_TIMER, // 15 PF1  A1
    NOT_ON_TIMER, // 16 PF2  A2
    NOT_ON_TIMER, // 17 PF3  A3
    NOT_ON_TIMER, // 18 PA2  A4
    NOT_ON_TIMER, // 19 PA3  A5
    NOT_ON_TIMER, // 20 PA1  D20 BTN_BUILTIN
    NOT_ON_TIMER, // 21 PA0  LED
    NOT_ON_TIMER, // 22 PD7  AREF
    NOT_ON_TIMER, // 23 PF6  RESET
    NOT_ON_TIMER  // 24 PF7  UPDI
  };
#endif

/* =================================================================
 *  USB identity   (AVR DU = USB-native part, treated like the 32U4)
 * =================================================================
 *  USBCON enables Arduino's HID / Keyboard / Mouse / etc. on this board.
 *  NOTE: the native USB-CDC descriptor's VID/PID/product string are taken
 *  from cores/dxcore/usb_descriptors.{h,c} (which does NOT include this
 *  file), so the values below are informational only - keep them in sync.
 *  Effective app identity (dev/test): 0x1209:0x000C, product "UkiUkiduino".
 *  These are pid.codes TEST-range placeholders; replace with the official
 *  pid.codes VID/PID before release.
 */
#ifndef USBCON
  #define USBCON
#endif
#ifndef USB_VID
  #define USB_VID                0x1209
#endif
#ifndef USB_PID
  #define USB_PID                0x000C
#endif
#ifndef USB_MANUFACTURER
  #define USB_MANUFACTURER       "Workshop Asahi"
#endif
#ifndef USB_PRODUCT
  #define USB_PRODUCT            "UkiUkiduino"
#endif

/* =================================================================
 *  Serial -> native USB CDC   (Leonardo/Micro convention)
 * =================================================================
 *  Serial  = USBSerial (on-chip USB CDC)              <- primary / USB serial monitor
 *  Serial1 = USART0   (D0/D1, ALT1 - the Uno R3 hardware UART; alias of Serial0)
 *  (USART1 has no usable pins here; its object is omitted - see
 *   WAZAMONO_SERIAL1_IS_USART0 above)
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

/* ---- analogReference(): EXTERNAL is SUPPORTED on UkiUkiduino ----
 * PD7 (VREFA) is wired to the Uno R3 AREF header pin, so
 * analogReference(EXTERNAL) works as on a classic Uno. The internal
 * references (1.024/2.048/2.5/4.096 V) and VDD remain available. */

#endif
