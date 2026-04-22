#include "gwaveform.h"
#include "gmsi.h"
#include "segger_rtt/SEGGER_RTT.h"
#include "gdebug/util_debug.h"
#include <string.h>

#if GWAVEFORM_ENABLE

#define MASK_BYTES  ((GWAVEFORM_MAX_CHANNELS + 7) / 8)

/*============================ TYPES =========================================*/

typedef struct {
    /*
     * Ping-pong frame buffers.
     * Step (ISR context) writes to achBuf[chWriteIdx] then toggles chWriteIdx.
     * Poll (main loop)  reads  from achBuf[chReadIdx] after bFrameReady is set.
     * On Cortex-M (single-core), a uint8_t/bool write is atomic, so this
     * lock-free handoff is safe as long as Poll saves chReadIdx before clearing
     * bFrameReady (see gwaveform_Poll implementation).
     */
    uint8_t             achBuf[2][GWAVEFORM_FRAME_SIZE];
    uint16_t            ahwBufLen[2];       /* packed byte count for each buf  */
    volatile uint8_t    chWriteIdx;         /* Step writes here                */
    volatile uint8_t    chReadIdx;          /* Poll reads from here            */
    volatile uint32_t   wWriteCount;        /* Incremented on every successful Step */
    volatile uint32_t   wReadCount;         /* Poll syncs with this to detect new frames */

    /* Diagnostics: count frames overwritten before Poll consumed them */
    volatile uint32_t   wDropCount;
    volatile uint32_t   wTotalCount;        /* Total attempted frames */

    /* RTT shared memory (host reads this via SWD) */
    uint8_t             achRTTBuffer[GWAVEFORM_RTT_BUFFER_SIZE];

    /* Channel descriptors & latest sample scratch-pad */
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
    bool                bExternalDrive;     /* true = user calls Step from their own ISR */
} gwaveform_cb_t;

/*============================ PRIVATE DATA ==================================*/
static gwaveform_cb_t s_tWave;

/*============================ PUBLIC API ====================================*/

