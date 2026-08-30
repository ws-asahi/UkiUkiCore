/* pins_arduino.h - Variant definition for the Wazamono Tachi (AVR64DU32)
 * ---------------------------------------------------------------------------
 * Part of WazamonoCore (a product-specific fork of SpenceKonde/DxCore).
 * DxCore is (C) Spence Konde 2021-2022, open source (LGPL 2.1, see LICENSE.md),
 * based on existing Arduino cores. This variant (C) Workshop Asahi 2026.
 *
 * Board   : Wazamono Tachi  (Pro Micro form factor, AVR64DU32, USB-C)
 * MCU     : AVR64DU32  (TQFP-32; rev.4 returned from the 28-pin part to the
 *           32-pin part - routing needed the TQFP, and the 28-pin variant
 *           saved almost nothing while costing features)
 * Clock   : internal 24 MHz OSCHF, crystal-less (boards.txt: clocksource = 0).
 *           USB CLK_USB (48 MHz) = OSCHF + PLL48M, auto-tuned to the USB SOF.
 *           PA0/PA1 (XTALHF pins) carry the TX/RX activity LEDs - see below.
 *
 *  ===== Pin numbering: Pro Micro compatible (NONCANONICAL), rev.5 =====
 *  (source: WazamonoTachi_Schematics.net + WazamonoTachi_ProMicro互換用ピン構成.csv)
 *   D#   MCU   Pro Micro role / notes                        A#,  AIN
 *   D0   PA5   RX  (Serial1 = USART0 RX, ALT1)               A12, AIN25
 *   D1   PA4   TX  (Serial1 = USART0 TX, ALT1)               A13, AIN24
 *   D2   PA2   SDA (I2C)                                     A14, AIN22
 *   D3   PA3   SCL (I2C) | ~PWM(TCB1 WO, default outlet)     A15, AIN23
 *   D4   PC3   ~PWM(TCB1 via LUT1-OUT) | LUT1-OUT            A6,  AIN31
 *   D5   PD0   ~PWM(TCA0 WO0) | LUT2-IN0                     A16, AIN0
 *   D6   PD1   ~PWM(TCA0 WO1) | LUT2-IN1                     A7,  AIN1
 *   D7   PA6   ~PWM(TCB1 via LUT0-OUT alt) | USART0 XCK      A17, AIN26
 *   D8   PA7   USART0 XDIR | AC0 OUT | EVOUTA | CLKOUT       A8,  AIN27
 *   D9   PD2   ~PWM(TCA0 WO2) | LUT2-IN2 | AINP0 | EVOUTD    A9,  AIN2
 *   D10  PD3   ~PWM(TCA0 WO3) | LUT2-OUT | AINN0             A10, AIN3
 *   D11..D13   (do not exist - gap, like the 32U4 Pro Micro)
 *   D14  PD5   MISO | ~PWM(TCA0 WO5)                         A18, AIN5
 *   D15  PD6   SCK  | Serial2 TX (USART1 ALT2)               A19, AIN6
 *   D16  PD4   MOSI | ~PWM(TCA0 WO4)                         A20, AIN4
 *   D17  PF3   LED_BUILTIN (on-board user LED, active-LOW,
 *              no header) | LUT3-OUT                         A21, AIN19
 *   D18  PD7   A0 | SPI SS | Serial2 RX (USART1 ALT2) | VREFA    AIN7
 *   D19  PF0   A1 | LUT3-IN0                                 AIN16
 *   D20  PF1   A2 | LUT3-IN1                                 AIN17
 *   D21  PF2   A3 | LUT3-IN2 | EVOUTF                        AIN18
 *   D22  PF4   A4 (test pad TP1 only, no header)             AIN20
 *   D23  PF5   A5 (test pad TP2 only, no header)             AIN21
 *   D24..D29   (do not exist - gap)
 *   D30  PA0   TX LED = LED_BUILTIN_TX (on-board, active-LOW, no header)
 *   D31  PA1   RX LED = LED_BUILTIN_RX (on-board, active-LOW, no header)
 *   --- not exposed as a numbered Dn (appended so the arrays are complete) ---
 *        PF6   RESET                                   index 32
 *        PF7   UPDI                                    index 33  (== PIN_PF7, highest)
 *
 *  ===== Peripheral routing (set by this variant + boards.txt) =====
 *   TCA0  -> PORTD (WO0..WO5 = PD0..PD5 = D5/D6/D9/D10/D16/D14) : TCA0_PINS below
 *            -> classic Pro Micro PWM set D3/D5/D6/D9/D10 is fully reproduced;
 *               D14/D16 gain bonus PWM (mutually exclusive with SPI use).
 *   TCB1  -> ONE waveform, THREE exclusive outlets (WAZAMONO_TCB1_PWMMUX):
 *            D3 = PA3 (TCB1 WO, default PORTMUX position, outlet = CCMPEN),
 *            D7 = PA6 (LUT0-OUT alternate position),
 *            D4 = PC3 (LUT1-OUT default position).
 *            The last analogWrite() among the three owns the route (default D3).
 *   millis-> TCB0  : boards.txt MUST pass -DMILLIS_USE_TIMERB0 so TCB1 is free for D3/D4/D7 PWM
 *   SPI0  -> ALT4 (PD4/PD5/PD6/PD7 = D16/D14/D15/D18), the ONLY position offered:
 *            the DEFAULT position (PA4..PA7) is occupied by USART0/Serial1.
 *            Board is SPI host; chip-selects are user GPIO (auto-SS not used).
 *   TWI0  -> default (PA2 SDA / PA3 SCL) = D2/D3, the Pro Micro I2C position.
 *            No UART sharing: Wire and Serial1/Serial2 can all run together.
 *   USART0-> Serial1, ALT1 (PA4 TX / PA5 RX / PA6 XCK / PA7 XDIR) = D1/D0/D7/D8.
 *            Full-function position: XCK/XDIR enable USART-SPI-host ("SPI1")
 *            and RS-485.
 *   USART1-> Serial2, ALT2 (PD6 TX / PD7 RX) = D15/D18. (Only usable USART1
 *            position.) Shares pins with SPI SCK/SS, so Serial2 and SPI are
 *            mutually exclusive (use one or the other).
 *   CCL   -> LUT0: OUT = D7 (alternate position; IN0/IN1 = PA0/PA1 are the
 *            TX/RX LEDs, IN2 = D2 is SDA - use internal/event inputs);
 *            LUT1: OUT = D4 (IN0..IN2 = PC0..PC2 do not exist on the 32-pin
 *            part - internal/event inputs only);
 *            LUT2: IN0/IN1/IN2 = D5/D6/D9, OUT = D10, alt OUT = D15;
 *            LUT3: IN0/IN1/IN2 = D19/D20/D21 (= A1/A2/A3), OUT = D17 (LED_BUILTIN).
 *   AREF  -> PD7 (VREFA) doubles as analog input A0 (D18). Using an external
 *            analog reference costs A0 (and SPI SS / Serial2 RX).
 *   LED   -> three on-board LEDs, all active-LOW (anode to +3V3 via 330R/1k):
 *            PF3 (D17) = LED_BUILTIN (dedicated user LED, not touched by the
 *            core); PA0 (D30) = TX LED = LED_BUILTIN_TX and PA1 (D31) = RX LED
 *            = LED_BUILTIN_RX (USB-CDC activity, driven by
 *            wazamono_tachi_init.cpp). Sketches may drive all three as GPIO.
 *   Serial-> native USB CDC (USBSerial), Leonardo/Micro convention.
 *
 *   NOTE on names: in DxCore USART0 is "Serial0" and USART1 is "Serial1".
 *   On this board the Pro Micro D0/D1 UART is USART0, so the user-facing name
 *   Serial1 must attach to USART0. WAZAMONO_SERIAL1_IS_USART0 (below) makes
 *   the core emit Serial1 as an alias of the Serial0 object (UART0.cpp), and
 *   WAZAMONO_SERIAL2_IS_USART1 renames USART1's object to Serial2 (UART1.cpp,
 *   HardwareSerial.h). Serial0 remains as the internal alias for USART0.
 */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <avr/pgmspace.h>
