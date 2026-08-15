/* pins_arduino.h - Variant definition for the Wazamono Tsurugi (AVR64DU32)
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * DxCore is (C) Spence Konde 2021-2022, open source (LGPL 2.1, see LICENSE.md),
 * based on existing Arduino cores. This variant (C) Workshop Asahi 2026.
 *
 * Board   : Wazamono Tsurugi (Arduino Uno R3 form factor, AVR64DU32, USB-C)
 * MCU     : AVR64DU32  (32-pin TQFP/VQFN)
 * Clock   : internal OSCHF, 24 MHz fixed (no crystal on the board; boards.txt
 *           pins build.clocksource=0, no IDE clock menu). USB CLK_USB (48 MHz)
 *           is produced by OSCHF + PLL48M and auto-tuned to the USB SOF.
 *           PA0/PA1 (the XTALHF pads) are repurposed as the USB-CDC TX/RX
 *           activity LEDs (active-LOW, 3.3 V rail).
 *
 *  ===== Pin numbering: Arduino Uno R3 compatible (NONCANONICAL) =====
 *   D#   MCU   Uno R3 role / notes                          A#,  AIN
 *   D0   PA5   RX  (Serial0 / USART0 RX, ALT1)              A6,  AIN25
 *   D1   PA4   TX  (Serial0 / USART0 TX, ALT1)              A7,  AIN24
 *   D2   PA7   (USART0 XDIR | AC0 OUT | EVOUTA | CLKOUT)    A8,  AIN27
 *   D3   PA6   ~PWM(TCB1 via CCL LUT0) | USART0 XCK         A9,  AIN26
 *   D4   PC3   ~PWM(TCB1 via CCL LUT1) (VDD-driven, confirmed) A10, AIN31
 *   D5   PD0   ~PWM(TCA0 WO0) | CCL                         A11, AIN0
 *   D6   PD1   ~PWM(TCA0 WO1) | CCL                         A12, AIN1
 *   D7   PF4   (general I/O; no PWM - TCB0 is millis)          A13, AIN20
 *   D8   PF5   (general I/O; see D3 PWM note below)         A14, AIN21
 *   D9   PD2   ~PWM(TCA0 WO2) | CCL | AC0 AINP0 | EVOUTD    A15, AIN2
 *   D10  PD3   ~PWM(TCA0 WO3) | CCL | AC0 AINN0 | (SS)      A16, AIN3
 *   D11  PD4   ~PWM(TCA0 WO4) | SPI MOSI                    A17, AIN4
 *   D12  PD5   ~PWM(TCA0 WO5) | SPI MISO                    A18, AIN5
 *   D13  PD6   SPI SCK | LED_BUILTIN (op-amp buffered LED)  A19, AIN6
 *   D14  PF0   A0 | CCL                                     A0,  AIN16
 *   D15  PF1   A1 | CCL                                     A1,  AIN17
 *   D16  PF2   A2 | CCL | EVOUTF                            A2,  AIN18
 *   D17  PF3   A3 | CCL                                     A3,  AIN19
 *   D18  PA2   A4 | SDA (I2C)                               A4,  AIN22
 *   D19  PA3   A5 | SCL (I2C)                               A5,  AIN23
 *   --- appended pins (beyond the Uno R3 header numbering) ---
 *   D20  PD7   AREF | GPIO | SPI0 SS(ALT4) | Serial2 RX      A20, AIN7
 *        (usable as plain GPIO whenever no external reference is applied
 *         to the AREF header pin - a modern-AVR capability the Uno R3 lacks)
 *        PA0   TX activity LED (active-LOW)         index 21  (PIN_LED_TX)
 *        PA1   RX activity LED (active-LOW)         index 22  (PIN_LED_RX)
 *        PF6   RESET                                index 23
 *        PF7   UPDI                                 index 24  (== PIN_PF7, highest)
 *
 *  ===== Peripheral routing (set by this variant + boards.txt) =====
 *   TCA0  -> PORTD (WO0..WO5 = PD0,PD1,PD2,PD3,PD4,PD5 = D6,D5,D9,D10,D11,D12)
 *   TCB1  -> D3 PWM *through CCL LUT0*: D3 is PA6 = LUT0-OUT (alternate
 *           position). TCB1 runs in 8-bit PWM mode and its internal WO signal
 *           feeds LUT0 (INSEL1 = TCB, DS40002548A 30.2.2.1); the LUT passes it
 *           to the pin. analogWrite(D3, x) does all of this transparently
 *           (core wazamono_lutpwm.c). TCB1's own WO pin position is parked on
 *           ALT1/PF5 (TCB1_PINS) and its pin override stays off by default.
 *   millis-> TCB0 : boards.txt passes -DMILLIS_USE_TIMERB0, so TCB1 is free for D3.
 *   tone()-> TCB1 : Tone.cpp auto-selects TCB1 when millis is on TCB0. tone() and
 *           D3 PWM share TCB1, so calling tone() suspends D3 PWM only; D11 PWM
 *           (TCA0) and millis (TCB0) keep running (cf. Uno R3 Timer2 = D3+D11).
 *           Likewise, taking LUT0 over (direct CCL register use) suspends D3 PWM:
 *           analogWrite(D3) then falls back to plain digital output.
 *   SPI0  -> ALT4 (PD4 MOSI / PD5 MISO / PD6 SCK = D11/D12/D13). Board is SPI host;
 *           default CS exposed as PIN_SPI_SS = PD3 (D10, Uno convention). The ALT4
 *           hardware SS is PD7 (= AREF here); run SPI host with Client Select
 *           Disable (SPI0.CTRLB.SSD = 1) so the AREF level cannot flip host ->
 *           client mode.
 *   TWI0  -> default (PA2 SDA / PA3 SCL) = D18/D19 = A4/A5 (Uno I2C convention).
 *   USART0-> "Serial1", ALT1 (PA4 TX / PA5 RX) = D1/D0. The Uno R3 D0/D1 UART,
 *           named Serial1 to match the Uno R4 convention (WAZAMONO_SERIAL1_IS_
 *           USART0; the object is Serial0, Serial1 is its alias - see UART0.cpp).
 *   USART1-> "Serial2", ALT2 fixed (PD6 TX = D13 / PD7 RX = D20, the AREF
 *           pin) - the only DU USART1 position that exists here. Shares its
 *           pins with SPI0 SCK and the AREF function (user-level conflict).
 *   LED   -> on-board LED follows D13 (PD6, SPI SCK) through a unity-gain op-amp
 *           buffer on the board - the same arrangement as the Arduino Uno R3 - so
 *           the LED never loads the SCK line. No firmware is involved.
 *   AREF  -> PD7 (VREFA) is wired to the AREF header pin for an external analog
 *           reference; it has no digital pin number.
 *   Serial-> native USB CDC (USBSerial), Leonardo/Micro convention.
 *
 *   NOTE on names: "Serial" is the native USB CDC (serial monitor), so day-to-day
 *   Serial.print() behaves exactly like a classic Uno. The D0/D1 hardware UART is
 *   "Serial1", matching the Arduino Uno R4. (Serial0 also works - same port.)
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
#define WAZAMONO_BOARD_TSURUGI 1  /* Board identification macro (matches bootloader convention) */
#define NONCANONICAL_PIN_NUMBERS

