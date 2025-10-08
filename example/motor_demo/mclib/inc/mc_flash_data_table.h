/**
  **************************************************************************
  * @file     mc_flash_data_table.h
  * @brief    Definition of flash address and sector size for writing parameters table in different MCU types
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#ifndef __MC_FLASH_DATA_TABLE_H
#define __MC_FLASH_DATA_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

/* ----------------------------------------------------------------------
** Data Address(char/uint32), Sector Size
** ------------------------------------------------------------------- */
#if defined (AT32F403AxG) || defined (AT32F407xG)
#define MC_VectStoreAddr1              "0x080FF800" 		/*1024K*/
#define MC_VectStoreAddr_UINT32         0x080FF800  	 /*1024K*/
#define MC_VectStoreAddr2              "0x080FFB20"   /*1024K*/
#define MC_VectStoreAddr_FLOAT          0x080FFB20    /*1024K*/
#define SECTOR_SIZE                     2048 			      /*1024K*/
#elif defined (AT32F403AxE) || defined (AT32F407xE)
#define MC_VectStoreAddr1              "0x0807F800" 		/*512K*/
#define MC_VectStoreAddr_UINT32         0x0807F800  	 /*512K*/
#define MC_VectStoreAddr2              "0x0807FB20"   /*512K*/
#define MC_VectStoreAddr_FLOAT          0x0807FB20    /*512K*/
#define SECTOR_SIZE                     2048 			      /*512K*/
#elif defined (AT32F413xC) || defined (AT32F415xC) || defined (AT32F423xC) || defined (AT32F403AxC) || defined (AT32F407xC)
#define MC_VectStoreAddr1              "0x0803F800" 		/*256K*/
#define MC_VectStoreAddr_UINT32         0x0803F800 			/*256K*/
#define MC_VectStoreAddr2              "0x0803FB20"   /*256K*/
#define MC_VectStoreAddr_FLOAT          0x0803FB20    /*256K*/
#define SECTOR_SIZE                     2048 			      /*256K*/
#elif defined (AT32F413xB) || defined (AT32F415xB) || defined (AT32F423xB)
#define MC_VectStoreAddr1              "0x0801FC00"			/*128K*/
#define MC_VectStoreAddr_UINT32         0x0801FC00			 /*128K*/
#define MC_VectStoreAddr2              "0x0801FF20"   /*128K*/
#define MC_VectStoreAddr_FLOAT          0x0801FF20    /*128K*/
#define SECTOR_SIZE                     1024			       /*128K*/
#elif defined (AT32F413x8) || defined (AT32F421x8) || defined (AT32F415x8) || defined (AT32F423x8) || defined (AT32F425x8)
#define MC_VectStoreAddr1              "0x0800FC00"			/*64K*/
#define MC_VectStoreAddr_UINT32         0x0800FC00			 /*64K*/
#define MC_VectStoreAddr2              "0x0800FF20"   /*64K*/
#define MC_VectStoreAddr_FLOAT          0x0800FF20    /*64K*/
#define SECTOR_SIZE                     1024			       /*64K*/
#elif defined (AT32F421x6) || defined (AT32F425x6)
#define MC_VectStoreAddr1              "0x08007C00"			/*32K*/
#define MC_VectStoreAddr_UINT32         0x08007C00			 /*32K*/
#define MC_VectStoreAddr2              "0x08007F20"   /*32K*/
#define MC_VectStoreAddr_FLOAT          0x08007F20    /*32K*/
#define SECTOR_SIZE                     1024			       /*32K*/
#elif defined (AT32F421x4)
#define MC_VectStoreAddr1              "0x08003C00"			/*16K*/
#define MC_VectStoreAddr_UINT32         0x08003C00			 /*16K*/
#define MC_VectStoreAddr2              "0x08003F20"   /*16K*/
#define MC_VectStoreAddr_FLOAT          0x08003F20    /*16K*/
#define SECTOR_SIZE                     1024			       /*16K*/
#endif

/* ----------------------------------------------------------------------
** Data Table
** Total Size : 1K
** ------------------------------------------------------------------- */
#define INT32_SIZE_ARRAY              (SECTOR_SIZE/4)
#define INT32_SIZE_ARRAY1             (200)
#define INT32_SIZE_ARRAY2             (INT32_SIZE_ARRAY-INT32_SIZE_ARRAY1)

extern const int32_t intCoeffs32[];
extern const float floatCoeffs32[];

#ifdef __cplusplus
}
#endif

#endif
