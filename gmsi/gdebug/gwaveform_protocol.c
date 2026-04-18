#include "gwaveform_protocol.h"
#include <string.h>

#define SYNC_H              0xAA
#define SYNC_L              0x55
#define FRAME_TYPE_DATA     0x00
#define FRAME_TYPE_DESC     0xFD

static uint8_t calc_crc8(const uint8_t *pchData, uint16_t hwLen)
{
    uint8_t chCRC = 0xFF;
    for (uint16_t i = 0; i < hwLen; i++) {
        chCRC ^= pchData[i];
    }
    return chCRC;
}

static uint16_t default_pack_data(uint8_t *pchBuffer, const int16_t *ahwSamples, const uint8_t *abMask, uint8_t chCount, uint8_t chSeq)
{
    uint16_t hwIdx = 0;
    uint8_t chMaskBytes = (chCount + 7) / 8;
    if(chMaskBytes == 0) chMaskBytes = 1; /* At least 1 byte mask */

    pchBuffer[hwIdx++] = SYNC_H;
    pchBuffer[hwIdx++] = SYNC_L;
    pchBuffer[hwIdx++] = chSeq;
    
    memcpy(&pchBuffer[hwIdx], abMask, chMaskBytes);
    hwIdx += chMaskBytes;
    
    for (uint8_t i = 0; i < chCount; i++) {
        if (abMask[i / 8] & (1u << (i % 8))) {
            pchBuffer[hwIdx++] = (uint8_t)(ahwSamples[i] & 0xFF);
            pchBuffer[hwIdx++] = (uint8_t)((ahwSamples[i] >> 8) & 0xFF);
        }
    }
    
    pchBuffer[hwIdx] = calc_crc8(&pchBuffer[2], hwIdx - 2);
    hwIdx++;
    
    return hwIdx;
}

static uint16_t default_pack_desc(uint8_t *pchBuffer, const gwaveform_ch_desc_t *atChannels, uint8_t chCount)
{
    uint16_t hwIdx = 0;

    pchBuffer[hwIdx++] = SYNC_H;
    pchBuffer[hwIdx++] = SYNC_L;
    pchBuffer[hwIdx++] = FRAME_TYPE_DESC;
    pchBuffer[hwIdx++] = chCount;

    for (uint8_t i = 0; i < chCount; i++) {
        memcpy(&pchBuffer[hwIdx], atChannels[i].achName, 8);
        hwIdx += 8;
        memcpy(&pchBuffer[hwIdx], &atChannels[i].fScale, 4);
        hwIdx += 4;
    }

    pchBuffer[hwIdx] = calc_crc8(&pchBuffer[2], hwIdx - 2);
    hwIdx++;

    return hwIdx;
}

const gwaveform_protocol_t default_waveform_protocol = {
    .pack_data = default_pack_data,
    .pack_desc = default_pack_desc,
};
