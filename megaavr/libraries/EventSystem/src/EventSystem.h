/* EventSystem.h - beginner-friendly event routing for the Wazamono boards
 * -----------------------------------------------------------------------
 * Part of WazamonoCore. (C) Workshop Asahi 2026, LGPL 2.1 (see LICENSE.md).
 *
 * CLEAN-ROOM NOTICE: this library is an original work, written for
 * WazamonoCore from the AVR64DU28/32 data sheet (DS40002548A, section 16,
 * EVSYS) alone. It is not derived from DxCore's Event library or any other
 * event-system implementation.
 *
 * The event system is the chip's internal wiring loom: it carries a signal
 * from one place to another with no CPU involvement at all - it keeps
 * working while your sketch is busy, and even while the CPU sleeps.
 *
 * There are six independent connections, ready-made:
 *
 *   EventSystem, EventSystem1, EventSystem2, ... EventSystem5
 *
 * Each carries ONE signal from ONE source to ANY NUMBER of destinations:
 *
 *   EventSystem.connect(8, 2);       // pin D8 now drives pin D2 - that's all
 *
 * Sources (what to carry):
 *   - any Arduino pin number (at most two pins per port at a time - a
 *     hardware limit of the port event generators)
 *   - EVENT_ANALOG_COMP   the AnalogComp result (AC0)
 *   - EVENT_CUSTOM_LOGIC  the CustomLogic output  (EVENT_CUSTOM_LOGIC1 for
 *                         the second unit - not on Kunai)
 *   - EVENT_SOFTWARE      nothing, until trigger() sends a one-clock pulse
 *
 * Destinations (where to deliver):
 *   - an event-output pin. These are FIXED per board:
 *                    EVOUTA        EVOUTD        EVOUTF
 *       Tachi        D2  (PA2)     D0 (PD7)      D7 (PF2)
 *       Tsurugi      D8  (PA7)     D9 (PD2)      A2 (PF2)
 *       UkiUkiduino  D8  (PA7)     D9 (PD2)      A2 (PF2)
 *       Kunai        D0  (PA7)     D7 (PD7)      -
 *   - a CustomLogic event input: EVENT_TO_LOGIC_A / EVENT_TO_LOGIC_B
 *     (EVENT_TO_LOGIC1_A / _B for the second unit - not on Kunai), paired
 *     with CustomLogic.setInputINn(LOGIC_EVENT_A / _B) on the logic side.
 */
#ifndef WAZAMONO_EVENTSYSTEM_H
#define WAZAMONO_EVENTSYSTEM_H

#include <Arduino.h>

/* What a connection carries (besides a plain pin, given by number). */
enum EventSource : uint8_t {
  EVENT_ANALOG_COMP = 0xF0, /* the AnalogComp result (AC0) - just begin() it */
  EVENT_CUSTOM_LOGIC,       /* the CustomLogic unit's output                 */
  EVENT_CUSTOM_LOGIC1,      /* the CustomLogic1 unit's output (not on Kunai) */
  EVENT_SOFTWARE            /* idle until trigger() sends a one-clock pulse  */
};

/* Where a connection delivers (besides a pin, given by number). */
enum EventDestination : uint8_t {
  EVENT_TO_LOGIC_A = 0xF8,  /* CustomLogic's EVENT_A input  */
  EVENT_TO_LOGIC_B,         /* CustomLogic's EVENT_B input  */
  EVENT_TO_LOGIC1_A,        /* CustomLogic1's EVENT_A input (not on Kunai) */
  EVENT_TO_LOGIC1_B         /* CustomLogic1's EVENT_B input (not on Kunai) */
};

class EventSystemClass {
public:
  constexpr EventSystemClass(uint8_t channel) : _channel(channel) {}

  /* Wire a source to a destination. The first call claims this connection;
   * calling connect() again replaces the source and keeps the destinations.
   * Returns false if the source or destination is not available (wrong pin,
   * both port generators busy, destination already fed by another
   * connection, or this connection's channel is in use by something else -
   * e.g. CustomLogic.addOutput()). */
  bool connect(uint8_t fromPin, uint8_t toPin);
  bool connect(uint8_t fromPin, EventDestination to);
  bool connect(EventSource from, uint8_t toPin);
  bool connect(EventSource from, EventDestination to);

  /* One source can feed several destinations at once - add more here. */
  bool addDestination(uint8_t toPin);
  bool addDestination(EventDestination to);

  /* Detach one destination; the rest keep working. */
  void disconnect(uint8_t toPin);
  void disconnect(EventDestination to);

  /* Send a one-clock pulse down this connection (for EVENT_SOFTWARE, but
   * works on any connection: the pulse inverts whatever the source drives). */
  void trigger();

  /* Is this connection set up? */
  bool connected() const { return _active; }

  /* Tear the whole connection down and release everything it used. */
  void end();

private:
  bool claimChannel();
  bool setSourcePin(uint8_t pin);
  bool setSource(EventSource src);
  bool addDestPin(uint8_t pin);
  bool addDestLogic(EventDestination to);

  const uint8_t _channel;
  uint8_t _evoutMask   = 0;         /* bit0 = EVOUTA, bit1 = EVOUTD, bit2 = EVOUTF */
  uint8_t _logicMask   = 0;         /* bit n = EventDestination base + n           */
  uint8_t _sourcePin   = NOT_A_PIN; /* pin generator we claimed (if any)           */
  bool    _active      = false;
};

extern EventSystemClass EventSystem;   /* connection 0 */
extern EventSystemClass EventSystem1;  /* connection 1 */
extern EventSystemClass EventSystem2;  /* connection 2 */
extern EventSystemClass EventSystem3;  /* connection 3 */
extern EventSystemClass EventSystem4;  /* connection 4 */
extern EventSystemClass EventSystem5;  /* connection 5 */

#endif
