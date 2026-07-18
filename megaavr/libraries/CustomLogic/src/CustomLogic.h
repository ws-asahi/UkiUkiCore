/* CustomLogic.h - Beginner-friendly CCL logic gates for WazamonoCore.
 *
 * Part of WazamonoCore for the AVR DU-series Wazamono boards.
 * (C) 2026 Yusuke Shimizu - Workshop Asahi. LGPL 2.1.
 *
 * Provenance: this library is an original, independent implementation
 * written from the AVR64DU28/32 data sheet (DS40002548A) and the official
 * Microchip device headers only. It does not use, include, or derive code
 * from the DxCore/megaTinyCore "Logic" library or any other CCL library.
 *
 * Pre-instantiated objects turn a CCL look-up table into a hardware logic
 * gate that keeps working with zero CPU involvement:
 *
 *   CustomLogic.begin(AND);          // OUT = IN0 AND IN1
 *   CustomLogic.begin(AND, OR);      // OUT = (IN0 AND IN1) OR IN2
 *   CustomLogic.beginTruthTable(0x96, 3);   // any function of 3 inputs
 *
 * Units and pins (fixed by the CCL hardware):
 *                 IN0        IN1        IN2        OUT        OUT (alt)
 *   CustomLogic
 *     Tachi       A3 (PD0)   A2 (PD1)   A1 (PD2)   A0 (PD3)   D1  (PD6)
 *     Tsurugi     D5 (PD0)   D6 (PD1)   D9 (PD2)   D10 (PD3)  D13 (PD6)
 *     Kunai       D4 (PA0)   D5 (PA1)   D3 (PA2)   D2 (PA3)   D8  (PA6)
 *   CustomLogic1  (not available on Kunai)
 *     Tachi       D5 (PF0)   D6 (PF1)   D7 (PF2)   D8 (PF3)   -
 *     Tsurugi     A0 (PF0)   A1 (PF1)   A2 (PF2)   A3 (PF3)   -
 *
 * The result can also be sent to the event-output pins - setOutput()/
 * addOutput() take care of the event system for you. These pins are FIXED
 * per board (one pin per event output, from the pin-configuration table):
 *                 EVOUTA        EVOUTD        EVOUTF
 *     Tachi       D2  (PA2)     D0 (PD7)      D7 (PF2)
 *     Tsurugi     D8  (PA7)     D9 (PD2)      A2 (PF2)
 *     Kunai       D0  (PA7)     D7 (PD7)      -
 *
 * Input pins are configured with pull-ups, so you can wire buttons
 * straight to GND; driven logic signals simply override the pull-up.
 */
#ifndef CUSTOMLOGIC_H
#define CUSTOMLOGIC_H

#include <Arduino.h>

#if !defined(WAZAMONO_BOARD_TACHI) && !defined(WAZAMONO_BOARD_TSURUGI) && !defined(WAZAMONO_BOARD_KUNAI)
  #error "CustomLogic supports Wazamono boards only."
#endif

/* Logic operations for begin() */
enum LogicType : uint8_t {
  AND = 0,  /* HIGH while both sides are HIGH        */
  OR,       /* HIGH while either side is HIGH        */
  XOR,      /* HIGH while exactly one side is HIGH   */
  NAND,     /* inverted AND                          */
  NOR,      /* inverted OR                           */
  XNOR,     /* inverted XOR                          */
  NOT,      /* inverter - only valid as begin(NOT), using IN0 alone */
  NOP       /* no operation: begin(NOP) = buffer on IN0;
             * begin(NOP, x) skips IN0 -> x applies to IN1 and IN2;
             * begin(x, NOP) skips IN2 (same as begin(x)) */
};

/* Where an input comes from. Every input starts out as LOGIC_PIN, i.e. the
 * unit's own IN0/IN1/IN2 pin - change it with setInputIN0/IN1/IN2().
 *
 * The other sources need no wire at all: they are internal connections
 * inside the chip, so they cost no pin and no CPU time. */
enum LogicInput : uint8_t {
  LOGIC_PIN = 0,      /* the unit's own input pin (default)                    */
  LOGIC_ANALOG_COMP,  /* the AnalogComp result (AC0) - just call
                       * AnalogComp.begin(); no output pin is needed           */
  LOGIC_OWN_OUTPUT,   /* this unit's own output - the way to build latches
                       * (only on CustomLogic; see the note below)             */
  LOGIC_OTHER_UNIT,   /* the other CustomLogic unit's output - two-stage logic
                       * (Tachi and Tsurugi only)                              */
  LOGIC_EVENT_A,      /* an event connection - the way to feed ANY pin into
                       * a LUT; wire it with the EventSystem library:          */
  LOGIC_EVENT_B       /* EventSystem.connect(pin, EVENT_TO_LOGIC_A / _B)       */
};

