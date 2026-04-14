#include "gstorage.h"
#include "glog.h"
#include "utilities/util_debug.h"
#include <string.h>

int gstorage_Run(uintptr_t wObjectAddr);
int gstorage_Clock(uintptr_t wObjectAddr);

static uint16_t gstorage_CalculateCrc16(uint8_t *pchData, uint16_t hwLen)
{
    uint16_t hwCrc = 0xFFFF;
    for (uint16_t i = 0; i < hwLen; i++) {
        hwCrc ^= (uint16_t)pchData[i];
        for (int j = 0; j < 8; j++) {
            if (hwCrc & 0x0001) {
                hwCrc = (hwCrc >> 1) ^ 0xA001; // CRC-16-Modbus
            } else {
                hwCrc >>= 1;
            }
        }
    }
    return hwCrc;
}

// Define a global storage base of type gmsi_base_t
static gmsi_base_t s_tStorageBase;
// Define and initialize a global storage base configuration of type gmsi_base_cfg_t
gmsi_base_cfg_t s_tStorageBaseCfg = {
    .wId = GMSI_STORAGE,
    .wParent = 0,
    .FcnInterface = {
        .Clock = gstorage_Clock,
        .Run   = gstorage_Run,
    },
};

/** @brief 模块级默认 Flash 设备，由 gmsi_Init 通过 gstorage_SetDefaultFlash() 注入 */
static gdi_flash_t *s_ptDefaultFlash = NULL;

void gstorage_SetDefaultFlash(void *ptFlash)
{
    s_ptDefaultFlash = (gdi_flash_t *)ptFlash;
}

/**
 * @brief Handle storage events.
 */
void gstorage_EventHandle(gstorage_t *ptThis, uint32_t wEvent)
{
    if (ptThis == NULL || ptThis->ptStorageObject == NULL) {
        GLOG(E, "Error: ptThis or ptThis->ptStorageObject is NULL.\n");
        return;
    }

    gstorage_data_t *ptObj  = ptThis->ptStorageObject;
    gdi_flash_t     *ptFlash = ptObj->ptFlash;
    uint32_t         wAddr   = ptObj->wFlashAddr;
    uint16_t         len     = ptObj->hwStorageLength;

    if (wEvent & Event_Storage)
    {
        uint16_t hwCrc = gstorage_CalculateCrc16(ptObj->pchStorageStartAddr, len);

        /* Append CRC at the end (2 bytes, little-endian) */
        ptObj->pchStorageStartAddr[len]     = (uint8_t)(hwCrc & 0xFF);
        ptObj->pchStorageStartAddr[len + 1] = (uint8_t)((hwCrc >> 8) & 0xFF);

        GLOGF(I, "GStorage: Saving data, CRC: 0x%04X\n", hwCrc);

        gdi_flash_Unlock(ptFlash);
        gdi_flash_Erase(ptFlash, wAddr, (uint32_t)(len + 2));
        int32_t nRet = gdi_flash_Write(ptFlash, wAddr, ptObj->pchStorageStartAddr, (uint32_t)(len + 2));
        gdi_flash_Lock(ptFlash);

        if (nRet >= 0) {
            ptThis->hwLastCrc = hwCrc;
        } else {
            GLOG(E, "GStorage: Write Failed!\n");
        }
    }

    if (wEvent & Event_ResetDefault)
    {
        GLOG(W, "GStorage: Blanking Flash (RAM untouched)...\n");
        /* 只清空 Flash，不修改 RAM。
         * 系统继续以当前 RAM 数据运行，硬件行为不受影响。
         * 下次上电时 gstorage_Init 检测到 Flash 全 FF，
         * 会将编译期静态默认值保存到 Flash 并恢复。 */
        gdi_flash_Unlock(ptFlash);
        gdi_flash_Erase(ptFlash, wAddr, (uint32_t)(len + 2));
        gdi_flash_Lock(ptFlash);

        /* 同步 hwLastCrc 到当前 RAM 的 CRC，
         * 防止 Clock 检测到"CRC 变化"立即把当前 RAM 重写回 Flash。 */
        ptThis->hwLastCrc = gstorage_CalculateCrc16(
            ptObj->pchStorageStartAddr, len);
        GLOG(I, "GStorage: Flash blanked. Reboot to apply defaults.\n");
    }

    if (wEvent & Event_GetData)
    {
        GLOG(I, "GStorage: Loading data...\n");
        int32_t nRet = gdi_flash_Read(ptFlash, wAddr, ptObj->pchStorageStartAddr, (uint32_t)(len + 2));

        if (nRet >= 0) {
            uint16_t hwReadCrc = ptObj->pchStorageStartAddr[len];
            hwReadCrc |= (uint16_t)(ptObj->pchStorageStartAddr[len + 1] << 8);

            uint16_t hwCalcCrc = gstorage_CalculateCrc16(ptObj->pchStorageStartAddr, len);

            if (hwReadCrc == hwCalcCrc) {
                ptThis->hwLastCrc = hwReadCrc;
                GLOG(I, "GStorage: Load Success, CRC Match.\n");
            } else {
                GLOGF(E, "GStorage: CRC Mismatch! Read: 0x%04X, Calc: 0x%04X\n", hwReadCrc, hwCalcCrc);
            }
        }
    }
}

