/*
 * SX1280 @ 2.4 GHz - Single Device Test for STM Nucleo L073RZ
 * 
 * Tests SX1280 shield on Nucleo board
 * Transmits test packets every 3 seconds
 * 
 * Hardware:
 *   - STM NUCLEO-L073RZ
 *   - SX1280RF1ZHP shield
 *   - Antenna connected to SX1280
 * 
 * Pinout (Standard Arduino Shield):
 *   NSS   = D10 (PB_6)
 *   DIO1  = D2  (PA_10) - or try D3 if D2 doesn't work
 *   NRST  = D8  (PA_9)  - or try A0 if D8 doesn't work
 *   BUSY  = D7  (PA_8)  - or try A1 if D7 doesn't work
 *   MOSI  = D11 (PA_7)  - Hardware SPI
 *   MISO  = D12 (PA_6)  - Hardware SPI
 *   SCK   = D13 (PA_5)  - Hardware SPI
 */

#include <RadioLib.h>

// Pin definitions for SX1280RF1ZHP shield on Nucleo
// The SX1280RF1ZHP uses MBED Arduino shield connector pinout

// Standard MBED Arduino connector pins for SX1280 shield:
#define SX1280_NSS    D7      // CS pin
#define SX1280_DIO1   D11     // DIO1 / IRQ pin  
#define SX1280_NRST   A0      // RESET pin
#define SX1280_BUSY   D3      // BUSY pin

// If above doesn't work, try these alternative pins:
// #define SX1280_NSS    D10     // Alternative CS
// #define SX1280_DIO1   D2      // Alternative IRQ
// #define SX1280_NRST   D8      // Alternative RESET
// #define SX1280_BUSY   D9      // Alternative BUSY

// Create radio object
SX1280 radio = new Module(SX1280_NSS, SX1280_DIO1, SX1280_NRST, SX1280_BUSY);

// Test parameters (2.4 GHz LoRaWAN compatible)
const float   FREQ_MHZ  = 2440.0f;      // 2440 MHz
const float   BW_KHZ    = 203.125f;     // 203.125 kHz
const uint8_t SF        = 9;            // SF9
const uint8_t CR        = 5;            // CR 4/5
const int8_t  TX_PWR    = 10;           // 10 dBm
const uint8_t PREAMBLE  = 8;            // 8 symbols