/* ---- Digital pin number for each MCU pin (Uno R3 layout, D0..D19 contiguous) ---- */
#define PIN_PA5 (0)   // D0  RX  (USART0 RX, ALT1)
#define PIN_PA4 (1)   // D1  TX  (USART0 TX, ALT1)
#define PIN_PA7 (2)   // D2  (USART0 XDIR / AC0 OUT / EVOUTA / CLKOUT)
#define PIN_PA6 (3)   // D3  ~PWM(TCB1 via CCL LUT0) / USART0 XCK
#define PIN_PC3 (4)   // D4  ~PWM(TCB1 via CCL LUT1); VDD-driven output (confirmed by measurement)
#define PIN_PD0 (5)   // D5  TCA0 WO0
#define PIN_PD1 (6)   // D6  TCA0 WO1
#define PIN_PF4 (7)   // D7  general I/O (no PWM: TCB0 = millis)
#define PIN_PF5 (8)   // D8  general I/O (TCB1 WO parking position - see D3 PWM note)
#define PIN_PD2 (9)   // D9  TCA0 WO2
#define PIN_PD3 (10)  // D10 TCA0 WO3 / SS (Uno convention, SSD=1)
#define PIN_PD4 (11)  // D11 TCA0 WO4 / SPI MOSI
#define PIN_PD5 (12)  // D12 TCA0 WO5 / SPI MISO
#define PIN_PD6 (13)  // D13 SPI SCK / LED_BUILTIN (op-amp buffered on the board)
#define PIN_PF0 (14)  // D14 A0
#define PIN_PF1 (15)  // D15 A1
#define PIN_PF2 (16)  // D16 A2
#define PIN_PF3 (17)  // D17 A3
#define PIN_PA2 (18)  // D18 A4 / SDA
#define PIN_PA3 (19)  // D19 A5 / SCL
#define PIN_PD7 (20)  // D20/A20 = AREF (VREFA) | GPIO | SPI0 SS(ALT4) | Serial2 RX
#define PIN_PA0 (21)  // TX activity LED (active-LOW; USB-CDC traffic, variant-driven)
#define PIN_PA1 (22)  // RX activity LED (active-LOW; USB-CDC traffic, variant-driven)
#define PIN_PF6 (23)  // RESET
#define PIN_PF7 (24)  // UPDI  (highest index -> sets NUM_DIGITAL_PINS = 25)

