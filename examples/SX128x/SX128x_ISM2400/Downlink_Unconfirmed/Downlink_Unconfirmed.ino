/*
  Downlink Unconfirmed - LoRaWAN ISM2400 (SX1280) Example

  Demonstrates reception of UNCONFIRMED downlink messages from the
  network server on the 2.4 GHz ISM band.

  LoRaWAN Class A fundamentals
  ─────────────────────────────
  A Class-A device can ONLY receive during the two RX windows that
  follow EVERY uplink transmission (RX1 at +1 s, RX2 at +2 s after
  the end of the TX burst).  There is no way to "just listen"; the
  device MUST send an uplink first to open those windows.

  TTN "Schedule downlink" flow
  ─────────────────────────────
  1. Open your device page on TTN → Messaging → Schedule downlink.
  2. Enter payload hex (e.g. 01 02 03) and confirm.
  3. TTN queues the frame; it will be delivered the next time your
     device opens a downlink window (i.e. after the next uplink).
  4. This sketch sends an unconfirmed uplink every 30 seconds, so
     the queued downlink arrives within one TX cycle.

  Behaviour:
    - OTAA join with nonces saved to EEPROM
    - Sends a minimal unconfirmed uplink every 30 seconds
    - Prints any downlink payload received in RX1 or RX2
    - Does NOT send an ACK for the downlink (unconfirmed mode)

  sendReceive() return value:
    < 0   hard error
    = 0   TX OK, no downlink received
    = 1   TX OK, downlink received in RX1
    = 2   TX OK, downlink received in RX2

  For more information visit:
    https://github.com/jgromes/RadioLib/
*/

#include <RadioLib.h>
#include <EEPROM.h>

// ============ HARDWARE PINS ============
#define SX1280_NSS   D7
#define SX1280_DIO1  D5
#define SX1280_NRST  A0
#define SX1280_BUSY  D3

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

bool joined = false;

// ============ NONCES PERSISTENCE ============
static const uint32_t NONCES_MAGIC  = 0x4C574E31UL;
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
  if(magic != NONCES_MAGIC) {
    Serial.println(F("No saved nonces - will perform fresh join"));
    return false;
  }
  uint8_t nonces[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];
  for(size_t i = 0; i < RADIOLIB_LORAWAN_NONCES_BUF_SIZE; i++) {
    nonces[i] = EEPROM.read(EE_ADDR_NONCES + i);
  }
  int16_t st = node.setBufferNonces(nonces);
  if(st == RADIOLIB_ERR_NONE) {
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
  for(size_t i = 0; i < RADIOLIB_LORAWAN_NONCES_BUF_SIZE; i++) {
    EEPROM.write(EE_ADDR_NONCES + i, p[i]);
  }
  eepromCommitIfNeeded();
}

void tryJoin() {
  Serial.println(F("Starting OTAA join..."));
  int16_t st = node.activateOTAA();
  saveNonces();
  if(st == RADIOLIB_LORAWAN_NEW_SESSION || st == RADIOLIB_LORAWAN_SESSION_RESTORED) {
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

// Helper: pretty-print a downlink payload.
static void printDownlink(const uint8_t* data, size_t len, uint8_t rxWindow) {
  Serial.println(F("  +--------------------------+"));
  Serial.print(F("  | DOWNLINK RECEIVED in RX"));
  Serial.print(rxWindow);
  Serial.println(F(" |"));
  Serial.println(F("  +--------------------------+"));
  Serial.print(F("  Length : "));
  Serial.print(len);
  Serial.println(F(" bytes"));
  Serial.print(F("  HEX    : "));
  for(size_t i = 0; i < len; i++) {
    if(data[i] < 0x10) Serial.print(F("0"));
    Serial.print(data[i], HEX);
    Serial.print(F(" "));
  }
  Serial.println();
  Serial.print(F("  ASCII  : "));
  for(size_t i = 0; i < len; i++) {
    char c = (char)data[i];
    Serial.print(isPrintable(c) ? c : '.');
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while(!Serial && millis() < 5000) {}
  Serial.println(F("\n=== LoRaWAN ISM2400: DOWNLINK UNCONFIRMED ==="));
  Serial.println(F("Sends uplink every 30 s to open RX windows."));
  Serial.println(F("Schedule a downlink on TTN; it arrives after the next TX.\n"));

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
  if(st != RADIOLIB_ERR_NONE) {
    Serial.print(F("Radio init failed: "));
    Serial.println(st);
    while(true) { delay(1000); }
  }

  st = node.beginOTAA(joinEUI, devEUI, NULL, appKey);
  if(st != RADIOLIB_ERR_NONE) {
    Serial.print(F("LoRaWAN init failed: "));
    Serial.println(st);
    while(true) { delay(1000); }
  }

  // DR0 = SF12, maximum range.
  st = node.setDatarate(0);
  if(st != RADIOLIB_ERR_NONE) {
    Serial.print(F("setDatarate failed: "));
    Serial.println(st);
    while(true) { delay(1000); }
  }

  eepromBeginIfNeeded();
  restoreNonces();

  // Widen the RX window guard to 50 ms (see Uplink_Unconfirmed for details).
  node.scanGuard = 50;

  tryJoin();
}

void loop() {
  if(!joined) {
    Serial.println(F("Not joined - retrying in 10 s"));
    delay(10000);
    tryJoin();
    return;
  }

  static uint32_t counter = 0;

  // Minimal uplink payload: 4-byte counter.
  // An empty payload (lenUp = 0) would also work, but a non-zero
  // payload makes it easy to correlate TX/RX in the TTN live-data view.
  uint8_t up[4] = {
    (uint8_t)(counter >> 24),
    (uint8_t)(counter >> 16),
    (uint8_t)(counter >>  8),
    (uint8_t)(counter      )
  };

  uint8_t down[256];
  size_t  downLen = 0;

  Serial.print(F("[UL] counter="));
  Serial.print(counter);
  Serial.print(F(" -> "));

  // Unconfirmed uplink; RX windows are still opened and any queued
  // downlink will be received.
  int16_t st = node.sendReceive(up, sizeof(up), 1, down, &downLen, false);

  if(st < 0) {
    Serial.print(F("ERROR "));
    Serial.println(st);
    if(st == RADIOLIB_ERR_NETWORK_NOT_JOINED) {
      joined = false;
    }

  } else if(st == 0) {
    Serial.println(F("OK (no downlink in this cycle)"));
    counter++;

  } else {
    // st = 1 or 2: a downlink frame was received.
    Serial.println(F("OK"));
    if(downLen > 0) {
      printDownlink(down, downLen, (uint8_t)st);
    } else {
      // Downlink with no application payload (MAC-only frame).
      Serial.print(F("  MAC-only downlink in RX"));
      Serial.println(st);
    }
    counter++;
  }

  delay(30000);
}
