#include "mwaveform.h"
#include "modus.h"
#include "segger_rtt/SEGGER_RTT.h"
#include "mdebug/util_debug.h"
#include "perfc_port.h"
#include <string.h>

#if MWAVEFORM_ENABLE

#define MASK_BYTES  ((MWAVEFORM_MAX_CHANNELS + 7) / 8)

/*============================ TYPES =========================================*/

typedef struct {
#if MWAVEFORM_BATCH_ENABLE
    mwaveform_batch_sample_t atBatchSamples[MWAVEFORM_BATCH_DEPTH];
    uint8_t                 achBatchFrame[MWAVEFORM_BATCH_MAX_FRAME_SIZE];

    volatile uint32_t       wBatchWCount;
    volatile uint32_t       wBatchRCount;
    volatile uint32_t       wSampleIndex;
    int64_t                 lLastBatchFlushMs;
#else
    uint8_t                 achBuf[MWAVEFORM_FIFO_DEPTH][MWAVEFORM_FRAME_SIZE];
    uint16_t                ahwBufLen[MWAVEFORM_FIFO_DEPTH];

    volatile uint32_t       wWCount;
    volatile uint32_t       wRCount;
#endif

    volatile uint32_t       wDropCount;
    volatile uint32_t       wIntervalDropCount;
    volatile uint32_t       wLastIntervalDrops;
    volatile uint32_t       wTotalCount;

    uint8_t                 achRTTBuffer[MWAVEFORM_RTT_BUFFER_SIZE]
                                __attribute__((aligned(4)));
    volatile uint32_t       wRTTFullCount;
    volatile uint32_t       wDefaultStepCount;

    mwaveform_ch_desc_t     atChannels[MWAVEFORM_MAX_CHANNELS];
    uint32_t                awChannelRateHz[MWAVEFORM_MAX_CHANNELS];
    uint8_t                 achChannelDiv[MWAVEFORM_MAX_CHANNELS];
    uint8_t                 achChannelCounters[MWAVEFORM_MAX_CHANNELS];
    void                    *apvVariables[MWAVEFORM_MAX_CHANNELS];
    uint8_t                 achVariableTypes[MWAVEFORM_MAX_CHANNELS];
    int16_t                 ahwSamples[MWAVEFORM_MAX_CHANNELS];
    uint8_t                 abMask[MASK_BYTES];
    uint8_t                 abEverMask[MASK_BYTES];

#if MWAVEFORM_SNAPSHOT_ENABLE
    mwaveform_batch_sample_t atSnapshotSamples[MWAVEFORM_SNAPSHOT_DEPTH];
    uint8_t                 achSnapshotFrame[MWAVEFORM_SNAPSHOT_MAX_FRAME_SIZE];

    volatile uint16_t       hwSnapshotWrite;
    volatile uint16_t       hwSnapshotDepth;
    volatile uint16_t       hwSnapshotValidCount;
    volatile uint32_t       wSnapshotSampleIndex;
    volatile uint16_t       hwSnapshotFrameLen;
    volatile uint32_t       wSnapshotId;
    volatile uint32_t       wSnapshotPeriodNs;
    volatile bool           bSnapshotArmed;
    volatile bool           bSnapshotReady;
    volatile bool           bSnapshotPending;
#endif

    const mwaveform_protocol_t *ptProtocol;

    uint8_t                 chCount;
    uint8_t                 chSeq;
    uint8_t                 chDecimCounter;
    uint8_t                 chDecimation;
    uint32_t                wSamplePeriodNs;
    uint32_t                wIsrPeriodNs;
    bool                    bIsRunning;
    bool                    bRequestDesc;
    bool                    bRequestMeta;
    bool                    bExternalDrive;
} mwaveform_cb_t;

/*============================ PRIVATE DATA ==================================*/
static mwaveform_cb_t s_tWave;

/*============================ PRIVATE HELPERS ===============================*/

static uint16_t mwaveform_pack_meta(uint8_t *pchBuffer, uint8_t chCount,
                                    uint32_t periodNs, uint16_t batchDepth)
{
    if (s_tWave.ptProtocol != NULL && s_tWave.ptProtocol->pack_meta != NULL) {
        return s_tWave.ptProtocol->pack_meta(pchBuffer, chCount,
                                             periodNs, batchDepth);
    }
    return default_waveform_protocol.pack_meta(pchBuffer, chCount,
                                               periodNs, batchDepth);
}

static uint16_t mwaveform_pack_batch(
    uint8_t *pchBuffer, const mwaveform_batch_sample_t *atSamples,
    uint8_t chCount, uint16_t ringDepth, uint16_t startOffset,
    uint16_t sampleCount, uint32_t startSampleIndex, uint32_t periodNs)
{
    if (s_tWave.ptProtocol != NULL && s_tWave.ptProtocol->pack_batch != NULL) {
        return s_tWave.ptProtocol->pack_batch(
            pchBuffer, atSamples, chCount, ringDepth, startOffset,
            sampleCount, startSampleIndex, periodNs);
    }
    return default_waveform_protocol.pack_batch(
        pchBuffer, atSamples, chCount, ringDepth, startOffset,
        sampleCount, startSampleIndex, periodNs);
}

