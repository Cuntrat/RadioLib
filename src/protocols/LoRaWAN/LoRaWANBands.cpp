#include "LoRaWAN.h"

#if !RADIOLIB_EXCLUDE_LORAWAN

// array of pointers to currently supported LoRaWAN bands
const LoRaWANBand_t* LoRaWANBands[RADIOLIB_LORAWAN_NUM_SUPPORTED_BANDS] = {
  &EU868,
  &US915,
  &EU433,
  &AU915,
  &CN470,
  &AS923,
  &AS923_2,
  &AS923_3,
  &AS923_4,
  &KR920,
  &IN865,
  &ISM2_4GHz
};

const LoRaWANBand_t EU868 = {
  .bandNum = BandEU868,
  .bandType = RADIOLIB_LORAWAN_BAND_DYNAMIC,
  .freqMin = 8630000,
  .freqMax = 8700000,
  .payloadLenMax = {  51,  51,  51, 115, 242, 242, 242, 242,  50, 115,  50, 115,   0,   0,   0 },
  .powerMax = 16,
  .powerNumSteps = 7,
  .dutyCycle = 36000,
  .dwellTimeUp = 0,
  .dwellTimeDn = 0,
  .txParamSupported = false,
  .txFreqs = {
    { .idx = 0, .freq = 8681000, .drMin = 0, .drMax = 5, .dr = 3 },
    { .idx = 1, .freq = 8683000, .drMin = 0, .drMax = 5, .dr = 3 },
    { .idx = 2, .freq = 8685000, .drMin = 0, .drMax = 5, .dr = 3 },
  },
  .numTxSpans = 0,
  .txSpans = {
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE
  },
  .rx1Span = RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
  .rx1DrTable = {
    {    0,    0,    0,    0,    0,    0, 0x0F, 0x0F },
    {    1,    0,    0,    0,    0,    0, 0x0F, 0x0F },
    {    2,    1,    0,    0,    0,    0, 0x0F, 0x0F },
    {    3,    2,    1,    0,    0,    0, 0x0F, 0x0F },
    {    4,    3,    2,    1,    0,    0, 0x0F, 0x0F },
    {    5,    4,    3,    2,    1,    0, 0x0F, 0x0F },
    {    6,    5,    4,    3,    2,    1, 0x0F, 0x0F },
    {    7,    6,    5,    4,    3,    2, 0x0F, 0x0F },
    {    1,    0,    0,    0,    0,    0, 0x0F, 0x0F },
    {    2,    1,    0,    0,    0,    0, 0x0F, 0x0F },
    {    1,    0,    0,    0,    0,    0, 0x0F, 0x0F },
    {    2,    1,    0,    0,    0,    0, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F }
  },
  .rx2 = { .idx = 0, .freq = 8695250, .drMin = 0, .drMax = 7, .dr = 0 },
  .txWoR = {
    { .idx = 0, .freq = 8651000, .drMin = 3, .drMax = 3, .dr = 3 },
    { .idx = 1, .freq = 8655000, .drMin = 3, .drMax = 3, .dr = 3 }
  },
  .txAck = {
    { .idx = 0, .freq = 8653000, .drMin = 3, .drMax = 3, .dr = 3 },
    { .idx = 1, .freq = 8659000, .drMin = 3, .drMax = 3, .dr = 3 }
  },
  .dataRates = {
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {12, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {11, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 250, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_FSK,    .dr = {.fsk  = {50, 25}},     .pc = {.fsk  = {40, 24, 2}}},
    { .modem = RADIOLIB_MODEM_LRFHSS, .dr = {.lrFhss = {2, 3, 1}},  .pc = {.lrFhss = {3}}},
    { .modem = RADIOLIB_MODEM_LRFHSS, .dr = {.lrFhss = {2, 1, 1}},  .pc = {.lrFhss = {2}}},
    { .modem = RADIOLIB_MODEM_LRFHSS, .dr = {.lrFhss = {4, 3, 1}},  .pc = {.lrFhss = {3}}},
    { .modem = RADIOLIB_MODEM_LRFHSS, .dr = {.lrFhss = {4, 1, 1}},  .pc = {.lrFhss = {2}}},
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE
  }
};

const LoRaWANBand_t US915 = {
  .bandNum = BandUS915,
  .bandType = RADIOLIB_LORAWAN_BAND_FIXED,
  .freqMin = 9020000,
  .freqMax = 9280000,
  .payloadLenMax = {  11,  53, 125, 242, 242,  50, 125,   0,  53, 129, 242, 242, 242, 242,   0 },
  .powerMax = 30,
  .powerNumSteps = 14,
  .dutyCycle = 0,
  .dwellTimeUp = RADIOLIB_LORAWAN_DWELL_TIME,
  .dwellTimeDn = 0,
  .txParamSupported = false,
  .txFreqs = {
    RADIOLIB_LORAWAN_CHANNEL_NONE,
    RADIOLIB_LORAWAN_CHANNEL_NONE,
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .numTxSpans = 2,
  .txSpans = {
    {
      .numChannels = 64,
      .freqStart = 9023000,
      .freqStep = 2000,
      .drMin = 0,
      .drMax = 3,
      .drJoinRequest = 0
    },
    {
      .numChannels = 8,
      .freqStart = 9030000,
      .freqStep = 16000,
      .drMin = 4,
      .drMax = 4,
      .drJoinRequest = 4
    }
  },
  .rx1Span = {
    .numChannels = 8,
    .freqStart = 9233000,
    .freqStep = 6000,
    .drMin = 8,
    .drMax = 13,
    .drJoinRequest = RADIOLIB_LORAWAN_DATA_RATE_UNUSED 
  },
  .rx1DrTable = {
    {   10,    9,    8,    8, 0x0F, 0x0F, 0x0F, 0x0F },
    {   11,   10,    9,    8, 0x0F, 0x0F, 0x0F, 0x0F },
    {   12,   11,   10,    9, 0x0F, 0x0F, 0x0F, 0x0F },
    {   13,   12,   11,   10, 0x0F, 0x0F, 0x0F, 0x0F },
    {   13,   13,   12,   11, 0x0F, 0x0F, 0x0F, 0x0F },
    {   10,    9,    8,    8, 0x0F, 0x0F, 0x0F, 0x0F },
    {   11,   10,    9,    8, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F }
  },
  .rx2 = { .idx = 0, .freq = 9233000, .drMin = 8, .drMax = 13, .dr = 8 },
  .txWoR = {
    { .idx = 0, .freq = 9167000, .drMin = 10, .drMax = 10, .dr = 10 },
    { .idx = 1, .freq = 9199000, .drMin = 10, .drMax = 10, .dr = 10 }
  },
  .txAck = {
    { .idx = 0, .freq = 9183000, .drMin = 10, .drMax = 10, .dr = 10 },
    { .idx = 1, .freq = 9215000, .drMin = 10, .drMax = 10, .dr = 10 }
  },
  .dataRates = {
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LRFHSS, .dr = {.lrFhss = {8, 3, 0}},  .pc = {.lrFhss = {3}}},
    { .modem = RADIOLIB_MODEM_LRFHSS, .dr = {.lrFhss = {8, 1, 0}},  .pc = {.lrFhss = {2}}},
    RADIOLIB_DATARATE_NONE,
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {12, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {11, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    RADIOLIB_DATARATE_NONE
  }
};

const LoRaWANBand_t EU433 = {
  .bandNum = BandEU433,
  .bandType = RADIOLIB_LORAWAN_BAND_DYNAMIC,
  .freqMin = 4330000,
  .freqMax = 4340000,
  .payloadLenMax = {  51,  51,  51, 115, 242, 242, 242, 242,   0,   0,   0,   0,   0,   0,   0 },
  .powerMax = 12,
  .powerNumSteps = 5,
  .dutyCycle = 36000,
  .dwellTimeUp = 0,
  .dwellTimeDn = 0,
  .txParamSupported = false,
  .txFreqs = {
    { .idx = 0, .freq = 4331750, .drMin = 0, .drMax = 5, .dr = 3 },
    { .idx = 1, .freq = 4333750, .drMin = 0, .drMax = 5, .dr = 3 },
    { .idx = 2, .freq = 4335750, .drMin = 0, .drMax = 5, .dr = 3 },
  },
  .numTxSpans = 0,
  .txSpans = {
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE
  },
  .rx1Span = RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
  .rx1DrTable = {
    {    0,    0,    0,    0,    0,    0, 0x0F, 0x0F },
    {    1,    0,    0,    0,    0,    0, 0x0F, 0x0F },
    {    2,    1,    0,    0,    0,    0, 0x0F, 0x0F },
    {    3,    2,    1,    0,    0,    0, 0x0F, 0x0F },
    {    4,    3,    2,    1,    0,    0, 0x0F, 0x0F },
    {    5,    4,    3,    2,    1,    0, 0x0F, 0x0F },
    {    6,    5,    4,    3,    2,    1, 0x0F, 0x0F },
    {    7,    6,    5,    4,    3,    2, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F }
  },
  .rx2 = { .idx = 0, .freq = 4346650, .drMin = 0, .drMax = 7, .dr = 0 },
  .txWoR = {
    RADIOLIB_LORAWAN_CHANNEL_NONE,
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .txAck = {
    RADIOLIB_LORAWAN_CHANNEL_NONE,
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .dataRates = {
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {12, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {11, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 250, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_FSK,    .dr = {.fsk  = {50, 25}},     .pc = {.fsk  = {40, 24, 2}}},
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE
  }
};

const LoRaWANBand_t AU915 = {
  .bandNum = BandAU915,
  .bandType = RADIOLIB_LORAWAN_BAND_FIXED,
  .freqMin = 9150000,
  .freqMax = 9280000,
  .payloadLenMax = {  51,  51,  51, 115, 242, 242, 242,  50,  53, 129, 242, 242, 242, 242,   0 },
  .powerMax = 30,
  .powerNumSteps = 10,
  .dutyCycle = 0,
  .dwellTimeUp = RADIOLIB_LORAWAN_DWELL_TIME,
  .dwellTimeDn = 0,
  .txParamSupported = true,
  .txFreqs = {
    RADIOLIB_LORAWAN_CHANNEL_NONE,
    RADIOLIB_LORAWAN_CHANNEL_NONE,
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .numTxSpans = 2,
  .txSpans = {
    {
      .numChannels = 64,
      .freqStart = 9152000,
      .freqStep = 2000,
      .drMin = 0,
      .drMax = 5,
      .drJoinRequest = 2
    },
    {
      .numChannels = 8,
      .freqStart = 9159000,
      .freqStep = 16000,
      .drMin = 6,
      .drMax = 6,
      .drJoinRequest = 6
    }
  },
  .rx1Span = {
    .numChannels = 8,
    .freqStart = 9233000,
    .freqStep = 6000,
    .drMin = 8,
    .drMax = 13,
    .drJoinRequest = RADIOLIB_LORAWAN_DATA_RATE_UNUSED 
  },
  .rx1DrTable = {
    {    8,    8,    8,    8,    8,    8, 0x0F, 0x0F },
    {    9,    8,    8,    8,    8,    8, 0x0F, 0x0F },
    {   10,    9,    8,    8,    8,    8, 0x0F, 0x0F },
    {   11,   10,    9,    8,    8,    8, 0x0F, 0x0F },
    {   12,   11,   10,    9,    8,    8, 0x0F, 0x0F },
    {   13,   12,   11,   10,    9,    8, 0x0F, 0x0F },
    {   13,   13,   12,   11,   10,    9, 0x0F, 0x0F },
    {    9,    8,    8,    8,    8,    8, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F }
  },
  .rx2 = { .idx = 0, .freq = 9233000, .drMin = 8, .drMax = 13, .dr = 8 },
  .txWoR = {
    { .idx = 0, .freq = 9167000, .drMin = 10, .drMax = 10, .dr = 10 },
    { .idx = 1, .freq = 9199000, .drMin = 10, .drMax = 10, .dr = 10 }
  },
  .txAck = {
    { .idx = 0, .freq = 9183000, .drMin = 10, .drMax = 10, .dr = 10 },
    { .idx = 1, .freq = 9215000, .drMin = 10, .drMax = 10, .dr = 10 }
  },
  .dataRates = {
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {12, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {11, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LRFHSS, .dr = {.lrFhss = {8, 3, 0}},  .pc = {.lrFhss = {3}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {12, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {11, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 500, 5}}, .pc = {.lora = {8, false, true, false}}},
    RADIOLIB_DATARATE_NONE
  }
};

const LoRaWANBand_t CN470 = {
  .bandNum = BandCN470,
  .bandType = RADIOLIB_LORAWAN_BAND_FIXED,
  .freqMin = 4700000,
  .freqMax = 5100000,
  .payloadLenMax = {  51,  51,  51, 115, 242, 242,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
  .powerMax = 19,
  .powerNumSteps = 7,
  .dutyCycle = 0,
  .dwellTimeUp = 0,
  .dwellTimeDn = 0,
  .txParamSupported = false,
  .txFreqs = {
    RADIOLIB_LORAWAN_CHANNEL_NONE,
    RADIOLIB_LORAWAN_CHANNEL_NONE,
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .numTxSpans = 1,
  .txSpans = {
    {
      .numChannels = 96,
      .freqStart = 4703000,
      .freqStep = 2000,
      .drMin = 0,
      .drMax = 5,
      .drJoinRequest = 3
    },
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE
  },
  .rx1Span = {
    .numChannels = 48,
    .freqStart = 5003000,
    .freqStep = 2000,
    .drMin = 0,
    .drMax = 5,
    .drJoinRequest = RADIOLIB_LORAWAN_DATA_RATE_UNUSED 
  },
  .rx1DrTable = {
    {    0,    0,    0,    0,    0,    0, 0x0F, 0x0F },
    {    1,    1,    1,    1,    1,    1, 0x0F, 0x0F },
    {    2,    1,    1,    1,    1,    1, 0x0F, 0x0F },
    {    3,    2,    1,    1,    1,    1, 0x0F, 0x0F },
    {    4,    3,    2,    1,    1,    1, 0x0F, 0x0F },
    {    5,    4,    3,    2,    1,    1, 0x0F, 0x0F },
    {    6,    5,    4,    3,    2,    1, 0x0F, 0x0F },
    {    7,    6,    5,    4,    3,    2, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F }
  },
  .rx2 = { .idx = 0, .freq = 5053000, .drMin = 0, .drMax = 5, .dr = 0 },
  .txWoR = {
    RADIOLIB_LORAWAN_CHANNEL_NONE,
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .txAck = {
    RADIOLIB_LORAWAN_CHANNEL_NONE,
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .dataRates = {
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {12, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {11, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE
  }
};

const LoRaWANBand_t AS923 = {
  .bandNum = BandAS923,
  .bandType = RADIOLIB_LORAWAN_BAND_DYNAMIC,
  .freqMin = 9150000,
  .freqMax = 9280000,
  .payloadLenMax = {  51,  51, 115, 115, 242, 242, 242, 242,   0,   0,   0,   0,   0,   0,   0 },
  .powerMax = 16,
  .powerNumSteps = 7,
  .dutyCycle = 36000,
  .dwellTimeUp = RADIOLIB_LORAWAN_DWELL_TIME,
  .dwellTimeDn = RADIOLIB_LORAWAN_DWELL_TIME,
  .txParamSupported = true,
  .txFreqs = {
    { .idx = 0, .freq = 9232000, .drMin = 0, .drMax = 5, .dr = 3 },
    { .idx = 1, .freq = 9234000, .drMin = 0, .drMax = 5, .dr = 3 },
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .numTxSpans = 0,
  .txSpans = {
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE
  },
  .rx1Span = RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
  .rx1DrTable = {
    {    0,    0,    0,    0,    0,    0,    1,    2 }, // note:
    {    1,    0,    0,    0,    0,    0,    2,    3 }, // when downlinkDwellTime is one
    {    2,    1,    0,    0,    0,    0,    3,    4 }, // we should clip any value <2 to 2
    {    3,    2,    1,    0,    0,    0,    4,    5 },
    {    4,    3,    2,    1,    0,    0,    5,    6 },
    {    5,    4,    3,    2,    1,    0,    6,    7 },
    {    6,    5,    4,    3,    2,    1,    7,    7 },
    {    7,    6,    5,    4,    3,    2,    7,    7 },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F }
  },
  .rx2 = { .idx = 0, .freq = 9232000, .drMin = 0, .drMax = 7, .dr = 2 },
  .txWoR = {
    { .idx = 0, .freq = 9236000, .drMin = 3, .drMax = 3, .dr = 3 },
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .txAck = {
    { .idx = 0, .freq = 9238000, .drMin = 3, .drMax = 3, .dr = 3 },
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .dataRates = {
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {12, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {11, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 250, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_FSK,    .dr = {.fsk  = {50, 25}},     .pc = {.fsk  = {40, 24, 2}}},
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE
  }
};

const LoRaWANBand_t AS923_2 = {
  .bandNum = BandAS923_2,
  .bandType = RADIOLIB_LORAWAN_BAND_DYNAMIC,
  .freqMin = 9150000,
  .freqMax = 9280000,
  .payloadLenMax = {  51,  51, 115, 115, 242, 242, 242, 242,   0,   0,   0,   0,   0,   0,   0 },
  .powerMax = 16,
  .powerNumSteps = 7,
  .dutyCycle = 36000,
  .dwellTimeUp = RADIOLIB_LORAWAN_DWELL_TIME,
  .dwellTimeDn = RADIOLIB_LORAWAN_DWELL_TIME,
  .txParamSupported = true,
  .txFreqs = {
    { .idx = 0, .freq = 9214000, .drMin = 0, .drMax = 5, .dr = 3 },
    { .idx = 1, .freq = 9216000, .drMin = 0, .drMax = 5, .dr = 3 },
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .numTxSpans = 0,
  .txSpans = {
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE
  },
  .rx1Span = RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
  .rx1DrTable = {
    {    0,    0,    0,    0,    0,    0,    1,    2 }, // note:
    {    1,    0,    0,    0,    0,    0,    2,    3 }, // when downlinkDwellTime is one
    {    2,    1,    0,    0,    0,    0,    3,    4 }, // we should clip any value <2 to 2
    {    3,    2,    1,    0,    0,    0,    4,    5 },
    {    4,    3,    2,    1,    0,    0,    5,    6 },
    {    5,    4,    3,    2,    1,    0,    6,    7 },
    {    6,    5,    4,    3,    2,    1,    7,    7 },
    {    7,    6,    5,    4,    3,    2,    7,    7 },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F }
  },
  .rx2 = { .idx = 0, .freq = 9214000, .drMin = 0, .drMax = 7, .dr = 2 },
  .txWoR = {
    { .idx = 0, .freq = 9218000, .drMin = 3, .drMax = 3, .dr = 3 },
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .txAck = {
    { .idx = 0, .freq = 9220000, .drMin = 3, .drMax = 3, .dr = 3 },
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .dataRates = {
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {12, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {11, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 250, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_FSK,    .dr = {.fsk  = {50, 25}},     .pc = {.fsk  = {40, 24, 2}}},
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE
  }
};

const LoRaWANBand_t AS923_3 = {
  .bandNum = BandAS923_3,
  .bandType = RADIOLIB_LORAWAN_BAND_DYNAMIC,
  .freqMin = 9150000,
  .freqMax = 9280000,
  .payloadLenMax = {  51,  51, 115, 115, 242, 242, 242, 242,   0,   0,   0,   0,   0,   0,   0 },
  .powerMax = 16,
  .powerNumSteps = 7,
  .dutyCycle = 36000,
  .dwellTimeUp = RADIOLIB_LORAWAN_DWELL_TIME,
  .dwellTimeDn = RADIOLIB_LORAWAN_DWELL_TIME,
  .txParamSupported = true,
  .txFreqs = {
    { .idx = 0, .freq = 9166000, .drMin = 0, .drMax = 5, .dr = 3 },
    { .idx = 1, .freq = 9168000, .drMin = 0, .drMax = 5, .dr = 3 },
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .numTxSpans = 0,
  .txSpans = {
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE
  },
  .rx1Span = RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
  .rx1DrTable = {
    {    0,    0,    0,    0,    0,    0,    1,    2 }, // note:
    {    1,    0,    0,    0,    0,    0,    2,    3 }, // when downlinkDwellTime is one
    {    2,    1,    0,    0,    0,    0,    3,    4 }, // we should clip any value <2 to 2
    {    3,    2,    1,    0,    0,    0,    4,    5 },
    {    4,    3,    2,    1,    0,    0,    5,    6 },
    {    5,    4,    3,    2,    1,    0,    6,    7 },
    {    6,    5,    4,    3,    2,    1,    7,    7 },
    {    7,    6,    5,    4,    3,    2,    7,    7 },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F }
  },
  .rx2 = { .idx = 0, .freq = 9166000, .drMin = 0, .drMax = 7, .dr = 2 },
  .txWoR = {
    { .idx = 0, .freq = 9170000, .drMin = 3, .drMax = 3, .dr = 3 },
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .txAck = {
    { .idx = 0, .freq = 9172000, .drMin = 3, .drMax = 3, .dr = 3 },
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .dataRates = {
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {12, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {11, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 250, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_FSK,    .dr = {.fsk  = {50, 25}},     .pc = {.fsk  = {40, 24, 2}}},
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE
  }
};

const LoRaWANBand_t AS923_4 = {
  .bandNum = BandAS923_4,
  .bandType = RADIOLIB_LORAWAN_BAND_DYNAMIC,
  .freqMin = 9170000,
  .freqMax = 9200000,
  .payloadLenMax = {  51,  51, 115, 115, 242, 242, 242, 242,   0,   0,   0,   0,   0,   0,   0 },
  .powerMax = 16,
  .powerNumSteps = 7,
  .dutyCycle = 36000,
  .dwellTimeUp = RADIOLIB_LORAWAN_DWELL_TIME,
  .dwellTimeDn = RADIOLIB_LORAWAN_DWELL_TIME,
  .txParamSupported = true,
  .txFreqs = {
    { .idx = 0, .freq = 9173000, .drMin = 0, .drMax = 5, .dr = 3 },
    { .idx = 1, .freq = 9175000, .drMin = 0, .drMax = 5, .dr = 3 },
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .numTxSpans = 0,
  .txSpans = {
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE
  },
  .rx1Span = RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
  .rx1DrTable = {
    {    0,    0,    0,    0,    0,    0,    1,    2 }, // note:
    {    1,    0,    0,    0,    0,    0,    2,    3 }, // when downlinkDwellTime is one
    {    2,    1,    0,    0,    0,    0,    3,    4 }, // we should clip any value <2 to 2
    {    3,    2,    1,    0,    0,    0,    4,    5 },
    {    4,    3,    2,    1,    0,    0,    5,    6 },
    {    5,    4,    3,    2,    1,    0,    6,    7 },
    {    6,    5,    4,    3,    2,    1,    7,    7 },
    {    7,    6,    5,    4,    3,    2,    7,    7 },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F }
  },
  .rx2 = { .idx = 0, .freq = 9173000, .drMin = 0, .drMax = 7, .dr = 2 },
  .txWoR = {
    { .idx = 0, .freq = 9177000, .drMin = 3, .drMax = 3, .dr = 3 },
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .txAck = {
    { .idx = 0, .freq = 9179000, .drMin = 3, .drMax = 3, .dr = 3 },
    RADIOLIB_LORAWAN_CHANNEL_NONE
  },
  .dataRates = {
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {12, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {11, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 250, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_FSK,    .dr = {.fsk  = {50, 25}},     .pc = {.fsk  = {40, 24, 2}}},
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE
  }
};

const LoRaWANBand_t KR920 = {
  .bandNum = BandKR920,
  .bandType = RADIOLIB_LORAWAN_BAND_DYNAMIC,
  .freqMin = 9209000,
  .freqMax = 9233000,
  .payloadLenMax = {  51,  51,  51, 115, 242, 242,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
  .powerMax = 14,
  .powerNumSteps = 7,
  .dutyCycle = 0,
  .dwellTimeUp = 0,
  .dwellTimeDn = 0,
  .txParamSupported = false,
  .txFreqs = {
    { .idx = 0, .freq = 9221000, .drMin = 0, .drMax = 5, .dr = 3 },
    { .idx = 1, .freq = 9223000, .drMin = 0, .drMax = 5, .dr = 3 },
    { .idx = 2, .freq = 9225000, .drMin = 0, .drMax = 5, .dr = 3 }
  },
  .numTxSpans = 0,
  .txSpans = {
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE
  },
  .rx1Span = RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
  .rx1DrTable = {
    {    0,    0,    0,    0,    0,    0, 0x0F, 0x0F },
    {    1,    0,    0,    0,    0,    0, 0x0F, 0x0F },
    {    2,    1,    0,    0,    0,    0, 0x0F, 0x0F },
    {    3,    2,    1,    0,    0,    0, 0x0F, 0x0F },
    {    4,    3,    2,    1,    0,    0, 0x0F, 0x0F },
    {    5,    4,    3,    2,    1,    0, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F }
  },
  .rx2 = { .idx = 0, .freq = 9219000, .drMin = 0, .drMax = 5, .dr = 0 },
  .txWoR = {
    { .idx = 0, .freq = 9227000, .drMin = 3, .drMax = 3, .dr = 3 },
    { .idx = 1, .freq = 9231000, .drMin = 3, .drMax = 3, .dr = 3 }
  },
  .txAck = {
    { .idx = 0, .freq = 9229000, .drMin = 3, .drMax = 3, .dr = 3 },
    { .idx = 1, .freq = 9231000, .drMin = 3, .drMax = 3, .dr = 3 }
  },
  .dataRates = {
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {12, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {11, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE
  }
};

const LoRaWANBand_t IN865 = {
  .bandNum = BandIN865,
  .bandType = RADIOLIB_LORAWAN_BAND_DYNAMIC,
  .freqMin = 8650000,
  .freqMax = 8670000,
  .payloadLenMax = {  51,  51,  51, 115, 242, 242,   0, 242,   0,   0,   0,   0,   0,   0,   0 },
  .powerMax = 30,
  .powerNumSteps = 10,
  .dutyCycle = 0,
  .dwellTimeUp = 0,
  .dwellTimeDn = 0,
  .txParamSupported = false,
  .txFreqs = {
    { .idx = 0, .freq = 8650625, .drMin = 0, .drMax = 5, .dr = 3 },
    { .idx = 1, .freq = 8654025, .drMin = 0, .drMax = 5, .dr = 3 },
    { .idx = 2, .freq = 8659850, .drMin = 0, .drMax = 5, .dr = 3 }
  },
  .numTxSpans = 0,
  .txSpans = {
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE
  },
  .rx1Span = RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
  .rx1DrTable = {
    {    0,    0,    0,    0,    0,    0,    1,    2 },
    {    1,    0,    0,    0,    0,    0,    2,    3 },
    {    2,    1,    0,    0,    0,    0,    3,    4 },
    {    3,    2,    1,    0,    0,    0,    4,    5 },
    {    4,    3,    2,    1,    0,    0,    5,    5 },
    {    5,    4,    3,    2,    1,    0,    5,    7 },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    {    7,    6,    5,    4,    3,    2,    7,    7 },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F }
  },
  .rx2 = { .idx = 0, .freq = 8665500, .drMin = 0, .drMax = 7, .dr = 2 },
  .txWoR = {
    { .idx = 0, .freq = 8660000, .drMin = 3, .drMax = 3, .dr = 3 },
    { .idx = 1, .freq = 8667000, .drMin = 3, .drMax = 3, .dr = 3 }
  },
  .txAck = {
    { .idx = 0, .freq = 8662000, .drMin = 3, .drMax = 3, .dr = 3 },
    { .idx = 1, .freq = 8669000, .drMin = 3, .drMax = 3, .dr = 3 }
  },
  .dataRates = {
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {12, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {11, 125, 5}}, .pc = {.lora = {8, false, true, true}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = {10, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 9, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 8, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    { .modem = RADIOLIB_MODEM_LORA,   .dr = {.lora = { 7, 125, 5}}, .pc = {.lora = {8, false, true, false}}},
    RADIOLIB_DATARATE_NONE,
    { .modem = RADIOLIB_MODEM_FSK,    .dr = {.fsk  = {50, 25}},     .pc = {.fsk  = {40, 24, 2}}},
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE,
    RADIOLIB_DATARATE_NONE
  }
};

/*
 * PATCH: ISM2_4GHz band definition corrected
 * Based on:
 *   - Semtech SX1280 Datasheet (DS.SX1280-1.W.APP)
 *   - LoRa Alliance RP002-1.0.4 (ISM 2.4 GHz is a custom/proprietary band)
 *   - SX1280ED1ZHP User Guide (UG.DEV.SX1280-1.W.APP)
 */

const LoRaWANBand_t ISM2_4GHz = {
  .bandNum  = BandISM2_4GHz,
  .bandType = RADIOLIB_LORAWAN_BAND_DYNAMIC,

  /* SX1280 operating range 2400–2500 MHz; values in 100 Hz steps */
  .freqMin  = 24000000,   /* 2400.0 MHz */
  .freqMax  = 25000000,   /* 2500.0 MHz */

  /*
   * Maximum application payload per DR (N-value, excl. LoRaWAN overhead).
   * DR | SF | BW (kHz) | Max payload
   * ───┼────┼──────────┼────────────
   *  0 | 12 |  203.125 |  51 B
   *  1 | 11 |  203.125 |  51 B
   *  2 | 10 |  203.125 | 115 B
   *  3 |  9 |  203.125 | 115 B
   *  4 |  8 |  406.25  | 222 B
   *  5 |  7 |  812.5   | 222 B
   *  6 |  5 | 1625.0   | 200 B  (conservative)
   */
  .payloadLenMax = { 51, 51, 115, 115, 222, 222, 200, 0, 0, 0, 0, 0, 0, 0, 0 },

  /* 
   * SX1280 Tx power range: −18 … +12.5 dBm
   * NOTE: powerMax represents device capability, not regulatory limit.
   * For ISM 2.4 GHz there is no LoRa Alliance standard; using SX1280 hardware max.
   */
  .powerMax      = 12,   /* +12.5 dBm rounded down (SX1280 maximum) */
  .powerNumSteps = 15,   /* 15 steps of 2 dBm: 12, 10, 8, ... -16, -18 dBm */

  /* No regulatory duty cycle in the global 2.4 GHz ISM band */
  .dutyCycle   = 0,
  .dwellTimeUp = 0,
  .dwellTimeDn = 0,

  /* TxParamSetup MAC command supported (network may cap EIRP) */
  .txParamSupported = true,

  /* 
   * Three default uplink channels (dynamic band)
   * NOTE: These frequencies are NOT defined by LoRa Alliance standard.
   * Chosen to avoid WiFi congestion: 2440, 2450, 2460 MHz (between WiFi channels).
   * Adjust based on local regulations and gateway configuration.
   */
  .txFreqs = {
    { .idx = 0, .freq = 24400000, .drMin = 0, .drMax = 6, .dr = 3 }, /* 2440.0 MHz */
    { .idx = 1, .freq = 24500000, .drMin = 0, .drMax = 6, .dr = 3 }, /* 2450.0 MHz */
    { .idx = 2, .freq = 24600000, .drMin = 0, .drMax = 6, .dr = 3 }, /* 2460.0 MHz */
  },

  .numTxSpans = 0,
  .txSpans = {
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,
    RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE
  },
  .rx1Span = RADIOLIB_LORAWAN_CHANNEL_SPAN_NONE,

  .rx1DrTable = {
    /* DR0  */ {    0,    0,    0,    0,    0,    0,    0,    0 },
    /* DR1  */ {    1,    0,    0,    0,    0,    0,    0,    0 },
    /* DR2  */ {    2,    1,    0,    0,    0,    0,    0,    0 },
    /* DR3  */ {    3,    2,    1,    0,    0,    0,    0,    0 },
    /* DR4  */ {    4,    3,    2,    1,    0,    0,    0,    0 },
    /* DR5  */ {    5,    4,    3,    2,    1,    0,    0,    0 },
    /* DR6  */ {    6,    5,    4,    3,    2,    1,    0,    0 },
    /* DR7  */ { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    /* DR8  */ { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    /* DR9  */ { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    /* DR10 */ { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    /* DR11 */ { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    /* DR12 */ { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    /* DR13 */ { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
    /* DR14 */ { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },
  },

  /* Rx2 window: 2470.0 MHz at DR2 (SF10 / BW 203.125 kHz) */
  .rx2 = { .idx = 0, .freq = 24700000, .drMin = 0, .drMax = 6, .dr = 2 },

  /* WoR / ACK relay channels (not part of standard LoRaWAN; kept for
   * structural completeness — set equal to mid-band channels) */
  .txWoR = {
    { .idx = 0, .freq = 24450000, .drMin = 3, .drMax = 3, .dr = 3 },
    { .idx = 1, .freq = 24550000, .drMin = 3, .drMax = 3, .dr = 3 }
  },
  .txAck = {
    { .idx = 0, .freq = 24450000, .drMin = 3, .drMax = 3, .dr = 3 },
    { .idx = 1, .freq = 24550000, .drMin = 3, .drMax = 3, .dr = 3 }
  },

  /*
   * Data rates for SX1280 LoRa @ 2.4 GHz
   * ─────────────────────────────────────────────────────────────
   * DR | SF |    BW (kHz) | Approx. raw bitrate | Rx sensitivity
   * ───┼────┼─────────────┼─────────────────────┼────────────────
   *  0 | 12 |   203.125   |    ~293 bps         |  −130 dBm
   *  1 | 11 |   203.125   |    ~537 bps         |  −127 dBm
   *  2 | 10 |   203.125   |    ~977 bps         |  −124 dBm
   *  3 |  9 |   203.125   |   ~1758 bps         |  −121 dBm
   *  4 |  8 |   406.25    |   ~6250 bps         |  −116 dBm
   *  5 |  7 |   812.5     |  ~21875 bps         |  −112 dBm
   *  6 |  5 |  1625.0     |  ~202 kbps          |  − 99 dBm
   * ──────────────────────────────────────────────────────────────
   */
  .dataRates = {
    /* DR0 */ { .modem = RADIOLIB_MODEM_LORA,
                .dr = {.lora = {12, 203.125f, 5}},
                .pc = {.lora = { 8, false, true, true  }} },
    /* DR1 */ { .modem = RADIOLIB_MODEM_LORA,
                .dr = {.lora = {11, 203.125f, 5}},
                .pc = {.lora = { 8, false, true, true  }} },
    /* DR2 */ { .modem = RADIOLIB_MODEM_LORA,
                .dr = {.lora = {10, 203.125f, 5}},
                .pc = {.lora = { 8, false, true, false }} },
    /* DR3 */ { .modem = RADIOLIB_MODEM_LORA,
                .dr = {.lora = { 9, 203.125f, 5}},
                .pc = {.lora = { 8, false, true, false }} },
    /* DR4 */ { .modem = RADIOLIB_MODEM_LORA,
                .dr = {.lora = { 8, 406.25f,  5}},
                .pc = {.lora = { 8, false, true, false }} },
    /* DR5 */ { .modem = RADIOLIB_MODEM_LORA,
                .dr = {.lora = { 7, 812.5f,   5}},
                .pc = {.lora = { 8, false, true, false }} },
    /* DR6 */ { .modem = RADIOLIB_MODEM_LORA,
                .dr = {.lora = { 5, 1625.0f,  5}},
                .pc = {.lora = { 8, false, true, false }} },
    RADIOLIB_DATARATE_NONE,  /* DR7  – unused */
    RADIOLIB_DATARATE_NONE,  /* DR8  – unused */
    RADIOLIB_DATARATE_NONE,  /* DR9  – unused */
    RADIOLIB_DATARATE_NONE,  /* DR10 – unused */
    RADIOLIB_DATARATE_NONE,  /* DR11 – unused */
    RADIOLIB_DATARATE_NONE,  /* DR12 – unused */
    RADIOLIB_DATARATE_NONE,  /* DR13 – unused */
    RADIOLIB_DATARATE_NONE,  /* DR14 – unused */
  }
};

#endif
