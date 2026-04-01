/*
 * LoRaWAN_2_4GHz_test.ino
 * ─────────────────────────────────────────────────────────────────────────────
 * LoRaWAN 2.4 GHz test sketch — Arduino MKR WAN 1310 + SX1280 breakout
 *
 * WHAT IT DOES
 * ────────────
 *  1. Initialises the SX1280 radio (SPI).
 *  2. Performs an OTAA join on the ISM 2.4 GHz LoRaWAN band.
 *     On failure it retries with exponential back-off (max 5 min).
 *  3. Every UPLINK_PERIOD_MS it sends a 6-byte payload:
 *       [counter_hi, counter_lo, temp_hi, temp_lo, vbatt_hi, vbatt_lo]
 *  4. It reads any downlink received in the Rx1/Rx2 windows and prints it.
 *  5. The uplink DataRate starts at DR3 (SF9/203 kHz); ADR is enabled so
 *     the server can optimise it automatically.
 *
 * PREREQUISITES
 * ─────────────
 *  - RadioLib ≥ 7.x  with the two bug-fixes applied:
 *      • ISM2_4GHz_fixed.cpp    (correct rx1DrTable)
 *      • SX128x_setSyncWord_fixed.cpp  (LoRa sync-word routing)
 *  - LoRaWAN-2.4 GHz-capable LNS (e.g. Chirpstack with custom band)
 *    configured with the same default channels and Rx2 parameters.
 *
 * WIRING — Arduino MKR WAN 1310 + SX1280 breakout
 * ─────────────────────────────────────────────────
 *  MKR WAN 1310          SX1280 breakout
 *  ─────────────────────────────────────
 *  3.3V              →   VDD / VBAT_IO
 *  GND               →   GND
 *  COPI (PA16 / MOSI)→   MOSI
 *  CIPO (PA19 / MISO)→   MISO
 *  SCK  (PA17)       →   SCK
 *  D7   (PA20)       →   NSS / CS      ← SX1280_NSS
 *  D6   (PA19/INT3)  →   DIO1 / IRQ    ← SX1280_DIO1  (interrupt-capable)
 *  D5   (PA06)       →   NRST          ← SX1280_NRST
 *  D4   (PA07)       →   BUSY          ← SX1280_BUSY
 *
 *  Note: D6 on MKR WAN 1310 = PA19 = external interrupt INT[3]. ✓
 *
 * LORAWAN CREDENTIALS
 * ───────────────────
 *  Replace the placeholder values in the "Credentials" section below
 *  with the real keys from your LNS device registration.
 *  The sketch uses LoRaWAN 1.1 (both AppKey and NwkKey).
 *  For LoRaWAN 1.0 pass NULL as nwkKey in beginOTAA().
 *
 * SERIAL OUTPUT
 * ─────────────
 *  Open the Serial Monitor at 115200 baud.
 *  All events are logged with a timestamp in milliseconds.
 * ─────────────────────────────────────────────────────────────────────────────
 */

#include <RadioLib.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * CONFIGURATION — edit these values
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ── Pin assignment (MKR WAN 1310 + SX1280 breakout) ── */
#define SX1280_NSS    7    /* D7  — chip-select                */
#define SX1280_DIO1   6    /* D6  — interrupt (must be an INT pin)  */
#define SX1280_NRST   5    /* D5  — reset                      */
#define SX1280_BUSY   4    /* D4  — busy                       */

/* ── LoRaWAN credentials — REPLACE WITH YOUR OWN ── */
static const uint64_t JOIN_EUI  = 0x0000000000000000ULL;
static const uint64_t DEV_EUI   = 0x0000000000000000ULL;

/* LoRaWAN 1.1: AppKey used for session keys, NwkKey for MIC / integrity */
static const uint8_t APP_KEY[16] = {
  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00
};
static const uint8_t NWK_KEY[16] = {
  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00
};

/* ── Application settings ── */
static const uint8_t  FPORT              = 1;       /* Application fPort        */
static const uint32_t UPLINK_PERIOD_MS   = 60000UL; /* 60 s between uplinks     */
static const uint8_t  INITIAL_DATARATE   = 3;       /* DR3 = SF9 / BW 203.125   */

/* ═══════════════════════════════════════════════════════════════════════════ */

/* Radio and LoRaWAN node objects */
SX1280 radio = new Module(SX1280_NSS, SX1280_DIO1, SX1280_NRST, SX1280_BUSY);
LoRaWANNode node(&radio, &ISM2_4GHz);