#include "timers.h"

/* Board identification: ARDUINO_AVR_TACHI is THE macro that identifies this
 * board, following the Arduino convention (ARDUINO_AVR_PROMICRO, ARDUINO_UNOWIFIR4).
 * The Arduino build system defines it from boards.txt (build.board=AVR_TACHI
 * -> -DARDUINO_AVR_TACHI); the fallback below covers builds outside the IDE
 * (e.g. the bootloader tree, which passes it explicitly). It is used by the
 * core, the bundled libraries, the examples and the bootloader sources alike.
 * MCU identification comes from the compiler (-mmcu): __AVR_AVR64DU32__,
 * plus the core's family tags __AVR_DU__ / _AVR_FAMILY / _AVR_PINCOUNT
 * (core_devices.h).
 * DU_32PIN_PINOUT keeps DU-32 feature assumptions in the core;
 * NONCANONICAL_PIN_NUMBERS tells the core to derive (port,bit) from the tables
 * below instead of assuming pin number == port order. */
#ifndef ARDUINO_AVR_TACHI
  #define ARDUINO_AVR_TACHI 1
#endif
#define DU_32PIN_PINOUT
#define NONCANONICAL_PIN_NUMBERS

/* ---- User-facing serial names (core support in UART0.cpp / UART1.cpp /
 * HardwareSerial.h; this header is included by Arduino.h *before* UART.h, so
 * these are visible to every core translation unit): ---------------------
 *   Serial1 -> USART0 (D0/D1, the Pro Micro hardware UART)  [alias of Serial0]
 *   Serial2 -> USART1 (D15 TX / D18 RX)                     [renamed object]  */
#ifndef WAZAMONO_SERIAL1_IS_USART0
  #define WAZAMONO_SERIAL1_IS_USART0
#endif
#ifndef WAZAMONO_SERIAL2_IS_USART1
  #define WAZAMONO_SERIAL2_IS_USART1
#endif

         /*##  ### #   #  ###
          #   #  #  ##  # #
          ####   #  # # #  ###
          #      #  #  ##     #
          #     ### #   #  # */
