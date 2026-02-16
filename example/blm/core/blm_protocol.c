/**
 * @file blm_protocol.c
 * @brief Ymodem Protocol Implementation using PERFC_PT State Machine
 * 
 * Follows perf_counter pt_example.c conventions
 */

/*============================ INCLUDES ======================================*/
#include "blm_protocol.h"
#include "../port/blm_port.h"
#include <string.h>
#include "utilities/util_debug.h"

#ifndef LOG_OUT
#define LOG_OUT(...)         TRACE_TOSTR(__VA_ARGS__)
#endif

#if defined(__IS_COMPILER_ARM_COMPILER_5__)
#   pragma diag_suppress 550, 546, 111
#endif

/*============================ MACROS ========================================*/
#undef this
#define this    (*ptThis)

/*============================ IMPLEMENTATION ================================*/

/**
 * @brief Initialize protocol control block
 */
blm_protocol_cb_t *blm_protocol_Init(blm_protocol_cb_t *ptThis, 
                                      uint8_t *pchBuffer, 
                                      uint16_t hwSize)
{
    if (NULL == ptThis || NULL == pchBuffer) {
        return NULL;
    }
    
    memset(ptThis, 0, sizeof(this));
    this.pchBuffer = pchBuffer;
    this.hwBufferSize = hwSize;
    this.chExpectedSeq = 0;
    
    return ptThis;
}

/**
 * @brief Send 'C' to request CRC mode
 */
int blm_protocol_SendC(void)
{
    uint8_t chC = YMODEM_C;
    return blm_port_UartSend(&chC, 1);
}

/**
 * @brief Send ACK
 */
int blm_protocol_SendAck(void)
{
    uint8_t chAck = YMODEM_ACK;
    return blm_port_UartSend(&chAck, 1);
}

/**
 * @brief Send NAK
 */
int blm_protocol_SendNak(void)
{
    uint8_t chNak = YMODEM_NAK;
    return blm_port_UartSend(&chNak, 1);
}

/**
 * @brief Send CAN (cancel) - send twice
 */
int blm_protocol_SendCancel(void)
{
    uint8_t achCan[2] = {YMODEM_CAN, YMODEM_CAN};
    return blm_port_UartSend(achCan, 2);
}

/**
 * @brief Calculate CRC16-CCITT
 */
uint16_t blm_protocol_Crc16(const uint8_t *pchData, uint16_t hwLen)
{
    uint16_t hwCrc = 0;
    
    while (hwLen--) {
        hwCrc ^= (uint16_t)(*pchData++) << 8;
        for (uint8_t i = 0; i < 8; i++) {
            if (hwCrc & 0x8000) {
                hwCrc = (hwCrc << 1) ^ 0x1021;
            } else {
                hwCrc <<= 1;
            }
        }
    }
    
    return hwCrc;
}

/**
 * @brief Receive and parse one packet using PERFC_PT state machine
 * 
 * Following perf_counter pt_example.c convention
 */
