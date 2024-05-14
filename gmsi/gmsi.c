#include <stdint.h>
#include "gmsi.h"
#include "gstorage.h"

#ifdef LINUX_POSIX
#include <stdio.h>
#endif

//! \name GMSI Interface Type
//! @{
//! general purpose interface for normal MCU applicatoin
#define GENERAL_PURPOSE                 0           
//! @}

#define GMSI_PURPOSE                     GENERAL_PURPOSE

/*! \brief interface version
 *! \note Change this when new generation of interface is released
 */
#define GMSI_INTERFACE_VERSION           1

/*! \brief major version for specified gsf interface
 *! \note major version is rarely changed
 */
#define GMSI_MAJOR_VERSION               1

/*! \brief minor version for normal maintaince
 *! \note update this for function update and bug fixing
 */
#define GMSI_MINOR_VERSION               0

/*! \brief GSF version 
 *         (InterfaceType, InterfaceVersion, MajorVersion, MinorVersion)
 */
#define GMSI_VERSION                 {                                      \
                                        GMSI_PURPOSE,                       \
                                        GMSI_INTERFACE_VERSION,             \
                                        GMSI_MAJOR_VERSION,                 \
                                        GMSI_MINOR_VERSION,                 \
                                    }

const struct {
    uint8_t chPurpose;          //!< software framework purpose
    uint8_t chInterface;        //!< interface version
    uint8_t chMajor;            //!< major version
    uint8_t chMinor;            //!< minor version
} GMSIVersion = GMSI_VERSION;

gstorage_cfg_t tGstorageCfg = {
    .ptData = NULL,
    .hwStorageTimeOut = 60000,
};
gstorage_t tGstorage;

void gmsi_Init(gmsi_t *ptGmsi)
{
    LOG_OUT("GMSI VERSION :");
    LOG_OUT((uint8_t *)&GMSIVersion, 4);

    if(NULL != ptGmsi->ptData)
    {
        GVAL_PRINTF((uint32_t)ptGmsi->ptData);
        tGstorageCfg.ptData = ptGmsi->ptData;
        gstorage_Init((uint32_t)&tGstorage, (uint32_t)&tGstorageCfg);
    }
    gcoroutine_Init();

    gbase_DegugListBase();
}

void gmsi_Run(void)
{
    // read point only
    struct xLIST *const ptListObject = gbase_GetBaseList();
    // read value only
    const struct xLIST_ITEM *ptListItemDes = ptListObject->xListEnd.pxPrevious;
    gmsi_base_t *ptBaseDes;

    while(ptListItemDes != &ptListObject->xListEnd){
        ptBaseDes = ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptBaseDes);
        ptBaseDes->pFcnInterface->Run(ptBaseDes->wParent);

        ptListItemDes = ptListItemDes->pxPrevious;
    }

    gcoroutine_Run();
}

void gmsi_Clock(void)
{
    // read point only
    struct xLIST *const ptListObject = gbase_GetBaseList();
    // read value only
    const struct xLIST_ITEM *ptListItemDes = ptListObject->xListEnd.pxPrevious;
    gmsi_base_t *ptBaseDes;

    while(ptListItemDes != &ptListObject->xListEnd){
        ptBaseDes = ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptBaseDes);
        ptBaseDes->pFcnInterface->Clock(ptBaseDes->wParent);

        ptListItemDes = ptListItemDes->pxPrevious;
    }
}


void assert_failed(char *file, uint32_t line)
{
    LOG_OUT("assert failed-->");
    LOG_OUT(file);
    LOG_OUT(" on line:");
    LOG_OUT((uint16_t)line);
    LOG_OUT("\n");
    while(1);
}