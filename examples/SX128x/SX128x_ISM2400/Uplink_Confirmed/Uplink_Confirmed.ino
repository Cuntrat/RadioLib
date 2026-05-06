/*
  Uplink Confirmed - LoRaWAN ISM2400 Example

  Demonstrates CONFIRMED uplink transmission (server SENDS ACK).
  - Sends counter every second
  - Waits for server acknowledgement
  - Retries if no ACK received (up to default retries)
  - Prints ACK status to serial
*/

#include <RadioLib.h>
#include <EEPROM.h>

// ============ PINS ============
#define SX1280_NSS   D7
#define SX1280_DIO1  D5
#define SX1280_NRST  A0
#define SX1280_BUSY  D3

// ============ CREDENTIALS (MSB) ============
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
static const uint32_t NONCES_MAGIC = 0x4C574E31UL; // "LWN1"
static const int EE_ADDR_MAGIC  = 0;
static const int EE_ADDR_NONCES = EE_ADDR_MAGIC + sizeof(uint32_t);
static const int EE_SIZE_BYTES  = EE_ADDR_NONCES + RADIOLIB_LORAWAN_NONCES_BUF_SIZE;

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
    Serial.println(F("No saved nonces"));
    return false;
  }

  uint8_t nonces[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];
  for(size_t i = 0; i < RADIOLIB_LORAWAN_NONCES_BUF_SIZE; i++) {
    nonces[i] = EEPROM.read(EE_ADDR_NONCES + i);
  }

  int16_t st = node.setBufferNonces(nonces);
  if(st == RADIOLIB_ERR_NONE) {
    Serial.println(F("Nonces restored"));
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
  Serial.println(F("\n>>> Starting OTAA Join <<<"));
  int16_t st = node.activateOTAA();

  // always save nonces, also on failure
  saveNonces();

  if(st == RADIOLIB_LORAWAN_NEW_SESSION || st == RADIOLIB_LORAWAN_SESSION_RESTORED) {
    joined = true;
    Serial.println(F("JOIN SUCCESSFUL"));
    Serial.print(F("DevAddr: 0x"));
    Serial.println(node.getDevAddr(), HEX);
  } else {
    joined = false;
    Serial.print(F("JOIN FAILED: "));
    Serial.println(st);
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial && millis() < 5000) {}

  Serial.println(F("\n=== LoRaWAN ISM2400: UPLINK CONFIRMED ==="));
  Serial.println(F("Mode: Send counter with ACK requirement\n"));

  SPI.begin();

  int16_t st = radio.begin(
    2403.0,                              // MHz
    812.5,                               // kHz
    12,                                  // SF
    8,                                   // CR base
    RADIOLIB_SX128X_SYNC_WORD_LORAWAN,   // 0x21
    10,                                  // dBm
    8                                    // preamble
  );
  if(st != RADIOLIB_ERR_NONE) {
    Serial.print(F("Radio init failed: "));
    Serial.println(st);
    while(true) delay(1000);
  }

  // LoRaWAN 1.0.4
  st = node.beginOTAA(joinEUI, devEUI, NULL, appKey);
  if(st != RADIOLIB_ERR_NONE) {
    Serial.print(F("LoRaWAN init failed: "));
    Serial.println(st);
    while(true) delay(1000);
  }

  st = node.setDatarate(0); // DR0 = SF12
  if(st != RADIOLIB_ERR_NONE) {
    Serial.print(F("setDatarate failed: "));
    Serial.println(st);
    while(true) delay(1000);
  }

  eepromBeginIfNeeded();
  restoreNonces();
  tryJoin();
}

void loop() {
  if(!joined) {
    delay(10000);
    tryJoin();
    return;
  }

  static uint32_t counter = 0;
  uint8_t up[4] = {
    (uint8_t)(counter >> 24),
    (uint8_t)(counter >> 16),
    (uint8_t)(counter >> 8),
    (uint8_t)(counter)
  };

  // CONFIRMED uplink: use sendReceive with fPort=1 and confirmed flag (true)
  uint8_t down[256];
  size_t downLen = 0;

  Serial.print(F("Sending CONFIRMED uplink [counter="));
  Serial.print(counter);
  Serial.print(F("]... "));

  int16_t st = node.sendReceive(up, sizeof(up), 1, down, &downLen, true);
  Serial.print(F("Result: "));
  Serial.println(st);

  if(st == RADIOLIB_ERR_RX_TIMEOUT) {
    Serial.println(F("  ⚠ NO ACK RECEIVED (timeout)"));
    // In confirmed mode, failed ACK means frame might not be received
  } else if(st == RADIOLIB_ERR_NETWORK_NOT_JOINED) {
    joined = false;
  } else if(st == 0) {
    // Result 0 = ACK received, no downlink data
    Serial.println(F("  ✓ ACK RECEIVED (clean)"));
    counter++;
  } else if(st > 0) {
    // Result > 0 = ACK received + downlink bytes
    Serial.print(F("  ✓ ACK RECEIVED + DOWNLINK ("));
    Serial.print(st);
    Serial.println(F(" bytes)"));
    
    Serial.print(F("  Hex: "));
    for(size_t i = 0; i < downLen; i++) {
      if(down[i] < 0x10) Serial.print(F("0"));
      Serial.print(down[i], HEX);
      Serial.print(F(" "));
    }
    Serial.println();
    
    Serial.print(F("  ASCII: "));
    for(size_t i = 0; i < downLen; i++) {
      if(down[i] >= 32 && down[i] <= 126) {
        Serial.print((char)down[i]);
      } else {
        Serial.print(F("."));
      }
    }
    Serial.println();
    
    counter++;
  } else {
    Serial.print(F("  ✗ ERROR: "));
    Serial.println(st);
  }

  delay(30000); // Send every 30 seconds (to ensure RX windows complete)
}
