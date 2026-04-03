/*
 * LoRaWAN OTAA Example - 2.4 GHz ISM Band (SX1280)
 * 
 * This example demonstrates proper LoRaWAN Over-The-Air Activation (OTAA)
 * using the ISM2_4GHz band with the SX1280 radio.
 * 
 * IMPORTANT: This requires a LoRaWAN gateway supporting 2.4 GHz!
 * Unlike raw LoRa, LoRaWAN implements:
 *   - Network join procedure (OTAA)
 *   - AES-128 encryption
 *   - Frame counters for replay protection
 *   - MAC commands (ADR, duty cycle, etc.)
 *   - Network server integration
 * 
 * Hardware:
 *   - STM NUCLEO-L073RZ (or other Arduino-compatible board)
 *   - SX1280RF1ZHP shield or SX1280 module
 *   - 2.4 GHz antenna
 * 
 * Pinout (MBED Arduino Shield - adjust for your board):
 *   NSS   = D7
 *   DIO1  = D11
 *   NRST  = A0
 *   BUSY  = D3
 *   MOSI  = D11 (hardware SPI)
 *   MISO  = D12 (hardware SPI)
 *   SCK   = D13 (hardware SPI)
 * 
 * Network Server Configuration:
 *   - Add device with OTAA credentials below
 *   - Configure gateway for 2.4 GHz reception
 *   - Set frequencies: 2440, 2450, 2460 MHz (or adjust txFreqs in band definition)
 */

#include <RadioLib.h>

// Pin definitions for SX1280RF1ZHP shield on Nucleo
// Adjust these for your hardware!
#define SX1280_NSS    D7      // CS pin
#define SX1280_DIO1   D11     // DIO1 / IRQ pin  
#define SX1280_NRST   A0      // RESET pin
#define SX1280_BUSY   D3      // BUSY pin

// Create radio module instance
SX1280 radio = new Module(SX1280_NSS, SX1280_DIO1, SX1280_NRST, SX1280_BUSY);

// Create LoRaWAN node instance using ISM2_4GHz band
LoRaWANNode node(&radio, &ISM2_4GHz);

// LoRaWAN OTAA credentials
// IMPORTANT: Replace these with your device credentials from network server!
// These are example values - they will NOT work without proper registration.

// JoinEUI (formerly AppEUI) - 64-bit, LSB first
uint64_t joinEUI = 0x0000000000000000;

// DevEUI - 64-bit, unique device identifier, LSB first
uint64_t devEUI = 0x0000000000000000;

// AppKey - 128-bit AES key for application layer encryption
uint8_t appKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// NwkKey - 128-bit AES key for network layer encryption (LoRaWAN 1.1+)
// For LoRaWAN 1.0.x, this is the same as AppKey
uint8_t nwkKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Uplink counter (for ABP mode or session restore)
uint32_t fCntUp = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000);  // Wait max 5s for serial
  
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║  LoRaWAN OTAA - 2.4 GHz ISM Band (SX1280)      ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  Serial.println();
  
  Serial.println(F("Hardware: SX1280 @ 2.4 GHz"));
  Serial.print(F("Pins: NSS=D")); Serial.print(SX1280_NSS);
  Serial.print(F(" DIO1=D")); Serial.print(SX1280_DIO1);
  Serial.print(F(" NRST=A")); Serial.print(SX1280_NRST - A0);
  Serial.print(F(" BUSY=D")); Serial.println(SX1280_BUSY);
  Serial.println();

  // Initialize SPI
  SPI.begin();
  
  Serial.println(F("Initializing SX1280..."));
  
  // Initialize radio with default settings for ISM2_4GHz band
  // Frequency will be managed by LoRaWAN stack
  int16_t state = radio.begin(
    2440.0,                              // Initial frequency (MHz)
    203.125,                             // Bandwidth (kHz) - DR2 default
    10,                                  // Spreading Factor - DR2 default
    5,                                   // Coding Rate 4/5
    RADIOLIB_SX128X_SYNC_WORD_LORAWAN,  // LoRaWAN sync word (0x34)
    10,                                  // TX power (dBm)
    8                                    // Preamble length
  );
  
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("✗ Radio initialization FAILED with code: "));
    Serial.println(state);
    while (true) { delay(1000); }
  }
  
  Serial.println(F("✓ SX1280 initialized"));
  Serial.println();
  
  // Initialize LoRaWAN node with OTAA credentials
  Serial.println(F("Initializing LoRaWAN node..."));
  state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
  
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("✗ LoRaWAN initialization FAILED with code: "));
    Serial.println(state);
    while (true) { delay(1000); }
  }
  
  Serial.println(F("✓ LoRaWAN node initialized"));
  Serial.println();
  
  // Print configuration
  Serial.println(F("Configuration:"));
  Serial.println(F("  Band:       ISM2_4GHz"));
  Serial.println(F("  Frequencies: 2440, 2450, 2460 MHz"));
  Serial.println(F("  Data Rates: DR0-DR6 (SF12-SF5)"));
  Serial.println(F("  Sync Word:  0x34 (LoRaWAN public)"));
  Serial.println(F("  Join Type:  OTAA"));
  Serial.println();
  
  // Attempt OTAA join
  Serial.println(F("Attempting OTAA join..."));
  Serial.println(F("(This requires a LoRaWAN gateway at 2.4 GHz!)"));
  Serial.println();
  
  // Try to join the network
  // This will send Join Request packets and wait for Join Accept
  state = node.activateOTAA();
  
  if (state == RADIOLIB_LORAWAN_NEW_SESSION) {
    Serial.println(F("✓✓✓ JOIN SUCCESSFUL! ✓✓✓"));
    Serial.println(F("Network session established"));
    Serial.println();
    
    // Print session info
    Serial.println(F("Session Information:"));
    Serial.print(F("  DevAddr:  0x"));
    Serial.println(node.getDevAddr(), HEX);
    Serial.println();
    
  } else if (state == RADIOLIB_ERR_NETWORK_NOT_JOINED) {
    Serial.println(F("✗ Join failed - no response from network"));
    Serial.println(F(""));
    Serial.println(F("Possible causes:"));
    Serial.println(F("  1. No gateway in range"));
    Serial.println(F("  2. Gateway not configured for 2.4 GHz"));
    Serial.println(F("  3. Wrong credentials (JoinEUI/DevEUI/Keys)"));
    Serial.println(F("  4. Gateway frequencies don't match (2440/2450/2460 MHz)"));
    Serial.println();
    Serial.println(F("Will retry in loop..."));
    
  } else {
    Serial.print(F("✗ Join failed with code: "));
    Serial.println(state);
    Serial.println(F("Check wiring and credentials"));
  }
  
  Serial.println(F("Ready! Entering main loop..."));
  Serial.println(F("════════════════════════════════════════════════"));
  Serial.println();
}