/* ── Utility: print RadioLib status code ── */
static void printState(const char* label, int16_t state) {
  Serial.print(F("  ["));
  Serial.print(label);
  Serial.print(F("] "));
  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("OK"));
  } else {
    Serial.print(F("FAILED  code = "));
    Serial.println(state);
  }
}

/* ── Utility: print a hex buffer ── */
static void printHex(const uint8_t* buf, size_t len) {
  for(size_t i = 0; i < len; i++) {
    if(buf[i] < 0x10) Serial.print('0');
    Serial.print(buf[i], HEX);
    if(i < len - 1) Serial.print(' ');
  }
  Serial.println();
}

/* ── Utility: timestamp prefix ── */
static void ts() {
  Serial.print('[');
  Serial.print(millis());
  Serial.print(F("] "));
}

/* ─────────────────────────────────────────────────────────────────────────── */
void setup() {
  Serial.begin(115200);
  while(!Serial && millis() < 5000);   /* wait up to 5 s for USB-CDC */
  Serial.println();
  Serial.println(F("╔══════════════════════════════════════════╗"));
  Serial.println(F("║  LoRaWAN 2.4 GHz Test — SX1280           ║"));
  Serial.println(F("║  RadioLib + ISM2_4GHz band               ║"));
  Serial.println(F("╚══════════════════════════════════════════╝"));
  Serial.println();

  /* ── 1. Initialise the SX1280 ── */
  ts(); Serial.println(F("Initialising SX1280 ..."));
  int16_t state = radio.begin();
  printState("radio.begin", state);
  if(state != RADIOLIB_ERR_NONE) {
    Serial.println(F("FATAL: check wiring. Halting."));
    while(true) delay(100);
  }

  /* ── 2. Configure LoRaWAN credentials ── */
  ts(); Serial.println(F("Setting LoRaWAN credentials (OTAA 1.1) ..."));
  state = node.beginOTAA(JOIN_EUI, DEV_EUI, NWK_KEY, APP_KEY);
  printState("node.beginOTAA", state);
  if(state != RADIOLIB_ERR_NONE) {
    Serial.println(F("FATAL: invalid credentials. Halting."));
    while(true) delay(100);
  }

  /* ── 3. Configure MAC behaviour ── */
  node.setADR(true);               /* ADR enabled — server controls DR    */
  node.setDutyCycle(false);        /* no duty-cycle limit at 2.4 GHz ISM  */
  node.setDatarate(INITIAL_DATARATE);

  /* ── 4. OTAA join with exponential back-off ── */
  ts(); Serial.println(F("Attempting OTAA join ..."));
  uint32_t backoff = 10000UL;      /* start at 10 s, cap at 5 min         */
  while(true) {
    LoRaWANJoinEvent_t joinEvt;
    state = node.activateOTAA(&joinEvt);

    if(state == RADIOLIB_LORAWAN_NEW_SESSION ||
       state == RADIOLIB_LORAWAN_SESSION_RESTORED) {
      ts();
      Serial.print(F("Joined! DevNonce="));
      Serial.print(joinEvt.devNonce);
      Serial.print(F("  JoinNonce="));
      Serial.print(joinEvt.joinNonce);
      Serial.println(state == RADIOLIB_LORAWAN_SESSION_RESTORED ?
                     F("  (session restored)") : F("  (new session)"));
      break;
    }

    ts();
    Serial.print(F("Join failed (code "));
    Serial.print(state);
    Serial.print(F("), retrying in "));
    Serial.print(backoff / 1000);
    Serial.println(F(" s ..."));
    delay(backoff);
    backoff = min(backoff * 2, 300000UL);   /* cap at 5 min */
  }

  ts(); Serial.println(F("Ready.\n"));
}

