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
    // Set the ID to GMSI_STORAGE
    .wId = GMSI_STORAGE,
    .wParent = 0,
    // Set the interface functions
    .FcnInterface = {
        .Clock = gstorage_Clock,
        .Run = gstorage_Run,
    },
};

/**
 * @brief Handle storage events.
 */
void gstorage_EventHandle(gstorage_t *ptThis, uint32_t wEvent)
{
    if (ptThis == NULL || ptThis->ptStorageObject == NULL) {
        GLOG_PRINTF("Error: ptThis or ptThis->ptStorageObject is NULL.\n");
        return;
    }

    if (wEvent & Event_Storage)
    {
        uint16_t hwCrc = gstorage_CalculateCrc16(
            ptThis->ptStorageObject->pchStorageStartAddr, 
            ptThis->ptStorageObject->hwStorageLength
        );
        
        // Append CRC at the end (2 bytes)
        uint16_t len = ptThis->ptStorageObject->hwStorageLength;
        ptThis->ptStorageObject->pchStorageStartAddr[len] = 
            (uint8_t)(hwCrc & 0xFF);
        ptThis->ptStorageObject->pchStorageStartAddr[len + 1] = 
            (uint8_t)((hwCrc >> 8) & 0xFF);
        
        LOG_OUT("GStorage: Saving data, CRC: ");
        LOG_OUT(hwCrc);
        LOG_OUT("\n");

        if (GMSI_SUCCESS == ptThis->ptStorageObject->fcnWrite(
                ptThis->ptStorageObject->pchStorageStartAddr, 
                len + 2)) {
            ptThis->hwLastCrc = hwCrc;
        } else {
            LOG_OUT("GStorage: Write Failed!\n");
        }
    }
    
    if (wEvent & Event_GetData)
    {
        LOG_OUT("GStorage: Loading data...\n");
        uint16_t len = ptThis->ptStorageObject->hwStorageLength;
        if (GMSI_SUCCESS == ptThis->ptStorageObject->fcnRead(
                ptThis->ptStorageObject->pchStorageStartAddr, 
                len + 2)) {
            
            uint16_t hwReadCrc = 
                ptThis->ptStorageObject->pchStorageStartAddr[len];
            hwReadCrc |= (uint16_t)(
                ptThis->ptStorageObject->pchStorageStartAddr[len + 1] 
                << 8);
            
            uint16_t hwCalcCrc = gstorage_CalculateCrc16(
                ptThis->ptStorageObject->pchStorageStartAddr, 
                len
            );
            
            if (hwReadCrc == hwCalcCrc) {
                ptThis->hwLastCrc = hwReadCrc;
                LOG_OUT("GStorage: Load Success, CRC Match.\n");
            } else {
                LOG_OUT("GStorage: CRC Mismatch! Read: ");
                LOG_OUT(hwReadCrc);
                LOG_OUT(", Calc: ");
                LOG_OUT(hwCalcCrc);
                LOG_OUT("\n");
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
        GLOG_PRINTF("Error: ptThis or ptThis->ptBase is NULL.\n");
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
    gstorage_t *ptThis = (gstorage_t *)wObjectAddr;
    gstorage_cfg_t *ptCfg = (gstorage_cfg_t *)wObjectCfgAddr;

    if (ptThis == NULL || ptCfg == NULL) {
        GLOG_PRINTF("Error: ptThis or ptCfg is NULL.");
        return GMSI_EFAIL;
    }

    ptThis->ptStorageObject = ptCfg->ptStorageObject;
    ptThis->ptBase = &s_tStorageBase;
    ptThis->hwStorageTimeOut = ptCfg->hwStorageTimeOut;
    ptThis->wTimer = ptCfg->hwStorageTimeOut;

    s_tStorageBaseCfg.wParent = wObjectAddr;
    int wRet = gbase_Init(ptThis->ptBase, &s_tStorageBaseCfg);
    if (wRet < 0) {
        GERR_PRINTF(wRet);
        return wRet;
    }

    if (ptThis->ptStorageObject && ptThis->ptStorageObject->fcnRead) {
        uint16_t len = ptThis->ptStorageObject->hwStorageLength;
        ptThis->ptStorageObject->fcnRead(
            ptThis->ptStorageObject->pchStorageStartAddr, 
            len + 2
        );
        
        bool bIsBlank = true;
        for (uint16_t i = 0; i < len + 2; i++) {
            if (ptThis->ptStorageObject->pchStorageStartAddr[i] != 0xFF) {
                bIsBlank = false;
                break;
            }
        }

        if (bIsBlank) {
            LOG_OUT("GStorage: Storage blank. Initializing defaults...\n");
            gbase_EventPost(ptThis->ptBase->wId, Event_Storage);
        } else {
            uint16_t hwReadCrc = 
                ptThis->ptStorageObject->pchStorageStartAddr[len];
            hwReadCrc |= (uint16_t)(
                ptThis->ptStorageObject->pchStorageStartAddr[len + 1] 
                << 8);
            
            uint16_t hwCalcCrc = gstorage_CalculateCrc16(
                ptThis->ptStorageObject->pchStorageStartAddr, 
                len
            );
            
            if (hwReadCrc == hwCalcCrc) {
                ptThis->hwLastCrc = hwReadCrc;
                LOG_OUT("GStorage: Load Success, CRC: ");
                LOG_OUT(hwReadCrc);
                LOG_OUT("\n");
            } else {
                LOG_OUT("GStorage: CRC Error! Calc: ");
                LOG_OUT(hwCalcCrc);
                LOG_OUT(", Stored: ");
                LOG_OUT(hwReadCrc);
                LOG_OUT("\n");
            }
        }
    }

    return GMSI_SUCCESS;
}