/* ---- Counts ---- */
#define PINS_COUNT                     (25)  // length of the pin tables (incl. reserved)
#define NUM_ANALOG_INPUTS              (31)  // highest ADC channel in use is AIN31 (PC3)
// NUM_DIGITAL_PINS / NUM_TOTAL_PINS  -> auto = PIN_PF7 + 1 = 25
// NUM_INTERNALLY_USED_PINS           -> auto = 0 (no crystal; PA0/PA1 = TX/RX LEDs, still writable GPIO)

#if !defined(LED_BUILTIN)
  #define LED_BUILTIN                  (PIN_PD6)   // D13, on-board LED (Uno convention)
#endif
/* USB-CDC activity LEDs (Pro Micro convention, both active-LOW, 3.3 V rail).
 * Driven from the CDC hooks in wazamono_tsurugi_init.cpp; sketches may also
 * digitalWrite() them - they just fight the ~100 ms activity pulses. */
#define PIN_LED_TX                     (PIN_PA0)
#define PIN_LED_RX                     (PIN_PA1)
#define LED_BUILTIN_TX                 PIN_LED_TX
#define LED_BUILTIN_RX                 PIN_LED_RX

/* ---- Event output pins: FIXED by the board's pin-configuration table ----
 * One pin per event output, no alternatives. Libraries (CustomLogic, and the
 * Event library in its Wazamono form) route event outputs to these pins only.
 *   EVOUTA -> PA7 = D8   (PORTMUX ALT1; PA2/D18 stays I2C SDA)
 *   EVOUTD -> PD2 = D9   (PORTMUX default; the hardware offers only PD2 or
 *                         PD7, and PD7 is the AREF pin on this board)
 *   EVOUTF -> PF2 = A2   (PORTMUX default) */
#define WAZAMONO_EVOUTA_PIN            (PIN_PA7)
#define WAZAMONO_EVOUTA_ALT            (1)
#define WAZAMONO_EVOUTD_PIN            (PIN_PD2)
#define WAZAMONO_EVOUTD_ALT            (0)
#define WAZAMONO_EVOUTF_PIN            (PIN_PF2)
#define WAZAMONO_EVOUTF_ALT            (0)

#ifdef CORE_ATTACH_OLD
  #define EXTERNAL_NUM_INTERRUPTS      (48)