/* ─────────────────────────────────────────────────────────────────────────── */
void loop() {
  static uint32_t lastUplink = 0;
  static uint16_t counter    = 0;

  /* wait for the uplink interval */
  if(millis() - lastUplink < UPLINK_PERIOD_MS && lastUplink != 0) {
    delay(100);
    return;
  }
  lastUplink = millis();

  /* ── Build a small demo payload ──────────────────────────────────────── */
  /*   Bytes 0–1 : 16-bit uplink counter
       Bytes 2–3 : mock temperature × 10  (e.g. 240 = 24.0 °C)
       Bytes 4–5 : mock battery voltage × 100  (e.g. 330 = 3.30 V)       */
  uint16_t temperature = 240;   /* replace with a real sensor read */
  uint16_t battMv      = 330;   /* replace with analogRead(VBATT)/... */

  uint8_t upBuf[6];
  upBuf[0] = (counter >> 8) & 0xFF;
  upBuf[1] =  counter       & 0xFF;
  upBuf[2] = (temperature >> 8) & 0xFF;
  upBuf[3] =  temperature       & 0xFF;
  upBuf[4] = (battMv >> 8) & 0xFF;
  upBuf[5] =  battMv       & 0xFF;
  counter++;

  /* ── Uplink + receive ────────────────────────────────────────────────── */
  uint8_t  downBuf[RADIOLIB_LORAWAN_MAX_PAYLOAD_SIZE + 1];
  size_t   downLen = 0;
  LoRaWANEvent_t upEvt, downEvt;

  ts();
  Serial.print(F("Uplink #"));
  Serial.print(counter - 1);
  Serial.print(F("  fPort="));
  Serial.print(FPORT);
  Serial.print(F("  payload: "));
  printHex(upBuf, sizeof(upBuf));

  int16_t state = node.sendReceive(
      upBuf, sizeof(upBuf), FPORT,
      downBuf, &downLen,
      /*isConfirmed=*/false,
      &upEvt, &downEvt);

  /* ── Report uplink event ─────────────────────────────────────────────── */
  ts();
  Serial.print(F("  TX  DR="));
  Serial.print(upEvt.datarate);
  Serial.print(F("  freq="));
  Serial.print(upEvt.freq, 4);
  Serial.print(F(" MHz  pwr="));
  Serial.print(upEvt.power);
  Serial.print(F(" dBm  fCnt="));
  Serial.println(upEvt.fCnt);

  /* ── Report downlink / errors ────────────────────────────────────────── */
  if(state > 0) {
    /* state = Rx window number (1 = Rx1, 2 = Rx2) */
    ts();
    Serial.print(F("  RX  window=Rx"));
    Serial.print(state);
    Serial.print(F("  DR="));
    Serial.print(downEvt.datarate);
    Serial.print(F("  freq="));
    Serial.print(downEvt.freq, 4);
    Serial.print(F(" MHz  fCnt="));
    Serial.println(downEvt.fCnt);

    if(downLen > 0) {
      ts();
      Serial.print(F("  Downlink  fPort="));
      Serial.print(downEvt.fPort);
      Serial.print(F("  len="));
      Serial.print(downLen);
      Serial.print(F("  data: "));
      printHex(downBuf, downLen);

      /* ── Application-level downlink handling ── */
      handleDownlink(downEvt.fPort, downBuf, downLen);

    } else {
      ts(); Serial.println(F("  Downlink: no application payload (MAC only)."));
    }

    if(downEvt.frmPending) {
      ts(); Serial.println(F("  NOTE: server has more frames pending."));
    }

  } else if(state == 0) {
    ts(); Serial.println(F("  No downlink received."));

  } else {
    /* Negative state = hard error */
    ts();
    Serial.print(F("  ERROR code = "));
    Serial.println(state);

    if(state == RADIOLIB_ERR_NETWORK_NOT_JOINED) {
      ts(); Serial.println(F("  Session lost — attempting re-join ..."));
      uint32_t backoff = 10000UL;
      while(true) {
        LoRaWANJoinEvent_t joinEvt;
        int16_t jState = node.activateOTAA(&joinEvt);
        if(jState == RADIOLIB_LORAWAN_NEW_SESSION ||
           jState == RADIOLIB_LORAWAN_SESSION_RESTORED) {
          ts(); Serial.println(F("  Re-joined."));
          break;
        }
        delay(backoff);
        backoff = min(backoff * 2, 300000UL);
      }
    }
  }

  Serial.println();
}

/* ─────────────────────────────────────────────────────────────────────────── */
/* handleDownlink — called for every application downlink received.
 * Add your own logic here (e.g. LED toggle, config update, OTA trigger).    */
void handleDownlink(uint8_t fPort, const uint8_t* data, size_t len) {
  switch(fPort) {
    case 1:
      /* Example: if byte[0] == 0xFF → blink LED */
      if(len >= 1 && data[0] == 0xFF) {
        ts(); Serial.println(F("  CMD: blink"));
        pinMode(LED_BUILTIN, OUTPUT);
        for(int i = 0; i < 6; i++) {
          digitalWrite(LED_BUILTIN, i % 2);
          delay(200);
        }
      }
      break;

    case 2:
      /* Example: update uplink interval (2 bytes, big-endian, seconds) */
      if(len >= 2) {
        uint16_t newPeriodS = ((uint16_t)data[0] << 8) | data[1];
        ts();
        Serial.print(F("  CMD: new uplink period = "));
        Serial.print(newPeriodS);
        Serial.println(F(" s (not applied — demo only)"));
      }
      break;

    default:
      ts();
      Serial.print(F("  Unknown fPort "));
      Serial.println(fPort);
      break;
  }
}
