#ifndef __GMSI_GRINGBUF_H__
#define __GMSI_GRINGBUF_H__

#include <stdint.h>
#include <stdbool.h>

/*============================ MACROS ========================================*/

/*============================ TYPES =========================================*/

/**
 * @brief High performance SPSC lock-free ring buffer
 */
typedef struct {
    uint8_t             *pchBuffer;
    volatile uint16_t    hwHead;    /* Write pointer */
    volatile uint16_t    hwTail;    /* Read pointer */
    uint16_t             hwSizeMask;/* length - 1, length must be 2^n */
} gringbuf_t;

/*============================ PROTOTYPES ====================================*/

/**
 * @brief Initialize ring buffer
 * @param ptRB Pointer to ring buffer structure
 * @param pchBuf Pointer to data buffer
 * @param hwLen Length of buffer, MUST be power of 2
 * @return int GMSI_SUCCESS on success, GMSI_EINVAL if hwLen is not power of 2
 */
int gringbuf_Init(gringbuf_t *ptRB, uint8_t *pchBuf, uint16_t hwLen);

/**
 * @brief Write a byte to ring buffer
 * @return uint16_t Number of bytes written (0 or 1)
 */
uint16_t gringbuf_Write(gringbuf_t *ptRB, uint8_t chByte);

/**
 * @brief Read a byte from ring buffer
 * @return uint16_t Number of bytes read (0 or 1)
 */
uint16_t gringbuf_Read(gringbuf_t *ptRB, uint8_t *pchByte);

/**
 * @brief Bulk write to ring buffer
 * @return uint16_t Number of bytes successfully written
 */
uint16_t gringbuf_WriteBulk(gringbuf_t *ptRB, const uint8_t *pchData, uint16_t hwLen);

/**
 * @brief Bulk read from ring buffer
 * @return uint16_t Number of bytes successfully read
 */
uint16_t gringbuf_ReadBulk(gringbuf_t *ptRB, uint8_t *pchData, uint16_t hwLen);

/**
 * @brief Peek data from ring buffer without moving tail
 * @return uint16_t Number of bytes successfully peeked
 */
uint16_t gringbuf_PeekBulk(gringbuf_t *ptRB, uint8_t *pchData, uint16_t hwLen);

/**
 * @brief Get used space in bytes
 */
uint16_t gringbuf_GetUsed(gringbuf_t *ptRB);

/**
 * @brief Get free space in bytes
 */
uint16_t gringbuf_GetFree(gringbuf_t *ptRB);

#endif /* __GMSI_GRINGBUF_H__ */