#endif

/* ---- Explicit maps (NONCANONICAL numbering: arithmetic shortcuts cannot be used) ---- */
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
 * D3 PWM and for tone(). D3 (PA6) is not a TCB WO pin: it is LUT0's alternate
 * output, and analogWrite(D3) delivers TCB1's 8-bit PWM waveform through LUT0
 * (see the WAZAMONO_TCB1_LUTPWM_* block below and cores/dxcore/wazamono_lutpwm.h).
 * tone(): Tone.cpp routes tone to TCB1 whenever millis is on TCB0, so tone shares
 * TCB1 with D3 PWM - calling tone() suspends D3 PWM only, while D11 PWM on TCA0
 * keeps running (cf. Uno R3 Timer2 = D3+D11).
 * TCA0 -> PORTD: WO0..WO5 = PD0,PD1,PD2,PD3,PD4,PD5 = D6,D5,D9,D10,D11,D12. */
#if defined(MILLIS_USE_TIMERB1)
  #define digitalPinHasPWMTCB(p) (0)                   /* TCB1 is millis; no TCB PWM */
#elif defined(MILLIS_USE_TIMERB0)
  /* TCB0 = millis -> TCB1 free. ONE waveform, THREE selectable outlets:
   * D3 (LUT0 alt), D4 (LUT1 default), D8 (TCB1 WO, ALT1). Exclusive - the
   * last analogWrite() among the three owns the route; default outlet D3. */
  #define digitalPinHasPWMTCB(p) ((p) == PIN_PA6 || (p) == PIN_PC3 || (p) == PIN_PF5)
#else
  #define digitalPinHasPWMTCB(p) ((p) == PIN_PA6 || (p) == PIN_PC3 || (p) == PIN_PF5)
#endif
#define digitalPinHasPWMTCA(p) ( \
    (p) == PIN_PD0 || (p) == PIN_PD1 || (p) == PIN_PD2 || \
    (p) == PIN_PD3 || (p) == PIN_PD4 || (p) == PIN_PD5 )

/* TCA0 routed to PORTD: WO0..WO5 = PD0..PD5 = D6,D5,D9,D10,D11,D12. */
#define TCA0_PINS                       (PORTMUX_TCA0_PORTD_gc)
#define TCB0_PINS                       (0x00)   // TCB0 = millis; WO unused (default PA2 pos)
#define TCB1_PINS                       (0x02)   // PORTMUX.TCBROUTEA bit 1: TCB1 WO parked on ALT1
                                                 // (PF5 = D8). The D3 waveform travels through LUT0,
                                                 // not through a WO pin; parking keeps the default
                                                 // position PA3 (= A5/SCL) clear. (DS40002548A 17.3.5)

#define PIN_TCA0_WO0_INIT               (PIN_PD0)
#define PIN_TCB0_WO_INIT                (PIN_PA2)   // TCB0 = millis; not enabled for PWM
#define PIN_TCB1_WO_INIT                (PIN_PF5)   // parked (see TCB1_PINS); PWM leaves via LUT0/D3

/* ---- D3 PWM: TCB1 through CCL LUT0 (core mechanism, wazamono_lutpwm.{h,c}) ----
 * analogWrite(D3, x) makes TCB1's PWM8 waveform reach D3 = PA6 = LUT0-OUT (alt):
 *   TCB1 WO (internal) -> LUT0 INSEL1 = TCB -> TRUTH 0xCC -> PA6.
 * The digital_pin_to_timer[] entry for D3 is TIMERB1, which sends analogWrite()
 * down the standard TCB path; the macros below tell the core hook which LUT
 * carries the waveform. Conflicts disable D3 analogWrite automatically:
 * tone()/TCB1 reconfiguration (CNTMODE check) and LUT0 use by other code
 * (signature check in wazamono_lutpwm.c).
 * CCMPEN stays 0: the CCL taps the internal WO signal, which was measured on
 * silicon to run regardless of that bit (see wazamono_lutpwm.h). D8 (= PF5,
 * TCB1's parked WO position) therefore stays a plain GPIO while D3 PWM runs. */