/**
 * @brief Run the storage object.
 */
int gstorage_Run(uintptr_t wObjectAddr)
{
    gstorage_t *ptThis = (gstorage_t *)wObjectAddr;

    if (ptThis == NULL || ptThis->ptBase == NULL) {
        GLOG(E, "Error: ptThis or ptThis->ptBase is NULL.\n");
        return GMSI_EFAIL;
    }

    uint32_t wEvent = gbase_EventPend(ptThis->ptBase);
    if (wEvent) {
        gstorage_EventHandle(ptThis, wEvent);
    }

    return GMSI_SUCCESS;
}

/**
 * @brief Clock task for storage object.
 */
int gstorage_Clock(uintptr_t wObjectAddr)
{
    gstorage_t *ptThis = (gstorage_t *)wObjectAddr;

    if (ptThis == NULL || ptThis->ptStorageObject == NULL) {
        return GMSI_EFAIL;
    }

    if (ptThis->hwStorageTimeOut == 0) {
        return GMSI_SUCCESS;
    }

    if (ptThis->wTimer > 0) {
        ptThis->wTimer--;
    } else {
        ptThis->wTimer = ptThis->hwStorageTimeOut;

        uint16_t hwCurrentCrc = gstorage_CalculateCrc16(
            ptThis->ptStorageObject->pchStorageStartAddr,
            ptThis->ptStorageObject->hwStorageLength
        );

        if (hwCurrentCrc != ptThis->hwLastCrc) {
            gbase_EventPost(ptThis->ptBase->wId, Event_Storage);
        }
    }

    return GMSI_SUCCESS;
}

/**
 * @brief Initialize the storage object.
 */
int gstorage_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    gstorage_t     *ptThis = (gstorage_t *)wObjectAddr;
    gstorage_cfg_t *ptCfg  = (gstorage_cfg_t *)wObjectCfgAddr;

    if (ptThis == NULL || ptCfg == NULL) {
        GLOG(E, "Error: ptThis or ptCfg is NULL.\n");
        return GMSI_EFAIL;
    }

    ptThis->ptStorageObject  = ptCfg->ptStorageObject;
    ptThis->ptBase           = &s_tStorageBase;
    ptThis->hwStorageTimeOut = ptCfg->hwStorageTimeOut;
    ptThis->wTimer           = ptCfg->hwStorageTimeOut;

    s_tStorageBaseCfg.wParent = wObjectAddr;
    int wRet = gbase_Init(ptThis->ptBase, &s_tStorageBaseCfg);
    if (wRet < 0) {
        GLOGF(E, "gbase_Init failed: %d\n", wRet);
        return wRet;
    }

    gstorage_data_t *ptObj  = ptThis->ptStorageObject;

    /* 若配置时 ptFlash 未填，使用 gmsi_Init 注入的全局默认值 */
    if (ptObj != NULL && ptObj->ptFlash == NULL) {
        ptObj->ptFlash = s_ptDefaultFlash;
    }

    gdi_flash_t     *ptFlash = ptObj ? ptObj->ptFlash : NULL;

    if (ptObj == NULL || ptFlash == NULL) {
        return GMSI_SUCCESS; /* No flash configured, skip init read */
    }

    uint32_t wAddr = ptObj->wFlashAddr;
    uint16_t len   = ptObj->hwStorageLength;

    /* Initial read from Flash */
    gdi_flash_Read(ptFlash, wAddr, ptObj->pchStorageStartAddr, (uint32_t)(len + 2));

    bool bIsBlank = true;
    for (uint16_t i = 0; i < len + 2; i++) {
        if (ptObj->pchStorageStartAddr[i] != 0xFF) {
            bIsBlank = false;
            break;
        }
    }

    if (bIsBlank) {
        GLOG(W, "GStorage: Storage blank. Initializing defaults...\n");
        gbase_EventPost(ptThis->ptBase->wId, Event_Storage);
    } else {
        uint16_t hwReadCrc = ptObj->pchStorageStartAddr[len];
        hwReadCrc |= (uint16_t)(ptObj->pchStorageStartAddr[len + 1] << 8);

        uint16_t hwCalcCrc = gstorage_CalculateCrc16(ptObj->pchStorageStartAddr, len);

        if (hwReadCrc == hwCalcCrc) {
            ptThis->hwLastCrc = hwReadCrc;
            GLOGF(I, "GStorage: Load Success, CRC: 0x%04X\n", hwReadCrc);
        } else {
            GLOGF(E, "GStorage: CRC Error! Calc: 0x%04X, Stored: 0x%04X\n", hwCalcCrc, hwReadCrc);
        }
    }

    return GMSI_SUCCESS;
}