fsm_rt_t blm_protocol_ReceivePacket(blm_protocol_cb_t *ptThis, 
                                     uint8_t *pchData, 
                                     uint16_t *phwLen)
{
    int wRet;
    
    if (NULL == ptThis || NULL == pchData || NULL == phwLen) {
        return fsm_rt_err;
    }

PERFC_PT_BEGIN(this.chState)

    /* Reset state */
    *phwLen = 0;
    this.hwFrameIdx = 0;
    this.tResult = PROTO_TIMEOUT;
    this.lRecvStartMs = get_system_ms();
    
    /* Wait for packet header byte (with overall timeout) */
PERFC_PT_WAIT_UNTIL(
    (wRet > 0 || (get_system_ms() - this.lRecvStartMs) >= BLM_PACKET_TIMEOUT_MS),
    wRet = blm_port_UartRecv(&this.achFrame[0], 1, 10);
)
    
    /* Check if timed out without data */
    if (wRet <= 0) {
        this.tResult = PROTO_TIMEOUT;
        goto label_exit;
    }

    /* Check header byte */
    if (this.achFrame[0] == YMODEM_SOH) {
        /* SOH: 128 byte data only (STX not supported for size optimization) */
        uint16_t hwDataLen = 128;
        uint16_t hwFrameLen = 133; /* SOH + SEQ + ~SEQ + DATA[128] + CRC[2] */

        /* Use blocking read for remaining bytes */
        this.lRecvStartMs = get_system_ms();

        wRet = blm_port_UartRecv(&this.achFrame[1], hwFrameLen - 1, BLM_PACKET_TIMEOUT_MS);

        if (wRet < (hwFrameLen - 2)) {
            /* Allow CRC (len-1) or Checksum (len-2) */
            this.tResult = PROTO_TIMEOUT;
            goto label_exit;
        }
    } else if (this.achFrame[0] == YMODEM_EOT) {
        this.tResult = PROTO_EOT;
        goto label_exit;
    } else if (this.achFrame[0] == YMODEM_CAN) {
        /* Check for second CAN */
        PERFC_PT_WAIT_UNTIL(
            (wRet != 0),
            wRet = blm_port_UartRecv(&this.achFrame[1], 1, 100);
        )
        if (wRet > 0 && this.achFrame[1] == YMODEM_CAN) {
            this.tResult = PROTO_CANCEL;
        } else {
            this.tResult = PROTO_ERROR;
        }
        goto label_exit;
    } else {
        this.tResult = PROTO_ERROR;
        goto label_exit;
    }
    
    /* Verify sequence number */
    {
        uint8_t chSeq = this.achFrame[1];
        uint8_t chSeqInv = this.achFrame[2];
        
        if (chSeq != this.chExpectedSeq || (uint8_t)(chSeq + chSeqInv) != 0xFF) {
            this.tResult = PROTO_SEQ_ERROR;
            goto label_exit;
        }
    }
    
    /* Verify CRC */
    {
        uint16_t hwDataLen = 128;  /* Only SOH packets */
        uint16_t hwCrcCalc = blm_protocol_Crc16(&this.achFrame[3], hwDataLen);
        uint16_t hwCrcReceived = ((uint16_t)this.achFrame[hwDataLen + 3] << 8) | this.achFrame[hwDataLen + 4];

        if (hwCrcCalc != hwCrcReceived) {
            /* If CRC failed, try Checksum (for compatibility if wRet was len-2) */
            uint8_t chSum = 0;
            for (uint16_t i = 0; i < hwDataLen; i++) chSum += this.achFrame[3+i];
            if (chSum != this.achFrame[hwDataLen + 3]) {
                this.tResult = PROTO_CRC_ERROR;
                goto label_exit;
            }
        }

        /* Copy data and update sequence */
        memcpy(pchData, &this.achFrame[3], hwDataLen);
        *phwLen = hwDataLen;
        this.chExpectedSeq++;
        this.tResult = PROTO_OK;
    }

label_exit:

    // Removed R: log

PERFC_PT_END()

    return fsm_rt_cpl;
}

/**
 * @brief Parse file info from packet 0
 */
int blm_protocol_ParseFileInfo(blm_protocol_cb_t *ptThis, const uint8_t *pchData)
{
    if (NULL == ptThis || NULL == pchData) {
        return -1;
    }
    
    /* Packet 0 format: filename\0filesize\0 */
    const char *pchFileName = (const char *)pchData;
    
    LOG_OUT("FI");
    LOG_OUT(pchData[0]);
    LOG_OUT(pchData[1]);
    LOG_OUT("\r\n");

    uint16_t hwNameLen = strlen(pchFileName);
    
    if (hwNameLen == 0) {
        LOG_OUT("EN\r\n");
        /* Empty filename - end of batch */
        this.wFileSize = 0;
        return 0;
    }
    
    /* Copy filename */
    if (hwNameLen < sizeof(this.achFileName)) {
        memcpy(this.achFileName, pchFileName, hwNameLen + 1);
    }
    
    /* Parse file size */
    const char *pchSizeStr = pchFileName + hwNameLen + 1;
    this.wFileSize = 0;
    while (*pchSizeStr >= '0' && *pchSizeStr <= '9') {
        this.wFileSize = this.wFileSize * 10 + (*pchSizeStr - '0');
        pchSizeStr++;
    }
    
    LOG_OUT("SZ"); LOG_OUT((int)this.wFileSize); LOG_OUT("\r\n");
    
    return 0;
}
