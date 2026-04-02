#ifndef __GSTORAGE_H__
#define __GSTORAGE_H__

#include <stdint.h>
#include "gbase.h"
#include "gdi/gdi.h"

/**
 * @brief Storage data descriptor
 *
 * ptFlash      — GDI Flash device pointer (chip implements gdi_flash_t)
 * wFlashAddr   — Flash address where data is stored
 * pchStorageStartAddr — RAM buffer pointer (data mirror in RAM)
 * hwStorageLength     — Length of user data (bytes, excluding CRC)
 */
typedef struct {
    gdi_flash_t *ptFlash;           /**< GDI Flash device               */
    uint32_t     wFlashAddr;        /**< Flash storage start address     */
    uint8_t     *pchStorageStartAddr; /**< RAM buffer pointer            */
    uint16_t     hwStorageLength;   /**< User data length (excl. CRC)   */
} gstorage_data_t;

typedef struct {
    gstorage_data_t *ptStorageObject;
    uint16_t         hwStorageTimeOut;
} gstorage_cfg_t;

typedef struct {
    gmsi_base_t     *ptBase;
    gstorage_data_t *ptStorageObject;
    uint32_t         wTimer;
    uint16_t         hwStorageTimeOut;
    uint16_t         hwLastCrc;
    uint8_t          chStorageFlag;
} gstorage_t;

int gstorage_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

/**
 * @brief 设置模块级默认 Flash 设备。
 *
 * 由 gmsi_Init 内部调用。当 gstorage_data_t.ptFlash == NULL 时，
 * gstorage_Init 使用此默认值。
 */
void gstorage_SetDefaultFlash(void *ptFlash);


#endif  /* __GSTORAGE_H__ */