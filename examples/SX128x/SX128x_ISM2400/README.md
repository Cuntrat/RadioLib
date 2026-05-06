# LoRaWAN ISM2400 Examples

Comprehensive examples demonstrating all major LoRaWAN communication patterns on the SX1280 radio in the 2.4 GHz ISM band.

## Overview

This directory contains 4 example scripts showing different LoRaWAN uplink/downlink communication modes:

| Script | Communication Type | Use Case |
|--------|-------------------|----------|
| **Uplink_Unconfirmed** | Unconfirmed → Server | Fire-and-forget sensors (no ACK needed) |
| **Uplink_Confirmed** | Confirmed → Server | Important data that needs delivery guarantee |
| **Downlink_Unconfirmed** | Server → Device | Receive commands without confirmation |
| **Downlink_Confirmed** | Server ↔ Device | Receive commands and send ACK automatically |

## Requirements

- **Hardware**: Arduino-compatible board with SX1280 radio
- **RadioLib**: Latest version with ISM2400 support
- **LoRaWAN Network**: TTN (The Things Network) or compatible server
- **Credentials**: 
  - JoinEUI (MSB)
  - DevEUI (MSB)
  - AppKey (16 bytes)

## Configuration

All scripts share the same hardware pin definitions and credentials. Edit these values:

```cpp
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
```

## Uplink Examples

### Uplink_Unconfirmed

**What it does:**
- Performs OTAA join once
- Sends a counter value every 10 seconds
- Does NOT expect server acknowledgement
- Prints send result to serial

**Serial Output:**
```
=== LoRaWAN ISM2400: UPLINK UNCONFIRMED ===
Mode: Send counter every second (no ACK expected)

Sending unconfirmed uplink [counter=0]... Result: 0
Sending unconfirmed uplink [counter=1]... Result: 0
```

**When to use:**
- Battery-powered IoT sensors (minimize RX window power draw)
- Non-critical telemetry (humidity, temperature, etc.)
- High-frequency data where occasional loss is acceptable

---

### Uplink_Confirmed

**What it does:**
- Performs OTAA join once
- Sends a counter value every 30 seconds
- **WAITS** for server ACK (opens RX1/RX2 windows)
- Retries if no ACK (up to default retries)
- Prints ACK status to serial

**Serial Output:**
```
=== LoRaWAN ISM2400: UPLINK CONFIRMED ===
Mode: Send counter with ACK requirement

Sending CONFIRMED uplink [counter=0]... Result: 0
  ✓ ACK received from server
```

**When to use:**
- Critical alerts that must be delivered (fire alarm, security)
- Configuration changes that need confirmation
- High-value commands
- Any data where delivery guarantee is mandatory

**Note:** Confirmed uplink uses more power (longer RX windows).

---

## Downlink Examples

### Downlink_Unconfirmed

**What it does:**
- Performs OTAA join once
- Sends minimal uplink every 30 seconds
- Listens for unconfirmed downlink in RX1/RX2 windows
- Prints any received downlink to serial
- Does NOT send ACK for received messages

**Serial Output:**
```
=== LoRaWAN ISM2400: DOWNLINK UNCONFIRMED ===
Mode: Listen for unconfirmed downlink (no ACK required)

Sending uplink [counter=0]... OK (no downlink)
Sending uplink [counter=1]... 
  ╔════════════════════════════════════════╗
  ║ ✓ DOWNLINK RECEIVED (UNCONFIRMED)     ║
  ╚════════════════════════════════════════╝
  Length: 5 bytes
  Hex: 48 65 6C 6C 6F 
  ASCII: Hello
```

**When to use:**
- One-way commands from server (e.g., on/off toggle)
- Configuration updates where acknowledgement isn't critical
- Polling for data (device initiates communication)

---

### Downlink_Confirmed

**What it does:**
- Performs OTAA join once
- Sends **confirmed** uplink every 30 seconds
- Listens for confirmed downlink in RX1/RX2 windows
- **Automatically sends ACK** for received downlink
- Prints any received downlink to serial

**Serial Output:**
```
=== LoRaWAN ISM2400: DOWNLINK CONFIRMED ===
Mode: Listen for confirmed downlink (ACK required)

Sending CONFIRMED uplink [counter=0]... OK
Sending CONFIRMED uplink [counter=1]... OK

  ╔════════════════════════════════════════╗
  ║ ✓ DOWNLINK RECEIVED (CONFIRMED)       ║
  ║   [ACK will be sent automatically]    ║
  ╚════════════════════════════════════════╝
  Length: 4 bytes
  Hex: 01 02 03 04 
  ASCII: ....
```

