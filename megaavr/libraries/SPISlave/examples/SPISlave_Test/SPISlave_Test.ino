/* SPISlave_Test - Wazamono SPI client ("slave") demo.
 *
 * Runs on a Wazamono Tachi (SS = D4) or Kunai (SS = D1) and answers questions
 * from an SPI host. Pair it with the SPISlave_Host example on a second board.
 * The flow mirrors the SPISlave_Test example of the ESP8266 Arduino core.
 *
 * Wiring (client <-> host):
 *   MOSI <- host MOSI     Tachi: D16 / Kunai: D10
 *   MISO -> host MISO     Tachi: D14 / Kunai: D9
 *   SCK  <- host SCK      Tachi: D15 / Kunai: D8
 *   SS   <- host CS pin   Tachi: D4  / Kunai: D1
 *   GND  -- GND
 *
 * Callbacks run in interrupt context: this sketch only copies data there and
 * does its Serial printing from loop().
 */

#include <SPISlave.h>

volatile bool     got_message = false;
char              message[SPISLAVE_BUFFER_SIZE + 1];

void setup() {
  Serial.begin(115200);

  // Received a transaction from the host (fires when SS returns high).
  SPISlave.onData([](uint8_t *data, size_t len) {
    memcpy(message, data, len);
    message[len] = '\0';
    got_message = true;
    // Stage the answer the host will clock out on its NEXT transaction.
    if (strcmp(message, "Are you alive?") == 0) {
      SPISlave.setData("Yes, alive!");
    } else {
      SPISlave.setData("Say what?");
    }
  });

  // The host clocked out the complete staged answer.
  SPISlave.onDataSent([]() {
    // Keep it short - interrupt context. loop() reports via got_message.
  });

  SPISlave.begin();               // SPI mode 0, MSB first
  SPISlave.setData("Hello Host!");  // answer for the very first read
}

void loop() {
  if (got_message) {
    got_message = false;
    Serial.print(F("Question: "));
    Serial.println(message);
  }
}