#if MWAVEFORM_SNAPSHOT_ENABLE
static uint16_t mwaveform_pack_snapshot(
    uint8_t *pchBuffer, const mwaveform_batch_sample_t *atSamples,
    uint8_t chCount, uint16_t ringDepth, uint16_t startOffset,
    uint16_t sampleCount, uint32_t periodNs, uint32_t snapshotId)
{
    if (s_tWave.ptProtocol != NULL &&
        s_tWave.ptProtocol->pack_snapshot != NULL) {
        return s_tWave.ptProtocol->pack_snapshot(
            pchBuffer, atSamples, chCount, ringDepth, startOffset,
            sampleCount, periodNs, snapshotId);
    }
    return default_waveform_protocol.pack_snapshot(
        pchBuffer, atSamples, chCount, ringDepth, startOffset,
        sampleCount, periodNs, snapshotId);
}
#endif

/*============================ PUBLIC API ====================================*/

static int mwaveform_Init(const mwaveform_protocol_t *ptProtocol)
{
    memset(&s_tWave, 0, sizeof(s_tWave));

    SEGGER_RTT_ConfigUpBuffer(MWAVEFORM_RTT_CHANNEL, "Waveform",
                              s_tWave.achRTTBuffer, MWAVEFORM_RTT_BUFFER_SIZE,
                              SEGGER_RTT_MODE_NO_BLOCK_SKIP);

    s_tWave.chDecimation    = MWAVEFORM_DECIMATION;
    s_tWave.wSamplePeriodNs = MWAVEFORM_DEFAULT_SAMPLE_PERIOD_NS;
    s_tWave.wIsrPeriodNs    = 0u;
    s_tWave.ptProtocol      = ptProtocol;
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

static uint8_t mwaveform_AddVariable(const char *pchName, float fScale,
                                     void *pvValue, uint8_t chType)
{
    if (pvValue == NULL ||
        (chType != MWAVEFORM_VAR_FLOAT && chType != MWAVEFORM_VAR_RAW)) {
        return 0xFF;
    }

    uint8_t chID = mwaveform_AddChannel(pchName, fScale);
    if (chID == 0xFF) return 0xFF;

    s_tWave.apvVariables[chID]   = pvValue;
    s_tWave.achVariableTypes[chID] = chType;
    s_tWave.abEverMask[chID / 8] |= (uint8_t)(1u << (chID % 8));
    return chID;
}

static void mwaveform_Start(void)
{
    s_tWave.bIsRunning   = true;
    s_tWave.bRequestDesc = true;
    s_tWave.bRequestMeta = true;
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
    s_tWave.abEverMask[chID / 8] |= (uint8_t)(1u << (chID % 8));
}

static void mwaveform_PushRaw(uint8_t chID, int16_t hwValue)
{
    if (chID >= s_tWave.chCount) return;
    s_tWave.ahwSamples[chID] = hwValue;
    s_tWave.abMask[chID / 8] |= (1u << (chID % 8));
    s_tWave.abEverMask[chID / 8] |= (uint8_t)(1u << (chID % 8));
}

static uint32_t mwaveform_GetStreamRateHz(void)
{
    if (s_tWave.wSamplePeriodNs == 0u) return 0u;
    return 1000000000u / s_tWave.wSamplePeriodNs;
}

static void mwaveform_RecomputeChannelDividers(void)
{
    uint32_t wStreamRateHz = mwaveform_GetStreamRateHz();

    for (uint8_t i = 0; i < s_tWave.chCount; i++) {
        uint32_t wTargetHz = s_tWave.awChannelRateHz[i];
        if (wTargetHz == 0u || wStreamRateHz == 0u ||
            wTargetHz >= wStreamRateHz) {
            s_tWave.achChannelDiv[i]     = 1u;
            s_tWave.achChannelCounters[i] = 0u;
            continue;
        }

        uint32_t wDiv = (wStreamRateHz + wTargetHz / 2u) / wTargetHz;
        if (wDiv < 1u) wDiv = 1u;
        if (wDiv > 255u) wDiv = 255u;
        s_tWave.achChannelDiv[i]     = (uint8_t)wDiv;
        s_tWave.achChannelCounters[i] = 0u;
    }
}

static void mwaveform_BuildSampleMask(uint8_t *pchSampleMask)
{
    memset(pchSampleMask, 0, MASK_BYTES);

    for (uint8_t i = 0; i < s_tWave.chCount; i++) {
        uint8_t chMask = s_tWave.abMask[i / 8];
        if ((chMask & (1u << (i % 8))) == 0u &&
            s_tWave.apvVariables[i] == NULL) {
            continue;
        }

        uint8_t chDiv = s_tWave.achChannelDiv[i];
        if (chDiv <= 1u) {
            pchSampleMask[i / 8] |= (1u << (i % 8));
            continue;
        }

        s_tWave.achChannelCounters[i]++;
        if (s_tWave.achChannelCounters[i] >= chDiv) {
            s_tWave.achChannelCounters[i] = 0u;
            pchSampleMask[i / 8] |= (1u << (i % 8));
        }
    }
}

static void mwaveform_UpdateBoundSamples(const uint8_t *pchSampleMask)
{
    for (uint8_t i = 0; i < s_tWave.chCount; i++) {
        if ((pchSampleMask[i / 8] & (1u << (i % 8))) == 0u) continue;
        if (s_tWave.abMask[i / 8] & (1u << (i % 8))) continue;
        if (s_tWave.apvVariables[i] == NULL) continue;

        if (s_tWave.achVariableTypes[i] == MWAVEFORM_VAR_FLOAT) {
            float fValue = *(volatile float *)s_tWave.apvVariables[i];
            s_tWave.ahwSamples[i] =
                (int16_t)(fValue * s_tWave.atChannels[i].fScale);
        } else if (s_tWave.achVariableTypes[i] == MWAVEFORM_VAR_RAW) {
            s_tWave.ahwSamples[i] =
                *(volatile int16_t *)s_tWave.apvVariables[i];
        }
    }
}

/*
 * mwaveform_Step — ISR-safe, non-blocking.
 *
 * In batch mode this stores a sparse sample (mask + active channels) into an
 * SPSC ring. The main loop packs and sends it later.
 */
static void mwaveform_Step(void)
{
    if (!s_tWave.bIsRunning) {
        memset(s_tWave.abMask, 0, MASK_BYTES);
        return;
    }

    if (s_tWave.chDecimation > 1) {
        if (++s_tWave.chDecimCounter < s_tWave.chDecimation) {
            return;
        }
        s_tWave.chDecimCounter = 0;
    }

    uint8_t abSampleMask[MASK_BYTES];
    mwaveform_BuildSampleMask(abSampleMask);

#if MWAVEFORM_BATCH_ENABLE
    if (s_tWave.wBatchWCount - s_tWave.wBatchRCount >=
        MWAVEFORM_BATCH_DEPTH) {
        s_tWave.wDropCount++;
        s_tWave.wIntervalDropCount++;
        s_tWave.wTotalCount++;
        s_tWave.wSampleIndex++;
        return;
    }

    mwaveform_UpdateBoundSamples(abSampleMask);

    uint8_t chW = s_tWave.wBatchWCount % MWAVEFORM_BATCH_DEPTH;
    s_tWave.atBatchSamples[chW].wSampleIndex = s_tWave.wSampleIndex++;
    memcpy(s_tWave.atBatchSamples[chW].abMask, abSampleMask, MASK_BYTES);
    memcpy(s_tWave.atBatchSamples[chW].ahwSamples, s_tWave.ahwSamples,
           sizeof(s_tWave.ahwSamples[0]) * s_tWave.chCount);

    s_tWave.wBatchWCount++;
    s_tWave.wTotalCount++;
    for (uint8_t i = 0; i < MASK_BYTES; i++) {
        s_tWave.abMask[i] &= (uint8_t)~abSampleMask[i];
    }
#else
    if (s_tWave.wWCount - s_tWave.wRCount >= MWAVEFORM_FIFO_DEPTH) {
        s_tWave.wDropCount++;
        s_tWave.wIntervalDropCount++;
        s_tWave.wTotalCount++;
        return;
    }

    mwaveform_UpdateBoundSamples(abSampleMask);

    if (s_tWave.chSeq == MWAVEFORM_FRAME_TYPE_DESC ||
        s_tWave.chSeq == MWAVEFORM_FRAME_TYPE_META ||
        s_tWave.chSeq == MWAVEFORM_FRAME_TYPE_BATCH ||
        s_tWave.chSeq == MWAVEFORM_FRAME_TYPE_SNAPSHOT ||
        s_tWave.chSeq == 0xFFu) {
        s_tWave.chSeq++;
    }

    uint8_t chW = s_tWave.wWCount % MWAVEFORM_FIFO_DEPTH;
    s_tWave.ahwBufLen[chW] = s_tWave.ptProtocol->pack_data(
        s_tWave.achBuf[chW], s_tWave.ahwSamples, abSampleMask,
        s_tWave.chCount, s_tWave.chSeq++);

    s_tWave.wWCount++;
    s_tWave.wTotalCount++;
    for (uint8_t i = 0; i < MASK_BYTES; i++) {
        s_tWave.abMask[i] &= (uint8_t)~abSampleMask[i];
    }
#endif
}

static void send_descriptor_frame(void)
{
    uint8_t achBuf[4 + 12 * MWAVEFORM_MAX_CHANNELS + 1];
    uint16_t hwLen = s_tWave.ptProtocol->pack_desc(
        achBuf, s_tWave.atChannels, s_tWave.chCount);
    SEGGER_RTT_Write(MWAVEFORM_RTT_CHANNEL, achBuf, hwLen);
}

static void send_metadata_frame(int64_t lNow)
{
    uint8_t achBuf[32];
    uint16_t hwLen = mwaveform_pack_meta(
        achBuf, s_tWave.chCount, s_tWave.wSamplePeriodNs,
        (uint16_t)MWAVEFORM_BATCH_DEPTH);

    if (SEGGER_RTT_Write(MWAVEFORM_RTT_CHANNEL, achBuf, hwLen) == hwLen) {
        s_tWave.bRequestMeta = false;
        (void)lNow;
    } else {
        s_tWave.wRTTFullCount++;
    }
}

/*
 * mwaveform_Poll — call from main loop (non-ISR context).
 */
static void mwaveform_Poll(void)
{
    static int64_t s_lLastDescTick = 0;
    static int64_t s_lLastMetaTick = 0;
    static int64_t s_lLastDropTick = 0;
    extern int64_t get_system_ms(void);
    int64_t lNow = get_system_ms();

    if (s_tWave.bIsRunning &&
        (s_tWave.bRequestDesc || (lNow - s_lLastDescTick >= 1000))) {
        s_lLastDescTick      = lNow;
        s_tWave.bRequestDesc = false;
        send_descriptor_frame();
    }

    if (s_tWave.bIsRunning &&
        (s_tWave.bRequestMeta || (lNow - s_lLastMetaTick >= 1000))) {
        s_lLastMetaTick = lNow;
        send_metadata_frame(lNow);
    }

    if (lNow - s_lLastDropTick >= 1000) {
        s_lLastDropTick = lNow;
        s_tWave.wLastIntervalDrops = s_tWave.wIntervalDropCount;
        s_tWave.wIntervalDropCount = 0;
    }

#if MWAVEFORM_SNAPSHOT_ENABLE
    if (s_tWave.bSnapshotPending) {
        if (SEGGER_RTT_Write(MWAVEFORM_RTT_CHANNEL,
                             s_tWave.achSnapshotFrame,
                             s_tWave.hwSnapshotFrameLen) ==
            s_tWave.hwSnapshotFrameLen) {
            s_tWave.bSnapshotPending = false;
        } else {
            s_tWave.wRTTFullCount++;
        }
    }

    if (s_tWave.bSnapshotReady) {
        uint32_t wState = perfc_port_disable_global_interrupt();
        if (s_tWave.bSnapshotReady && s_tWave.hwSnapshotDepth > 0u) {
            uint16_t hwValid = s_tWave.hwSnapshotValidCount;
            uint16_t hwStart = 0u;

            if (hwValid > s_tWave.hwSnapshotDepth) {
                hwValid = s_tWave.hwSnapshotDepth;
            }
            if (s_tWave.hwSnapshotWrite >= s_tWave.hwSnapshotDepth) {
                hwStart = (uint16_t)(s_tWave.hwSnapshotWrite %
                                     s_tWave.hwSnapshotDepth);
            }
            s_tWave.hwSnapshotFrameLen = mwaveform_pack_snapshot(
                s_tWave.achSnapshotFrame, s_tWave.atSnapshotSamples,
                s_tWave.chCount, s_tWave.hwSnapshotDepth, hwStart,
                hwValid, s_tWave.wSnapshotPeriodNs,
                s_tWave.wSnapshotId++);
            s_tWave.bSnapshotReady   = false;
            s_tWave.bSnapshotPending = true;
        } else {
            s_tWave.bSnapshotReady = false;
        }
        perfc_port_resume_global_interrupt(wState);
    }
#endif

#if MWAVEFORM_BATCH_ENABLE
    {
        uint32_t wPending = s_tWave.wBatchWCount - s_tWave.wBatchRCount;
        bool bFlush = true;

        while (bFlush && wPending > 0u && s_tWave.chCount > 0u) {
            bFlush = (wPending >= MWAVEFORM_BATCH_SIZE) ||
                     (lNow - s_tWave.lLastBatchFlushMs >=
                      MWAVEFORM_BATCH_FLUSH_MS);
            if (!bFlush) break;

            uint16_t hwMax = (MWAVEFORM_RTT_BUFFER_SIZE - 17u) /
                             (MASK_BYTES + 2u * s_tWave.chCount);
            if (hwMax == 0u) {
                s_tWave.wRTTFullCount++;
                break;
            }

            uint16_t hwToSend = (wPending > MWAVEFORM_BATCH_SIZE)
                ? MWAVEFORM_BATCH_SIZE : (uint16_t)wPending;
            if (hwToSend > hwMax) hwToSend = hwMax;

            uint8_t chR = s_tWave.wBatchRCount % MWAVEFORM_BATCH_DEPTH;
            uint16_t hwLen = mwaveform_pack_batch(
                s_tWave.achBatchFrame, s_tWave.atBatchSamples,
                s_tWave.chCount, MWAVEFORM_BATCH_DEPTH, chR, hwToSend,
                s_tWave.atBatchSamples[chR].wSampleIndex,
                s_tWave.wSamplePeriodNs);

            if (SEGGER_RTT_Write(MWAVEFORM_RTT_CHANNEL,
                                 s_tWave.achBatchFrame, hwLen) == hwLen) {
                s_tWave.wBatchRCount += hwToSend;
                s_tWave.lLastBatchFlushMs = lNow;
            } else {
                s_tWave.wRTTFullCount++;
                break;
            }

            wPending = s_tWave.wBatchWCount - s_tWave.wBatchRCount;
        }
    }
#else
    {
        uint32_t wWCount = s_tWave.wWCount;
        uint32_t wRCount = s_tWave.wRCount;
        uint8_t chLimit = MWAVEFORM_FIFO_DEPTH;

        while (wWCount != wRCount && chLimit--) {
            uint8_t chR = wRCount % MWAVEFORM_FIFO_DEPTH;
            if (s_tWave.ahwBufLen[chR] > 0) {
                uint32_t wSent = SEGGER_RTT_Write(
                    MWAVEFORM_RTT_CHANNEL, s_tWave.achBuf[chR],
                    s_tWave.ahwBufLen[chR]);
                if (wSent < s_tWave.ahwBufLen[chR]) {
                    s_tWave.wRTTFullCount++;
                    break;
                }
            }
            wRCount++;
        }
        s_tWave.wRCount = wRCount;
    }
#endif
}

static void mwaveform_SetRate(uint32_t wDecimation)
{
    if (wDecimation == 0) {
        s_tWave.bExternalDrive = true;
    } else {
        s_tWave.bExternalDrive = false;
        s_tWave.chDecimation   = (wDecimation > 255u) ? 255u :
                                 (uint8_t)wDecimation;
        s_tWave.chDecimCounter = 0;
    }
}

static void mwaveform_SetDecimation(uint8_t decimation)
{
    if (decimation == 0u) decimation = 1u;
    s_tWave.chDecimation    = decimation;
    s_tWave.chDecimCounter  = 0u;
}

static uint32_t mwaveform_SetStreamRate(uint32_t isrPeriodNs,
                                        uint32_t targetHz)
{
    if (isrPeriodNs == 0u) return 0u;

    uint32_t wIsrHz = 1000000000u / isrPeriodNs;
    uint32_t wDecimation = 1u;

    if (targetHz > 0u && targetHz < wIsrHz) {
        wDecimation = (wIsrHz + targetHz / 2u) / targetHz;
        if (wDecimation < 1u) wDecimation = 1u;
        if (wDecimation > 255u) wDecimation = 255u;
    }

    s_tWave.chDecimation   = (uint8_t)wDecimation;
    s_tWave.chDecimCounter = 0u;
    s_tWave.wIsrPeriodNs   = isrPeriodNs;
    s_tWave.wSamplePeriodNs = isrPeriodNs * wDecimation;
    s_tWave.bRequestMeta    = true;
    mwaveform_RecomputeChannelDividers();

    return mwaveform_GetStreamRateHz();
}

static uint32_t mwaveform_SetChannelRate(uint8_t chID, uint32_t hz)
{
    if (chID >= s_tWave.chCount) return 0u;

    s_tWave.awChannelRateHz[chID] = hz;
    mwaveform_RecomputeChannelDividers();

    uint32_t wStreamRateHz = mwaveform_GetStreamRateHz();
    if (hz == 0u) return wStreamRateHz;
    if (wStreamRateHz == 0u) return hz;
    return wStreamRateHz / s_tWave.achChannelDiv[chID];
}

static void mwaveform_SetSamplePeriodNs(uint32_t periodNs)
{
    if (periodNs == 0u) return;
    if (s_tWave.wSamplePeriodNs != periodNs) {
        s_tWave.wSamplePeriodNs = periodNs;
        s_tWave.bRequestMeta     = true;
        mwaveform_RecomputeChannelDividers();
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
    s_tWave.wDropCount        = 0;
    s_tWave.wTotalCount       = 0;
    s_tWave.wIntervalDropCount = 0;
    s_tWave.wLastIntervalDrops = 0;
    s_tWave.wRTTFullCount     = 0;
}

uint32_t mwaveform_GetRTTFullCount(void)
{
    return s_tWave.wRTTFullCount;
}

#if MWAVEFORM_SNAPSHOT_ENABLE
static int mwaveform_SnapshotStart(uint16_t depth, uint32_t periodNs)
{
    uint16_t hwMaxByRtt;

    if (depth == 0u || periodNs == 0u || s_tWave.chCount == 0u) {
        return MODUS_EINVAL;
    }

    hwMaxByRtt = (MWAVEFORM_RTT_BUFFER_SIZE - 21u) /
                 (MASK_BYTES + 2u * s_tWave.chCount);
    if (hwMaxByRtt == 0u) return MODUS_ENOMEM;
    if (depth > MWAVEFORM_SNAPSHOT_DEPTH) depth = MWAVEFORM_SNAPSHOT_DEPTH;
    if (depth > hwMaxByRtt) depth = hwMaxByRtt;

    uint32_t wState = perfc_port_disable_global_interrupt();
    s_tWave.hwSnapshotWrite    = 0u;
    s_tWave.hwSnapshotValidCount = 0u;
    s_tWave.wSnapshotSampleIndex = 0u;
    s_tWave.hwSnapshotDepth    = depth;
    s_tWave.wSnapshotPeriodNs  = periodNs;
    s_tWave.bSnapshotArmed     = true;
    s_tWave.bSnapshotReady     = false;
    s_tWave.bSnapshotPending   = false;
    perfc_port_resume_global_interrupt(wState);

    return MODUS_SUCCESS;
}

static void mwaveform_SnapshotCapture(void)
{
    uint8_t abMask[MASK_BYTES];

    memset(abMask, 0, MASK_BYTES);

    for (uint8_t i = 0; i < s_tWave.chCount; i++) {
        uint8_t bBit = (uint8_t)(1u << (i % 8));

        if (s_tWave.apvVariables[i] != NULL ||
            (s_tWave.abEverMask[i / 8] & bBit) != 0u) {
            abMask[i / 8] |= bBit;

            if (s_tWave.apvVariables[i] != NULL) {
                if (s_tWave.achVariableTypes[i] == MWAVEFORM_VAR_FLOAT) {
                    s_tWave.ahwSamples[i] = (int16_t)(
                        *(volatile float *)s_tWave.apvVariables[i] *
                        s_tWave.atChannels[i].fScale);
                } else {
                    s_tWave.ahwSamples[i] =
                        *(volatile int16_t *)s_tWave.apvVariables[i];
                }
            }
        }
    }

    if (s_tWave.hwSnapshotWrite >= s_tWave.hwSnapshotDepth) {
        s_tWave.hwSnapshotValidCount = s_tWave.hwSnapshotDepth;
    } else {
        s_tWave.hwSnapshotValidCount =
            (uint16_t)(s_tWave.hwSnapshotWrite + 1u);
    }

    uint16_t hwIdx = (uint16_t)(s_tWave.hwSnapshotWrite %
                                s_tWave.hwSnapshotDepth);
    s_tWave.atSnapshotSamples[hwIdx].wSampleIndex =
        s_tWave.wSnapshotSampleIndex++;
    memcpy(s_tWave.atSnapshotSamples[hwIdx].abMask, abMask, MASK_BYTES);
    memcpy(s_tWave.atSnapshotSamples[hwIdx].ahwSamples,
           s_tWave.ahwSamples,
           sizeof(s_tWave.ahwSamples[0]) * s_tWave.chCount);
    s_tWave.hwSnapshotWrite++;
}

static void mwaveform_SnapshotFeed(void)
{
    if (!s_tWave.bSnapshotArmed || s_tWave.bSnapshotReady ||
        !s_tWave.bIsRunning || s_tWave.hwSnapshotDepth == 0u) {
        return;
    }

    mwaveform_SnapshotCapture();
}

static int mwaveform_SnapshotTrigger(void)
{
    if (!s_tWave.bSnapshotArmed || s_tWave.bSnapshotReady ||
        s_tWave.bSnapshotPending ||
        s_tWave.hwSnapshotValidCount == 0u) {
        return MODUS_EBUSY;
    }

    uint32_t wState = perfc_port_disable_global_interrupt();
    if (!s_tWave.bSnapshotArmed || s_tWave.bSnapshotPending) {
        perfc_port_resume_global_interrupt(wState);
        return MODUS_EBUSY;
    }
    s_tWave.bSnapshotReady = true;
    perfc_port_resume_global_interrupt(wState);
    return MODUS_SUCCESS;
}

static void mwaveform_SnapshotStop(void)
{
    uint32_t wState = perfc_port_disable_global_interrupt();
    s_tWave.bSnapshotArmed = false;
    perfc_port_resume_global_interrupt(wState);
}

static int mwaveform_SnapshotIsArmed(void)
{
    return s_tWave.bSnapshotArmed && !s_tWave.bSnapshotReady;
}

static uint16_t mwaveform_GetSnapshotDepth(void)
{
    return s_tWave.hwSnapshotDepth;
}
#else
static int dummy_SnapshotStart(uint16_t depth, uint32_t periodNs)
{
    (void)depth;
    (void)periodNs;
    return MODUS_ENODEV;
}
static void dummy_SnapshotFeed(void) {}
static int dummy_SnapshotTrigger(void) { return MODUS_ENODEV; }
static void dummy_SnapshotStop(void) {}
static int dummy_SnapshotIsArmed(void) { return 0; }
static uint16_t dummy_GetSnapshotDepth(void) { return 0u; }
#endif

const mwaveform_api_t mwaveform = {
    .Init           = mwaveform_Init,
    .AddChannel     = mwaveform_AddChannel,
    .AddVariable    = mwaveform_AddVariable,
    .Start          = mwaveform_Start,
    .Stop           = mwaveform_Stop,
    .Push           = mwaveform_Push,
    .PushRaw        = mwaveform_PushRaw,
    .Step           = mwaveform_Step,
    .Poll           = mwaveform_Poll,
    .SetRate        = mwaveform_SetRate,
    .SetDecimation  = mwaveform_SetDecimation,
    .SetStreamRate  = mwaveform_SetStreamRate,
    .SetChannelRate = mwaveform_SetChannelRate,
    .SetSamplePeriodNs = mwaveform_SetSamplePeriodNs,
    .GetDropCount   = mwaveform_GetDropCount,
    .GetLastIntervalDrops = mwaveform_GetLastIntervalDrops,
    .GetRTTFullCount = mwaveform_GetRTTFullCount,
    .ClearDropCount = mwaveform_ClearDropCount,
#if MWAVEFORM_SNAPSHOT_ENABLE
    .SnapshotStart  = mwaveform_SnapshotStart,
    .SnapshotFeed   = mwaveform_SnapshotFeed,
    .SnapshotTrigger = mwaveform_SnapshotTrigger,
    .SnapshotStop   = mwaveform_SnapshotStop,
    .SnapshotIsArmed = mwaveform_SnapshotIsArmed,
    .GetSnapshotDepth = mwaveform_GetSnapshotDepth,
#else
    .SnapshotStart  = dummy_SnapshotStart,
    .SnapshotFeed   = dummy_SnapshotFeed,
    .SnapshotTrigger = dummy_SnapshotTrigger,
    .SnapshotStop   = dummy_SnapshotStop,
    .SnapshotIsArmed = dummy_SnapshotIsArmed,
    .GetSnapshotDepth = dummy_GetSnapshotDepth,
#endif
};

/* Weak callback — invoked by the MODUS 1kHz tick. */
__attribute__((weak)) void mwaveform_Default_Step_Callback(void) {
    s_tWave.wDefaultStepCount++;
    if (!s_tWave.bExternalDrive) {
        mwaveform.Step();
    }
}

/*============================ MSHELL INTEGRATION ============================*/
#include "mshell.h"

static void cmd_snapshot(const char *args)
{
    if (strncmp(args, "start", 5) == 0) {
        uint16_t hwDepth = MWAVEFORM_SNAPSHOT_DEPTH;
        uint32_t wPeriod = 50000u;
        const char *p = args + 5;

        while (*p == ' ') p++;
        if (*p >= '0' && *p <= '9') {
            hwDepth = 0u;
            while (*p >= '0' && *p <= '9') {
                hwDepth = (uint16_t)(hwDepth * 10u + (uint16_t)(*p++ - '0'));
            }
        }
        while (*p == ' ') p++;
        if (*p >= '0' && *p <= '9') {
            wPeriod = 0u;
            while (*p >= '0' && *p <= '9') {
                wPeriod = wPeriod * 10u + (uint32_t)(*p++ - '0');
            }
        }

        if (mwaveform.SnapshotStart(hwDepth, wPeriod) == MODUS_SUCCESS) {
            MLOGF(I, "Snapshot armed: depth %u period %lu ns\r\n",
                  (unsigned)mwaveform.GetSnapshotDepth(),
                  (unsigned long)wPeriod);
        } else {
            MLOG(E, "Snapshot start failed.\r\n");
        }
    } else if (strncmp(args, "trigger", 7) == 0) {
        if (mwaveform.SnapshotTrigger() == MODUS_SUCCESS) {
            MLOG(I, "Snapshot triggered.\r\n");
        } else {
            MLOG(W, "Snapshot not armed or busy.\r\n");
        }
    } else if (strncmp(args, "stop", 4) == 0) {
        mwaveform.SnapshotStop();
        MLOG(I, "Snapshot stopped.\r\n");
    } else if (strncmp(args, "status", 6) == 0) {
        MLOGF(I, "Snapshot armed: %d depth: %u valid: %u\r\n",
              mwaveform.SnapshotIsArmed(),
              mwaveform.GetSnapshotDepth(),
              s_tWave.hwSnapshotValidCount);
    } else {
        MLOG(I, "Usage: wave snap <start [depth period_ns]|trigger|stop|status>\r\n");
    }
}

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
            MLOGF(I, "Waveform decimation set to %lu.\r\n",
                  (unsigned long)wRate);
        }
    } else if (strncmp(args, "period", 6) == 0) {
        uint32_t wPeriod = 0;
        const char *p = args + 6;
        while (*p == ' ') p++;
        while (*p >= '0' && *p <= '9') {
            wPeriod = wPeriod * 10u + (uint32_t)(*p++ - '0');
        }
        if (wPeriod == 0u) {
            MLOG(I, "Usage: wave period <ns>\r\n");
        } else {
            mwaveform.SetSamplePeriodNs(wPeriod);
            MLOGF(I, "Waveform sample period set to %lu ns.\r\n",
                  (unsigned long)wPeriod);
        }
    } else if (strncmp(args, "stream", 6) == 0) {
        uint32_t wHz = 0;
        const char *p = args + 6;
        while (*p == ' ') p++;
        while (*p >= '0' && *p <= '9') {
            wHz = wHz * 10u + (uint32_t)(*p++ - '0');
        }
        if (wHz == 0u) {
            MLOG(I, "Usage: wave stream <hz>\r\n");
        } else {
            uint32_t wIsrPeriodNs = (s_tWave.wIsrPeriodNs != 0u)
                ? s_tWave.wIsrPeriodNs : 50000u;
            uint32_t wActual = mwaveform.SetStreamRate(wIsrPeriodNs, wHz);
            MLOGF(I, "Waveform stream rate set to %lu Hz.\r\n",
                  (unsigned long)wActual);
        }
    } else if (strncmp(args, "chrate", 6) == 0) {
        uint32_t wCh = 0;
        uint32_t wHz = 0;
        const char *p = args + 6;
        while (*p == ' ') p++;
        while (*p >= '0' && *p <= '9') {
            wCh = wCh * 10u + (uint32_t)(*p++ - '0');
        }
        while (*p == ' ') p++;
        while (*p >= '0' && *p <= '9') {
            wHz = wHz * 10u + (uint32_t)(*p++ - '0');
        }
        uint32_t wActual = mwaveform.SetChannelRate((uint8_t)wCh, wHz);
        MLOGF(I, "Channel %lu refresh rate set to %lu Hz.\r\n",
              (unsigned long)wCh, (unsigned long)wActual);
    } else if (strncmp(args, "snap", 4) == 0) {
        const char *p = args + 4;
        while (*p == ' ') p++;
        cmd_snapshot(p);
    } else if (strncmp(args, "status", 6) == 0) {
        MLOGF(I, "Waveform: running=%d external=%d rate=%lu Hz default_steps=%lu\r\n",
              s_tWave.bIsRunning ? 1 : 0,
              s_tWave.bExternalDrive ? 1 : 0,
              (unsigned long)mwaveform_GetStreamRateHz(),
              (unsigned long)s_tWave.wDefaultStepCount);
    } else if (strncmp(args, "rtt", 3) == 0) {
        MLOGF(I, "Waveform RTT full: %lu\r\n",
              (unsigned long)mwaveform.GetRTTFullCount());
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
        MLOG(I, "Usage: wave <start|stop|status|list|rate <n>|stream <hz>|chrate <ch> <hz>|period <ns>|snap|drop|rtt>\r\n");
    }
}

