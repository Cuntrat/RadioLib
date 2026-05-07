/*
  SX1280 ISM2400 LoRaWAN Example

  Demonstrates OTAA join and periodic uplink on the 2.4 GHz ISM band.
  Before joining, the sketch prompts over Serial to choose confirmed or
  unconfirmed uplinks.  Both modes open RX windows after every TX, so
  any downlink scheduled on the network server will be received and printed.

  sendReceive() return value:
    < 0   hard error
    = 0   TX done, no downlink received
    = 1   downlink received in RX1
    = 2   downlink received in RX2

  For more information visit:
    https://github.com/jgromes/RadioLib/
*/

#include <RadioLib.h>
#include <EEPROM.h>

// ============ HARDWARE PINS ============
// SX1280 connections (STM32 Nucleo-64 / Arduino header):
// NSS pin:   7  (D7 = PA8)
// DIO1 pin:  5  (D5 = PB4)
// NRST pin:  A0 (PA0)
// BUSY pin:  3  (D3 = PB3)
#define SX1280_NSS   7
#define SX1280_DIO1  5
#define SX1280_NRST  A0
#define SX1280_BUSY  3

// ============ LORAWAN CREDENTIALS (MSB order) ============
uint64_t joinEUI = 0xA001C5DC0FA37C02ULL;
uint64_t devEUI  = 0x6B7107D07ED5B370ULL;

uint8_t appKey[16] = {
  0xD5, 0xA8, 0xD2, 0x3B, 0xDE, 0x2D, 0x8D, 0xA4,
  0x72, 0xDF, 0x25, 0x95, 0x25, 0xB6, 0x65, 0xE6
};

// ============ RADIO / NODE ============
LoRaWANBand_t myBand = ISM2400;
SX1280 radio = new Module(SX1280_NSS, SX1280_DIO1, SX1280_NRST, SX1280_BUSY);
LoRaWANNode node(&radio, &myBand);

bool joined          = false;
// set by Serial prompt in setup()
bool uplinkConfirmed = false;

// ============ NONCES PERSISTENCE ============
static const uint32_t NONCES_MAGIC   = 0x4C574E31UL;
static const int      EE_ADDR_MAGIC  = 0;
static const int      EE_ADDR_NONCES = EE_ADDR_MAGIC + sizeof(uint32_t);
static const int      EE_SIZE_BYTES  = EE_ADDR_NONCES + RADIOLIB_LORAWAN_NONCES_BUF_SIZE;

static void eepromBeginIfNeeded() {
#if defined(ESP8266) || defined(ESP32)
  EEPROM.begin(EE_SIZE_BYTES);
#endif
}

static void eepromCommitIfNeeded() {
#if defined(ESP8266) || defined(ESP32)
  EEPROM.commit();
#endif
}

bool restoreNonces() {
  uint32_t magic = 0;
  EEPROM.get(EE_ADDR_MAGIC, magic);
  if (magic != NONCES_MAGIC) {
    Serial.println(F("No saved nonces - will perform fresh join"));
    return false;
  }
  uint8_t nonces[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];
  for (size_t i = 0; i < RADIOLIB_LORAWAN_NONCES_BUF_SIZE; i++) {
    nonces[i] = EEPROM.read(EE_ADDR_NONCES + i);
  }
  int16_t st = node.setBufferNonces(nonces);
  if (st == RADIOLIB_ERR_NONE) {
    Serial.println(F("Nonces restored from EEPROM"));
    return true;
  }
  Serial.print(F("Nonces restore failed: "));
  Serial.println(st);
  return false;
}

void saveNonces() {
  uint8_t* p = node.getBufferNonces();
  EEPROM.put(EE_ADDR_MAGIC, NONCES_MAGIC);
  for (size_t i = 0; i < RADIOLIB_LORAWAN_NONCES_BUF_SIZE; i++) {
    EEPROM.write(EE_ADDR_NONCES + i, p[i]);
  }
  eepromCommitIfNeeded();
}

void tryJoin() {
  Serial.println(F("Starting OTAA join..."));
  int16_t st = node.activateOTAA();
  saveNonces();
  if (st == RADIOLIB_LORAWAN_NEW_SESSION || st == RADIOLIB_LORAWAN_SESSION_RESTORED) {
    joined = true;
    Serial.println(F("Join OK"));
    Serial.print(F("DevAddr: 0x"));
    Serial.println(node.getDevAddr(), HEX);
  } else {
    joined = false;
    Serial.print(F("Join failed: "));
    Serial.println(st);
  }
}

// waits up to timeoutMs for a Serial character; defaults to unconfirmed on timeout
static bool askConfirmedUplink(uint32_t timeoutMs) {
  Serial.println(F(""));
  Serial.println(F("┌────────────────────────────────────────┐"));
  Serial.println(F("│  Uplink mode selection                 │"));
  Serial.println(F("│  Type  y  for CONFIRMED uplink         │"));
  Serial.println(F("│  Type  n  for UNCONFIRMED uplink       │"));
  Serial.println(F("│  (defaults to UNCONFIRMED after 10 s)  │"));
  Serial.println(F("└────────────────────────────────────────┘"));
  Serial.print(F("Your choice: "));

  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    if (Serial.available()) {
      char c = (char)Serial.read();
      // flush the rest of the line
      while (Serial.available()) { Serial.read(); }
      if (c == 'y' || c == 'Y') {
        Serial.println(F("y  ->  CONFIRMED uplink"));
        return true;
      } else {
        Serial.println(F("n  ->  UNCONFIRMED uplink"));
        return false;
      }
    }
  }
  Serial.println(F("(timeout)  ->  UNCONFIRMED uplink"));
  return false;
}

