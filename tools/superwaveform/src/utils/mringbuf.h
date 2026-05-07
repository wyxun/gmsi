#ifndef __MODUS_MRINGBUF_H__
#define __MODUS_MRINGBUF_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef MODUS_SUCCESS
#define MODUS_SUCCESS 0
#endif

#ifndef MODUS_EINVAL
#define MODUS_EINVAL -1
#endif

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
} mringbuf_t;

/*============================ PROTOTYPES ====================================*/

/**
 * @brief Initialize ring buffer
 * @param ptRB Pointer to ring buffer structure
 * @param pchBuf Pointer to data buffer
 * @param hwLen Length of buffer, MUST be power of 2
 * @return int MODUS_SUCCESS on success, MODUS_EINVAL if hwLen is not power of 2
 */
int mringbuf_Init(mringbuf_t *ptRB, uint8_t *pchBuf, uint16_t hwLen);

/**
 * @brief Write a byte to ring buffer
 * @return uint16_t Number of bytes written (0 or 1)
 */
uint16_t mringbuf_Write(mringbuf_t *ptRB, uint8_t chByte);

/**
 * @brief Read a byte from ring buffer
 * @return uint16_t Number of bytes read (0 or 1)
 */
uint16_t mringbuf_Read(mringbuf_t *ptRB, uint8_t *pchByte);

/**
 * @brief Bulk write to ring buffer
 * @return uint16_t Number of bytes successfully written
 */
uint16_t mringbuf_WriteBulk(mringbuf_t *ptRB, const uint8_t *pchData, uint16_t hwLen);

/**
 * @brief Bulk read from ring buffer
 * @return uint16_t Number of bytes successfully read
 */
uint16_t mringbuf_ReadBulk(mringbuf_t *ptRB, uint8_t *pchData, uint16_t hwLen);

/**
 * @brief Get used space in bytes
 */
uint16_t mringbuf_GetUsed(mringbuf_t *ptRB);

/**
 * @brief Get free space in bytes
 */
uint16_t mringbuf_GetFree(mringbuf_t *ptRB);

#endif /* __MODUS_MRINGBUF_H__ */
