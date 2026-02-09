/**
 * @file gblinfo.c
 * @brief Bootloader Information Shared Module Implementation
 */
#include "global_define.h"
#include "glog.h"
#include "gblinfo.h"
#include <string.h>

int gblinfo_Run(uintptr_t wObjectAddr);
int gblinfo_Clock(uintptr_t wObjectAddr);

/* Static module data */
static gmsi_base_t s_tGblinfoBase;
static gblinfo_t *s_ptGblinfoInstance = NULL;

static gmsi_base_cfg_t s_tGblinfoBaseCfg = {
    .wId = GMSI_GBLINFO,
    .wParent = 0,
    .FcnInterface = {
        .Clock = gblinfo_Clock,
        .Run = gblinfo_Run,
    },
};

/**
 * @brief Get pointer to shared info structure
 */
static gblinfo_shared_t* gblinfo_GetSharedPtr(void)
{
    if (s_ptGblinfoInstance == NULL) {
        return NULL;
    }
    return (gblinfo_shared_t *)(s_ptGblinfoInstance->wSharedInfoAddr);
}

/**
 * @brief Check if bootloader is present by validating magic number
 */
int gblinfo_IsBootloaderPresent(void)
{
    gblinfo_shared_t *ptShared = gblinfo_GetSharedPtr();
    if (ptShared == NULL) {
        return GMSI_EINVAL;
    }
    
    if (ptShared->wMagic != GBLINFO_MAGIC) {
        return GMSI_ENODEV;
    }
    
    return GMSI_SUCCESS;
}

/**
 * @brief Get bootloader version
 */
int gblinfo_GetBlVersion(uint8_t *pchMajor, uint8_t *pchMinor)
{
    if (gblinfo_IsBootloaderPresent() != GMSI_SUCCESS) {
        return GMSI_ENODEV;
    }
    
    gblinfo_shared_t *ptShared = gblinfo_GetSharedPtr();
    if (pchMajor != NULL) {
        *pchMajor = ptShared->chBlMajor;
    }
    if (pchMinor != NULL) {
        *pchMinor = ptShared->chBlMinor;
    }
    
    return GMSI_SUCCESS;
}

/**
 * @brief Get application version
 */
int gblinfo_GetAppVersion(uint8_t *pchMajor, uint8_t *pchMinor)
{
    if (gblinfo_IsBootloaderPresent() != GMSI_SUCCESS) {
        return GMSI_ENODEV;
    }
    
    gblinfo_shared_t *ptShared = gblinfo_GetSharedPtr();
    if (pchMajor != NULL) {
        *pchMajor = ptShared->chAppMajor;
    }
    if (pchMinor != NULL) {
        *pchMinor = ptShared->chAppMinor;
    }
    
    return GMSI_SUCCESS;
}

/**
 * @brief Get application CRC
 */
int gblinfo_GetAppCrc(uint32_t *pwCrc)
{
    if (pwCrc == NULL) {
        return GMSI_EINVAL;
    }
    if (gblinfo_IsBootloaderPresent() != GMSI_SUCCESS) {
        return GMSI_ENODEV;
    }
    
    gblinfo_shared_t *ptShared = gblinfo_GetSharedPtr();
    *pwCrc = ptShared->wAppCrc;
    
    return GMSI_SUCCESS;
}

/**
 * @brief Get device ID
 */
int gblinfo_GetDeviceId(uint32_t *pwDeviceId)
{
    if (pwDeviceId == NULL) {
        return GMSI_EINVAL;
    }
    if (gblinfo_IsBootloaderPresent() != GMSI_SUCCESS) {
        return GMSI_ENODEV;
    }
    
    gblinfo_shared_t *ptShared = gblinfo_GetSharedPtr();
    *pwDeviceId = ptShared->wDeviceId;
    
    return GMSI_SUCCESS;
}

/**
 * @brief Get bootloader build time
 */
int gblinfo_GetBlBuildTime(uint32_t *pwBuildTime)
{
    if (pwBuildTime == NULL) {
        return GMSI_EINVAL;
    }
    if (gblinfo_IsBootloaderPresent() != GMSI_SUCCESS) {
        return GMSI_ENODEV;
    }
    
    gblinfo_shared_t *ptShared = gblinfo_GetSharedPtr();
    *pwBuildTime = ptShared->wBlBuildTime;
    
    return GMSI_SUCCESS;
}

/**
 * @brief Get application build time
 */
int gblinfo_GetAppBuildTime(uint32_t *pwBuildTime)
{
    if (pwBuildTime == NULL) {
        return GMSI_EINVAL;
    }
    if (gblinfo_IsBootloaderPresent() != GMSI_SUCCESS) {
        return GMSI_ENODEV;
    }
    
    gblinfo_shared_t *ptShared = gblinfo_GetSharedPtr();
    *pwBuildTime = ptShared->wAppBuildTime;
    
    return GMSI_SUCCESS;
}