/* Digital pin number for each MCU pin (Pro Micro layout, rev.5 / AVR64DU32). */
#define PIN_PA5 (0)   // D0  RX  (Serial1 = USART0 RX)
#define PIN_PA4 (1)   // D1  TX  (Serial1 = USART0 TX)
#define PIN_PA2 (2)   // D2  SDA
#define PIN_PA3 (3)   // D3  SCL / TCB1 PWM (WO, default outlet)
#define PIN_PC3 (4)   // D4  TCB1 PWM via LUT1-OUT
#define PIN_PD0 (5)   // D5  TCA0 WO0
#define PIN_PD1 (6)   // D6  TCA0 WO1
#define PIN_PA6 (7)   // D7  TCB1 PWM via LUT0-OUT (alt) / USART0 XCK
#define PIN_PA7 (8)   // D8  USART0 XDIR / AC0 OUT / EVOUTA / CLKOUT
#define PIN_PD2 (9)   // D9  TCA0 WO2 / AINP0 / EVOUTD
#define PIN_PD3 (10)  // D10 TCA0 WO3 / AINN0
//  no  D11..D13              (gap)
#define PIN_PD5 (14)  // D14 MISO / TCA0 WO5
#define PIN_PD6 (15)  // D15 SCK / Serial2 TX
#define PIN_PD4 (16)  // D16 MOSI / TCA0 WO4
#define PIN_PF3 (17)  // D17 LED_BUILTIN (active-LOW, on-board only) / LUT3-OUT
#define PIN_PD7 (18)  // D18 A0 / SPI SS / Serial2 RX / VREFA
#define PIN_PF0 (19)  // D19 A1 / LUT3-IN0
#define PIN_PF1 (20)  // D20 A2 / LUT3-IN1
#define PIN_PF2 (21)  // D21 A3 / LUT3-IN2 / EVOUTF
#define PIN_PF4 (22)  // D22 A4 (test pad only)
#define PIN_PF5 (23)  // D23 A5 (test pad only)
//  no  D24..D29              (gap)
#define PIN_PA0 (30)  // D30 TX LED = LED_BUILTIN_TX (active-LOW, on-board only)
#define PIN_PA1 (31)  // D31 RX LED = LED_BUILTIN_RX (active-LOW, on-board only)
#define PIN_PF6 (32)  // RESET
#define PIN_PF7 (33)  // UPDI  (highest index -> sets NUM_DIGITAL_PINS = 34)

/* ---- Event output pins: FIXED by the board's pin-configuration table ----
 * One pin per event output, no alternatives. Libraries (CustomLogic, and the
 * EventSystem library in its Wazamono form) route event outputs to these only.
 *   EVOUTA -> PA7 = D8   (PORTMUX ALT1;    default PA2 is the Grove SDA)
 *   EVOUTD -> PD2 = D9   (PORTMUX default)
 *   EVOUTF -> PF2 = D21  (PORTMUX default) */
#define WAZAMONO_EVOUTA_PIN            (PIN_PA7)
#define WAZAMONO_EVOUTA_ALT            (1)
#define WAZAMONO_EVOUTD_PIN            (PIN_PD2)
#define WAZAMONO_EVOUTD_ALT            (0)
#define WAZAMONO_EVOUTF_PIN            (PIN_PF2)
#define WAZAMONO_EVOUTF_ALT            (0)

         /*##   ##   ###  ###  ###  ###
          #   # #  # #      #  #    #
          ####  ####  ###   #  #     ###
          #   # #  #     #  #  #        #
          ####  #  # ####  ###  ###  # */
#define PINS_COUNT                     (34)  // length of the pin tables (incl. gaps/reserved)
#define NUM_ANALOG_INPUTS              (31)  // highest ADC channel in use is AIN31 (PC3)
// NUM_DIGITAL_PINS / NUM_TOTAL_PINS  -> auto = PIN_PF7 + 1 = 34
// NUM_INTERNALLY_USED_PINS           -> auto = 0 (crystal-less: no reserved GPIO)

#if !defined(LED_BUILTIN)
  #define LED_BUILTIN                  (PIN_PF3)   // D17, dedicated user LED (active-LOW)
#endif
#define LED_BUILTIN_TX                 (PIN_PA0)   // D30 = USB-CDC TX activity LED (active-LOW)
#define LED_BUILTIN_RX                 (PIN_PA1)   // D31 = USB-CDC RX activity LED (active-LOW)

/* 32U4 Leonardo / Pro Micro activity-LED helper macros, same semantics:
 * xxLED1 = ON, xxLED0 = OFF (both LEDs are active-LOW on this board too;
 * TX = PA0/D30, RX = PA1/D31). The 32U4 TX_RX_LED_INIT only touched DDR;
 * here OUT is set HIGH (= off) before DIR so the LEDs cannot flash on.
 * As on the 32U4 (USBCore.cpp), the core's own CDC activity one-shot
 * (wazamono_tachi_init.cpp) drives the same pins, so a sketch holding an
 * LED on with these macros will see the same brief fight over the pin. */
#define TX_RX_LED_INIT                 (PORTA.OUTSET = (PIN0_bm | PIN1_bm), \
                                        PORTA.DIRSET = (PIN0_bm | PIN1_bm))
#define TXLED1                         (PORTA.OUTCLR = PIN0_bm)   /* TX LED (PA0/D30) ON  */
#define TXLED0                         (PORTA.OUTSET = PIN0_bm)   /* TX LED (PA0/D30) OFF */
#define RXLED1                         (PORTA.OUTCLR = PIN1_bm)   /* RX LED (PA1/D31) ON  */
#define RXLED0                         (PORTA.OUTSET = PIN1_bm)   /* RX LED (PA1/D31) OFF */

