/*
  Uplink Unconfirmed - LoRaWAN ISM2400 (SX1280) Example

  Demonstrates UNCONFIRMED uplink on the 2.4 GHz ISM band.
  The server does NOT send an acknowledgement (fire-and-forget).

  Behaviour:
    - OTAA join with nonces saved to EEPROM (survives reset)
    - Sends a 4-byte counter every 30 seconds
    - Opens RX1/RX2 windows after every TX; prints any opportunistic
      downlink that arrives, but does not expect one
    - sendReceive() return value:
        < 0   hard error (e.g. RADIOLIB_ERR_NETWORK_NOT_JOINED = -1101)
        = 0   TX OK, no downlink received in RX windows
        = 1   TX OK, downlink received in RX1 window
        = 2   TX OK, downlink received in RX2 window

  For more information visit:
    https://github.com/jgromes/RadioLib/
*/

#include <RadioLib.h>
#include <EEPROM.h>

// ============ HARDWARE PINS ============
// Adjust to match your board wiring.
#define SX1280_NSS   D7
#define SX1280_DIO1  D5
#define SX1280_NRST  A0
#define SX1280_BUSY  D3

// ============ LORAWAN CREDENTIALS (MSB order) ============
// Replace with the values shown in your TTN / LNS device page.
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
// Nonces must survive reset so the join counter never repeats.
static const uint32_t NONCES_MAGIC  = 0x4C574E31UL; // "LWN1"
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

  // Always save nonces even on failure (devNonce must not be reused).
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
  Serial.println(F("\n=== LoRaWAN ISM2400: UPLINK UNCONFIRMED ==="));

  SPI.begin();

  // Initialise the SX1280 with the ISM2400 LoRaWAN physical layer
  // settings. The LoRaWANNode will override modulation parameters
  // per-DR; these initial values only matter for the radio driver init.
  int16_t st = radio.begin(
    2403.0,                            // centre frequency [MHz]
    812.5,                             // bandwidth [kHz]
    12,                                // spreading factor
    8,                                 // coding rate denominator
    RADIOLIB_SX128X_SYNC_WORD_LORAWAN, // sync word 0x21
    10,                                // output power [dBm]
    8                                  // preamble length
  );
  if(st != RADIOLIB_ERR_NONE) {
    Serial.print(F("Radio init failed: "));
    Serial.println(st);
    while(true) { delay(1000); }
  }

  // LoRaWAN 1.0.4 uses only AppKey (nwkKey = NULL).
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

  // Build a minimal 4-byte payload: big-endian counter.
  uint8_t up[4] = {
    (uint8_t)(counter >> 24),
    (uint8_t)(counter >> 16),
    (uint8_t)(counter >>  8),
    (uint8_t)(counter      )
  };

  // Buffer for any opportunistic downlink.
  uint8_t  down[256];
  size_t   downLen = 0;

  Serial.print(F("[UL-UNCONF] counter="));
  Serial.print(counter);
  Serial.print(F(" ... "));

  // sendReceive(dataUp, lenUp, fPort, dataDown, &lenDown, isConfirmed)
  // isConfirmed = false  ->  unconfirmed uplink (MType 0x40)
  int16_t st = node.sendReceive(up, sizeof(up), 1, down, &downLen, false);

  if(st < 0) {
    // Hard error.
    Serial.print(F("ERROR "));
    Serial.println(st);
    if(st == RADIOLIB_ERR_NETWORK_NOT_JOINED) {
      joined = false;
    }

  } else if(st == 0) {
    // TX succeeded; no downlink arrived in RX1 or RX2. This is the
    // expected result for unconfirmed uplink in a normal deployment.
    Serial.println(F("OK (no downlink)"));
    counter++;

  } else {
    // st = 1 (RX1) or 2 (RX2): opportunistic downlink received.
    Serial.print(F("OK + downlink in RX"));
    Serial.print(st);
    Serial.print(F(" ("));
    Serial.print(downLen);
    Serial.println(F(" bytes)"));
    if(downLen > 0) {
      Serial.print(F("  HEX: "));
      for(size_t i = 0; i < downLen; i++) {
        if(down[i] < 0x10) Serial.print(F("0"));
        Serial.print(down[i], HEX);
        Serial.print(F(" "));
      }
      Serial.println();
    }
    counter++;
  }

  // LoRaWAN fair-use / duty-cycle: wait at least 30 s between uplinks.
  delay(30000);
}
