/**
 * @file mblinfo.h
 * @brief Bootloader Information Shared Module
 * 
 * This module provides shared information between bootloader and application.
 * The shared data is stored at a fixed flash address (MBLINFO_SHARED_ADDR).
 */
#ifndef __MBLINFO_H__
#define __MBLINFO_H__

#include "mbase.h"

/**
 * @brief Shared information structure between bootloader and app
 * 
 * This structure is stored at MBLINFO_SHARED_ADDR in flash.
 * Both bootloader and app can read/write this area.
 */
typedef struct {
    /* Validation */
    uint32_t wMagic;                // Magic number (MBLINFO_MAGIC)
    uint32_t wStructCrc;            // CRC32 of this structure
    
    /* Bootloader Info */
    uint8_t  chBlMajor;             // Bootloader major version
    uint8_t  chBlMinor;             // Bootloader minor version
    uint8_t  achBlReserved[2];      // Reserved for alignment
    uint32_t wBlBuildTime;          // Bootloader build timestamp
    
    /* Application Info */
    uint8_t  chAppMajor;            // App major version
    uint8_t  chAppMinor;            // App minor version
    uint8_t  achAppReserved[2];     // Reserved for alignment
    uint32_t wAppBuildTime;         // App build timestamp
    uint32_t wAppCrc;               // App firmware CRC32
    uint32_t wAppSize;              // App firmware size in bytes
    
    /* Control */
    uint8_t  chUpgradeFlag;         // Upgrade request flag
    uint8_t  chBootCount;           // Boot counter
    uint8_t  chLastBootStatus;      // Last boot status
    uint8_t  chHwVersion;           // Hardware version
    
    /* Device */
    uint32_t wDeviceId;             // Device unique ID
} mblinfo_shared_t;

/**
 * @brief Configuration structure for mblinfo module
 */
typedef struct {
    uint32_t wSharedInfoAddr;       // Shared info flash address
} mblinfo_cfg_t;

/**
 * @brief Object structure for mblinfo module
 */
typedef struct {
    modus_base_t *ptBase;
    uint32_t wSharedInfoAddr;
    bool bBootloaderPresent;        // Bootloader detected flag
} mblinfo_t;

/* Initialization */
int mblinfo_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

/* Read API - For App */
int mblinfo_IsBootloaderPresent(void);
int mblinfo_GetBlVersion(uint8_t *pchMajor, uint8_t *pchMinor);
int mblinfo_GetAppVersion(uint8_t *pchMajor, uint8_t *pchMinor);
int mblinfo_GetAppCrc(uint32_t *pwCrc);
int mblinfo_GetDeviceId(uint32_t *pwDeviceId);
int mblinfo_GetBlBuildTime(uint32_t *pwBuildTime);
int mblinfo_GetAppBuildTime(uint32_t *pwBuildTime);

/* Write API - For Bootloader */
int mblinfo_SetUpgradeFlag(uint8_t chFlag);
int mblinfo_UpdateAppInfo(uint8_t chMajor, uint8_t chMinor, 
                          uint32_t wCrc, uint32_t wSize, uint32_t wBuildTime);
int mblinfo_UpdateBlInfo(uint8_t chMajor, uint8_t chMinor, uint32_t wBuildTime);
int mblinfo_UpdateBootStatus(uint8_t chStatus);
int mblinfo_IncrementBootCount(void);

#endif  // __MBLINFO_H__