uint32_t packetCount = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000);  // Wait max 5s for serial
  
  Serial.println(F("\n╔══════════════════════════════════════════╗"));
  Serial.println(F("║  SX1280 @ 2.4 GHz Test - Nucleo L073RZ   ║"));
  Serial.println(F("╚══════════════════════════════════════════╝"));
  Serial.println();
  
  Serial.println(F("Hardware: STM NUCLEO-L073RZ + SX1280RF1ZHP"));
  Serial.print(F("Pins: NSS=D")); Serial.print(SX1280_NSS);
  Serial.print(F(" DIO1=D")); Serial.print(SX1280_DIO1);
  Serial.print(F(" NRST=D")); Serial.print(SX1280_NRST);
  Serial.print(F(" BUSY=D")); Serial.println(SX1280_BUSY);
  Serial.println();

  // Initialize SPI (already done by RadioLib, but explicit is good)
  SPI.begin();
  
  Serial.println(F("Initializing SX1280..."));
  
  // Initialize radio
  // Using PRIVATE sync word (0x12) for testing - NOT LoRaWAN yet
  int16_t state = radio.begin(FREQ_MHZ, 
                              BW_KHZ, 
                              SF, 
                              CR, 
                              RADIOLIB_SX128X_SYNC_WORD_PRIVATE,  // 0x12 for test
                              TX_PWR, 
                              PREAMBLE);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("✓ SX1280 initialized successfully!"));
    Serial.println();
    Serial.println(F("Configuration:"));
    Serial.print(F("  Frequency:  ")); Serial.print(FREQ_MHZ); Serial.println(F(" MHz"));
    Serial.print(F("  Bandwidth:  ")); Serial.print(BW_KHZ); Serial.println(F(" kHz"));
    Serial.print(F("  SF:         ")); Serial.println(SF);
    Serial.print(F("  CR:         4/")); Serial.println(CR);
    Serial.print(F("  TX Power:   ")); Serial.print(TX_PWR); Serial.println(F(" dBm"));
    Serial.print(F("  Sync Word:  0x12 (private)"));
    Serial.println();
    Serial.println(F("Ready! Transmitting test packet every 3 seconds..."));
    Serial.println(F("──────────────────────────────────────────"));
    Serial.println();
    
  } else {
    Serial.print(F("✗ SX1280 initialization FAILED with code: "));
    Serial.println(state);
    Serial.println();
    Serial.println(F("ERROR CODES:"));
    Serial.println(F("  -2   = RADIOLIB_ERR_CHIP_NOT_FOUND (SPI issue)"));
    Serial.println(F("  -706 = RADIOLIB_ERR_INVALID_FREQUENCY"));
    Serial.println(F("  -707 = RADIOLIB_ERR_INVALID_BANDWIDTH"));
    Serial.println();
    Serial.println(F("TROUBLESHOOTING:"));
    Serial.println(F("  1. Check antenna is connected"));
    Serial.println(F("  2. Verify shield is properly seated"));
    Serial.println(F("  3. Try different pin assignments (see sketch comments)"));
    Serial.println(F("  4. Check 3.3V power supply"));
    
    while (true) {
      delay(1000);  // Halt on error
    }
  }
}

void loop() {
  // Build test packet
  packetCount++;
  char message[64];
  snprintf(message, sizeof(message), "SX1280 Test #%lu | 2.4GHz OK", packetCount);
  
  // Transmit
  Serial.print(F("[TX #")); 
  Serial.print(packetCount);
  Serial.print(F("] \""));
  Serial.print(message);
  Serial.print(F("\" ... "));
  
  // Start transmission (non-blocking)
  uint32_t txStart = millis();
  int16_t state = radio.startTransmit((uint8_t*)message, strlen(message));
  
  if (state != RADIOLIB_ERR_NONE) {
    // Failed to start transmission
    uint32_t txTime = millis() - txStart;
  } else {
    // Poll for transmission complete (IRQ-free mode)
    uint32_t timeout = millis() + 5000;  // 5 second timeout
    bool txDone = false;
    
    while (millis() < timeout) {
      uint16_t irq = radio.getIrqStatus();
      
      // Check for TX done
      if (irq & RADIOLIB_SX128X_IRQ_TX_DONE) {
        txDone = true;
        break;
      }
      
      // Check for timeout
      if (irq & RADIOLIB_SX128X_IRQ_RX_TX_TIMEOUT) {
        state = RADIOLIB_ERR_TX_TIMEOUT;
        break;
      }
      
      delay(10);
    }
    
    if (txDone) {
      state = radio.finishTransmit();
    } else if (state == RADIOLIB_ERR_NONE) {
      state = RADIOLIB_ERR_TX_TIMEOUT;
    }
  }
  
  uint32_t txTime = millis() - txStart;
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.print(F("✓ SUCCESS ("));
    Serial.print(txTime);
    Serial.println(F(" ms)"));
    
    // Show some stats every 10 packets
    if (packetCount % 10 == 0) {
      Serial.println();
      Serial.print(F("  Total packets sent: "));
      Serial.println(packetCount);
      Serial.print(F("  Average ToA: ~"));
      Serial.print(txTime);
      Serial.println(F(" ms"));
      Serial.println(F("  Radio is working correctly!"));
      Serial.println();
    }
    
  } else {
    Serial.print(F("✗ FAILED (code: "));
    Serial.print(state);
    Serial.println(F(")"));
  }
  
  // Wait 3 seconds before next packet
  delay(3000);
}