#ifdef CORE_ATTACH_OLD
  #define EXTERNAL_NUM_INTERRUPTS      (48)
#endif

         /*   #  ###   ### ####   ###   ###
          ## ## #   # #    #   # #   # #
          # # # ##### #    ####  #   #  ###
          #   # #   # #    # #   #   #     #
          #   # #   #  ### #  #   ###   ## */
/* Explicit maps (NONCANONICAL numbering: arithmetic shortcuts cannot be used).
 * PA0/PA1 (the LEDs) have no ADC channel on the DU (AIN20..27 are PF4..PF7 and
 * PA2..PA7 - DS40002548B, ADC MUXPOS) and are not mapped. */
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
/* millis lives on TCB0 (boards.txt -DMILLIS_USE_TIMERB0), leaving TCB1 free for
 * PWM and for tone(). ONE TCB1 waveform, THREE exclusive outlets
 * (WAZAMONO_TCB1_PWMMUX, core mechanism in cores/dxcore/wazamono_lutpwm.{h,c}):
 *   D3 = PA3  TCB1's own WO pin (default PORTMUX position, outlet = CCMPEN)
 *   D7 = PA6  LUT0-OUT, alternate position (CCLROUTEA LUT0 = 1)
 *   D4 = PC3  LUT1-OUT, default position
 * The last analogWrite() among the three owns the route (default D3); the
 * other two return to plain GPIO. All three share TCB1's frequency and duty.
 * tone() uses TCB1 (millis on TCB0), so tone() suspends D3/D4/D7 PWM - the
 * Pro Micro equivalent is tone() stopping Timer3 PWM on D5.
 * A LUT already configured by other code (CustomLogic / direct registers) is
 * never clobbered: analogWrite() falls back to digital output on that pin. */
#if defined(MILLIS_USE_TIMERB1)
  #define digitalPinHasPWMTCB(p) (0)                   /* TCB1 is millis; no TCB PWM */
#else
  #define digitalPinHasPWMTCB(p) ((p) == PIN_PA3 || (p) == PIN_PA6 || (p) == PIN_PC3)
#endif
#define digitalPinHasPWMTCA(p) ( \
    (p) == PIN_PD0 || (p) == PIN_PD1 || (p) == PIN_PD2 || \
    (p) == PIN_PD3 || (p) == PIN_PD4 || (p) == PIN_PD5 )

/* TCA0 is routed to PORTD, so WO0..WO5 land on PD0..PD5 = D5/D6/D9/D10/D16/D14.
 * Together with TCB1 on D3 this reproduces the classic Pro Micro PWM pin set
 * (D3, D5, D6, D9, D10) exactly, plus bonus PWM on D4/D7 (TCB1 route) and
 * D14/D16 (SPI-exclusive). */
#define TCA0_PINS                       (PORTMUX_TCA0_PORTD_gc)
#define TCB0_PINS                       (0x00)   // TCB0 = millis; WO unused (default PA2 position)
#define TCB1_PINS                       (0x00)   // TCB1 WO on PA3 (default) = D3, the default outlet

#define PIN_TCA0_WO0_INIT               (PIN_PD0)
#define PIN_TCB0_WO_INIT                (PIN_PA2)   // TCB0 = millis; not enabled for PWM
#define PIN_TCB1_WO_INIT                (PIN_PA3)

#define WAZAMONO_TCB1_PWMMUX            (1)         /* exclusive D3/D4/D7 routing (default D3) */
#define WAZAMONO_TCB1_PWM_WO_PIN        (PIN_PA3)   /* D3: TCB1 WO itself (TCB1_PINS = default), outlet = CCMPEN */
#define WAZAMONO_TCB1_PWM_LUT0_PIN      (PIN_PA6)   /* D7: LUT0-OUT, alternate position */
#define WAZAMONO_TCB1_PWM_LUT0          (0)
#define WAZAMONO_TCB1_PWM_LUT0_ALT      (1)
#define WAZAMONO_TCB1_PWM_LUT1_PIN      (PIN_PC3)   /* D4: LUT1-OUT, default position */
#define WAZAMONO_TCB1_PWM_LUT1          (1)
#define WAZAMONO_TCB1_PWM_LUT1_ALT      (0)

#define digitalPinHasPWM(p)             (digitalPinHasPWMTCB(p) || digitalPinHasPWMTCA(p))

         /*##   ###  ####  ##### #   # #   # #   #
          #   # #   # #   #   #   ## ## #   #  # #
          ####  #   # ####    #   # # # #   #   #
          #     #   # #  #    #   #   # #   #  # #
          #      ###  #   #   #   #   #  ###  #   */
#define SPI_INTERFACES_COUNT            (1)

