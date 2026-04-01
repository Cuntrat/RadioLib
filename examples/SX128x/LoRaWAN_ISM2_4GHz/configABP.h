/*
 * configABP.h – ABP credentials for LoRaWAN 2.4 GHz test
 *
 * Copy this file into the same folder as LoRaWAN_2_4GHz_node.ino
 * and replace the values with your own device credentials.
 *
 * For TTN / ChirpStack:
 *   1. Create a new device with LoRaWAN 1.0.4
 *   2. Select ABP activation
 *   3. Copy DevAddr, NwkSKey, AppSKey below
 *   4. Set the network server to accept 2.4 GHz traffic (custom band)
 */

#pragma once

// 4-byte device address (MSB first)
#define LORAWAN_DEV_ADDR    0x01234567UL

// 16-byte network session encryption key
// (also used as fNwkSIntKey and sNwkSIntKey in LoRaWAN 1.0.4)
#define LORAWAN_NWK_S_KEY   { \
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, \
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C  \
}

// 16-byte application session key
#define LORAWAN_APP_S_KEY   { \
  0xA8, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, \
  0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD  \
}

// Uplink fPort (1–223)
#define LORAWAN_FPORT       1

// Uplink interval in milliseconds (min 1000 to avoid issues)
#define LORAWAN_UPLINK_INTERVAL_MS   30000UL
