/**
 * @file mblinfo.c
 * @brief Bootloader Information Shared Module Implementation
 */
#include "global_define.h"
#include "mlog.h"
#include "mblinfo.h"
#include "perf_counter.h"
#include <string.h>


int mblinfo_Run(uintptr_t wObjectAddr);
int mblinfo_Clock(uintptr_t wObjectAddr);

/* Static module data */
static modus_base_t s_tGblinfoBase;
static mblinfo_t *s_ptGblinfoInstance = NULL;

static modus_base_cfg_t s_tGblinfoBaseCfg = {
    .wId = MODUS_MBLINFO,
    .wParent = 0,
    .FcnInterface = {
        .Clock = mblinfo_Clock,
        .Run = mblinfo_Run,
    },
};

/**
 * @brief Get pointer to shared info structure
 */
static mblinfo_shared_t* mblinfo_GetSharedPtr(void)
{
    if (s_ptGblinfoInstance == NULL) {
        return NULL;
    }
    return (mblinfo_shared_t *)(s_ptGblinfoInstance->wSharedInfoAddr);
}

/**
 * @brief Check if bootloader is present by validating magic number
 */
int mblinfo_IsBootloaderPresent(void)
{
    mblinfo_shared_t *ptShared = mblinfo_GetSharedPtr();
    if (ptShared == NULL) {
        return MODUS_EINVAL;
    }
    
    if (ptShared->wMagic != MBLINFO_MAGIC) {
        return MODUS_ENODEV;
    }
    
    return MODUS_SUCCESS;
}

/**
 * @brief Get bootloader version
 */
int mblinfo_GetBlVersion(uint8_t *pchMajor, uint8_t *pchMinor)
{
    if (mblinfo_IsBootloaderPresent() != MODUS_SUCCESS) {
        return MODUS_ENODEV;
    }
    
    mblinfo_shared_t *ptShared = mblinfo_GetSharedPtr();
    if (pchMajor != NULL) {
        *pchMajor = ptShared->chBlMajor;
    }
    if (pchMinor != NULL) {
        *pchMinor = ptShared->chBlMinor;
    }
    
    return MODUS_SUCCESS;
}

/**
 * @brief Get application version
 */
int mblinfo_GetAppVersion(uint8_t *pchMajor, uint8_t *pchMinor)
{
    if (mblinfo_IsBootloaderPresent() != MODUS_SUCCESS) {
        return MODUS_ENODEV;
    }
    
    mblinfo_shared_t *ptShared = mblinfo_GetSharedPtr();
    if (pchMajor != NULL) {
        *pchMajor = ptShared->chAppMajor;
    }
    if (pchMinor != NULL) {
        *pchMinor = ptShared->chAppMinor;
    }
    
    return MODUS_SUCCESS;
}

/**
 * @brief Get application CRC
 */
int mblinfo_GetAppCrc(uint32_t *pwCrc)
{
    if (pwCrc == NULL) {
        return MODUS_EINVAL;
    }
    if (mblinfo_IsBootloaderPresent() != MODUS_SUCCESS) {
        return MODUS_ENODEV;
    }
    
    mblinfo_shared_t *ptShared = mblinfo_GetSharedPtr();
    *pwCrc = ptShared->wAppCrc;
    
    return MODUS_SUCCESS;
}

/**
 * @brief Get device ID
 */
int mblinfo_GetDeviceId(uint32_t *pwDeviceId)
{
    if (pwDeviceId == NULL) {
        return MODUS_EINVAL;
    }
    if (mblinfo_IsBootloaderPresent() != MODUS_SUCCESS) {
        return MODUS_ENODEV;
    }
    
    mblinfo_shared_t *ptShared = mblinfo_GetSharedPtr();
    *pwDeviceId = ptShared->wDeviceId;
    
    return MODUS_SUCCESS;
}

/**
 * @brief Get bootloader build time
 */
int mblinfo_GetBlBuildTime(uint32_t *pwBuildTime)
{
    if (pwBuildTime == NULL) {
        return MODUS_EINVAL;
    }
    if (mblinfo_IsBootloaderPresent() != MODUS_SUCCESS) {
        return MODUS_ENODEV;
    }
    
    mblinfo_shared_t *ptShared = mblinfo_GetSharedPtr();
    *pwBuildTime = ptShared->wBlBuildTime;
    
    return MODUS_SUCCESS;
}

/**
 * @brief Get application build time
 */
int mblinfo_GetAppBuildTime(uint32_t *pwBuildTime)
{
    if (pwBuildTime == NULL) {
        return MODUS_EINVAL;
    }
    if (mblinfo_IsBootloaderPresent() != MODUS_SUCCESS) {
        return MODUS_ENODEV;
    }
    
    mblinfo_shared_t *ptShared = mblinfo_GetSharedPtr();
    *pwBuildTime = ptShared->wAppBuildTime;
    
    return MODUS_SUCCESS;
}