// SPI 0  (host; chip-selects are user GPIO). ALT4 (PD4..PD7) is the ONLY
// position on this board: the DEFAULT position (PA4..PA7) belongs to
// USART0/Serial1, so it is deliberately not offered to the SPI library.
// (USART0's MSPI host mode on D0/D1/D7 is the informal "SPI1" of the pin
// table; it is driven through the USART, not through this SPI library.)
#define SPI_MUX                         (PORTMUX_SPI0_ALT4_gc)
#define SPI_MUX_PINSWAP_NONE            (PORTMUX_SPI0_NONE_gc)
#define PIN_SPI_MOSI                    (PIN_PD4)   // D16
#define PIN_SPI_MISO                    (PIN_PD5)   // D14
#define PIN_SPI_SCK                     (PIN_PD6)   // D15
#define PIN_SPI_SS                      (PIN_PD7)   // D18 (hardware SS; host mode -> user GPIO)
#define PIN_SPI_SS_HARDWARE             (PIN_PD7)   // the ALT4 SS pin itself (client mode / SPISlave)

// TWI 0  (Grove I2C)
#define PIN_WIRE_SDA                    (PIN_PA2)   // D2
#define PIN_WIRE_SCL                    (PIN_PA3)   // D3
#define PIN_WIRE_SDA_PINSWAP_1          (NOT_A_PIN)
#define PIN_WIRE_SCL_PINSWAP_1          (NOT_A_PIN)
#define PIN_WIRE_SDA_PINSWAP_3          (NOT_A_PIN) // ALT3 = PA0/PA1 = the on-board TX/RX LEDs: not offered
#define PIN_WIRE_SCL_PINSWAP_3          (NOT_A_PIN)

// USART0 -> user-facing "Serial1" (internal Serial0), default position ALT1
// (PA4 TX / PA5 RX / PA6 XCK / PA7 XDIR = D1/D0/D7/D8): the full-function
// position, enabling USART-SPI-host mode and RS-485 XDIR on header pins.
#define HWSERIAL0_MUX                   (0x00 /* PORTMUX_USART0_DEFAULT_gc */)
#define HWSERIAL0_MUX_PINSWAP_1         (0x01 /* PORTMUX_USART0_ALT1_gc */)
#define HWSERIAL0_MUX_PINSWAP_2         (0x02 /* PORTMUX_USART0_ALT2_gc */)
#define HWSERIAL0_MUX_PINSWAP_3         (0x03 /* PORTMUX_USART0_ALT3_gc */)
#define HWSERIAL0_MUX_PINSWAP_NONE      (0x05)
#define HWSERIAL0_MUX_DEFAULT          (1)   /* Tachi default: USART0 ALT1 (PA4..PA7). DEFAULT (PA0/PA1) = the on-board TX/RX LEDs: not offered. */
#define PIN_HWSERIAL0_TX                (NOT_A_PIN) /* DEFAULT = PA0 (TX LED, no header) */
#define PIN_HWSERIAL0_RX                (NOT_A_PIN) /* DEFAULT = PA1 (RX LED, no header) */
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

// USART1 -> user-facing "Serial2", only usable position is ALT2 (PD6/PD7 = D15/D18)
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
/* Arduino analog aliases (rev.5). A0..A3 = D18..D21 match the Pro Micro
 * exactly. A4/A5 = D22/D23 (PF4/PF5) also match the SparkFun promicro variant
 * numerically (A4 = 22, A5 = 23); there they are unrouted 32U4 pins, here they
 * are real ADC inputs on test pads TP1/TP2 (no header).
 * A6..A10 point at the same physical pins as on the Pro Micro
 * (D4/D6/D8/D9/D10) but carry those pins' OWN numbers (4/6/8/9/10), not the
 * Leonardo duplicate numbers 24..28; analogRead(A6) etc. behave identically,
 * only the numeric value of the alias differs (documented incompatibility).
 * There is NO A11 gap: the Leonardo reserves A11 for D12, which does not
 * exist on the Pro Micro pinout at all, so no sketch that runs on this
 * board can rely on it. The extension aliases therefore continue densely:
 * A11..A16 over D0/D1/D2/D3/D5/D7 and A17..A20 over D14..D17 (A# = D# + 3
 * in that group). D30/D31 (PA0/PA1, the LEDs) have no ADC. */
#define PIN_A0   (PIN_PD7)   // D18 (also SPI SS / Serial2 RX / VREFA)
#define PIN_A1   (PIN_PF0)   // D19
#define PIN_A2   (PIN_PF1)   // D20
#define PIN_A3   (PIN_PF2)   // D21
#define PIN_A4   (PIN_PF4)   // D22 (test pad TP1)
#define PIN_A5   (PIN_PF5)   // D23 (test pad TP2)
#define PIN_A6   (PIN_PC3)   // D4
#define PIN_A7   (PIN_PD1)   // D6
#define PIN_A8   (PIN_PA7)   // D8
#define PIN_A9   (PIN_PD2)   // D9
#define PIN_A10  (PIN_PD3)   // D10
#define PIN_A11  (PIN_PA5)   // D0
#define PIN_A12  (PIN_PA4)   // D1
#define PIN_A13  (PIN_PA2)   // D2
#define PIN_A14  (PIN_PA3)   // D3
#define PIN_A15  (PIN_PD0)   // D5
#define PIN_A16  (PIN_PA6)   // D7
#define PIN_A17  (PIN_PD5)   // D14
#define PIN_A18  (PIN_PD6)   // D15
#define PIN_A19  (PIN_PD4)   // D16
#define PIN_A20  (PIN_PF3)   // D17 (LED_BUILTIN)

