/**
 * @file blm_protocol.h
 * @brief Ymodem Protocol for BLM Bootloader
 * 
 * Uses PERFC_PT state machine following perf_counter example conventions
 */
#ifndef __BLM_PROTOCOL_H__
#define __BLM_PROTOCOL_H__

/*============================ INCLUDES ======================================*/
#include <stdint.h>
#include <perf_counter.h>
#include "../userconfig.h"

/*============================ MACROS ========================================*/

/* Ymodem Control Characters */
#define YMODEM_SOH      0x01    // 128 byte data packet
#define YMODEM_STX      0x02    // 1024 byte data packet (NOT SUPPORTED - size optimization)
#define YMODEM_EOT      0x04    // End of transmission
#define YMODEM_ACK      0x06    // Acknowledge
#define YMODEM_NAK      0x15    // Negative acknowledge
#define YMODEM_CAN      0x18    // Cancel transfer
#define YMODEM_C        0x43    // 'C' - CRC mode request

#define YMODEM_DATA_SIZE    128     /* Only SOH packets (128 bytes) */
#define YMODEM_FRAME_SIZE   133     /* SOH + SEQ + ~SEQ + DATA[128] + CRC[2] */

/*============================ TYPES =========================================*/

/**
 * @brief Protocol Results
 */
typedef enum {
    PROTO_OK = 0,
    PROTO_TIMEOUT,
    PROTO_CRC_ERROR,
    PROTO_SEQ_ERROR,
    PROTO_CANCEL,
    PROTO_EOT,
    PROTO_ERROR
} blm_proto_result_t;

/**
 * @brief Protocol Control Block (following pt_example convention)
 */
typedef struct {
    /* PT state machine */
    uint8_t chState;
    
    /* Protocol context */
    uint8_t *pchBuffer;
    uint16_t hwBufferSize;
    uint32_t wFileSize;
    uint32_t wReceivedSize;
    uint8_t chExpectedSeq;
    uint8_t achFileName[32];  /* Reduced from 64 for size optimization */
    
    /* Frame receive state */
    uint8_t achFrame[YMODEM_FRAME_SIZE];
    uint16_t hwFrameIdx;
    
    /* Receive timeout tracking */
    int64_t lRecvStartMs;
    
    /* Result of last receive */
    blm_proto_result_t tResult;
} blm_protocol_cb_t;

/*============================ PROTOTYPES ====================================*/

/**
 * @brief Initialize protocol control block
 */
extern
blm_protocol_cb_t *blm_protocol_Init(blm_protocol_cb_t *ptThis, 
                                      uint8_t *pchBuffer, 
                                      uint16_t hwSize);

/**
 * @brief Send 'C' to request CRC mode
 */
extern int blm_protocol_SendC(void);

/**
 * @brief Send ACK
 */
extern int blm_protocol_SendAck(void);

/**
 * @brief Send NAK
 */
extern int blm_protocol_SendNak(void);

/**
 * @brief Send CAN (cancel)
 */
extern int blm_protocol_SendCancel(void);

/**
 * @brief Receive and parse one packet (PT state machine)
 * @param ptThis Protocol control block
 * @param pchData Output data buffer (128 bytes)
 * @param phwLen Output data length
 * @return fsm_rt_t state machine result
 */
extern
fsm_rt_t blm_protocol_ReceivePacket(blm_protocol_cb_t *ptThis, 
                                     uint8_t *pchData, 
                                     uint16_t *phwLen);

/**
 * @brief Parse file info from packet 0
 */
extern
int blm_protocol_ParseFileInfo(blm_protocol_cb_t *ptThis, const uint8_t *pchData);

/**
 * @brief Calculate CRC16-CCITT
 */
extern
uint16_t blm_protocol_Crc16(const uint8_t *pchData, uint16_t hwLen);

#endif  /* __BLM_PROTOCOL_H__ */
