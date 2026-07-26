#include "mwaveform.h"
#include "modus.h"
#include "segger_rtt/SEGGER_RTT.h"
#include "mdebug/util_debug.h"
#include <string.h>

#if MWAVEFORM_ENABLE

#define MASK_BYTES  ((MWAVEFORM_MAX_CHANNELS + 7) / 8)

/*============================ TYPES =========================================*/

typedef struct {
    /* 
     * Multi-level Block FIFO. 
     */

    uint8_t             achBuf[MWAVEFORM_FIFO_DEPTH][MWAVEFORM_FRAME_SIZE];
    uint16_t            ahwBufLen[MWAVEFORM_FIFO_DEPTH];

    volatile uint32_t   wWCount;            /* Produced packets */
    volatile uint32_t   wRCount;            /* Consumed packets */
    volatile uint32_t   wDropCount;         /* Overrun packets (cumulative) */
    volatile uint32_t   wIntervalDropCount; /* Overrun packets in current 1s window */
    volatile uint32_t   wLastIntervalDrops; /* Overrun packets in last 1s window */
    volatile uint32_t   wTotalCount;        /* Historical total */



    /* RTT shared memory (host reads this via SWD) */
    uint8_t             achRTTBuffer[MWAVEFORM_RTT_BUFFER_SIZE] __attribute__((aligned(4)));

    /* Cumulative count of RTT buffer full events (diagnostic) */
    volatile uint32_t   wRTTFullCount; 


    /* Channel descriptors & latest sample scratch-pad */
    mwaveform_ch_desc_t atChannels[MWAVEFORM_MAX_CHANNELS];
    int16_t             ahwSamples[MWAVEFORM_MAX_CHANNELS];
    uint8_t             abMask[MASK_BYTES];

    const mwaveform_protocol_t *ptProtocol;

    uint8_t             chCount;
    uint8_t             chSeq;
    uint8_t             chDecimCounter;
    uint8_t             chDecimation;
    bool                bIsRunning;
    bool                bRequestDesc;
    bool                bExternalDrive;     /* true = user calls Step from their own ISR */
} mwaveform_cb_t;

/*============================ PRIVATE DATA ==================================*/
static mwaveform_cb_t s_tWave;

/*============================ PUBLIC API ====================================*/

static int mwaveform_Init(const mwaveform_protocol_t *ptProtocol)
{
    memset(&s_tWave, 0, sizeof(s_tWave));

    SEGGER_RTT_ConfigUpBuffer(MWAVEFORM_RTT_CHANNEL, "Waveform",
                              s_tWave.achRTTBuffer, MWAVEFORM_RTT_BUFFER_SIZE,
                              SEGGER_RTT_MODE_NO_BLOCK_SKIP);

    s_tWave.chDecimation = MWAVEFORM_DECIMATION;
    s_tWave.ptProtocol   = ptProtocol;
    if (s_tWave.ptProtocol == NULL) {
        s_tWave.ptProtocol = &default_waveform_protocol;
    }

    return MODUS_SUCCESS;
}

static uint8_t mwaveform_AddChannel(const char *pchName, float fScale)
{
    if (s_tWave.chCount >= MWAVEFORM_MAX_CHANNELS) return 0xFF;

    uint8_t chID = s_tWave.chCount++;
    strncpy(s_tWave.atChannels[chID].achName, pchName, 7);
    s_tWave.atChannels[chID].achName[7] = '\0';
    s_tWave.atChannels[chID].fScale = fScale;

    return chID;
}

static void mwaveform_Start(void)
{
    s_tWave.bIsRunning  = true;
    s_tWave.bRequestDesc = true;
}

static void mwaveform_Stop(void)
{
    s_tWave.bIsRunning = false;
}

static void mwaveform_Push(uint8_t chID, float fValue)
{
    if (chID >= s_tWave.chCount) return;
    s_tWave.ahwSamples[chID] = (int16_t)(fValue * s_tWave.atChannels[chID].fScale);
    s_tWave.abMask[chID / 8] |= (1u << (chID % 8));
}

static void mwaveform_PushRaw(uint8_t chID, int16_t hwValue)
{
    if (chID >= s_tWave.chCount) return;
    s_tWave.ahwSamples[chID] = hwValue;
    s_tWave.abMask[chID / 8] |= (1u << (chID % 8));
}

/*
 * mwaveform_Step — ISR-safe, non-blocking.
 *
 * Toggles between two buffers. If Poll hasn't consumed the previous frame yet,
 * the new frame overwrites (newest-wins semantics).
 */
