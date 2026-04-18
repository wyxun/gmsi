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

#ifndef GWAVEFORM_RING_BUFFER_SIZE
#   define GWAVEFORM_RING_BUFFER_SIZE   8192
#endif

#ifndef GWAVEFORM_RTT_BUFFER_SIZE
#   define GWAVEFORM_RTT_BUFFER_SIZE    8192
#endif

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
    int     (*Init)(const gwaveform_protocol_t *ptProtocol);
    uint8_t (*AddChannel)(const char *pchName, float fScale);
    void    (*Start)(void);
    void    (*Stop)(void);
    void    (*Push)(uint8_t chID, float fValue);
    void    (*PushRaw)(uint8_t chID, int16_t hwValue);
    void    (*Step)(void);
    void    (*Poll)(void);
} gwaveform_api_t;

extern const gwaveform_api_t gwaveform;

/*============================ PROTOTYPES ====================================*/

#else /* GWAVEFORM_ENABLE == 0 */

typedef struct {
    int     (*Init)(const gwaveform_protocol_t *ptProtocol);
    uint8_t (*AddChannel)(const char *pchName, float fScale);
    void    (*Start)(void);
    void    (*Stop)(void);
    void    (*Push)(uint8_t chID, float fValue);
    void    (*PushRaw)(uint8_t chID, int16_t hwValue);
    void    (*Step)(void);
    void    (*Poll)(void);
} gwaveform_api_t;

extern const gwaveform_api_t gwaveform;

#endif /* GWAVEFORM_ENABLE */

#endif /* __GMSI_GWAVEFORM_H__ */