/* Note on LOGIC_OWN_OUTPUT: the CCL feeds back the output of the *even* LUT
 * of each LUT pair. CustomLogic is that even LUT (LUT2 on Tachi/Tsurugi, LUT0
 * on Kunai), so it can see its own output. CustomLogic1 is the odd LUT, so
 * what it would see there is CustomLogic's output - which is exactly what
 * LOGIC_OTHER_UNIT means. setInput() therefore rejects LOGIC_OWN_OUTPUT on
 * CustomLogic1, and LOGIC_OTHER_UNIT on Kunai (which has a single unit). */

class CustomLogicClass {
public:
  CustomLogicClass(uint8_t lut, const uint8_t *pins); /* internal */

  /* Two-input gate: OUT = IN0 (logic1) IN1. The unused IN2 is ignored
   * regardless of its pin state.
   *   begin(NOT) makes a one-input inverter on IN0.
   *   begin(NOP) makes a one-input buffer on IN0 (OUT follows IN0).
   * The result appears on the OUT pin immediately and permanently -
   * no code runs in loop(). */
  bool begin(LogicType logic1);

  /* Three-input logic, combining two operations left to right:
   *   OUT = (IN0 logic1 IN1) logic2 IN2
   * Examples: begin(OR, OR) is a 3-input OR; begin(AND, OR) is HIGH
   * while both IN0 and IN1 are HIGH - or IN2 is HIGH; begin(XOR, XOR)
   * is a 3-input parity.
   * NOP skips one side: begin(NOP, x) ignores IN0 and applies x to
   * IN1 and IN2; begin(x, NOP) ignores IN2 (same as begin(x)).
   * NOT cannot be combined, and (NOP, NOP) is invalid (returns false). */
  bool begin(LogicType logic1, LogicType logic2);

  /* Define the output for every input combination yourself. Bit i of
   * truthTable is the output when the inputs spell the number i
   * (IN2 = bit 2, IN1 = bit 1, IN0 = bit 0). Example: 0b10010110 (0x96)
   * makes a 3-input XOR (odd parity). */
  bool beginTruthTable(uint8_t truthTable, uint8_t numInputs = 3);

  /* Choose where an input comes from (LOGIC_PIN by default). May be called
   * before or after begin(); it takes effect immediately. Returns false if
   * the source is not available for this unit on this board.
   * An input that is not fed from a pin leaves that pin completely alone,
   * so it stays free for other uses. */
  bool setInput(uint8_t input, LogicInput source);   /* input = 0, 1 or 2 */
  bool setInputIN0(LogicInput source) { return setInput(0, source); }
  bool setInputIN1(LogicInput source) { return setInput(1, source); }
  bool setInputIN2(LogicInput source) { return setInput(2, source); }

  /* Send the result to a pin. Any of the unit's output pins works: its
   * dedicated OUT pin (the default), its alternate OUT pin, or one of the
   * event-output pins - the event system is set up for you, no Event library
   * needed. May be called before or after begin().
   *   setOutput(pin)  - the result appears HERE (and nowhere else)
   *   addOutput(pin)  - ...and here as well: the same result can drive
   *                     several pins at once (the dedicated pin plus up to
   *                     one event-output pin per port)
   *   disableOutput() - no output pin at all (interrupt / chaining only)
   * Returns false if the pin is not one of this unit's output pins, or if
   * no event channel is free. */
  bool setOutput(uint8_t pin);
  bool addOutput(uint8_t pin);
  void disableOutput();

  /* Stop the unit and release its pins. */
  void end();

  /* Current state of the OUT pin. */
  bool read();

  /* Call a function when the gate output changes, like attachInterrupt():
   * mode = RISING, FALLING or CHANGE. */
  void attachInterrupt(void (*callback)(), uint8_t mode);
  void detachInterrupt();

private:
  bool configure(uint8_t truth, uint8_t inputMask); /* bit n = INn used */
  void apply();
  uint8_t insel(uint8_t input);
  void releaseEventOutputs();
  const uint8_t *_pins;   /* IN0, IN1, IN2, OUT, OUT-alt */
  uint8_t _lut;
  uint8_t _numInputs = 0;
  uint8_t _truth = 0;
  uint8_t _inputMask = 0;
  uint8_t _claimed = 0;   /* bit n = we set the pull-up on INn's pin */
  LogicInput _source[3] = {LOGIC_PIN, LOGIC_PIN, LOGIC_PIN};
  uint8_t _outMode = 0;   /* 0 = dedicated pin, 1 = alternate pin, 2 = none */
  uint8_t _evoutPin[3] = {NOT_A_PIN, NOT_A_PIN, NOT_A_PIN}; /* EVOUTA/D/F */
  uint8_t _evChannel = 0xFF;
  uint8_t _readPin;       /* the pin read() looks at */
  bool _running = false;
};

extern CustomLogicClass CustomLogic;
#if !defined(WAZAMONO_BOARD_KUNAI)
extern CustomLogicClass CustomLogic1;
#endif

#endif
