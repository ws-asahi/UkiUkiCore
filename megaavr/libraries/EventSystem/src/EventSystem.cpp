/* EventSystem.cpp - see EventSystem.h for the concept and the clean-room
 * provenance notice. Everything here is derived from DS40002548A section 16
 * (EVSYS), section 17 (PORTMUX) and section 18 (PORT, EVGENCTRLA).
 */
#include "EventSystem.h"

EventSystemClass EventSystem(0);
EventSystemClass EventSystem1(1);
EventSystemClass EventSystem2(2);
EventSystemClass EventSystem3(3);
EventSystemClass EventSystem4(4);
EventSystemClass EventSystem5(5);

/* Channels this library holds that a register scan cannot see (a software
 * connection keeps its generator at OFF). CustomLogic's output routing picks
 * up this mask through a weak reference and leaves those channels alone. */
extern "C" {
  uint8_t _wazamono_evsys_reserved = 0;   /* bit n = channel n is ours */
}

static volatile uint8_t *channelReg(uint8_t ch) {
  return (&EVSYS.CHANNEL0) + ch;
}

/* ---- the board's fixed event-output pins -------------------------------- */
struct EvOutPin { uint8_t pin; uint8_t alt; };
/* index: 0 = EVOUTA, 1 = EVOUTD, 2 = EVOUTF; pin NOT_A_PIN = not offered */
static const EvOutPin s_evout[3] = {
  #if defined(WAZAMONO_EVOUTA_PIN)
  {WAZAMONO_EVOUTA_PIN, WAZAMONO_EVOUTA_ALT},
  #else
  {NOT_A_PIN, 0},
  #endif
  #if defined(WAZAMONO_EVOUTD_PIN)
  {WAZAMONO_EVOUTD_PIN, WAZAMONO_EVOUTD_ALT},
  #else
  {NOT_A_PIN, 0},
  #endif
  #if defined(WAZAMONO_EVOUTF_PIN)
  {WAZAMONO_EVOUTF_PIN, WAZAMONO_EVOUTF_ALT},
  #else
  {NOT_A_PIN, 0},
  #endif
};

static volatile uint8_t *evoutUser(uint8_t idx) {
  if (idx == 0) return &EVSYS.USEREVSYSEVOUTA;
  if (idx == 1) return &EVSYS.USEREVSYSEVOUTD;
  return &EVSYS.USEREVSYSEVOUTF;
}

static uint8_t evoutPortmuxBit(uint8_t idx) {
  if (idx == 0) return PORTMUX_EVOUTA_bm;
  if (idx == 1) return PORTMUX_EVOUTD_bm;
  return PORTMUX_EVOUTF_bm;
}

/* ---- the CustomLogic event inputs --------------------------------------- */
/* EVENT_TO_LOGIC_A/_B -> the CustomLogic unit's LUT; _1A/_1B -> CustomLogic1.
 * CustomLogic is LUT2 on Tachi/Tsurugi/UkiUkiduino and LUT0 on Kunai; CustomLogic1 is
 * LUT3 and does not exist on Kunai. */
static volatile uint8_t *logicUser(uint8_t n /* 0..3 */) {
  #if defined(WAZAMONO_BOARD_KUNAI)
  if (n == 0) return &EVSYS.USERCCLLUT0A;
  if (n == 1) return &EVSYS.USERCCLLUT0B;
  return nullptr;                       /* no CustomLogic1 on Kunai */
  #else
  if (n == 0) return &EVSYS.USERCCLLUT2A;
  if (n == 1) return &EVSYS.USERCCLLUT2B;
  if (n == 2) return &EVSYS.USERCCLLUT3A;
  return &EVSYS.USERCCLLUT3B;
  #endif
}

/* ---- pin sources: the two port event generators -------------------------- */
/* A pin reaches a channel through one of its port's two generator slots
 * (PORTx.EVGENCTRLA). The channel generator value follows a fixed pattern:
 * 0x40 + 2 * port_index + slot (PORTA 0x40/0x41 ... PORTF 0x4A/0x4B). */
static PORT_t *portStruct(uint8_t port) {
  return (PORT_t *)(uintptr_t)(0x0400 + 0x20 * port);
}