#define WAZAMONO_TCB1_PWMMUX            (1)         /* exclusive D3/D4/D8 routing (default D3) */
#define WAZAMONO_TCB1_PWM_LUT0_PIN      (PIN_PA6)   /* D3: LUT0-OUT, alternate position */
#define WAZAMONO_TCB1_PWM_LUT0          (0)
#define WAZAMONO_TCB1_PWM_LUT0_ALT      (1)
#define WAZAMONO_TCB1_PWM_LUT1_PIN      (PIN_PC3)   /* D4: LUT1-OUT, default position */
#define WAZAMONO_TCB1_PWM_LUT1          (1)
#define WAZAMONO_TCB1_PWM_LUT1_ALT      (0)
#define WAZAMONO_TCB1_PWM_WO_PIN        (PIN_PF5)   /* D8: TCB1 WO itself (TCB1_PINS = ALT1), outlet = CCMPEN */

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

/* ---- USART0 -> user-facing "Serial1" = the Uno R3 D0/D1 UART (Uno R4 naming).
 * Default position ALT1 (PA4/PA5). The object is Serial0; UART0.cpp emits the
 * Serial1 alias. USART1 has no usable pins here and is compiled out. ---- */
#ifndef WAZAMONO_SERIAL1_IS_USART0
  #define WAZAMONO_SERIAL1_IS_USART0
#endif
#ifndef WAZAMONO_SERIAL2_IS_USART1
  #define WAZAMONO_SERIAL2_IS_USART1   /* USART1 object is exposed as Serial2 (UART1.cpp) */
#endif
#define HWSERIAL0_MUX                   (0x00 /* PORTMUX_USART0_DEFAULT_gc - PA0/PA1 = TX/RX LEDs */)
#define HWSERIAL0_MUX_PINSWAP_1         (0x01 /* PORTMUX_USART0_ALT1_gc - PA4..PA7 */)
#define HWSERIAL0_MUX_PINSWAP_2         (0x02 /* PORTMUX_USART0_ALT2_gc - PA2/PA3 (= I2C) */)
#define HWSERIAL0_MUX_PINSWAP_3         (0x03 /* PORTMUX_USART0_ALT3_gc - PD4..PD7 (= SPI/AREF) */)
#define HWSERIAL0_MUX_PINSWAP_NONE      (0x05)
#define HWSERIAL0_MUX_DEFAULT          (1)   /* Tsurugi default: USART0 ALT1 (PA4/PA5). DEFAULT(PA0/PA1) drives the LEDs. */
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

/* ---- USART1 -> user-facing "Serial2" on ALT2: PD6 TX (= D13) / PD7 RX (= D20,
 *   the AREF pin). The only DU USART1 pin position that exists on this board.
 *   Serial2 shares its pins with SPI0 (SCK = PD6) and with the AREF function
 *   (PD7): Serial2.begin() while SPI is active - or while a shield feeds an
 *   external reference into AREF - is a user-level conflict, exactly like
 *   using D0/D1 as GPIO on a classic Uno while Serial is open. ---- */
