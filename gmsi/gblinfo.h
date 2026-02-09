/**
 * @file gblinfo.h
 * @brief Bootloader Information Shared Module
 * 
 * This module provides shared information between bootloader and application.
 * The shared data is stored at a fixed flash address (GBLINFO_SHARED_ADDR).
 */
#ifndef __GBLINFO_H__
#define __GBLINFO_H__

#include "gbase.h"

/**
 * @brief Shared information structure between bootloader and app
 * 
 * This structure is stored at GBLINFO_SHARED_ADDR in flash.
 * Both bootloader and app can read/write this area.
 */
typedef struct {
    /* Validation */
    uint32_t wMagic;                // Magic number (GBLINFO_MAGIC)
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
} gblinfo_shared_t;

/**
 * @brief Configuration structure for gblinfo module
 */
typedef struct {
    uint32_t wSharedInfoAddr;       // Shared info flash address
} gblinfo_cfg_t;

/**
 * @brief Object structure for gblinfo module
 */
typedef struct {
    gmsi_base_t *ptBase;
    uint32_t wSharedInfoAddr;
    bool bBootloaderPresent;        // Bootloader detected flag
} gblinfo_t;

/* Initialization */
int gblinfo_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

/* Read API - For App */
int gblinfo_IsBootloaderPresent(void);
int gblinfo_GetBlVersion(uint8_t *pchMajor, uint8_t *pchMinor);
int gblinfo_GetAppVersion(uint8_t *pchMajor, uint8_t *pchMinor);
int gblinfo_GetAppCrc(uint32_t *pwCrc);
int gblinfo_GetDeviceId(uint32_t *pwDeviceId);
int gblinfo_GetBlBuildTime(uint32_t *pwBuildTime);
int gblinfo_GetAppBuildTime(uint32_t *pwBuildTime);

/* Write API - For Bootloader */
int gblinfo_SetUpgradeFlag(uint8_t chFlag);
int gblinfo_UpdateAppInfo(uint8_t chMajor, uint8_t chMinor, 
                          uint32_t wCrc, uint32_t wSize, uint32_t wBuildTime);
int gblinfo_UpdateBlInfo(uint8_t chMajor, uint8_t chMinor, uint32_t wBuildTime);
int gblinfo_UpdateBootStatus(uint8_t chStatus);
int gblinfo_IncrementBootCount(void);

#endif  // __GBLINFO_H__
