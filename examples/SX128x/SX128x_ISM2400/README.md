# LoRaWAN ISM2400 Examples (SX1280)

Four examples covering every uplink/downlink confirmation mode for the
SX1280 2.4 GHz LoRaWAN implementation in RadioLib.

## Overview

| Sketch | TX type | RX type | Typical use |
|---|---|---|---|
| **Uplink_Unconfirmed** | Unconfirmed UP | opportunistic | Sensors, telemetry |
| **Uplink_Confirmed** | Confirmed UP | ACK + optional DL | Critical data delivery |
| **Downlink_Unconfirmed** | Unconfirmed UP | Unconfirmed DOWN | Server commands |
| **Downlink_Confirmed** | Confirmed UP | Confirmed DOWN + ACK | Critical server commands |

## Requirements

- Arduino-compatible board with SX1280 (2.4 GHz LoRa radio)
- RadioLib (this library)
- A LoRaWAN network server (e.g. TTN / The Things Stack)
- A registered OTAA device with JoinEUI, DevEUI, AppKey

## Hardware configuration

Edit the pin definitions at the top of each sketch to match your board:

```cpp
#define SX1280_NSS   D7   // SPI chip-select
#define SX1280_DIO1  D5   // IRQ / DIO1
#define SX1280_NRST  A0   // reset
#define SX1280_BUSY  D3   // busy indicator
```

## LoRaWAN credentials

Replace the placeholder values with the ones shown in your TTN device
overview (**MSB** byte order, as displayed by TTN):

```cpp
uint64_t joinEUI = 0xA001C5DC0FA37C02ULL;
uint64_t devEUI  = 0x6B7107D07ED5B370ULL;

uint8_t appKey[16] = { 0xD5, 0xA8, ... };
```

---

## LoRaWAN Class A — receive window fundamentals

> **Important:** All four sketches must transmit an uplink before the
> device can receive anything.  This is a hard requirement of the
> LoRaWAN Class A specification (the only class supported by the
> SX1280 in single-channel mode).

After every uplink the radio opens two short receive windows:

| Window | Opens after TX ends | Default frequency (ISM2400) | DR |
|---|---|---|---|
| RX1 | +1 s | same as TX channel | TX DR (DR0) |
| RX2 | +2 s | 2423.0 MHz | DR0 (SF12) |

If the network server has a downlink frame queued it will transmit it
in RX1 (preferred) or RX2.  If no frame is queued, both windows time
out and `sendReceive()` returns `0`.

---

## `sendReceive()` return values

```
< 0   hard error (see RADIOLIB_ERR_* codes in TypeDef.h)
= 0   TX succeeded; no downlink received in either window
= 1   TX succeeded; downlink received in RX1
= 2   TX succeeded; downlink received in RX2
```

For **confirmed uplink** (`isConfirmed = true`):

- `st = 0` → ACK was **not** received (both windows timed out).
  RadioLib still increments the frame counter.
- `st = 1` or `st = 2` → a downlink frame arrived.  Check
  `eventDown.confirming` to know whether it carries the ACK bit for
  your uplink.

---

## Uplink examples

### Uplink_Unconfirmed

Sends a 4-byte counter every 30 seconds without requesting an
acknowledgement (`isConfirmed = false`).  The server does not send an
ACK, but any queued downlink is still delivered in the RX windows.

Expected serial output (no queued downlink):
```
[UL-UNCONF] counter=0 ... OK (no downlink)
[UL-UNCONF] counter=1 ... OK (no downlink)
```

Expected serial output (downlink queued on TTN):
```
[UL-UNCONF] counter=2 ... OK + downlink in RX1 (5 bytes)
  HEX: 48 65 6C 6C 6F
```

### Uplink_Confirmed

Sends a 4-byte counter every 30 seconds and requests an ACK from the
server (`isConfirmed = true`).  The ACK arrives as a downlink frame in
RX1 or RX2 with no application payload (`eventDown.confirming = true`).

Expected serial output (ACK received):
```
[UL-CONF] counter=0 -> ACK received in RX1
[UL-CONF] counter=1 -> ACK received in RX1
```

Expected serial output (ACK not received):
```
[UL-CONF] counter=0 -> NO ACK (both windows timed out)
```

---

## Downlink examples

### How to schedule a downlink on TTN

1. Open **TTN Console → your application → your device**.
2. Click **Messaging → Downlink**.
3. Enter the payload in hex (e.g. `01 02 03`) and optionally enable
   **Confirmed downlink**.
4. Click **Schedule downlink** — TTN queues the frame.
5. The frame is delivered the next time the device opens RX windows,
   i.e. after the next `sendReceive()` call.

### Downlink_Unconfirmed

Sends a minimal unconfirmed uplink every 30 seconds to keep the RX
windows open.  Any unconfirmed (or confirmed) downlink queued on TTN
will be received and printed.

Expected serial output (downlink arrives):
```
[UL] counter=3 -> OK
  +----------------------------------+
  | DOWNLINK RECEIVED in RX1         |
  +----------------------------------+
  Length : 5 bytes
  HEX    : 48 65 6C 6C 6F
  ASCII  : Hello
```

### Downlink_Confirmed

Sends a confirmed uplink every 30 seconds.  Any confirmed downlink
queued on TTN is received; RadioLib **automatically** piggybacks the
ACK on the next uplink — no user code required.

`LoRaWANEvent_t` fields you can inspect:

| Field | Meaning |
|---|---|
| `evDown.confirmed` | `true` → this downlink is confirmed (server wants ACK) |
| `evDown.confirming` | `true` → this downlink carries ACK for our confirmed uplink |
| `evDown.frmPending` | `true` → server has more frames queued |

Expected serial output:
```
[UL-CONF] counter=4 -> OK
  +------------------------------------------+
  | DOWNLINK RECEIVED in RX1 [CONFIRMED - ACK will be piggybacked on next UL] |
  +------------------------------------------+
  fPort  : 1
  Length : 4 bytes
  HEX    : 01 02 03 04
  ASCII  : ....
```

---

## Nonces persistence

All sketches save the LoRaWAN nonces buffer to EEPROM after every join
attempt.  This ensures that `devNonce` is never reused across resets,
which is a hard requirement of LoRaWAN 1.0.x.

- **First boot**: fresh OTAA join; nonces saved.
- **Subsequent boots**: nonces restored; join may re-use the existing
  session (`RADIOLIB_LORAWAN_SESSION_RESTORED`) or issue a new one.
- **Force fresh join**: erase EEPROM before flashing.

---

## Data rate

All sketches use DR0 (SF12, BW 812.5 kHz, CR 4/8) — the most robust
and range-maximising option for ISM2400.  Change `node.setDatarate()`
to a higher DR (up to DR7 = SF5) for faster transmission and lower
time-on-air.

---

## Further reading

- LoRaWAN Specification 1.0.4: https://lora-alliance.org/
- LoRaWAN Regional Parameters RP002 (ISM2400): https://lora-alliance.org/
- The Things Network documentation: https://www.thethingsindustries.com/docs/
- RadioLib: https://github.com/jgromes/RadioLib/