#define HWSERIAL1_MUX                   (0x00 /* PORTMUX_USART1_DEFAULT_gc - no pins */)
#define HWSERIAL1_MUX_PINSWAP_1         (0x01 << 3 /* ALT1 absent on DU - placeholder */)
#define HWSERIAL1_MUX_PINSWAP_2         (0x02 << 3 /* PORTMUX_USART1_ALT2_gc - PD6 TX / PD7 RX */)
#define HWSERIAL1_MUX_PINSWAP_NONE      (0x03 << 3)
#define HWSERIAL1_MUX_DEFAULT          (2)   /* Tsurugi: USART1 fixed on ALT2 (PD6/PD7 = D13/D20) */
#define PIN_HWSERIAL1_TX                (NOT_A_PIN)
#define PIN_HWSERIAL1_RX                (NOT_A_PIN)
#define PIN_HWSERIAL1_XCK               (NOT_A_PIN)
#define PIN_HWSERIAL1_XDIR              (NOT_A_PIN)
#define PIN_HWSERIAL1_TX_PINSWAP_1      (NOT_A_PIN)
#define PIN_HWSERIAL1_RX_PINSWAP_1      (NOT_A_PIN)
#define PIN_HWSERIAL1_XCK_PINSWAP_1     (NOT_A_PIN)
#define PIN_HWSERIAL1_XDIR_PINSWAP_1    (NOT_A_PIN)
#define PIN_HWSERIAL1_TX_PINSWAP_2      (PIN_PD6)   /* D13 */
#define PIN_HWSERIAL1_RX_PINSWAP_2      (PIN_PD7)   /* D20 / AREF */
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
#define PIN_A8   (PIN_PA7)   // D2
#define PIN_A9   (PIN_PA6)   // D3
#define PIN_A10  (PIN_PC3)   // D4
#define PIN_A11  (PIN_PD0)   // D5
#define PIN_A12  (PIN_PD1)   // D6
#define PIN_A13  (PIN_PF4)   // D7
#define PIN_A14  (PIN_PF5)   // D8
#define PIN_A15  (PIN_PD2)   // D9
#define PIN_A16  (PIN_PD3)   // D10
#define PIN_A17  (PIN_PD4)   // D11
#define PIN_A18  (PIN_PD5)   // D12
#define PIN_A19  (PIN_PD6)   // D13
#define PIN_A20  (PIN_PD7)   // D20 / AREF

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
#undef D20
static const uint8_t D0  = PIN_PA5;  // RX
static const uint8_t D1  = PIN_PA4;  // TX
static const uint8_t D2  = PIN_PA7;
static const uint8_t D3  = PIN_PA6;  // ~PWM(TCB1 via CCL LUT0)
static const uint8_t D4  = PIN_PC3;  // ~PWM(TCB1 via CCL LUT1)
static const uint8_t D5  = PIN_PD0;
static const uint8_t D6  = PIN_PD1;
static const uint8_t D7  = PIN_PF4;
static const uint8_t D8  = PIN_PF5;
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
static const uint8_t D20 = PIN_PD7;  // AREF (VREFA | GPIO | SPI0 SS | Serial2 RX)

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
static const uint8_t A20  = PIN_A20;

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

