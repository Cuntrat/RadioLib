/*
  Downlink Confirmed - LoRaWAN ISM2400 (SX1280) Example

  Demonstrates reception of CONFIRMED downlink messages from the
  network server on the 2.4 GHz ISM band.

  LoRaWAN Class A fundamentals
  ─────────────────────────────
  A Class-A device can ONLY receive during the two RX windows that
  follow EVERY uplink transmission.  "Confirmed downlink" means the
  SERVER requests an acknowledgement from the device; RadioLib handles
  that automatically: it sets the ACK bit in the FCtrl field of the
  NEXT uplink frame (piggybacked, no extra transmission required).

  TTN "Schedule downlink" flow
  ─────────────────────────────
  1. Open your device page on TTN → Messaging → Schedule downlink.
  2. Enable "Confirmed downlink" toggle (if shown), enter payload hex,
     and confirm.  TTN queues the frame.
  3. This sketch sends a CONFIRMED uplink every 30 seconds.  The queued
     downlink will arrive in the RX1/RX2 window of the next uplink.
  4. The ACK for that confirmed downlink is piggybacked on the following
     uplink — you will see "Ack" flagged in the TTN live-data view.

  Why send a CONFIRMED uplink here?
  ──────────────────────────────────
  A confirmed uplink makes it easy to verify the round-trip on TTN:
  the network sends an ACK downlink in RX1/RX2 (evDown.confirming=true),
  which can carry the queued application payload at the same time.
  An unconfirmed uplink would also open the RX windows and is equally
  valid for receiving a confirmed downlink.

  sendReceive() return value:
    < 0   hard error
    = 0   TX done, no downlink received in either window
    = 1   downlink received in RX1
    = 2   downlink received in RX2

  LoRaWANEvent_t fields used here:
    evDown.confirming  true if the downlink carries the ACK bit for our
                       confirmed uplink
    evDown.confirmed   true if the downlink itself is a confirmed frame
                       (server wants us to ACK it; RadioLib sets the ACK
                       bit automatically in the next TX)
    evDown.frmPending  true if the server has more frames queued

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
static void printDownlink(const uint8_t* data, size_t len,
                          uint8_t rxWindow, const LoRaWANEvent_t& ev) {
  Serial.println(F("  +----------------------------------------+"));
  Serial.print(F("  | DOWNLINK RECEIVED in RX"));
  Serial.print(rxWindow);
  if(ev.confirmed) {
    Serial.print(F(" [CONFIRMED - ACK will be"));
    Serial.println(F(" piggybacked on next UL] |"));
  } else {
    Serial.println(F(" [UNCONFIRMED] |"));
  }
  Serial.println(F("  +----------------------------------------+"));
  if(ev.confirming) {
    Serial.println(F("  (also carries ACK for our confirmed uplink)"));
  }
  Serial.print(F("  fPort  : "));
  Serial.println(ev.fPort);
  Serial.print(F("  Length : "));
  Serial.print(len);
  Serial.println(F(" bytes"));
  if(len > 0) {
    Serial.print(F("  HEX    : "));
    for(size_t i = 0; i < len; i++) {
      if(data[i] < 0x10) Serial.print(F("0"));
      Serial.print(data[i], HEX);
      Serial.print(F(" "));
    }
    Serial.println();
  }
  if(ev.frmPending) {
    Serial.println(F("  [Server has more frames pending]"));
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial && millis() < 5000) {}
  Serial.println(F("\n=== LoRaWAN ISM2400: DOWNLINK CONFIRMED ==="));
  Serial.println(F("Sends confirmed uplink every 30 s to open RX windows."));
  Serial.println(F("Schedule a confirmed downlink on TTN; arrives after next TX.\n"));

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

  uint8_t up[4] = {
    (uint8_t)(counter >> 24),
    (uint8_t)(counter >> 16),
    (uint8_t)(counter >>  8),
    (uint8_t)(counter      )
  };

  uint8_t         down[256];
  size_t          downLen = 0;
  LoRaWANEvent_t  evDown;
  memset(&evDown, 0, sizeof(evDown));

  Serial.print(F("[UL-CONF] counter="));
  Serial.print(counter);
  Serial.print(F(" -> "));

  // Confirmed uplink opens RX windows; any queued downlink (confirmed
  // or unconfirmed) will arrive in RX1 or RX2.
  // RadioLib automatically piggybacks ACK bits for confirmed downlinks
  // on the NEXT uplink call -- no user action required.
  int16_t st = node.sendReceive(up, sizeof(up), 1, down, &downLen, true, NULL, &evDown);

  if(st < 0) {
    Serial.print(F("ERROR "));
    Serial.println(st);
    if(st == RADIOLIB_ERR_NETWORK_NOT_JOINED) {
      joined = false;
    }

  } else if(st == 0) {
    // Neither RX window returned a frame.
    Serial.println(F("NO ACK (both windows timed out)"));
    counter++;

  } else {
    // st = 1 (RX1) or 2 (RX2).
    if(evDown.confirming && downLen == 0) {
      // Pure ACK frame for our confirmed uplink, no application payload.
      Serial.print(F("ACK received in RX"));
      Serial.println(st);
    } else {
      Serial.println(F("OK"));
      printDownlink(down, downLen, (uint8_t)st, evDown);
    }
    counter++;
  }

  delay(30000);
}