/* Is this generator value referenced by any of the six channels? */
static bool generatorInUse(uint8_t genValue) {
  for (uint8_t i = 0; i < 6; i++) {
    if (*channelReg(i) == genValue) return true;
  }
  return false;
}

/* Find (or program) a port generator slot carrying this pin.
 * Returns the channel generator value, or 0 on failure. */
static uint8_t pinGenerator(uint8_t pin) {
  uint8_t port = digitalPinToPort(pin);
  uint8_t pos  = digitalPinToBitPosition(pin);
  if (port == NOT_A_PORT || pos == NOT_A_PIN) return 0;
  PORT_t *p = portStruct(port);
  uint8_t base = 0x40 + 2 * port;

  /* A slot already set to this pin? Use it (several connections may share). */
  uint8_t sel0 = (p->EVGENCTRLA & PORT_EVGEN0SEL_gm) >> PORT_EVGEN0SEL_gp;
  uint8_t sel1 = (p->EVGENCTRLA & PORT_EVGEN1SEL_gm) >> PORT_EVGEN1SEL_gp;
  if (sel0 == pos && generatorInUse(base + 0)) return base + 0;
  if (sel1 == pos && generatorInUse(base + 1)) return base + 1;

  /* Otherwise take a slot no channel is listening to. */
  if (!generatorInUse(base + 0)) {
    p->EVGENCTRLA = (p->EVGENCTRLA & ~PORT_EVGEN0SEL_gm) | (pos << PORT_EVGEN0SEL_gp);
    return base + 0;
  }
  if (!generatorInUse(base + 1)) {
    p->EVGENCTRLA = (p->EVGENCTRLA & ~PORT_EVGEN1SEL_gm) | (pos << PORT_EVGEN1SEL_gp);
    return base + 1;
  }
  return 0;   /* both of this port's generator slots are taken */
}

/* ---- channel ownership ---------------------------------------------------- */
bool EventSystemClass::claimChannel() {
  if (_active) return true;
  /* Refuse a channel that anything else set up: a non-zero generator means a
   * deliberate route (CustomLogic.addOutput(), or raw register use). */
  if (*channelReg(_channel) != 0) return false;
  if (_wazamono_evsys_reserved & (1 << _channel)) return false;
  _wazamono_evsys_reserved |= (1 << _channel);
  _active = true;
  return true;
}

/* ---- sources -------------------------------------------------------------- */
bool EventSystemClass::setSourcePin(uint8_t pin) {
  uint8_t gen = pinGenerator(pin);
  if (gen == 0) return false;
  /* Inputs get a pull-up so buttons to GND work out of the box; a driven
   * signal simply overrides it. */
  pinMode(pin, INPUT_PULLUP);
  if (_sourcePin != NOT_A_PIN && _sourcePin != pin) {
    pinMode(_sourcePin, INPUT);          /* release the previous source pin */
  }
  _sourcePin = pin;
  *channelReg(_channel) = gen;
  return true;
}

bool EventSystemClass::setSource(EventSource src) {
  uint8_t gen;
  switch (src) {
    case EVENT_ANALOG_COMP:  gen = EVSYS_CHANNEL_AC0_OUT_gc;  break;
    #if defined(WAZAMONO_BOARD_KUNAI)
    case EVENT_CUSTOM_LOGIC: gen = EVSYS_CHANNEL_CCL_LUT0_gc; break;
    #else
    case EVENT_CUSTOM_LOGIC: gen = EVSYS_CHANNEL_CCL_LUT2_gc; break;
    case EVENT_CUSTOM_LOGIC1: gen = EVSYS_CHANNEL_CCL_LUT3_gc; break;
    #endif
    case EVENT_SOFTWARE:     gen = 0;                          break;
    default:                 return false;
  }
  if (_sourcePin != NOT_A_PIN) {
    pinMode(_sourcePin, INPUT);
    _sourcePin = NOT_A_PIN;
  }
  *channelReg(_channel) = gen;
  return true;
}