void loop() {
  // Build uplink payload
  uint8_t payload[32];
  uint8_t payloadLen = 0;
  
  // Example: Send counter + temperature reading
  uint32_t counter = fCntUp;
  payload[payloadLen++] = (counter >> 24) & 0xFF;
  payload[payloadLen++] = (counter >> 16) & 0xFF;
  payload[payloadLen++] = (counter >> 8) & 0xFF;
  payload[payloadLen++] = counter & 0xFF;
  
  // Example: Add dummy sensor data (replace with real sensors!)
  int16_t temperature = 2350;  // 23.50°C in centidegrees
  payload[payloadLen++] = (temperature >> 8) & 0xFF;
  payload[payloadLen++] = temperature & 0xFF;
  
  Serial.print(F("[UPLINK #"));
  Serial.print(counter);
  Serial.print(F("] Port 1, "));
  Serial.print(payloadLen);
  Serial.print(F(" bytes: "));
  for (uint8_t i = 0; i < payloadLen; i++) {
    if (payload[i] < 16) Serial.print('0');
    Serial.print(payload[i], HEX);
    Serial.print(' ');
  }
  Serial.print(F("... "));
  
  // Send uplink and optionally receive downlink (Class A behavior)
  // Port 1 is used for application data
  int16_t state = node.sendReceive(payload, payloadLen, 1);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("✓ SENT"));
    fCntUp++;
    
    // Check for downlink
    if (node.downlinkReceived()) {
      Serial.println(F("  ↓ Downlink received!"));
      
      uint8_t downlink[256];
      size_t downlinkLen = node.downlinkLength();
      uint8_t port = node.downlinkPort();
      
      node.getDownlink(downlink, &downlinkLen);
      
      Serial.print(F("  Port "));
      Serial.print(port);
      Serial.print(F(", "));
      Serial.print(downlinkLen);
      Serial.print(F(" bytes: "));
      for (size_t i = 0; i < downlinkLen; i++) {
        if (downlink[i] < 16) Serial.print('0');
        Serial.print(downlink[i], HEX);
        Serial.print(' ');
      }
      Serial.println();
    }
    
  } else if (state == RADIOLIB_ERR_NETWORK_NOT_JOINED) {
    Serial.println(F("✗ NOT JOINED"));
    Serial.println(F("  Attempting rejoin..."));
    
    state = node.activateOTAA();
    if (state == RADIOLIB_LORAWAN_NEW_SESSION) {
      Serial.println(F("  ✓ Rejoin successful!"));
    } else {
      Serial.print(F("  ✗ Rejoin failed: "));
      Serial.println(state);
    }
    
  } else {
    Serial.print(F("✗ FAILED (code: "));
    Serial.print(state);
    Serial.println(F(")"));
  }
  
  Serial.println();
  
  // Wait before next uplink (Class A device)
  // Adjust based on your duty cycle requirements
  delay(30000);  // 30 seconds between transmissions
}