/* --- Uno R4 style number-prefixed digital pin aliases ---
 * D-number == Arduino digital pin number. Internal-only pins (PF6 RESET,
 * PF7 UPDI) are intentionally NOT exposed as Dn. The on-board LEDs D17/D30/D31
 * and the test pads D22/D23 ARE exposed so sketches can drive them by number.
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
#undef D22
#undef D23
#undef D30
#undef D31
static const uint8_t D0  = PIN_PA5;  // RX (Serial1)
static const uint8_t D1  = PIN_PA4;  // TX (Serial1)
static const uint8_t D2  = PIN_PA2;  // SDA
static const uint8_t D3  = PIN_PA3;  // SCL / ~PWM(TCB1 WO, default outlet)
static const uint8_t D4  = PIN_PC3;  // ~PWM(TCB1 via LUT1)
static const uint8_t D5  = PIN_PD0;  // ~PWM
static const uint8_t D6  = PIN_PD1;  // ~PWM
static const uint8_t D7  = PIN_PA6;  // ~PWM(TCB1 via LUT0 alt) / USART0 XCK
static const uint8_t D8  = PIN_PA7;  // USART0 XDIR / AC0 OUT / EVOUTA / CLKOUT
static const uint8_t D9  = PIN_PD2;  // ~PWM
static const uint8_t D10 = PIN_PD3;  // ~PWM
static const uint8_t D14 = PIN_PD5;  // MISO / ~PWM
static const uint8_t D15 = PIN_PD6;  // SCK / Serial2 TX
static const uint8_t D16 = PIN_PD4;  // MOSI / ~PWM
static const uint8_t D17 = PIN_PF3;  // LED_BUILTIN (active-LOW, on-board only)
static const uint8_t D18 = PIN_PD7;  // A0 / SPI SS / Serial2 RX
static const uint8_t D19 = PIN_PF0;  // A1
static const uint8_t D20 = PIN_PF1;  // A2
static const uint8_t D21 = PIN_PF2;  // A3 / EVOUTF
static const uint8_t D22 = PIN_PF4;  // A4 (test pad TP1)
static const uint8_t D23 = PIN_PF5;  // A5 (test pad TP2)
static const uint8_t D30 = PIN_PA0;  // TX LED = LED_BUILTIN_TX (active-LOW, on-board only)
static const uint8_t D31 = PIN_PA1;  // RX LED = LED_BUILTIN_RX (active-LOW, on-board only)

static const uint8_t A0   = PIN_A0;
static const uint8_t A1   = PIN_A1;
static const uint8_t A2   = PIN_A2;
static const uint8_t A3   = PIN_A3;
static const uint8_t A4   = PIN_A4;   // D22 / PF4 (test pad TP1)
static const uint8_t A5   = PIN_A5;   // D23 / PF5 (test pad TP2)
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

         /*##  ### #   #      ###  ####  ####   ###  #   #  ###
          #   #  #  ##  #     #   # #   # #   # #   #  # #  #
          ####   #  # # #     ##### ####  ####  #####   #    ###
          #      #  #  ##     #   # #  #  #  #  #   #   #       #
          #     ### #   #     #   # #   # #   # #   #   #    # */