/**
 * @brief Set upgrade flag (App requests upgrade mode)
 * @note This requires flash write operation - implementation depends on port
 */
int mblinfo_SetUpgradeFlag(uint8_t chFlag)
{
    if (mblinfo_IsBootloaderPresent() != MODUS_SUCCESS) {
        return MODUS_ENODEV;
    }
    
    mblinfo_shared_t *ptShared = mblinfo_GetSharedPtr();
    /* Note: Direct write to flash may require unlock/erase/write sequence
     * This is a simplified version - actual implementation needs port layer */
    ptShared->chUpgradeFlag = chFlag;
    
    return MODUS_SUCCESS;
}

/**
 * @brief Update application info (called by bootloader after successful upgrade)
 */
int mblinfo_UpdateAppInfo(uint8_t chMajor, uint8_t chMinor, 
                          uint32_t wCrc, uint32_t wSize, uint32_t wBuildTime)
{
    mblinfo_shared_t *ptShared = mblinfo_GetSharedPtr();
    if (ptShared == NULL) {
        return MODUS_EINVAL;
    }
    
    ptShared->chAppMajor = chMajor;
    ptShared->chAppMinor = chMinor;
    ptShared->wAppCrc = wCrc;
    ptShared->wAppSize = wSize;
    ptShared->wAppBuildTime = wBuildTime;
    
    return MODUS_SUCCESS;
}

/**
 * @brief Update bootloader info (called by bootloader on init)
 */
int mblinfo_UpdateBlInfo(uint8_t chMajor, uint8_t chMinor, uint32_t wBuildTime)
{
    mblinfo_shared_t *ptShared = mblinfo_GetSharedPtr();
    if (ptShared == NULL) {
        return MODUS_EINVAL;
    }
    
    ptShared->wMagic = MBLINFO_MAGIC;
    ptShared->chBlMajor = chMajor;
    ptShared->chBlMinor = chMinor;
    ptShared->wBlBuildTime = wBuildTime;
    
    return MODUS_SUCCESS;
}

/**
 * @brief Update boot status
 */
int mblinfo_UpdateBootStatus(uint8_t chStatus)
{
    mblinfo_shared_t *ptShared = mblinfo_GetSharedPtr();
    if (ptShared == NULL) {
        return MODUS_EINVAL;
    }
    
    ptShared->chLastBootStatus = chStatus;
    
    return MODUS_SUCCESS;
}

/**
 * @brief Increment boot count
 */
int mblinfo_IncrementBootCount(void)
{
    mblinfo_shared_t *ptShared = mblinfo_GetSharedPtr();
    if (ptShared == NULL) {
        return MODUS_EINVAL;
    }
    
    ptShared->chBootCount++;
    
    return MODUS_SUCCESS;
}

/**
 * @brief Run function (called in main loop)
 */
int mblinfo_Run(uintptr_t wObjectAddr)
{
    (void)wObjectAddr;
    return MODUS_SUCCESS;
}



/**
 * @brief Clock function (called in timer interrupt)
 */
int mblinfo_Clock(uintptr_t wObjectAddr)
{
    (void)wObjectAddr;
    return MODUS_SUCCESS;
}

/**
 * @brief Initialize mblinfo module
 */
int mblinfo_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    int wRet = MODUS_SUCCESS;
    
    mblinfo_t *ptThis = (mblinfo_t *)wObjectAddr;
    mblinfo_cfg_t *ptCfg = (mblinfo_cfg_t *)wObjectCfgAddr;
    
    if (ptThis == NULL || ptCfg == NULL) {
        MLOG_PRINTF("Error: mblinfo_Init ptThis or ptCfg is NULL.");
        return MODUS_EFAIL;
    }
    
    /* Store instance pointer for API access */
    s_ptGblinfoInstance = ptThis;
    
    /* Copy configuration */
    ptThis->wSharedInfoAddr = ptCfg->wSharedInfoAddr;
    
    /* Check if bootloader is present */
    mblinfo_shared_t *ptShared = (mblinfo_shared_t *)(ptThis->wSharedInfoAddr);
    if (ptShared->wMagic == MBLINFO_MAGIC) {
        ptThis->bBootloaderPresent = true;
        MLOG_PRINTF("mblinfo: Bootloader detected");
    } else {
        ptThis->bBootloaderPresent = false;
        MLOG_PRINTF("mblinfo: No bootloader detected");
    }
    
    /* Register with MODUS */
    ptThis->ptBase = &s_tGblinfoBase;
    if (ptThis->ptBase == NULL) {
        MLOG_PRINTF("Error: mblinfo ptBase is NULL.");
        return MODUS_EFAIL;
    }
    
    s_tGblinfoBaseCfg.wParent = wObjectAddr;
    wRet = mbase_Init(ptThis->ptBase, &s_tGblinfoBaseCfg);
    if (wRet < 0) {
        GERR_PRINTF(wRet);
        return wRet;
    }
    
    return wRet;
}
