/*
  Uplink Confirmed - LoRaWAN ISM2400 (SX1280) Example

  Demonstrates CONFIRMED uplink on the 2.4 GHz ISM band.
  The server MUST acknowledge every frame; if no ACK arrives the
  uplink is retransmitted up to nbTrans times (ADR-controlled).

  Behaviour:
    - OTAA join with nonces saved to EEPROM (survives reset)
    - Sends a 4-byte counter every 30 seconds with isConfirmed = true
    - Interprets sendReceive() return value correctly:
        < 0   hard error
        = 0   TX done, no downlink in either window
              (for confirmed uplink this means NO ACK was received;
               the frame counter is still incremented by RadioLib)
        = 1   downlink (ACK or data) received in RX1
        = 2   downlink (ACK or data) received in RX2
      eventDown.confirming == true means the downlink carries the ACK
      bit set in reply to our confirmed uplink.

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

void setup() {
  Serial.begin(115200);
  while(!Serial && millis() < 5000) {}
  Serial.println(F("\n=== LoRaWAN ISM2400: UPLINK CONFIRMED ==="));

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

  uint8_t          down[256];
  size_t           downLen = 0;
  LoRaWANEvent_t   evDown;
  memset(&evDown, 0, sizeof(evDown));

  Serial.print(F("[UL-CONF] counter="));
  Serial.print(counter);
  Serial.print(F(" ... "));

  // sendReceive(dataUp, lenUp, fPort, dataDown, &lenDown, isConfirmed, NULL, &eventDown)
  // isConfirmed = true  ->  confirmed uplink (MType 0x80)
  int16_t st = node.sendReceive(up, sizeof(up), 1, down, &downLen, true, NULL, &evDown);

  if(st < 0) {
    // Hard error.
    Serial.print(F("ERROR "));
    Serial.println(st);
    if(st == RADIOLIB_ERR_NETWORK_NOT_JOINED) {
      joined = false;
    }
    // Do NOT increment counter on error.

  } else if(st == 0) {
    // TX succeeded but neither RX1 nor RX2 returned a frame.
    // For a confirmed uplink this means the ACK was NOT received.
    // RadioLib still increments fCntUp internally; we mirror that here.
    Serial.println(F("NO ACK (both windows timed out)"));
    counter++;

  } else {
    // st = 1 (RX1) or 2 (RX2): a downlink frame arrived.
    // evDown.confirming == true  -> frame carries ACK for our uplink.
    // evDown.confirmed  == true  -> frame is itself a confirmed downlink
    //                               (server expects us to ACK it in the
    //                               next uplink, done automatically by
    //                               RadioLib).
    if(evDown.confirming) {
      Serial.print(F("ACK received in RX"));
      Serial.print(st);
    } else {
      Serial.print(F("Downlink (no ACK bit) in RX"));
      Serial.print(st);
    }

    if(downLen > 0) {
      Serial.print(F(" + "));
      Serial.print(downLen);
      Serial.println(F(" bytes payload:"));
      Serial.print(F("  HEX: "));
      for(size_t i = 0; i < downLen; i++) {
        if(down[i] < 0x10) Serial.print(F("0"));
        Serial.print(down[i], HEX);
        Serial.print(F(" "));
      }
      Serial.println();
    } else {
      Serial.println(F(" (no application payload)"));
    }

    counter++;
  }

  // Confirmed uplinks + RX windows cost more air-time: 30 s minimum.
  delay(30000);
}
