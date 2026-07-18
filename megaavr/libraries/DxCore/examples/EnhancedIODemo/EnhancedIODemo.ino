/*********************\\*****************************//************************
                       \\   Enhanced I/O API Demo   //
                        ^^-------------------------^^

The I/O hardware in the AVR DU-series is more sophisticated than that which
was featured in classic AVR products. This core
provides a few simple I/O functions to take advantage of the new pin I/O
capabilities. This file demonstrates their function and calling conventions in
brief.

In these examples, we use pins D9 and D8, which are plain GPIO on every
Wazamono board.

This sketch isn't meant to be used as is - it's more of a starting point, or
resource to cooy+paste starting points from.

******************************************************************************/

#define DEMO_PIN 9   // D9
#define DEMO_PIN2 8  // D8

void setup() {


}


void loop() {
  openDrainBitbang(0x0DF0AD8B); // or, with human endianness, 0x8BADF00D
}

/*-----------------------------------------------------------------------------
openDrain(pin,value) and openDrainFast(pin,value)

Nothing specific to the modernAVRs about openDrain() - it's the "missing"
digital I/O function. To get pullup, set it INPUT_PULLUP with pinMode first, then
call openDrain() - remember that, emulating the behavior of classic AVRs, the
core configures pins as inputs unless told otherwise.
  Usage:
    openDrain(DEMO_PIN, LOW);
    openDrain(DEMO_PIN, FLOATING);
    openDrain(DEMO_PIN, CHANGE);
    openDrainFast(DEMO_PIN, LOW);
    openDrainFast(DEMO_PIN, FLOATING);
    openDrainFast(DEMO_PIN, CHANGE);

  LOW sets pin mode to OUTPUT.
  FLOATING sets pin mode to INPUT. If there is a pullup
enabled or external pullup connected, the pin will be pulled up assuming
nothing else connected to it is driving the pin LOW; otherwise the pin will
float.
  CHANGE toggles the direction.

  Like all Fast I/O functions, you must pass a constant pin and you should try
  to pass a constant value as well. The function when both are constant, and
  the value is not CHANGE optimizes to a single cbi or sbi instruction, occupying
  2 bytes of flash and executing within a single clock cycle. This function is
  ((always_inline)) - but with a single 2 byte instruction, this is always
  more efficient. When the value is CHANGE, it uses two instructions, one a
  double-size STS instruction, for 6 bytes and 3 clock cycles.

-----------------------------------------------------------------------------*/

void openDrainBitbang(uint32_t data) {
  pinMode(DEMO_PIN, INPUT_PULLUP);
  pinMode(DEMO_PIN2, INPUT_PULLUP);
  openDrain(DEMO_PIN, FLOATING);
  openDrain(DEMO_PIN2, FLOATING);
  // Now both pins are open drains with their pullup enabled
  // now they use one as a clock, and the other as data in some sort of digital
  // communication scheme waiting to make sure the pins come back to HIGH like
  // I2C does.
  // When it's not waiting for the pins to rise back to HIGH, the code runs
  for (uint8_t i = 0; i < 32; i++) {
    while (digitalReadFast(DEMO_PIN) != HIGH || digitalReadFast(DEMO_PIN2) != HIGH);
    // Wait for them to be pulled high - probably won't loop, but maybe high capacitance on
    // the lines or weak pullups, or other device holding low (like I2C clock stretching)
    _NOPNOP(); // wait four clocks so the receiver has a chance to see the same thing as we did; We could even wait longer here
    _NOPNOP();
    if (((uint8_t)data) & 0x01) {
      openDrainFast(DEMO_PIN2, LOW); ///set up data line - this likely compiles to cbi, sbrc, sbi
      // (certainly that's what you;'d expect the compiler to to do, but sometimes it's not so smart)
    }
    _NOPNOP(); // wait four clocks so the receiver has a chance to see the same thing as we did; We could even wait longer here
    _NOPNOP();
    openDrainFast(DEMO_PIN, LOW);
    data >>= 1;  // if we'd immediately released it, it would only be low for a fraction of a microsecond.
    // doing that math in there is liks a quarter microsecond delay.
    openDrainFast(DEMO_PIN, FLOATING);
    openDrainFast(DEMO_PIN2, FLOATING);
    // release pins.
  }

}
