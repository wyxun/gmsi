#include "gwaveform.h"
#include "utilities/gringbuf.h"
#include "gmsi.h"
#include "segger_rtt/SEGGER_RTT.h"
#include "gdebug/util_debug.h"
#include <string.h>

#if GWAVEFORM_ENABLE

#define MASK_BYTES          ((GWAVEFORM_MAX_CHANNELS + 7) / 8)

/*============================ TYPES =========================================*/

typedef struct {
    gringbuf_t          tRB;
    uint8_t             achRBBuffer[GWAVEFORM_RING_BUFFER_SIZE];
    uint8_t             achRTTBuffer[GWAVEFORM_RTT_BUFFER_SIZE];
    
    gwaveform_ch_desc_t atChannels[GWAVEFORM_MAX_CHANNELS];
    int16_t             ahwSamples[GWAVEFORM_MAX_CHANNELS];
    uint8_t             abMask[MASK_BYTES];
    
    const gwaveform_protocol_t *ptProtocol;
    
    uint8_t             chCount;
    uint8_t             chSeq;
    uint8_t             chDecimCounter;
    uint8_t             chDecimation;
    bool                bIsRunning;
    bool                bRequestDesc;
} gwaveform_cb_t;

/*============================ PRIVATE DATA ==================================*/
static gwaveform_cb_t s_tWave;

/*============================ PUBLIC API ====================================*/

static int gwaveform_Init(const gwaveform_protocol_t *ptProtocol)
{
    memset(&s_tWave, 0, sizeof(s_tWave));
    gringbuf_Init(&s_tWave.tRB, s_tWave.achRBBuffer, GWAVEFORM_RING_BUFFER_SIZE);
    
    SEGGER_RTT_ConfigUpBuffer(GWAVEFORM_RTT_CHANNEL, "Waveform", 
                             s_tWave.achRTTBuffer, GWAVEFORM_RTT_BUFFER_SIZE, 
                             SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
                             
    s_tWave.chDecimation = GWAVEFORM_DECIMATION;
    s_tWave.ptProtocol = ptProtocol;
    if(s_tWave.ptProtocol == NULL) {
        s_tWave.ptProtocol = &default_waveform_protocol;
    }
    
    return GMSI_SUCCESS;
}

static uint8_t gwaveform_AddChannel(const char *pchName, float fScale)
{
    if (s_tWave.chCount >= GWAVEFORM_MAX_CHANNELS) return 0xFF;
    
    uint8_t chID = s_tWave.chCount++;
    strncpy(s_tWave.atChannels[chID].achName, pchName, 7);
    s_tWave.atChannels[chID].achName[7] = '\0';
    s_tWave.atChannels[chID].fScale = fScale;
    
    return chID;
}

static void gwaveform_Start(void)
{
    s_tWave.bIsRunning = true;
    s_tWave.bRequestDesc = true;
}

static void gwaveform_Stop(void)
{
    s_tWave.bIsRunning = false;
}

static void gwaveform_Push(uint8_t chID, float fValue)
{
    if (chID >= s_tWave.chCount) return;
    
    s_tWave.ahwSamples[chID] = (int16_t)(fValue * s_tWave.atChannels[chID].fScale);
    s_tWave.abMask[chID / 8] |= (1u << (chID % 8));
}

static void gwaveform_PushRaw(uint8_t chID, int16_t hwValue)
{
    if (chID >= s_tWave.chCount) return;
    
    s_tWave.ahwSamples[chID] = hwValue;
    s_tWave.abMask[chID / 8] |= (1u << (chID % 8));
}

static void gwaveform_Step(void)
{
    if (!s_tWave.bIsRunning) {
        memset(s_tWave.abMask, 0, MASK_BYTES);
        return;
    }

    if (++s_tWave.chDecimCounter < s_tWave.chDecimation) {
        return;
    }
    s_tWave.chDecimCounter = 0;

    /* Pack frame via protocol */
    uint8_t achFrame[4 + MASK_BYTES + 2 * GWAVEFORM_MAX_CHANNELS + 1];
    uint16_t hwLen = s_tWave.ptProtocol->pack_data(achFrame, s_tWave.ahwSamples, s_tWave.abMask, s_tWave.chCount, s_tWave.chSeq++);
    
    /* Write to ring buffer */
    gringbuf_WriteBulk(&s_tWave.tRB, achFrame, hwLen);

    /* Clear mask for next cycle */
    memset(s_tWave.abMask, 0, MASK_BYTES);
}

static void send_descriptor_frame(void)
{
    uint8_t achBuf[4 + 12 * GWAVEFORM_MAX_CHANNELS + 1];
    uint16_t hwLen = s_tWave.ptProtocol->pack_desc(achBuf, s_tWave.atChannels, s_tWave.chCount);
    SEGGER_RTT_Write(GWAVEFORM_RTT_CHANNEL, achBuf, hwLen);
}

static void gwaveform_Poll(void)
{
    static int64_t s_lLastDescTick = 0;
    extern int64_t get_system_ms(void);
    int64_t lNow = get_system_ms();

    /* Periodic descriptor frame (every 1000ms) to ensure host sync */
    if (s_tWave.bIsRunning && (lNow - s_lLastDescTick >= 1000)) {
        s_lLastDescTick = lNow;
        send_descriptor_frame();
    }

    uint8_t achTemp[512];
    uint16_t hwLen;
    
    hwLen = gringbuf_ReadBulk(&s_tWave.tRB, achTemp, sizeof(achTemp));
    if (hwLen > 0) {
        SEGGER_RTT_Write(GWAVEFORM_RTT_CHANNEL, achTemp, hwLen);
    }
}

const gwaveform_api_t gwaveform = {
    .Init = gwaveform_Init,
    .AddChannel = gwaveform_AddChannel,
    .Start = gwaveform_Start,
    .Stop = gwaveform_Stop,
    .Push = gwaveform_Push,
    .PushRaw = gwaveform_PushRaw,
    .Step = gwaveform_Step,
    .Poll = gwaveform_Poll,
};

/* Weak callback for default 1ms timer scheduling */
__attribute__((weak)) void gwaveform_Default_Step_Callback(void) {
    gwaveform.Step();
}

/*============================ GSHELL INTEGRATION ============================*/
#include "gshell.h"

static void cmd_wave(const char *args)
{
    if (strncmp(args, "start", 5) == 0) {
        gwaveform.Start();
        GLOG(I, "Waveform started.\r\n");
    } else if (strncmp(args, "stop", 4) == 0) {
        gwaveform.Stop();
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

#else /* GWAVEFORM_ENABLE == 0 */

static int dummy_Init(const gwaveform_protocol_t *ptProtocol) { return 0; }
static uint8_t dummy_AddChannel(const char *pchName, float fScale) { return 0; }
static void dummy_void(void) {}
static void dummy_Push(uint8_t chID, float fValue) {}
static void dummy_PushRaw(uint8_t chID, int16_t hwValue) {}

const gwaveform_api_t gwaveform = {
    .Init = dummy_Init,
    .AddChannel = dummy_AddChannel,
    .Start = dummy_void,
    .Stop = dummy_void,
    .Push = dummy_Push,
    .PushRaw = dummy_PushRaw,
    .Step = dummy_void,
    .Poll = dummy_void,
};

__attribute__((weak)) void gwaveform_Default_Step_Callback(void) {}

#endif /* GWAVEFORM_ENABLE */