// pretty-print a received downlink frame
static void printDownlink(const uint8_t* data, size_t len,
                          uint8_t rxWindow, const LoRaWANEvent_t& ev) {
  Serial.println(F("  +--------------------------------------------"));
  Serial.print(F("  | DOWNLINK received in RX"));
  Serial.print(rxWindow);
  if (ev.confirmed) {
    Serial.println(F(" [CONFIRMED - ACK piggybacked on next UL]"));
  } else {
    Serial.println(F(" [UNCONFIRMED]"));
  }
  Serial.println(F("  +--------------------------------------------"));
  if (ev.confirming) {
    Serial.println(F("  (downlink also carries ACK for our confirmed uplink)"));
  }
  Serial.print(F("  fPort  : "));
  Serial.println(ev.fPort);
  Serial.print(F("  Length : "));
  Serial.print(len);
  Serial.println(F(" bytes"));
  if (len > 0) {
    Serial.print(F("  HEX    : "));
    for (size_t i = 0; i < len; i++) {
      if (data[i] < 0x10) { Serial.print(F("0")); }
      Serial.print(data[i], HEX);
      Serial.print(F(" "));
    }
    Serial.println();
  }
  if (ev.frmPending) {
    Serial.println(F("  [Server has more frames pending]"));
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}
  Serial.println(F("\n=== LoRaWAN ISM2400: COMBINED EXAMPLE ==="));
  Serial.println(F("DR0 (SF12), scanGuard=50ms, 30s TX cycle."));

  // ask uplink mode before join so the label is shown from the first frame
  uplinkConfirmed = askConfirmedUplink(10000);
  Serial.print(F("\nMode: "));
  Serial.println(uplinkConfirmed ? F("CONFIRMED uplink") : F("UNCONFIRMED uplink"));
  Serial.println();

  SPI.begin();

  int16_t st = radio.begin(
    2403.0,
    812.5,
    12,
    8,
    RADIOLIB_SX128X_SYNC_WORD_LORAWAN,
    10,
    8
  );
  if (st != RADIOLIB_ERR_NONE) {
    Serial.print(F("Radio init failed: "));
    Serial.println(st);
    while (true) { delay(1000); }
  }

  st = node.beginOTAA(joinEUI, devEUI, NULL, appKey);
  if (st != RADIOLIB_ERR_NONE) {
    Serial.print(F("LoRaWAN init failed: "));
    Serial.println(st);
    while (true) { delay(1000); }
  }

  // DR0 = SF12, maximum range
  st = node.setDatarate(0);
  if (st != RADIOLIB_ERR_NONE) {
    Serial.print(F("setDatarate failed: "));
    Serial.println(st);
    while (true) { delay(1000); }
  }

  eepromBeginIfNeeded();
  restoreNonces();

  // widen the RX window guard to 50 ms for reliable downlink reception
  // with TTN ISM2400 + SF12 (rx1_delay = 5 s, timing jitter can exceed 10 ms)
  node.scanGuard = 50;

  tryJoin();
}

void loop() {
  if (!joined) {
    Serial.println(F("Not joined - retrying in 10 s"));
    delay(10000);
    tryJoin();
    return;
  }

  static uint32_t counter = 0;

  uint8_t up[4] = {
    (uint8_t)(counter >> 24),
    (uint8_t)(counter >> 16),
    (uint8_t)(counter >>  8),
    (uint8_t)(counter      )
  };

  uint8_t        down[256];
  size_t         downLen = 0;
  LoRaWANEvent_t evDown;
  memset(&evDown, 0, sizeof(evDown));

  Serial.print(uplinkConfirmed ? F("[UL-CONF] ") : F("[UL-UNCONF] "));
  Serial.print(F("counter="));
  Serial.print(counter);
  Serial.print(F(" -> "));

  int16_t st = node.sendReceive(up, sizeof(up), 1, down, &downLen,
                                uplinkConfirmed, NULL, &evDown);

  if (st < 0) {
    Serial.print(F("ERROR "));
    Serial.println(st);
    if (st == RADIOLIB_ERR_NETWORK_NOT_JOINED) {
      joined = false;
    }

  } else if (st == 0) {
    if (uplinkConfirmed) {
      Serial.println(F("NO ACK (both RX windows timed out)"));
    } else {
      Serial.println(F("OK (no downlink in this cycle)"));
    }
    counter++;

  } else {
    // st = 1 (RX1) or 2 (RX2): a downlink frame arrived
    if (uplinkConfirmed && evDown.confirming && downLen == 0) {
      // pure ACK for our confirmed uplink, no application payload
      Serial.print(F("ACK received in RX"));
      Serial.println(st);
    } else {
      Serial.println(F("OK"));
      printDownlink(down, downLen, (uint8_t)st, evDown);
    }
    counter++;
  }

  delay(10000);
}