MODUS_SHELL_CMD(wave, cmd_wave, "Waveform control (start/stop/rate/stream/chrate/period/snap/drop)");

#else /* MWAVEFORM_ENABLE == 0 */

static int      dummy_Init(const mwaveform_protocol_t *ptProtocol)
{ (void)ptProtocol; return 0; }
static uint8_t  dummy_AddChannel(const char *pchName, float fScale)
{ (void)pchName; (void)fScale; return 0; }
static uint8_t  dummy_AddVariable(const char *pchName, float fScale,
                                  void *pvValue, uint8_t chType)
{ (void)pchName; (void)fScale; (void)pvValue; (void)chType; return 0xFF; }
static void     dummy_void(void) {}
static void     dummy_Push(uint8_t chID, float fValue)
{ (void)chID; (void)fValue; }
static void     dummy_PushRaw(uint8_t chID, int16_t hwValue)
{ (void)chID; (void)hwValue; }
static void     dummy_SetRate(uint32_t wDecimation)
{ (void)wDecimation; }
static void     dummy_SetDecimation(uint8_t decimation)
{ (void)decimation; }
static uint32_t dummy_SetStreamRate(uint32_t isrPeriodNs, uint32_t targetHz)
{ (void)isrPeriodNs; (void)targetHz; return 0; }
static uint32_t dummy_SetChannelRate(uint8_t chID, uint32_t hz)
{ (void)chID; (void)hz; return 0; }
static void     dummy_SetSamplePeriodNs(uint32_t periodNs)
{ (void)periodNs; }
static uint32_t dummy_GetDropCount(void) { return 0; }
static uint32_t dummy_GetLastIntervalDrops(void) { return 0; }
static uint32_t dummy_GetRTTFullCount(void) { return 0; }
static int      dummy_SnapshotStart(uint16_t depth, uint32_t periodNs)
{ (void)depth; (void)periodNs; return MODUS_ENODEV; }
static int      dummy_SnapshotTrigger(void) { return MODUS_ENODEV; }
static int      dummy_SnapshotIsArmed(void) { return 0; }

