#ifndef __GMSI_GWAVEFORM_H__
#define __GMSI_GWAVEFORM_H__

#include <stdint.h>
#include <stdbool.h>
#include "global_define.h"
#include "gwaveform_protocol.h"

#ifdef GMSI_CFG_USER_CONFIG_INCLUSION
#   include GMSI_CFG_USER_CONFIG_INCLUSION
#endif

/*============================ CONFIGURATION =================================*/

#ifndef GWAVEFORM_ENABLE
#   define GWAVEFORM_ENABLE             0
#endif

#ifndef GWAVEFORM_MAX_CHANNELS
#   define GWAVEFORM_MAX_CHANNELS       16
#endif

#ifndef GWAVEFORM_RTT_BUFFER_SIZE
#   define GWAVEFORM_RTT_BUFFER_SIZE    1024        /* Default RTT buffer size */
#endif

#ifndef GWAVEFORM_FIFO_DEPTH
#   define GWAVEFORM_FIFO_DEPTH         16          /* Default FIFO depth */
#endif


/* Max data frame size: SYNC(2)+SEQ(1)+MASK+DATA+CRC(1) */
#define GWAVEFORM_FRAME_SIZE \
    (3 + ((GWAVEFORM_MAX_CHANNELS + 7) / 8) + 2 * GWAVEFORM_MAX_CHANNELS + 1)

#ifndef GWAVEFORM_RTT_CHANNEL
#   define GWAVEFORM_RTT_CHANNEL        1
#endif

#ifndef GWAVEFORM_DECIMATION
#   define GWAVEFORM_DECIMATION         1
#endif

/*============================ MACROS ========================================*/

#if GWAVEFORM_ENABLE

/*============================ TYPES =========================================*/

typedef struct {
    int      (*Init)(const gwaveform_protocol_t *ptProtocol);
    uint8_t  (*AddChannel)(const char *pchName, float fScale);
    void     (*Start)(void);
    void     (*Stop)(void);
    void     (*Push)(uint8_t chID, float fValue);
    void     (*PushRaw)(uint8_t chID, int16_t hwValue);
    void     (*Step)(void);
    void     (*Poll)(void);
    void     (*SetRate)(uint32_t wDecimation);  /* 0=external drive, n=every n-th Step call sends */
    uint32_t (*GetDropCount)(void);             /* cumulative overwritten frames */
    uint32_t (*GetLastIntervalDrops)(void);     /* frames dropped in the last 1s window */
    void     (*ClearDropCount)(void);

} gwaveform_api_t;

extern const gwaveform_api_t gwaveform;

/*============================ PROTOTYPES ====================================*/

#else /* GWAVEFORM_ENABLE == 0 */

typedef struct {
    int      (*Init)(const gwaveform_protocol_t *ptProtocol);
    uint8_t  (*AddChannel)(const char *pchName, float fScale);
    void     (*Start)(void);
    void     (*Stop)(void);
    void     (*Push)(uint8_t chID, float fValue);
    void     (*PushRaw)(uint8_t chID, int16_t hwValue);
    void     (*Step)(void);
    void     (*Poll)(void);
    void     (*SetRate)(uint32_t wDecimation);
    uint32_t (*GetDropCount)(void);
    void     (*ClearDropCount)(void);
} gwaveform_api_t;

extern const gwaveform_api_t gwaveform;

#endif /* GWAVEFORM_ENABLE */

#endif /* __GMSI_GWAVEFORM_H__ */
