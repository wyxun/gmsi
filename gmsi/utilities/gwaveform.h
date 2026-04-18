#ifndef __GMSI_GWAVEFORM_H__
#define __GMSI_GWAVEFORM_H__

#include <stdint.h>
#include <stdbool.h>
#include "global_define.h"

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

#define gwaveform_Push(CH_ID, VALUE)      __gwaveform_Push(CH_ID, VALUE)
#define gwaveform_PushRaw(CH_ID, HW_VAL)  __gwaveform_PushRaw(CH_ID, HW_VAL)
#define gwaveform_Commit()                __gwaveform_Commit()

/*============================ TYPES =========================================*/

/*============================ PROTOTYPES ====================================*/

/**
 * @brief Initialize waveform module. 
 *        Will be called by gmsi_Init via INIT_SECTION.
 */
int gwaveform_Init(uintptr_t wAddr, uintptr_t wUnused);

/**
 * @brief Register a channel
 * @param pchName Name (max 7 bytes)
 * @param fScale  Scaling factor (float value * fScale = int16_t storage)
 * @return uint8_t Channel ID, 0xFF on failure
 */
uint8_t gwaveform_AddChannel(const char *pchName, float fScale);

/**
 * @brief Manually start/stop streaming
 */
void gwaveform_Start(void);
void gwaveform_Stop(void);

/**
 * @brief Poll to carry data to RTT. Called automatically by gmsi_Run().
 */
void gwaveform_Poll(void);

/**
 * @brief Internal push functions
 */
void __gwaveform_Push(uint8_t chID, float fValue);
void __gwaveform_PushRaw(uint8_t chID, int16_t hwValue);
void __gwaveform_Commit(void);

#else /* GWAVEFORM_ENABLE == 0 */

#define gwaveform_Push(...)
#define gwaveform_PushRaw(...)
#define gwaveform_Commit()
#define gwaveform_Init(...)
#define gwaveform_Start()
#define gwaveform_Stop()
#define gwaveform_Poll()

#endif /* GWAVEFORM_ENABLE */

#endif /* __GMSI_GWAVEFORM_H__ */