static int gwaveform_Init(const gwaveform_protocol_t *ptProtocol)
{
    memset(&s_tWave, 0, sizeof(s_tWave));

    SEGGER_RTT_ConfigUpBuffer(GWAVEFORM_RTT_CHANNEL, "Waveform",
                              s_tWave.achRTTBuffer, GWAVEFORM_RTT_BUFFER_SIZE,
                              SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);

    s_tWave.chDecimation = GWAVEFORM_DECIMATION;
    s_tWave.ptProtocol   = ptProtocol;
    if (s_tWave.ptProtocol == NULL) {
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
    s_tWave.bIsRunning  = true;
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

/*
 * gwaveform_Step — ISR-safe, non-blocking.
 *
 * In internal-drive mode (bExternalDrive == false):
 *   Called by gwaveform_Default_Step_Callback at 1kHz. Decimation controls
 *   how many calls are skipped between output frames.
 *
 * In external-drive mode (bExternalDrive == true):
 *   User calls gwaveform.Step() directly from their own ISR (e.g. FOC loop).
 *   Every call produces one output frame — decimation is bypassed.
 *   Use gwaveform.SetRate(0) to enable this mode.
 *
 * If Poll hasn't consumed the previous frame yet, wDropCount is incremented
 * and the new frame overwrites (newest-wins semantics).
 */
static void gwaveform_Step(void)
{
    if (!s_tWave.bIsRunning) {
        memset(s_tWave.abMask, 0, MASK_BYTES);
        return;
    }

    /* Decimation gate — only applies in internal-drive mode */
    if (!s_tWave.bExternalDrive) {
        if (++s_tWave.chDecimCounter < s_tWave.chDecimation) {
            return;
        }
        s_tWave.chDecimCounter = 0;
    }

    /* Count attempted frames */
    s_tWave.wTotalCount++;

    /* Count frame overwrite if Poll hasn't consumed the previous one 
     * (i.e. WriteCount is ahead of ReadCount) */
    if (s_tWave.wWriteCount != s_tWave.wReadCount) {
        s_tWave.wDropCount++;
    }

    /* Pack into current write buffer */
    uint8_t chW = s_tWave.chWriteIdx;
    s_tWave.ahwBufLen[chW] = s_tWave.ptProtocol->pack_data(
        s_tWave.achBuf[chW],
        s_tWave.ahwSamples, s_tWave.abMask,
        s_tWave.chCount, s_tWave.chSeq++);

    /*
     * Publish handoff (Lock-free on Cortex-M):
     * 1. Set chReadIdx so Poll knows which buffer holds the new frame.
     * 2. Increment wWriteCount (Poll will see this change).
     * 3. Toggle chWriteIdx so the next Step call writes to the other buffer.
     */
    s_tWave.chReadIdx    = chW;
    s_tWave.wWriteCount++;
    s_tWave.chWriteIdx  ^= 1;

    memset(s_tWave.abMask, 0, MASK_BYTES);
}

static void send_descriptor_frame(void)
{
    uint8_t achBuf[4 + 12 * GWAVEFORM_MAX_CHANNELS + 1];
    uint16_t hwLen = s_tWave.ptProtocol->pack_desc(
        achBuf, s_tWave.atChannels, s_tWave.chCount);
    SEGGER_RTT_Write(GWAVEFORM_RTT_CHANNEL, achBuf, hwLen);
}

/*
 * gwaveform_Poll — call from main loop (non-ISR context).
 *
 * Drains the ping-pong buffer to RTT and sends periodic descriptor frames.
 * RTT_Write can block here (BLOCK_IF_FIFO_FULL mode) since this runs in the
 * main loop, not an ISR.
 */
static void gwaveform_Poll(void)
{
    static int64_t s_lLastDescTick = 0;
    extern int64_t get_system_ms(void);
    int64_t lNow = get_system_ms();

    /* Send descriptor on first start and every 1000ms for host re-sync */
    if (s_tWave.bIsRunning &&
        (s_tWave.bRequestDesc || (lNow - s_lLastDescTick >= 1000))) {
        s_lLastDescTick      = lNow;
        s_tWave.bRequestDesc = false;
        send_descriptor_frame();
    }

    /* Claim the frame: save chReadIdx before clearing bFrameReady so that a
     * preempting Step ISR cannot corrupt our index after we clear the flag. */
    /* Claim the frame: save chReadIdx before updating wReadCount */
    uint32_t wWCount = s_tWave.wWriteCount;
    if (wWCount != s_tWave.wReadCount) {
        s_tWave.wReadCount = wWCount;      /* Sync with writer */
        uint8_t chR = s_tWave.chReadIdx;   /* Get newest frame index */
        SEGGER_RTT_Write(GWAVEFORM_RTT_CHANNEL,
                         s_tWave.achBuf[chR],
                         s_tWave.ahwBufLen[chR]);
    }
}

/*
 * gwaveform_SetRate
 *
 * wDecimation == 0 : external-drive mode — disable internal tick, user calls
 *                    gwaveform.Step() from their own ISR at any frequency.
 * wDecimation >= 1 : internal-drive mode — framework tick calls Step at 1kHz,
 *                    one frame is emitted every wDecimation calls.
 *                    e.g. wDecimation=1 → 1kHz output, =2 → 500Hz, etc.
 */
static void gwaveform_SetRate(uint32_t wDecimation)
{
    if (wDecimation == 0) {
        s_tWave.bExternalDrive = true;
    } else {
        s_tWave.bExternalDrive = false;
        s_tWave.chDecimation   = (wDecimation > 255u) ? 255u : (uint8_t)wDecimation;
        s_tWave.chDecimCounter = 0;
    }
}

static uint32_t gwaveform_GetDropCount(void)
{
    return s_tWave.wDropCount;
}

static void gwaveform_ClearDropCount(void)
{
    s_tWave.wDropCount = 0;
    s_tWave.wTotalCount = 0;
}

const gwaveform_api_t gwaveform = {
    .Init           = gwaveform_Init,
    .AddChannel     = gwaveform_AddChannel,
    .Start          = gwaveform_Start,
    .Stop           = gwaveform_Stop,
    .Push           = gwaveform_Push,
    .PushRaw        = gwaveform_PushRaw,
    .Step           = gwaveform_Step,
    .Poll           = gwaveform_Poll,
    .SetRate        = gwaveform_SetRate,
    .GetDropCount   = gwaveform_GetDropCount,
    .ClearDropCount = gwaveform_ClearDropCount,
};

/* Weak callback — invoked by the GMSI 1kHz tick.
 * Silenced automatically when user enables external-drive mode via SetRate(0). */
__attribute__((weak)) void gwaveform_Default_Step_Callback(void) {
    if (!s_tWave.bExternalDrive) {
        gwaveform.Step();
    }
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
        uint32_t wRate = 0;
        const char *p = &args[5];
        while (*p >= '0' && *p <= '9') {
            wRate = wRate * 10u + (uint32_t)(*p++ - '0');
        }
        gwaveform.SetRate(wRate);
        if (wRate == 0) {
            GLOG(I, "Waveform: external-drive mode (call Step from your ISR).\r\n");
        } else {
            GLOGF(I, "Waveform decimation set to %lu.\r\n", (unsigned long)wRate);
        }

    } else if (strncmp(args, "drop clear", 10) == 0) {
        gwaveform.ClearDropCount();
        GLOG(I, "Waveform: Drop count cleared.\r\n");

    } else if (strncmp(args, "drop", 4) == 0) {
        uint32_t wDrop  = gwaveform.GetDropCount();
        uint32_t wTotal = s_tWave.wTotalCount;
        float    fRate  = 0.0f;
        if (wTotal > 0) {
            fRate = (float)wDrop * 100.0f / (float)wTotal;
        }
        GLOGF(I, "Waveform: Drop %lu / Total %lu (%.2f%%)\r\n",
              (unsigned long)wDrop, 
              (unsigned long)wTotal,
              fRate);

    } else if (strncmp(args, "list", 4) == 0) {
        GLOGF(I, "Channels: %d\r\n", s_tWave.chCount);
        for (uint8_t i = 0; i < s_tWave.chCount; i++) {
            GLOGF(I, " [%d] %s (scale: %.2f)\r\n",
                  i, s_tWave.atChannels[i].achName,
                  s_tWave.atChannels[i].fScale);
        }

    } else {
        GLOG(I, "Usage: wave <start|stop|list|rate <n>|drop|drop clear>\r\n");
        GLOG(I, "  rate 0 = external-drive, rate n = decimation factor\r\n");
    }
}

GMSI_SHELL_CMD(wave, cmd_wave, "Waveform control (start/stop/rate/list/drop)");

#else /* GWAVEFORM_ENABLE == 0 */

static int      dummy_Init(const gwaveform_protocol_t *ptProtocol) { (void)ptProtocol; return 0; }
static uint8_t  dummy_AddChannel(const char *pchName, float fScale) { (void)pchName; (void)fScale; return 0; }
static void     dummy_void(void) {}
static void     dummy_Push(uint8_t chID, float fValue) { (void)chID; (void)fValue; }
static void     dummy_PushRaw(uint8_t chID, int16_t hwValue) { (void)chID; (void)hwValue; }
static void     dummy_SetRate(uint32_t wDecimation) { (void)wDecimation; }
static uint32_t dummy_GetDropCount(void) { return 0; }

const gwaveform_api_t gwaveform = {
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

__attribute__((weak)) void gwaveform_Default_Step_Callback(void) {}

#endif /* GWAVEFORM_ENABLE */
