/*
 * SX1280 2.4 GHz LoRaWAN Test – ABP Activation, ISM2_4GHz band
 *
 * ─────────────────────────────────────────────────────────────
 * Purpose
 * ─────────────────────────────────────────────────────────────
 * End-to-end test of the ISM2_4GHz LoRaWANBand_t definition
 * using Activation-By-Personalisation (ABP), which avoids the
 * need for a real LoRaWAN network server during initial testing.
 *
 * The sketch:
 *   1. Initialises the SX1280 module
 *   2. Activates a LoRaWAN session (ABP)
 *   3. Sends an uplink every 30 s with sensor data
 *   4. Prints downlink data and event info if received
 *   5. Prints RADIOLIB_DEBUG_PROTOCOL output if enabled
 *
 * ─────────────────────────────────────────────────────────────
 * Hardware – MKR WAN 1310 + external SX1280 breakout
 * ─────────────────────────────────────────────────────────────
 *   MKR WAN 1310        SX1280 module
 *   ─────────────────   ─────────────
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
 * ─────────────────────────────────────────────────────────────
 * ABP credentials – replace with your own!
 * ─────────────────────────────────────────────────────────────
 * These are test-only values. For a real network server, copy
 * the values from your device registration page (TTN/ChirpStack).
 *
 * ─────────────────────────────────────────────────────────────
 * Enable debug output (optional)
 * ─────────────────────────────────────────────────────────────
 * In BuildOptUser.h uncomment:
 *   #define RADIOLIB_DEBUG_PROTOCOL (1)
 * to see LoRaWAN internals (MIC, frame counters, MAC commands…).
 */

#include <RadioLib.h>

// ── Pin definitions ─────────────────────────────────────────
#define PIN_CS    7
#define PIN_IRQ   4
#define PIN_RST   6
#define PIN_BUSY  5

// ── Radio object ────────────────────────────────────────────
SX1280 radio = new Module(PIN_CS, PIN_IRQ, PIN_RST, PIN_BUSY);

// ── LoRaWAN node with ISM2_4GHz band ────────────────────────
// subBand = 0 (only one set of channels in this custom band)
LoRaWANNode node(&radio, &ISM2400, 0);

// ── ABP credentials ─────────────────────────────────────────
// Device address  (4 bytes)
static const uint32_t DEV_ADDR = 0x01234567;

// NwkSEncKey / NwkSKey  (16 bytes)
static const uint8_t NWK_S_ENC_KEY[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

// AppSKey  (16 bytes)
static const uint8_t APP_S_KEY[16] = {
  0xA8, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD,
  0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD
};

// For LoRaWAN 1.0.4: fNwkSIntKey = sNwkSIntKey = NULL (library uses NwkSKey)
// For LoRaWAN 1.1:   provide separate keys.
// Using 1.0.4 here for simplicity.

// ── Uplink interval ─────────────────────────────────────────
static const uint32_t UPLINK_INTERVAL_MS = 30000UL;   // 30 seconds

// ── Uplink counter (for payload content) ────────────────────
static uint32_t uplinkCount = 0;

// ────────────────────────────────────────────────────────────
void printLoRaWANEvent(const char* dir, const LoRaWANEvent_t& ev) {
  Serial.print(F("  ["));  Serial.print(dir); Serial.println(F("]"));
  Serial.print(F("    freq:     ")); Serial.print(ev.freq, 4); Serial.println(F(" MHz"));
  Serial.print(F("    DR:       ")); Serial.println(ev.datarate);
  Serial.print(F("    fCnt:     ")); Serial.println(ev.fCnt);
  Serial.print(F("    fPort:    ")); Serial.println(ev.fPort);
  Serial.print(F("    confirmed:")); Serial.println(ev.confirmed  ? "yes" : "no");
  Serial.print(F("    confirming:")); Serial.println(ev.confirming ? "yes" : "no");
}

// ────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000);
  Serial.println(F("\n=== SX1280 2.4 GHz LoRaWAN ABP Test ==="));
  Serial.println(F("Band: ISM2_4GHz"));

  SPI.begin();

  // ── 1. Initialise SX1280 ─────────────────────────────────
  // begin() sets the *default* LoRa parameters; LoRaWANNode will
  // override them via setPhyProperties() at each uplink.
  int16_t state = radio.begin(2440.0f,      // frequency MHz
                               203.125f,    // bandwidth kHz – must be exact!
                               9,           // SF
                               5,           // CR denominator (4/5)
                               RADIOLIB_SX128X_SYNC_WORD_LORAWAN,  // 0x34
                               10,          // Tx power dBm
                               8);          // preamble symbols

  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("[SX1280] init error: ")); Serial.println(state);
    while (true);
  }
  Serial.println(F("[SX1280] initialised"));

  // ── 2. Configure LoRaWAN node ────────────────────────────
  // Disable duty cycle (no regulatory limit on global 2.4 GHz ISM band)
  node.setDutyCycle(false);

  // Disable ADR for initial testing (fix DR3 for determinism)
  node.setADR(false);

  // Set uplink datarate to DR3 (SF9 / BW203.125 / CR4-5)
  state = node.setDatarate(3);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("[LoRaWAN] setDatarate error: ")); Serial.println(state);
  }

  // ── 3. Set ABP credentials ──────────────────────────────
  state = node.beginABP(DEV_ADDR,
                        NULL,          // fNwkSIntKey  (1.0.4 → NULL)
                        NULL,          // sNwkSIntKey  (1.0.4 → NULL)
                        NWK_S_ENC_KEY,
                        APP_S_KEY);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("[LoRaWAN] beginABP error: ")); Serial.println(state);
    while (true);
  }

  // ── 4. Activate session ──────────────────────────────────
  state = node.activateABP();
  if (state == RADIOLIB_LORAWAN_NEW_SESSION || state == RADIOLIB_LORAWAN_SESSION_RESTORED) {
    Serial.println(F("[LoRaWAN] Session active"));
  } else {
    Serial.print(F("[LoRaWAN] activateABP error: ")); Serial.println(state);
    while (true);
  }

  Serial.print(F("DevAddr: 0x")); Serial.println(node.getDevAddr(), HEX);
  Serial.print(F("Max payload: ")); Serial.print(node.getMaxPayloadLen()); Serial.println(F(" bytes"));
  Serial.println(F("Ready – first uplink in 3 s\n"));

  delay(3000);
}