/* ---- Pin arrays (ARDUINO_MAIN). Indexed by digital pin number (0..24). ---- */
#ifdef ARDUINO_MAIN
  const uint8_t digital_pin_to_port[] = {
    PA,         //  0 PA5  D0  RX/USART0 RX
    PA,         //  1 PA4  D1  TX/USART0 TX
    PA,         //  2 PA7  D2  USART0 XDIR / AC0 OUT / EVOUTA
    PA,         //  3 PA6  D3  TCB1 PWM via CCL LUT0
    PC,         //  4 PC3  D4  TCB1 PWM via CCL LUT1
    PD,         //  5 PD0  D5  TCA0 WO0
    PD,         //  6 PD1  D6  TCA0 WO1
    PF,         //  7 PF4  D7
    PF,         //  8 PF5  D8
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
    PD,         // 20 PD7  AREF (VREFA)
    PA,         // 21 PA0  XTALHF1
    PA,         // 22 PA1  XTALHF2
    PF,         // 23 PF6  RESET
    PF          // 24 PF7  UPDI
  };

  const uint8_t digital_pin_to_bit_position[] = {
    PIN5_bp,   //  0 PA5  D0
    PIN4_bp,   //  1 PA4  D1
    PIN7_bp,   //  2 PA7  D2
    PIN6_bp,   //  3 PA6  D3
    PIN3_bp,   //  4 PC3  D4
    PIN0_bp,   //  5 PD0  D5
    PIN1_bp,   //  6 PD1  D6
    PIN4_bp,   //  7 PF4  D7
    PIN5_bp,   //  8 PF5  D8
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
    PIN7_bp,   // 20 PD7  D20/AREF
    #if ((CLOCK_SOURCE & 0x03) == 0) // Tsurugi: always true (internal OSCHF, fixed)
      PIN0_bp, // 21 PA0  TX LED
    #else                            // kept only as a safety net for exotic rebuilds
      NOT_A_PIN,
    #endif
    #if ((CLOCK_SOURCE & 0x03) == 1)
      NOT_A_PIN,
    #else
      PIN1_bp, // 22 PA1  RX LED
    #endif
    PIN6_bp,   // 23 PF6 RESET
    PIN7_bp    // 24 PF7 UPDI
  };

  const uint8_t digital_pin_to_bit_mask[] = {
    PIN5_bm,   //  0 PA5  D0
    PIN4_bm,   //  1 PA4  D1
    PIN7_bm,   //  2 PA7  D2
    PIN6_bm,   //  3 PA6  D3
    PIN3_bm,   //  4 PC3  D4
    PIN0_bm,   //  5 PD0  D5
    PIN1_bm,   //  6 PD1  D6
    PIN4_bm,   //  7 PF4  D7
    PIN5_bm,   //  8 PF5  D8
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
    PIN7_bm,   // 20 PD7  D20/AREF
    #if ((CLOCK_SOURCE & 0x03) == 0)
      PIN0_bm, // 21 PA0  TX LED
    #else
      NOT_A_PIN,
    #endif
    #if ((CLOCK_SOURCE & 0x03) == 1)
      NOT_A_PIN,
    #else
      PIN1_bm, // 22 PA1  RX LED
    #endif
    PIN6_bm,   // 23 PF6 RESET
    PIN7_bm    // 24 PF7 UPDI
  };

  /* TCA0 PWM is resolved dynamically from PORTMUX, so TCA0 pins are NOT_ON_TIMER
   * here. D3 (PA6) is marked TIMERB1: it carries TCB1's PWM through CCL LUT0
   * (see WAZAMONO_TCB1_LUTPWM_* above). millis = TCB0. */
  const uint8_t digital_pin_to_timer[] = {
    NOT_ON_TIMER, //  0 PA5  D0
    NOT_ON_TIMER, //  1 PA4  D1
    NOT_ON_TIMER, //  2 PA7  D2
    TIMERB1,      //  3 PA6  D3  (TCB1 PWM via CCL LUT0 - default outlet)
    TIMERB1,      //  4 PC3  D4  (TCB1 PWM via CCL LUT1 - exclusive with D3/D8)
    NOT_ON_TIMER, //  5 PD0  D5  (TCA0 WO0, dynamic)
    NOT_ON_TIMER, //  6 PD1  D6  (TCA0 WO1, dynamic)
    NOT_ON_TIMER, //  7 PF4  D7  (no PWM: TCB0 = millis)
    TIMERB1,      //  8 PF5  D8  (TCB1 WO direct - exclusive with D3/D4)
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
    NOT_ON_TIMER, // 20 PD7  D20/AREF
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
 *  Serial1 = USART0   (D0/D1, ALT1 - the Uno R3 hardware UART; Uno R4 naming.
 *                      Alias of Serial0 - both names reach the same port.)
 *  Serial2 = USART1   (D13 TX / D20(AREF) RX, ALT2 - shares pins with SPI0
 *                      SCK and the AREF function; see the USART1 note above).
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
    #define SERIAL_PORT_HARDWARE    Serial1     /* Uno R3 D0/D1 hardware UART (USART0) */
  #endif
#endif

/* ---- analogReference(EXTERNAL) is available: PD7 (VREFA) is wired to the
 * AREF header pin, exactly like the Uno R3 - but unlike the Uno R3 the pin
 * doubles as GPIO D20/A20 (and SPI0 hardware SS / Serial2 RX) whenever no
 * external reference is in use. Do not drive D20 as an output while a shield
 * feeds a reference into AREF, and do not select EXTERNAL while D20 is an
 * output. ---- */

#endif
