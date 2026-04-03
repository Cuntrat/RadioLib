/*
 * LoRaWAN ABP Example - 2.4 GHz ISM Band (SX1280)
 * 
 * This example demonstrates LoRaWAN Activation By Personalization (ABP)
 * using the ISM2_4GHz band with the SX1280 radio.
 * 
 * ABP vs OTAA:
 *   - ABP: Session keys pre-configured, no join needed (simpler, less secure)
 *   - OTAA: Join procedure required (more secure, preferred for production)
 * 
 * IMPORTANT: This requires a LoRaWAN gateway supporting 2.4 GHz!
 * 
 * Hardware:
 *   - STM NUCLEO-L073RZ (or other Arduino-compatible board)
 *   - SX1280RF1ZHP shield or SX1280 module
 *   - 2.4 GHz antenna
 * 
 * Pinout (MBED Arduino Shield):
 *   NSS   = D7
 *   DIO1  = D11
 *   NRST  = A0
 *   BUSY  = D3
 */

#include <RadioLib.h>

// Pin definitions
#define SX1280_NSS    D7
#define SX1280_DIO1   D11
#define SX1280_NRST   A0
#define SX1280_BUSY   D3

// Create instances
SX1280 radio = new Module(SX1280_NSS, SX1280_DIO1, SX1280_NRST, SX1280_BUSY);
LoRaWANNode node(&radio, &ISM2_4GHz);

// LoRaWAN ABP credentials
// IMPORTANT: Get these from your network server when registering device!

// DevAddr - 32-bit device address
uint32_t devAddr = 0x00000000;

// AppSKey - 128-bit Application Session Key
uint8_t appSKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// NwkSKey - 128-bit Network Session Key  
uint8_t nwkSKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Frame counters (start at 0, increment after each uplink)
uint32_t fCntUp = 0;
uint32_t fCntDown = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000);
  
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║  LoRaWAN ABP - 2.4 GHz ISM Band (SX1280)       ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  Serial.println();
  
  // Initialize SPI
  SPI.begin();
  
  Serial.println(F("Initializing SX1280..."));
  
  int16_t state = radio.begin(
    2440.0,                              // Frequency
    203.125,                             // Bandwidth
    10,                                  // SF10 (DR2)
    5,                                   // CR 4/5
    RADIOLIB_SX128X_SYNC_WORD_LORAWAN,  // 0x34
    10,                                  // TX power
    8                                    // Preamble
  );
  
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("✗ Radio init failed: "));
    Serial.println(state);
    while (true) delay(1000);
  }
  
  Serial.println(F("✓ SX1280 initialized"));
  Serial.println();
  
  // Initialize LoRaWAN with ABP
  Serial.println(F("Activating ABP session..."));
  state = node.beginABP(devAddr, nwkSKey, appSKey);
  
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("✗ ABP activation failed: "));
    Serial.println(state);
    while (true) delay(1000);
  }
  
  Serial.println(F("✓ ABP session active!"));
  Serial.println();
  
  Serial.println(F("Configuration:"));
  Serial.println(F("  Band:       ISM2_4GHz"));
  Serial.println(F("  Frequencies: 2440, 2450, 2460 MHz"));
  Serial.print(F("  DevAddr:    0x"));
  Serial.println(devAddr, HEX);
  Serial.println();
  
  Serial.println(F("Ready! Sending uplinks every 30 seconds..."));
  Serial.println(F("════════════════════════════════════════════════"));
  Serial.println();
}

void loop() {
  // Build payload
  uint8_t payload[16];
  uint8_t len = 0;
  
  // Counter (4 bytes)
  payload[len++] = (fCntUp >> 24) & 0xFF;
  payload[len++] = (fCntUp >> 16) & 0xFF;
  payload[len++] = (fCntUp >> 8) & 0xFF;
  payload[len++] = fCntUp & 0xFF;
  
  // Dummy sensor value (2 bytes)
  int16_t value = 2350;  // 23.50°C
  payload[len++] = (value >> 8) & 0xFF;
  payload[len++] = value & 0xFF;
  
  Serial.print(F("[TX #"));
  Serial.print(fCntUp);
  Serial.print(F("] "));
  Serial.print(len);
  Serial.print(F(" bytes: "));
  for (uint8_t i = 0; i < len; i++) {
    if (payload[i] < 16) Serial.print('0');
    Serial.print(payload[i], HEX);
    Serial.print(' ');
  }
  Serial.print(F("... "));
  
  // Send
  int16_t state = node.sendReceive(payload, len, 1);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("✓ SENT"));
    fCntUp++;
    
    // Check downlink
    if (node.downlinkReceived()) {
      Serial.println(F("  ↓ Downlink!"));
      
      uint8_t dl[256];
      size_t dlLen = node.downlinkLength();
      uint8_t port = node.downlinkPort();
      
      node.getDownlink(dl, &dlLen);
      
      Serial.print(F("  Port "));
      Serial.print(port);
      Serial.print(F(", "));
      Serial.print(dlLen);
      Serial.print(F(" bytes: "));
      for (size_t i = 0; i < dlLen; i++) {
        if (dl[i] < 16) Serial.print('0');
        Serial.print(dl[i], HEX);
        Serial.print(' ');
      }
      Serial.println();
    }
  } else {
    Serial.print(F("✗ FAIL ("));
    Serial.print(state);
    Serial.println(F(")"));
  }
  
  Serial.println();
  delay(30000);  // 30 seconds
}
