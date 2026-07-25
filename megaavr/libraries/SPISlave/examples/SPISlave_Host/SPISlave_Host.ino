/* SPISlave_Host - host-side counterpart for the SPISlave_Test example.
 *
 * Runs on any Arduino-compatible board with the standard SPI library
 * (including another Wazamono board). Sends a question, then clocks out the
 * client's staged answer in a second transaction.
 *
 * Connect MOSI/MISO/SCK to the client's SPI pins, CS_PIN to the client's SS
 * (Tachi: D4, Kunai: D1), and join GND.
 *
 * The client stages its reply from onData() - i.e. after the question's
 * transaction has ended - so the answer is fetched with a separate read
 * transaction. The short delay between the two leaves the client's interrupt
 * handlers time to run.
 */

#include <SPI.h>

const uint8_t CS_PIN = 10;   // any free pin wired to the client's SS
const uint32_t SPI_HZ = 1000000;

void spiSend(const char *msg) {
  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_PIN, LOW);
  while (*msg) {
    SPI.transfer(*msg++);
  }
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
}

void spiReadAnswer(char *buf, uint8_t maxlen) {
  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_PIN, LOW);
  for (uint8_t i = 0; i < maxlen; i++) {
    buf[i] = (char)SPI.transfer(0x00);
  }
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  buf[maxlen] = '\0';
}

void setup() {
  Serial.begin(115200);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();
}

void loop() {
  char answer[33];

  spiSend("Are you alive?");
  delay(1);                       // let the client stage its answer
  spiReadAnswer(answer, 32);
  Serial.print(F("Answer: "));
  Serial.println(answer);

  delay(1000);
}
