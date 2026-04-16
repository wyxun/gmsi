#include "gwaveform.h"
#include "gringbuf.h"
#include "segger_rtt/SEGGER_RTT.h"
#include "utilities/util_debug.h"
#include <string.h>

#if GWAVEFORM_ENABLE

/*============================ MACROS ========================================*/
#define SYNC_H              0xAA
#define SYNC_L              0x55
#define FRAME_TYPE_DATA     0x00
#define FRAME_TYPE_DESC     0xFD

#define MASK_BYTES          ((GWAVEFORM_MAX_CHANNELS + 7) / 8)

/*============================ TYPES =========================================*/

typedef struct {
    char    achName[8];
    float   fScale;
} gwaveform_ch_desc_t;

typedef struct {
    gringbuf_t          tRB;
    uint8_t             achRBBuffer[GWAVEFORM_RING_BUFFER_SIZE];
    uint8_t             achRTTBuffer[GWAVEFORM_RTT_BUFFER_SIZE];
    
    gwaveform_ch_desc_t atChannels[GWAVEFORM_MAX_CHANNELS];
    int16_t             ahwSamples[GWAVEFORM_MAX_CHANNELS];
    uint8_t             abMask[MASK_BYTES];
    
    uint8_t             chCount;
    uint8_t             chSeq;
    uint8_t             chDecimCounter;
    uint8_t             chDecimation;
    bool                bIsRunning;
    bool                bRequestDesc;
} gwaveform_cb_t;

/*============================ PRIVATE DATA ==================================*/
static gwaveform_cb_t s_tWave;

/*============================ PRIVATE HELPERS ===============================*/

static uint8_t calc_crc8(uint8_t *pchData, uint16_t hwLen)
{
    uint8_t chCRC = 0xFF;
    for (uint16_t i = 0; i < hwLen; i++) {
        chCRC ^= pchData[i];
    }
    return chCRC;
}

static void send_descriptor_frame(void)
{
    uint8_t achBuf[4 + 12 * GWAVEFORM_MAX_CHANNELS + 1];
    uint16_t hwIdx = 0;

    achBuf[hwIdx++] = SYNC_H;
    achBuf[hwIdx++] = SYNC_L;
    achBuf[hwIdx++] = FRAME_TYPE_DESC;
    achBuf[hwIdx++] = s_tWave.chCount;

    for (uint8_t i = 0; i < s_tWave.chCount; i++) {
        memcpy(&achBuf[hwIdx], s_tWave.atChannels[i].achName, 8);
        hwIdx += 8;
        memcpy(&achBuf[hwIdx], &s_tWave.atChannels[i].fScale, 4);
        hwIdx += 4;
    }

    achBuf[hwIdx] = calc_crc8(&achBuf[2], hwIdx - 2);
    hwIdx++;

    SEGGER_RTT_Write(GWAVEFORM_RTT_CHANNEL, achBuf, hwIdx);
}

/*============================ PUBLIC API ====================================*/

int gwaveform_Init(uintptr_t wAddr, uintptr_t wUnused)
{
    (void)wAddr; (void)wUnused;
    
    memset(&s_tWave, 0, sizeof(s_tWave));
    gringbuf_Init(&s_tWave.tRB, s_tWave.achRBBuffer, GWAVEFORM_RING_BUFFER_SIZE);
    
    SEGGER_RTT_ConfigUpBuffer(GWAVEFORM_RTT_CHANNEL, "Waveform", 
                             s_tWave.achRTTBuffer, GWAVEFORM_RTT_BUFFER_SIZE, 
                             SEGGER_RTT_MODE_NO_BLOCK_SKIP);
                             
    s_tWave.chDecimation = GWAVEFORM_DECIMATION;
    
    return GMSI_SUCCESS;
}

/* Auto-init registration */
INIT_SECTION const gmsi_init_info_t gwaveform_init_info = {
    .pfcnInitFunc = (init_func_t)gwaveform_Init,
    .wObjectAddr = 0,
    .wConfigAddr = 0
};

uint8_t gwaveform_AddChannel(const char *pchName, float fScale)
{
    if (s_tWave.chCount >= GWAVEFORM_MAX_CHANNELS) return 0xFF;
    
    uint8_t chID = s_tWave.chCount++;
    strncpy(s_tWave.atChannels[chID].achName, pchName, 7);
    s_tWave.atChannels[chID].achName[7] = '\0';
    s_tWave.atChannels[chID].fScale = fScale;
    
    return chID;
}