// ────────────────────────────────────────────────────────────
void loop() {
  // ── Build uplink payload (6 bytes) ──────────────────────
  // Byte 0-3: 32-bit uplink counter (big-endian)
  // Byte 4-5: 16-bit fake temperature (0.01 °C resolution, e.g. 2500 = 25.00 °C)
  uplinkCount++;
  uint16_t fakeTemp = 2500 + (uint16_t)(uplinkCount % 50);   // 25.00–25.49 °C

  uint8_t payload[6];
  payload[0] = (uplinkCount >> 24) & 0xFF;
  payload[1] = (uplinkCount >> 16) & 0xFF;
  payload[2] = (uplinkCount >>  8) & 0xFF;
  payload[3] =  uplinkCount        & 0xFF;
  payload[4] = (fakeTemp   >>  8) & 0xFF;
  payload[5] =  fakeTemp          & 0xFF;

  Serial.print(F("[UPLINK #")); Serial.print(uplinkCount); Serial.println(F("]"));
  Serial.print(F("  payload: "));
  for (uint8_t i = 0; i < sizeof(payload); i++) {
    if (payload[i] < 0x10) Serial.print('0');
    Serial.print(payload[i], HEX);
    Serial.print(' ');
  }
  Serial.println();

  // ── Events structs ────────────────────────────────────────
  LoRaWANEvent_t evUp   = {};
  LoRaWANEvent_t evDown = {};

  // ── Downlink buffer ───────────────────────────────────────
  uint8_t downBuf[RADIOLIB_LORAWAN_MAX_PAYLOAD_SIZE] = {0};
  size_t  downLen = 0;

  // ── Send uplink, wait for Rx1/Rx2 downlink ────────────────
  int16_t state = node.sendReceive(payload,
                                   sizeof(payload),
                                   1,            // fPort
                                   downBuf,
                                   &downLen,
                                   false,        // unconfirmed uplink
                                   &evUp,
                                   &evDown);

  // ── Print uplink event ───────────────────────────────────
  printLoRaWANEvent("UP", evUp);
  Serial.print(F("  ToA: ")); Serial.print(node.getLastToA()); Serial.println(F(" ms"));

  // ── Handle result ─────────────────────────────────────────
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("  No downlink received (Rx windows timed out)"));

  } else if (state > RADIOLIB_ERR_NONE) {
    // Positive value = Rx window number (1, 2, or 3 for RxC)
    Serial.print(F("  Downlink received in Rx")); Serial.println(state);
    printLoRaWANEvent("DOWN", evDown);

    if (downLen > 0) {
      Serial.print(F("  data (")); Serial.print(downLen); Serial.print(F(" bytes): "));
      for (size_t i = 0; i < downLen; i++) {
        if (downBuf[i] < 0x10) Serial.print('0');
        Serial.print(downBuf[i], HEX);
        Serial.print(' ');
      }
      Serial.println();
    } else {
      Serial.println(F("  (empty payload / MAC-only downlink)"));
    }

  } else {
    // Negative state = real error
    Serial.print(F("  sendReceive error: ")); Serial.println(state);
  }

  Serial.println();

  // ── Wait for next uplink ─────────────────────────────────
  // timeUntilUplink() returns 0 because dutyCycle is disabled;
  // we just use a fixed 30-second delay.
  delay(UPLINK_INTERVAL_MS);
}