/**
 * @brief Set upgrade flag (App requests upgrade mode)
 * @note This requires flash write operation - implementation depends on port
 */
int gblinfo_SetUpgradeFlag(uint8_t chFlag)
{
    if (gblinfo_IsBootloaderPresent() != GMSI_SUCCESS) {
        return GMSI_ENODEV;
    }
    
    gblinfo_shared_t *ptShared = gblinfo_GetSharedPtr();
    /* Note: Direct write to flash may require unlock/erase/write sequence
     * This is a simplified version - actual implementation needs port layer */
    ptShared->chUpgradeFlag = chFlag;
    
    return GMSI_SUCCESS;
}

/**
 * @brief Update application info (called by bootloader after successful upgrade)
 */
int gblinfo_UpdateAppInfo(uint8_t chMajor, uint8_t chMinor, 
                          uint32_t wCrc, uint32_t wSize, uint32_t wBuildTime)
{
    gblinfo_shared_t *ptShared = gblinfo_GetSharedPtr();
    if (ptShared == NULL) {
        return GMSI_EINVAL;
    }
    
    ptShared->chAppMajor = chMajor;
    ptShared->chAppMinor = chMinor;
    ptShared->wAppCrc = wCrc;
    ptShared->wAppSize = wSize;
    ptShared->wAppBuildTime = wBuildTime;
    
    return GMSI_SUCCESS;
}

/**
 * @brief Update bootloader info (called by bootloader on init)
 */
int gblinfo_UpdateBlInfo(uint8_t chMajor, uint8_t chMinor, uint32_t wBuildTime)
{
    gblinfo_shared_t *ptShared = gblinfo_GetSharedPtr();
    if (ptShared == NULL) {
        return GMSI_EINVAL;
    }
    
    ptShared->wMagic = GBLINFO_MAGIC;
    ptShared->chBlMajor = chMajor;
    ptShared->chBlMinor = chMinor;
    ptShared->wBlBuildTime = wBuildTime;
    
    return GMSI_SUCCESS;
}

/**
 * @brief Update boot status
 */
int gblinfo_UpdateBootStatus(uint8_t chStatus)
{
    gblinfo_shared_t *ptShared = gblinfo_GetSharedPtr();
    if (ptShared == NULL) {
        return GMSI_EINVAL;
    }
    
    ptShared->chLastBootStatus = chStatus;
    
    return GMSI_SUCCESS;
}

/**
 * @brief Increment boot count
 */
int gblinfo_IncrementBootCount(void)
{
    gblinfo_shared_t *ptShared = gblinfo_GetSharedPtr();
    if (ptShared == NULL) {
        return GMSI_EINVAL;
    }
    
    ptShared->chBootCount++;
    
    return GMSI_SUCCESS;
}

/**
 * @brief Run function (called in main loop)
 */
int gblinfo_Run(uintptr_t wObjectAddr)
{
    (void)wObjectAddr;
    return GMSI_SUCCESS;
}

/**
 * @brief Clock function (called in timer interrupt)
 */
int gblinfo_Clock(uintptr_t wObjectAddr)
{
    (void)wObjectAddr;
    return GMSI_SUCCESS;
}

/**
 * @brief Initialize gblinfo module
 */
int gblinfo_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    int wRet = GMSI_SUCCESS;
    
    gblinfo_t *ptThis = (gblinfo_t *)wObjectAddr;
    gblinfo_cfg_t *ptCfg = (gblinfo_cfg_t *)wObjectCfgAddr;
    
    if (ptThis == NULL || ptCfg == NULL) {
        GLOG_PRINTF("Error: gblinfo_Init ptThis or ptCfg is NULL.");
        return GMSI_EFAIL;
    }
    
    /* Store instance pointer for API access */
    s_ptGblinfoInstance = ptThis;
    
    /* Copy configuration */
    ptThis->wSharedInfoAddr = ptCfg->wSharedInfoAddr;
    
    /* Check if bootloader is present */
    gblinfo_shared_t *ptShared = (gblinfo_shared_t *)(ptThis->wSharedInfoAddr);
    if (ptShared->wMagic == GBLINFO_MAGIC) {
        ptThis->bBootloaderPresent = true;
        GLOG_PRINTF("gblinfo: Bootloader detected");
    } else {
        ptThis->bBootloaderPresent = false;
        GLOG_PRINTF("gblinfo: No bootloader detected");
    }
    
    /* Register with GMSI */
    ptThis->ptBase = &s_tGblinfoBase;
    if (ptThis->ptBase == NULL) {
        GLOG_PRINTF("Error: gblinfo ptBase is NULL.");
        return GMSI_EFAIL;
    }
    
    s_tGblinfoBaseCfg.wParent = wObjectAddr;
    wRet = gbase_Init(ptThis->ptBase, &s_tGblinfoBaseCfg);
    if (wRet < 0) {
        GERR_PRINTF(wRet);
        return wRet;
    }
    
    return wRet;
}