static void mwaveform_Step(void)
{
    if (!s_tWave.bIsRunning) {
        memset(s_tWave.abMask, 0, MASK_BYTES);
        return;
    }

    if (s_tWave.chDecimation > 1) {
        if (++s_tWave.chDecimCounter < s_tWave.chDecimation) {
            return; /* Decimate */
        }
        s_tWave.chDecimCounter = 0;
    }

    /* Overrun protection: Don't write to a slot that hasn't been read yet */
    if (s_tWave.wWCount - s_tWave.wRCount >= MWAVEFORM_FIFO_DEPTH) {
        s_tWave.wDropCount++;
        s_tWave.wIntervalDropCount++;
        s_tWave.wTotalCount++;
        memset(s_tWave.abMask, 0, MASK_BYTES);
        return;
    }


    /* 0xFD is the descriptor-frame type byte. A data frame carrying 0xFD in
     * the seq position is misframed as a descriptor by host parsers, which
     * then skip a whole descriptor length and eat ~5 data frames every time
     * seq wraps through 0xFD. Never emit the reserved value. */
    if (s_tWave.chSeq == 0xFD) {
        s_tWave.chSeq++;
    }

    /* Index to the current writing buffer */
    uint8_t chW = s_tWave.wWCount % MWAVEFORM_FIFO_DEPTH;

    s_tWave.ahwBufLen[chW] = s_tWave.ptProtocol->pack_data(
        s_tWave.achBuf[chW],
        s_tWave.ahwSamples, s_tWave.abMask,
        s_tWave.chCount, s_tWave.chSeq++);

    s_tWave.wWCount++;
    s_tWave.wTotalCount++;



    memset(s_tWave.abMask, 0, MASK_BYTES);
}


static void send_descriptor_frame(void)
{
    uint8_t achBuf[4 + 12 * MWAVEFORM_MAX_CHANNELS + 1];
    uint16_t hwLen = s_tWave.ptProtocol->pack_desc(
        achBuf, s_tWave.atChannels, s_tWave.chCount);
    SEGGER_RTT_Write(MWAVEFORM_RTT_CHANNEL, achBuf, hwLen);
}

/*
 * mwaveform_Poll — call from main loop (non-ISR context).
 *
 * Drains the ping-pong buffer to RTT and sends periodic descriptor frames.
 */
static void mwaveform_Poll(void)
{
    static int64_t s_lLastDescTick = 0;
    static int64_t s_lLastDropTick = 0;
    extern int64_t get_system_ms(void);
    int64_t lNow = get_system_ms();

    /* Send descriptor on first start and every 1000ms for host re-sync */
    if (s_tWave.bIsRunning &&
        (s_tWave.bRequestDesc || (lNow - s_lLastDescTick >= 1000))) {
        s_lLastDescTick      = lNow;
        s_tWave.bRequestDesc = false;
        send_descriptor_frame();
    }

    /* Update interval drop stats every 1000ms */
    if (lNow - s_lLastDropTick >= 1000) {
        s_lLastDropTick = lNow;
        s_tWave.wLastIntervalDrops = s_tWave.wIntervalDropCount;
        s_tWave.wIntervalDropCount = 0;
    }


    uint32_t wWCount = s_tWave.wWCount;
    uint32_t wRCount = s_tWave.wRCount;

    /* Continuous polling but throttled:
     * Send up to FIFO-depth packets per loop pass. SEGGER_RTT_Write is
     * non-blocking (NO_BLOCK_SKIP) and we break on the first congested
     * write, so a full drain adds no extra main-loop latency; the limit
     * only guards against pathological loops. 8/pass was too tight for a
     * 1 kHz frame rate with a slow main loop.
     */
    uint8_t chLimit = MWAVEFORM_FIFO_DEPTH;
    while (wWCount != wRCount && chLimit--) {
        uint8_t chR = wRCount % MWAVEFORM_FIFO_DEPTH;
        
        /* Ensure we have a valid packet there (len > 0) */
        if (s_tWave.ahwBufLen[chR] > 0) {
            /* Try to send one full packet */
            uint32_t wSent = SEGGER_RTT_Write(MWAVEFORM_RTT_CHANNEL,
                                             s_tWave.achBuf[chR],
                                             s_tWave.ahwBufLen[chR]);
            
            if (wSent < s_tWave.ahwBufLen[chR]) {
                /* Capture congestion event */
                s_tWave.wRTTFullCount++;
                break; 
            }

        }
        
        wRCount++;
    }
    s_tWave.wRCount = wRCount;
}



static void mwaveform_SetRate(uint32_t wDecimation)
{
    if (wDecimation == 0) {
        s_tWave.bExternalDrive = true;
    } else {
        s_tWave.bExternalDrive = false;
        s_tWave.chDecimation   = (wDecimation > 255u) ? 255u : (uint8_t)wDecimation;
        s_tWave.chDecimCounter = 0;
    }
}

static uint32_t mwaveform_GetDropCount(void)
{
    return s_tWave.wDropCount;
}

static uint32_t mwaveform_GetLastIntervalDrops(void)
{
    return s_tWave.wLastIntervalDrops;
}

static void mwaveform_ClearDropCount(void)
{
    s_tWave.wRCount = 0;
    s_tWave.wWCount = 0;
    s_tWave.wTotalCount = 0;
    s_tWave.wDropCount = 0;
    s_tWave.wIntervalDropCount = 0;
    s_tWave.wLastIntervalDrops = 0;
    s_tWave.wRTTFullCount = 0;
}