#ifdef ARDUINO_MAIN
  // Indexed by digital pin number (0..33). Gaps are NOT_A_PORT.
  const uint8_t digital_pin_to_port[] = {
    PA,         //  0 PA5  D0 RX (Serial1)
    PA,         //  1 PA4  D1 TX (Serial1)
    PA,         //  2 PA2  D2 SDA  (TCB0 WO - TCB0 is millis on this board)
    PA,         //  3 PA3  D3 SCL  (TCB1 WO - default PWM outlet, exclusive with D4/D7)
    PC,         //  4 PC3  D4      (TCB1 PWM via CCL LUT1 - exclusive with D3/D7)
    PD,         //  5 PD0  D5  (TCA0 WO0, dynamic)
    PD,         //  6 PD1  D6  (TCA0 WO1, dynamic)
    PA,         //  7 PA6  D7 XCK  (TCB1 PWM via CCL LUT0 alt - exclusive with D3/D4)
    PA,         //  8 PA7  D8 XDIR/AC0/EVOUTA
    PD,         //  9 PD2  D9  (TCA0 WO2, dynamic)
    PD,         // 10 PD3  D10 (TCA0 WO3, dynamic)
    NOT_A_PORT, // 11 (gap)
    NOT_A_PORT, // 12 (gap)
    NOT_A_PORT, // 13 (gap)
    PD,         // 14 PD5  D14 MISO (TCA0 WO5, dynamic)
    PD,         // 15 PD6  D15 SCK/Serial2 TX
    PD,         // 16 PD4  D16 MOSI (TCA0 WO4, dynamic)
    PF,         // 17 PF3  D17 LED_BUILTIN
    PD,         // 18 PD7  D18 A0/SS/Serial2 RX
    PF,         // 19 PF0  D19 A1
    PF,         // 20 PF1  D20 A2
    PF,         // 21 PF2  D21 A3/EVOUTF
    PF,         // 22 PF4  D22 A4 (test pad)
    PF,         // 23 PF5  D23 A5 (test pad)
    NOT_A_PORT, // 24 (gap)
    NOT_A_PORT, // 25 (gap)
    NOT_A_PORT, // 26 (gap)
    NOT_A_PORT, // 27 (gap)
    NOT_A_PORT, // 28 (gap)
    NOT_A_PORT, // 29 (gap)
    PA,         // 30 PA0  D30 TX LED
    PA,         // 31 PA1  D31 RX LED
    PF,         // 32 PF6  RESET
    PF          // 33 PF7  UPDI
  };

  /* Bit position within the port (for PINnCTRL access). */
  const uint8_t digital_pin_to_bit_position[] = {
    PIN5_bp,   //  0 PA5  D0 RX (Serial1)
    PIN4_bp,   //  1 PA4  D1 TX (Serial1)
    PIN2_bp,   //  2 PA2  D2 SDA  (TCB0 WO - TCB0 is millis on this board)
    PIN3_bp,   //  3 PA3  D3 SCL  (TCB1 WO - default PWM outlet, exclusive with D4/D7)
    PIN3_bp,   //  4 PC3  D4      (TCB1 PWM via CCL LUT1 - exclusive with D3/D7)
    PIN0_bp,   //  5 PD0  D5  (TCA0 WO0, dynamic)
    PIN1_bp,   //  6 PD1  D6  (TCA0 WO1, dynamic)
    PIN6_bp,   //  7 PA6  D7 XCK  (TCB1 PWM via CCL LUT0 alt - exclusive with D3/D4)
    PIN7_bp,   //  8 PA7  D8 XDIR/AC0/EVOUTA
    PIN2_bp,   //  9 PD2  D9  (TCA0 WO2, dynamic)
    PIN3_bp,   // 10 PD3  D10 (TCA0 WO3, dynamic)
    NOT_A_PIN, // 11 (gap)
    NOT_A_PIN, // 12 (gap)
    NOT_A_PIN, // 13 (gap)
    PIN5_bp,   // 14 PD5  D14 MISO (TCA0 WO5, dynamic)
    PIN6_bp,   // 15 PD6  D15 SCK/Serial2 TX
    PIN4_bp,   // 16 PD4  D16 MOSI (TCA0 WO4, dynamic)
    PIN3_bp,   // 17 PF3  D17 LED_BUILTIN
    PIN7_bp,   // 18 PD7  D18 A0/SS/Serial2 RX
    PIN0_bp,   // 19 PF0  D19 A1
    PIN1_bp,   // 20 PF1  D20 A2
    PIN2_bp,   // 21 PF2  D21 A3/EVOUTF
    PIN4_bp,   // 22 PF4  D22 A4 (test pad)
    PIN5_bp,   // 23 PF5  D23 A5 (test pad)
    NOT_A_PIN, // 24 (gap)
    NOT_A_PIN, // 25 (gap)
    NOT_A_PIN, // 26 (gap)
    NOT_A_PIN, // 27 (gap)
    NOT_A_PIN, // 28 (gap)
    NOT_A_PIN, // 29 (gap)
    PIN0_bp,   // 30 PA0  D30 TX LED
    PIN1_bp,   // 31 PA1  D31 RX LED
    PIN6_bp,   // 32 PF6  RESET
    PIN7_bp    // 33 PF7  UPDI
  };

  const uint8_t digital_pin_to_bit_mask[] = {
    PIN5_bm,   //  0 PA5  D0 RX (Serial1)
    PIN4_bm,   //  1 PA4  D1 TX (Serial1)
    PIN2_bm,   //  2 PA2  D2 SDA  (TCB0 WO - TCB0 is millis on this board)
    PIN3_bm,   //  3 PA3  D3 SCL  (TCB1 WO - default PWM outlet, exclusive with D4/D7)
    PIN3_bm,   //  4 PC3  D4      (TCB1 PWM via CCL LUT1 - exclusive with D3/D7)
    PIN0_bm,   //  5 PD0  D5  (TCA0 WO0, dynamic)
    PIN1_bm,   //  6 PD1  D6  (TCA0 WO1, dynamic)
    PIN6_bm,   //  7 PA6  D7 XCK  (TCB1 PWM via CCL LUT0 alt - exclusive with D3/D4)
    PIN7_bm,   //  8 PA7  D8 XDIR/AC0/EVOUTA
    PIN2_bm,   //  9 PD2  D9  (TCA0 WO2, dynamic)
    PIN3_bm,   // 10 PD3  D10 (TCA0 WO3, dynamic)
    NOT_A_PIN, // 11 (gap)
    NOT_A_PIN, // 12 (gap)
    NOT_A_PIN, // 13 (gap)
    PIN5_bm,   // 14 PD5  D14 MISO (TCA0 WO5, dynamic)
    PIN6_bm,   // 15 PD6  D15 SCK/Serial2 TX
    PIN4_bm,   // 16 PD4  D16 MOSI (TCA0 WO4, dynamic)
    PIN3_bm,   // 17 PF3  D17 LED_BUILTIN
    PIN7_bm,   // 18 PD7  D18 A0/SS/Serial2 RX
    PIN0_bm,   // 19 PF0  D19 A1
    PIN1_bm,   // 20 PF1  D20 A2
    PIN2_bm,   // 21 PF2  D21 A3/EVOUTF
    PIN4_bm,   // 22 PF4  D22 A4 (test pad)
    PIN5_bm,   // 23 PF5  D23 A5 (test pad)
    NOT_A_PIN, // 24 (gap)
    NOT_A_PIN, // 25 (gap)
    NOT_A_PIN, // 26 (gap)
    NOT_A_PIN, // 27 (gap)
    NOT_A_PIN, // 28 (gap)
    NOT_A_PIN, // 29 (gap)
    PIN0_bm,   // 30 PA0  D30 TX LED
    PIN1_bm,   // 31 PA1  D31 RX LED
    PIN6_bm,   // 32 PF6  RESET
    PIN7_bm    // 33 PF7  UPDI
  };

  /* TCA0 PWM is resolved dynamically from PORTMUX, so TCA0 pins are NOT_ON_TIMER
   * here. TIMERB1 is set on all three TCB1 outlets (D3 = WO, D4 = LUT1-OUT,
   * D7 = LUT0-OUT alt): the core's TCB analogWrite() path then asks the
   * WAZAMONO_TCB1_PWMMUX hook (wazamono_lutpwm.c) to open the right outlet. */
  const uint8_t digital_pin_to_timer[] = {
    NOT_ON_TIMER, //  0 PA5  D0 RX (Serial1)
    NOT_ON_TIMER, //  1 PA4  D1 TX (Serial1)
    TIMERB0,      //  2 PA2  D2 SDA  (TCB0 WO - TCB0 is millis on this board)
    TIMERB1,      //  3 PA3  D3 SCL  (TCB1 WO - default PWM outlet, exclusive with D4/D7)
    TIMERB1,      //  4 PC3  D4      (TCB1 PWM via CCL LUT1 - exclusive with D3/D7)
    NOT_ON_TIMER, //  5 PD0  D5  (TCA0 WO0, dynamic)
    NOT_ON_TIMER, //  6 PD1  D6  (TCA0 WO1, dynamic)
    TIMERB1,      //  7 PA6  D7 XCK  (TCB1 PWM via CCL LUT0 alt - exclusive with D3/D4)
    NOT_ON_TIMER, //  8 PA7  D8 XDIR/AC0/EVOUTA
    NOT_ON_TIMER, //  9 PD2  D9  (TCA0 WO2, dynamic)
    NOT_ON_TIMER, // 10 PD3  D10 (TCA0 WO3, dynamic)
    NOT_ON_TIMER, // 11 (gap)
    NOT_ON_TIMER, // 12 (gap)
    NOT_ON_TIMER, // 13 (gap)
    NOT_ON_TIMER, // 14 PD5  D14 MISO (TCA0 WO5, dynamic)
    NOT_ON_TIMER, // 15 PD6  D15 SCK/Serial2 TX
    NOT_ON_TIMER, // 16 PD4  D16 MOSI (TCA0 WO4, dynamic)
    NOT_ON_TIMER, // 17 PF3  D17 LED_BUILTIN
    NOT_ON_TIMER, // 18 PD7  D18 A0/SS/Serial2 RX
    NOT_ON_TIMER, // 19 PF0  D19 A1
    NOT_ON_TIMER, // 20 PF1  D20 A2
    NOT_ON_TIMER, // 21 PF2  D21 A3/EVOUTF
    NOT_ON_TIMER, // 22 PF4  D22 A4 (test pad)
    NOT_ON_TIMER, // 23 PF5  D23 A5 (test pad)
    NOT_ON_TIMER, // 24 (gap)
    NOT_ON_TIMER, // 25 (gap)
    NOT_ON_TIMER, // 26 (gap)
    NOT_ON_TIMER, // 27 (gap)
    NOT_ON_TIMER, // 28 (gap)
    NOT_ON_TIMER, // 29 (gap)
    NOT_ON_TIMER, // 30 PA0  D30 TX LED
    NOT_ON_TIMER, // 31 PA1  D31 RX LED
    NOT_ON_TIMER, // 32 PF6  RESET
    NOT_ON_TIMER  // 33 PF7  UPDI
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
 *  Serial1 = USART0   (D0/D1, ALT1 - the Pro Micro hardware UART; alias of the
 *                      core's Serial0 object, see WAZAMONO_SERIAL1_IS_USART0)
 *  Serial2 = USART1   (D15 TX / D18 RX, ALT2 - extra UART; shares pins with SPI
 *                      SCK/SS, so Serial2 and SPI are mutually exclusive)
 *  Serial0 remains the DxCore-internal name for USART0 (same object as Serial1).
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
    #define SERIAL_PORT_HARDWARE    Serial1     /* Pro Micro hardware UART on D0/D1 */
  #endif
  /* Pro Micro / Leonardo define SERIAL_PORT_HARDWARE_OPEN = Serial1 (the D0/D1
   * UART is the "first open" port there, and it is here too: Serial2 shares
   * its pins with SPI SCK/SS). Matching that value keeps sketches written
   * against the SparkFun variant on the same physical pins. */
  #ifndef SERIAL_PORT_HARDWARE_OPEN
    #define SERIAL_PORT_HARDWARE_OPEN  Serial1
  #endif
  /* 32U4-core alias used by Caterina-era and SAMD-portable sketches. */
  #ifndef SerialUSB
    #define SerialUSB               SERIAL_PORT_USBVIRTUAL
  #endif
#endif

#endif