/* ---- destinations ---------------------------------------------------------- */
bool EventSystemClass::addDestPin(uint8_t pin) {
  for (uint8_t i = 0; i < 3; i++) {
    if (s_evout[i].pin == NOT_A_PIN || s_evout[i].pin != pin) continue;
    volatile uint8_t *user = evoutUser(i);
    if (*user != 0 && *user != (uint8_t)(_channel + 1)) {
      return false;                      /* fed by another connection */
    }
    if (s_evout[i].alt) PORTMUX.EVSYSROUTEA |=  evoutPortmuxBit(i);
    else                PORTMUX.EVSYSROUTEA &= ~evoutPortmuxBit(i);
    *user = _channel + 1;
    pinMode(pin, OUTPUT);
    _evoutMask |= (1 << i);
    return true;
  }
  return false;   /* not one of this board's fixed event-output pins */
}

bool EventSystemClass::addDestLogic(EventDestination to) {
  uint8_t n = (uint8_t)to - (uint8_t)EVENT_TO_LOGIC_A;
  if (n > 3) return false;
  volatile uint8_t *user = logicUser(n);
  if (user == nullptr) return false;     /* no such unit on this board */
  if (*user != 0 && *user != (uint8_t)(_channel + 1)) {
    return false;                        /* fed by another connection */
  }
  *user = _channel + 1;
  _logicMask |= (1 << n);
  return true;
}

/* ---- the public face -------------------------------------------------------- */
bool EventSystemClass::connect(uint8_t fromPin, uint8_t toPin) {
  if (!claimChannel())        return false;
  if (!setSourcePin(fromPin)) return false;
  return addDestPin(toPin);
}

bool EventSystemClass::connect(uint8_t fromPin, EventDestination to) {
  if (!claimChannel())        return false;
  if (!setSourcePin(fromPin)) return false;
  return addDestLogic(to);
}

bool EventSystemClass::connect(EventSource from, uint8_t toPin) {
  if (!claimChannel())     return false;
  if (!setSource(from))    return false;
  return addDestPin(toPin);
}

bool EventSystemClass::connect(EventSource from, EventDestination to) {
  if (!claimChannel())     return false;
  if (!setSource(from))    return false;
  return addDestLogic(to);
}

bool EventSystemClass::addDestination(uint8_t toPin) {
  if (!_active) return false;
  return addDestPin(toPin);
}

bool EventSystemClass::addDestination(EventDestination to) {
  if (!_active) return false;
  return addDestLogic(to);
}

void EventSystemClass::disconnect(uint8_t toPin) {
  for (uint8_t i = 0; i < 3; i++) {
    if ((_evoutMask & (1 << i)) && s_evout[i].pin == toPin) {
      *evoutUser(i) = 0;
      pinMode(toPin, INPUT);
      _evoutMask &= ~(1 << i);
    }
  }
}

void EventSystemClass::disconnect(EventDestination to) {
  uint8_t n = (uint8_t)to - (uint8_t)EVENT_TO_LOGIC_A;
  if (n > 3 || !(_logicMask & (1 << n))) return;
  volatile uint8_t *user = logicUser(n);
  if (user != nullptr) *user = 0;
  _logicMask &= ~(1 << n);
}

void EventSystemClass::trigger() {
  /* One peripheral-clock pulse: the hardware inverts this channel's signal
   * for a single cycle (DS40002548A 16.5.1). */
  EVSYS.SWEVENTA = (1 << _channel);
}

void EventSystemClass::end() {
  if (!_active) return;
  for (uint8_t i = 0; i < 3; i++) {
    if (_evoutMask & (1 << i)) {
      *evoutUser(i) = 0;
      pinMode(s_evout[i].pin, INPUT);
    }
  }
  _evoutMask = 0;
  for (uint8_t n = 0; n < 4; n++) {
    if (_logicMask & (1 << n)) {
      volatile uint8_t *user = logicUser(n);
      if (user != nullptr) *user = 0;
    }
  }
  _logicMask = 0;
  if (_sourcePin != NOT_A_PIN) {
    pinMode(_sourcePin, INPUT);
    _sourcePin = NOT_A_PIN;
  }
  *channelReg(_channel) = 0;
  _wazamono_evsys_reserved &= ~(1 << _channel);
  _active = false;
}
