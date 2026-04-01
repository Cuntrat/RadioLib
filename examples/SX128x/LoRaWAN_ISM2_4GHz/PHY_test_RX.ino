/*
 * SX1280 2.4 GHz LoRa – PHY-level ping-pong TEST (Node B – Receiver/Echo)
 *
 * Companion to PHY_test_TX.ino.
 * Listens for a "PING N" packet and immediately replies "PONG N".
 *
 * Same wiring as Node A (see PHY_test_TX.ino).
 * Same LoRa parameters must be used on both nodes.
 */

#include <RadioLib.h>

#define PIN_CS    7
#define PIN_IRQ   4
#define PIN_RST   6
#define PIN_BUSY  5

SX1280 radio = new Module(PIN_CS, PIN_IRQ, PIN_RST, PIN_BUSY);

static const float   FREQ_MHZ  = 2440.0f;
static const float   BW_KHZ    = 203.125f;
static const uint8_t SF        = 9;
static const uint8_t CR        = 5;
static const int8_t  TX_DBM    = 10;
static const uint8_t PREAMBLE  = 8;

static volatile bool rxFlag = false;

void setFlag(void) { rxFlag = true; }

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000);
  Serial.println(F("\n=== SX1280 2.4GHz PHY RX/Echo Test ==="));

  SPI.begin();

  int16_t state = radio.begin(FREQ_MHZ, BW_KHZ, SF, CR, RADIOLIB_SX128X_SYNC_WORD_PRIVATE, TX_DBM, PREAMBLE);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("[SX1280] init failed, code ")); Serial.println(state);
    while (true);
  }

  radio.explicitHeader();
  radio.setPacketReceivedAction(setFlag);

  // Start continuous receive
  state = radio.startReceive();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("[SX1280] startReceive failed: ")); Serial.println(state);
  }

  Serial.println(F("Listening for PING packets…\n"));
}

void loop() {
  if (!rxFlag) return;
  rxFlag = false;

  // Read received packet
  uint8_t buf[64] = {0};
  int16_t state = radio.readData(buf, sizeof(buf) - 1);
  size_t  len   = radio.getPacketLength(true);

  if (state == RADIOLIB_ERR_NONE) {
    buf[len] = '\0';
    Serial.print(F("[RX] ")); Serial.print((char*)buf);
    Serial.print(F("  RSSI=")); Serial.print(radio.getRSSI());
    Serial.print(F(" dBm  SNR=")); Serial.print(radio.getSNR());
    Serial.println(F(" dB"));

    // Build PONG reply – replace "PING" with "PONG"
    char reply[64];
    strncpy(reply, (char*)buf, sizeof(reply) - 1);
    if (strncmp(reply, "PING", 4) == 0) {
      reply[1] = 'O';   // PING → PONG
    }

    // Small delay before Tx (let the other side switch to Rx)
    delay(50);

    state = radio.transmit((uint8_t*)reply, strlen(reply));
    if (state == RADIOLIB_ERR_NONE) {
      Serial.print(F("[TX] ")); Serial.println(reply);
    } else {
      Serial.print(F("  TX error: ")); Serial.println(state);
    }
  } else {
    Serial.print(F("[RX] error: ")); Serial.println(state);
  }

  // Back to receive
  state = radio.startReceive();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("  RX restart error: ")); Serial.println(state);
  }
}