const mwaveform_api_t mwaveform = {
    .Init           = dummy_Init,
    .AddChannel     = dummy_AddChannel,
    .AddVariable    = dummy_AddVariable,
    .Start          = dummy_void,
    .Stop           = dummy_void,
    .Push           = dummy_Push,
    .PushRaw        = dummy_PushRaw,
    .Step           = dummy_void,
    .Poll           = dummy_void,
    .SetRate        = dummy_SetRate,
    .SetDecimation  = dummy_SetDecimation,
    .SetStreamRate  = dummy_SetStreamRate,
    .SetChannelRate = dummy_SetChannelRate,
    .SetSamplePeriodNs = dummy_SetSamplePeriodNs,
    .GetDropCount   = dummy_GetDropCount,
    .GetLastIntervalDrops = dummy_GetLastIntervalDrops,
    .GetRTTFullCount = dummy_GetRTTFullCount,
    .ClearDropCount = dummy_void,
    .SnapshotStart  = dummy_SnapshotStart,
    .SnapshotFeed   = dummy_void,
    .SnapshotTrigger = dummy_SnapshotTrigger,
    .SnapshotStop   = dummy_void,
    .SnapshotIsArmed = dummy_SnapshotIsArmed,
    .GetSnapshotDepth = dummy_GetSnapshotDepth,
};

__attribute__((weak)) void mwaveform_Default_Step_Callback(void) {}

#endif /* MWAVEFORM_ENABLE */