uint32_t mwaveform_GetRTTFullCount(void)
{
    return s_tWave.wRTTFullCount;
}



const mwaveform_api_t mwaveform = {
    .Init           = mwaveform_Init,
    .AddChannel     = mwaveform_AddChannel,
    .Start          = mwaveform_Start,
    .Stop           = mwaveform_Stop,
    .Push           = mwaveform_Push,
    .PushRaw        = mwaveform_PushRaw,
    .Step           = mwaveform_Step,
    .Poll           = mwaveform_Poll,
    .SetRate        = mwaveform_SetRate,
    .GetDropCount   = mwaveform_GetDropCount,
    .GetLastIntervalDrops = mwaveform_GetLastIntervalDrops,
    .ClearDropCount = mwaveform_ClearDropCount,
};



/* Weak callback — invoked by the MODUS 1kHz tick.
 * Silenced automatically when user enables external-drive mode via SetRate(0). */
__attribute__((weak)) void mwaveform_Default_Step_Callback(void) {
    if (!s_tWave.bExternalDrive) {
        mwaveform.Step();
    }
}

/*============================ MSHELL INTEGRATION ============================*/
#include "mshell.h"

static void cmd_wave(const char *args)
{
    if (strncmp(args, "start", 5) == 0) {
        mwaveform.Start();
        MLOG(I, "Waveform started.\r\n");

    } else if (strncmp(args, "stop", 4) == 0) {
        mwaveform.Stop();
        MLOG(I, "Waveform stopped.\r\n");

    } else if (strncmp(args, "rate", 4) == 0) {
        uint32_t wRate = 0;
        const char *p = &args[5];
        while (*p >= '0' && *p <= '9') {
            wRate = wRate * 10u + (uint32_t)(*p++ - '0');
        }
        mwaveform.SetRate(wRate);
        if (wRate == 0) {
            MLOG(I, "Waveform: external-drive mode (call Step from your ISR).\r\n");
        } else {
            MLOGF(I, "Waveform decimation set to %lu.\r\n", (unsigned long)wRate);
        }

    } else if (strncmp(args, "drop clear", 10) == 0) {
        mwaveform.ClearDropCount();
        MLOG(I, "Waveform: Drop count cleared.\r\n");

    } else if (strncmp(args, "drop", 4) == 0) {
        uint32_t wDrop  = mwaveform.GetDropCount();
        uint32_t wTotal = s_tWave.wTotalCount;
        float    fRate  = 0.0f;
        if (wTotal > 0) {
            fRate = (float)wDrop * 100.0f / (float)wTotal;
        }
        MLOGF(I, "Waveform: Drop %lu / Total %lu (%.2f%%)\r\n",
              (unsigned long)wDrop, 
              (unsigned long)wTotal,
              fRate);

    } else if (strncmp(args, "list", 4) == 0) {
        MLOGF(I, "Channels: %d\r\n", s_tWave.chCount);
        for (uint8_t i = 0; i < s_tWave.chCount; i++) {
            MLOGF(I, " [%d] %s (scale: %.2f)\r\n",
                  i, s_tWave.atChannels[i].achName,
                  s_tWave.atChannels[i].fScale);
        }

    } else {
        MLOG(I, "Usage: wave <start|stop|list|rate <n>|drop|drop clear>\r\n");
        MLOG(I, "  rate 0 = external-drive, rate n = decimation factor\r\n");
    }
}

MODUS_SHELL_CMD(wave, cmd_wave, "Waveform control (start/stop/rate/list/drop)");

#else /* MWAVEFORM_ENABLE == 0 */

static int      dummy_Init(const mwaveform_protocol_t *ptProtocol) { (void)ptProtocol; return 0; }
static uint8_t  dummy_AddChannel(const char *pchName, float fScale) { (void)pchName; (void)fScale; return 0; }
static void     dummy_void(void) {}
static void     dummy_Push(uint8_t chID, float fValue) { (void)chID; (void)fValue; }
static void     dummy_PushRaw(uint8_t chID, int16_t hwValue) { (void)chID; (void)hwValue; }
static void     dummy_SetRate(uint32_t wDecimation) { (void)wDecimation; }
static uint32_t dummy_GetDropCount(void) { return 0; }

const mwaveform_api_t mwaveform = {
    .Init           = dummy_Init,
    .AddChannel     = dummy_AddChannel,
    .Start          = dummy_void,
    .Stop           = dummy_void,
    .Push           = dummy_Push,
    .PushRaw        = dummy_PushRaw,
    .Step           = dummy_void,
    .Poll           = dummy_void,
    .SetRate        = dummy_SetRate,
    .GetDropCount   = dummy_GetDropCount,
    .ClearDropCount = dummy_void,
};

__attribute__((weak)) void mwaveform_Default_Step_Callback(void) {}

#endif /* MWAVEFORM_ENABLE */
