/*
 * SX1280 2.4 GHz LoRa – PHY-level ping-pong TEST (Node A – Transmitter)
 *
 * Purpose: verify the physical layer (SX1280 + RadioLib) works correctly
 * at 2.4 GHz BEFORE testing LoRaWAN. If this sketch works, the hardware
 * wiring and SX1280 driver are functioning.
 *
 * Hardware – MKR WAN 1310 + external SX1280 breakout (or SX1280ED1ZHP board):
 *
 *   MKR WAN 1310        SX1280 module
 *   ─────────────       ─────────────
 *   SPI MOSI (8)  ───►  MOSI
 *   SPI SCK  (9)  ───►  SCK
 *   SPI MISO (10) ◄───  MISO
 *   D7 (CS)       ───►  NSS / CS
 *   D6 (RESET)    ───►  NRESET
 *   D5 (BUSY)     ◄───  BUSY
 *   D4 (DIO1/IRQ) ◄───  DIO1
 *   3V3           ───►  VCC
 *   GND           ───►  GND
 *
 * Flash this sketch on Node A; flash PHY_test_RX.ino on Node B.
 * Open Serial Monitor at 115200 baud.
 *
 * Library: RadioLib (with ISM2_4GHz patch applied)
 * Board:   Arduino MKR WAN 1310 (or any SAMD/ARM board with SPI)
 */

#include <RadioLib.h>

// ── Pin definitions ─────────────────────────────────────────
#define PIN_CS    7
#define PIN_IRQ   4
#define PIN_RST   6
#define PIN_BUSY  5

// ── Radio object ────────────────────────────────────────────
SX1280 radio = new Module(PIN_CS, PIN_IRQ, PIN_RST, PIN_BUSY);

// ── LoRa parameters (must match Node B) ────────────────────
//   Frequency: 2440.0 MHz (centre of ISM 2.4 GHz)
//   SF9, BW 203.125 kHz, CR 4/5  →  DR3 equivalent
static const float  FREQ_MHZ  = 2440.0f;
static const float  BW_KHZ    = 203.125f;
static const uint8_t SF       = 9;
static const uint8_t CR       = 5;   // RadioLib uses denominator (5 = 4/5)
static const int8_t  TX_DBM   = 10;
static const uint8_t PREAMBLE = 8;

// ── Application state ───────────────────────────────────────
static volatile bool rxFlag = false;
static uint32_t packetCount = 0;
static uint32_t ackCount    = 0;

void setFlag(void) { rxFlag = true; }

// ────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000);   // wait for USB CDC (skip after 4 s)
  Serial.println(F("\n=== SX1280 2.4GHz PHY TX Test ==="));

  SPI.begin();

  // Initialise radio in LoRa mode
  int16_t state = radio.begin(FREQ_MHZ, BW_KHZ, SF, CR, RADIOLIB_SX128X_SYNC_WORD_PRIVATE, TX_DBM, PREAMBLE);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("[SX1280] init failed, code "));
    Serial.println(state);
    while (true);
  }

  // Enable explicit header mode (default, but be explicit)
  radio.explicitHeader();

  // Attach DIO1 interrupt for RX-done detection
  radio.setPacketReceivedAction(setFlag);

  Serial.print(F("Freq: "));  Serial.print(FREQ_MHZ);   Serial.println(F(" MHz"));
  Serial.print(F("BW:   "));  Serial.print(BW_KHZ);     Serial.println(F(" kHz"));
  Serial.print(F("SF:   "));  Serial.println(SF);
  Serial.print(F("TX:   "));  Serial.print(TX_DBM);      Serial.println(F(" dBm"));
  Serial.println(F("Ready – sending ping every 3 s\n"));
}

// ────────────────────────────────────────────────────────────
void loop() {
  // ── 1. Transmit ping ─────────────────────────────────────
  char buf[32];
  snprintf(buf, sizeof(buf), "PING %lu", ++packetCount);

  Serial.print(F("[TX] ")); Serial.println(buf);
  int16_t state = radio.transmit((uint8_t*)buf, strlen(buf));
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("  TX error: ")); Serial.println(state);
  }

  // ── 2. Open Rx window to wait for pong ───────────────────
  rxFlag = false;
  state = radio.startReceive();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("  RX start error: ")); Serial.println(state);
  }

  // Wait up to 2000 ms for a reply
  uint32_t tStart = millis();
  while (!rxFlag && (millis() - tStart) < 2000) {
    delay(1);
  }

  if (rxFlag) {
    rxFlag = false;
    uint8_t reply[64] = {0};
    size_t  replyLen  = 0;
    state = radio.readData(reply, sizeof(reply) - 1);
    replyLen = radio.getPacketLength(true);

    if (state == RADIOLIB_ERR_NONE) {
      reply[replyLen] = '\0';
      ackCount++;
      Serial.print(F("[RX] "));
      Serial.print((char*)reply);
      Serial.print(F("  RSSI=")); Serial.print(radio.getRSSI());
      Serial.print(F(" dBm  SNR=")); Serial.print(radio.getSNR());
      Serial.println(F(" dB"));
    } else {
      Serial.print(F("  RX error: ")); Serial.println(state);
    }
  } else {
    radio.standby();
    Serial.println(F("  No reply (timeout)"));
  }

  // ── 3. Stats ─────────────────────────────────────────────
  Serial.print(F("  PER: "));
  Serial.print(100.0f * (1.0f - (float)ackCount / (float)packetCount), 1);
  Serial.println(F("%\n"));

  delay(3000);   // 3-second interval between pings
}