**When to use:**
- Critical downlink commands (firmware updates, security lockdown)
- Commands that change device state permanently
- Configuration where acknowledgement is mandatory
- Any scenario where server needs proof of delivery

**Note:** Most power-intensive (confirmed uplink + potential confirmed downlink + ACK).

---

## LoRaWAN Specification Reference

### Frame Types Used

| Type | Uplink | Downlink | Meaning |
|------|--------|----------|---------|
| UNCONFIRMED_UP | ✓ | - | Data sent, no ACK expected |
| CONFIRMED_UP | ✓ | - | Data sent, server sends ACK |
| UNCONFIRMED_DOWN | - | ✓ | Server sends data, no ACK needed |
| CONFIRMED_DOWN | - | ✓ | Server sends data, device sends ACK |

### ISM2400 Band Configuration

```
- Frequency: 2400-2480 MHz
- Default JoinReq Channels: 2403, 2425, 2479 MHz
- RX2 Downlink: 2423 MHz, DR0 (SF12, BW 812.5 kHz)
- Sync Word: 0x21 (LoRaWAN public network)
- Coding Rate: 4/8 with long interleaving
```

### Data Rate (DR) Support

All ISM2400 data rates support SF12, BW 812.5 kHz:
- DR0-DR7: SF12, spreading factor equivalents
- All operate at 812.5 kHz bandwidth (unlike sub-GHz ISM bands)

---

## Serial Monitor Setup

1. Open Arduino IDE
2. **Tools → Port** → Select your board's COM port
3. **Tools → Serial Monitor** (Ctrl+Shift+M)
4. Set baud rate to **115200**

## Nonces Persistence

All examples save/restore LoRaWAN nonces to EEPROM:
- **First run**: Device performs OTAA join, saves nonces
- **Subsequent runs**: Device restores nonces (faster startup)
- **If no nonces saved**: Device performs full OTAA join again

To force full join (reset nonces): Erase device EEPROM before upload.

## Troubleshooting

### No Serial Output
- Check baud rate is 115200
- Verify USB cable is connected
- Confirm correct COM port selected
- Check if bootloader is active (wait 5 seconds after reset)

### Join Fails (Error: -2)
- Verify JoinEUI/DevEUI/AppKey are correct (MSB format)
- Check TTN device is registered
- Verify network coverage
- Check radio initialization succeeded

### Uplink Times Out (Error: -5)
- Increase scanGuard value (see src/BuildOptUser.h)
- Verify RX window timing is correct
- Check if gateway is receiving/forwarding uplinks

### No Downlink Received
- Verify device has confirmed joined (check TTN device overview)
- Check if server has a downlink scheduled
- Verify RX window timing is compatible with gateway

---

## Example Test Flow

### Testing Uplink Unconfirmed:
```
1. Upload Uplink_Unconfirmed.ino
2. Open Serial Monitor (115200 baud)
3. Wait for "JOIN OK"
4. Watch counter increments in serial output
5. Check TTN console for received messages
```

### Testing Downlink:
```
1. Upload Downlink_Confirmed.ino
2. Open Serial Monitor (115200 baud)
3. Wait for "JOIN OK"
4. Go to TTN console → Device → Send Downlink
5. Enter message (e.g., "Hello" = 48 65 6C 6C 6F)
6. Watch serial monitor for received data
7. Check TTN console for ACK (only for Confirmed)
```

---

## Power Consumption Notes

**From lowest to highest power consumption:**

1. **Uplink Unconfirmed** (TX only)
   - Minimal RX window overhead
   - Best for battery-powered sensors

2. **Downlink Unconfirmed** (TX + short RX)
   - Minimal listen time
   - Good for occasional downlink needs

3. **Uplink Confirmed** (TX + RX windows)
   - Standard RX1/RX2 windows
   - Guarantees uplink delivery

4. **Downlink Confirmed** (TX + RX windows + TX ACK)
   - Full RX windows + retransmit for ACK
   - Most power-intensive
   - Use only for critical operations

---

## Further Reading

- LoRaWAN Specification: https://lora-alliance.org/
- TTN Documentation: https://www.thethingsnetwork.org/docs/
- RadioLib GitHub: https://github.com/jgromes/RadioLib/
- SX1280 Datasheet: Semtech official documentation

---

*These examples are part of the RadioLib ISM2400 implementation for SX1280.*