void gwaveform_Start(void)
{
    s_tWave.bIsRunning = true;
    s_tWave.bRequestDesc = true;
}

void gwaveform_Stop(void)
{
    s_tWave.bIsRunning = false;
}

void __gwaveform_Push(uint8_t chID, float fValue)
{
    if (chID >= s_tWave.chCount) return;
    
    s_tWave.ahwSamples[chID] = (int16_t)(fValue * s_tWave.atChannels[chID].fScale);
    s_tWave.abMask[chID / 8] |= (1u << (chID % 8));
}

void __gwaveform_PushRaw(uint8_t chID, int16_t hwValue)
{
    if (chID >= s_tWave.chCount) return;
    
    s_tWave.ahwSamples[chID] = hwValue;
    s_tWave.abMask[chID / 8] |= (1u << (chID % 8));
}

void __gwaveform_Commit(void)
{
    if (!s_tWave.bIsRunning) {
        memset(s_tWave.abMask, 0, MASK_BYTES);
        return;
    }

    if (++s_tWave.chDecimCounter < s_tWave.chDecimation) {
        return;
    }
    s_tWave.chDecimCounter = 0;

    /* Pack frame */
    uint8_t achFrame[4 + MASK_BYTES + 2 * GWAVEFORM_MAX_CHANNELS + 1];
    uint16_t hwIdx = 0;
    
    achFrame[hwIdx++] = SYNC_H;
    achFrame[hwIdx++] = SYNC_L;
    achFrame[hwIdx++] = s_tWave.chSeq++;
    
    memcpy(&achFrame[hwIdx], s_tWave.abMask, MASK_BYTES);
    hwIdx += MASK_BYTES;
    
    for (uint8_t i = 0; i < s_tWave.chCount; i++) {
        if (s_tWave.abMask[i / 8] & (1u << (i % 8))) {
            achFrame[hwIdx++] = (uint8_t)(s_tWave.ahwSamples[i] & 0xFF);
            achFrame[hwIdx++] = (uint8_t)((s_tWave.ahwSamples[i] >> 8) & 0xFF);
        }
    }
    
    achFrame[hwIdx] = calc_crc8(&achFrame[2], hwIdx - 2);
    hwIdx++;
    
    /* Write to ring buffer */
    gringbuf_WriteBulk(&s_tWave.tRB, achFrame, hwIdx);

    /* Clear mask for next cycle */
    memset(s_tWave.abMask, 0, MASK_BYTES);
}

void gwaveform_Poll(void)
{
    if (s_tWave.bRequestDesc) {
        s_tWave.bRequestDesc = false;
        send_descriptor_frame();
    }

    uint8_t achTemp[64];
    uint16_t hwLen;
    
    while ((hwLen = gringbuf_ReadBulk(&s_tWave.tRB, achTemp, sizeof(achTemp))) > 0) {
        SEGGER_RTT_Write(GWAVEFORM_RTT_CHANNEL, achTemp, hwLen);
    }
}

/*============================ GSHELL INTEGRATION ============================*/
#include "gshell.h"

static void cmd_wave(const char *args)
{
    if (strncmp(args, "start", 5) == 0) {
        gwaveform_Start();
        GLOG(I, "Waveform started.\r\n");
    } else if (strncmp(args, "stop", 4) == 0) {
        gwaveform_Stop();
        GLOG(I, "Waveform stopped.\r\n");
    } else if (strncmp(args, "rate", 4) == 0) {
        int wRate = 0;
        const char *p = &args[5];
        while (*p >= '0' && *p <= '9') {
            wRate = wRate * 10 + (*p++ - '0');
        }
        if (wRate > 0 && wRate <= 255) {
            s_tWave.chDecimation = (uint8_t)wRate;
            GLOGF(I, "Waveform decimation set to %d.\r\n", wRate);
        }
    } else if (strncmp(args, "list", 4) == 0) {
        GLOGF(I, "Registered channels: %d\r\n", s_tWave.chCount);
        for (uint8_t i = 0; i < s_tWave.chCount; i++) {
            GLOGF(I, " [%d] %s (scale: %.2f)\r\n", 
                             i, s_tWave.atChannels[i].achName, 
                             s_tWave.atChannels[i].fScale);
        }
    } else {
        GLOGF(I, "Usage: wave <start|stop|list|rate n>\r\n");
    }
}

GMSI_SHELL_CMD(wave, cmd_wave, "Waveform control (start/stop/rate/list)");

#endif /* GWAVEFORM_ENABLE */
