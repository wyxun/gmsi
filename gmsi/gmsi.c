#include <stdint.h>
#include "gmsi.h"
#ifdef PC_DEBUG
#include <assert.h>
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

void gmsi_Run(void)
{
    struct xLIST*  ptListObject = gbase_GetBaseList();
    struct xLIST_ITEM *ptListItemDes = ptListObject->xListEnd.pxPrevious;
    gmsi_base_t *ptBaseDes;

    // ��������
    while(ptListItemDes != &ptListObject->xListEnd){
        ptBaseDes = ptListItemDes->pvOwner;
        assert(NULL != ptBaseDes);
        ptBaseDes->pFcnInterface->Run(ptBaseDes->wParent);

        ptListItemDes = ptListItemDes->pxPrevious;
    }
}

void gmsi_Init(void)
{
    
}
void gmsi_Clock(void)
{
    struct xLIST*  ptListObject = gbase_GetBaseList();
    struct xLIST_ITEM *ptListItemDes = ptListObject->xListEnd.pxPrevious;
    gmsi_base_t *ptBaseDes;

    // ��������
    while(ptListItemDes != &ptListObject->xListEnd){
        ptBaseDes = ptListItemDes->pvOwner;
        assert(NULL != ptBaseDes);
        ptBaseDes->pFcnInterface->Clock(ptBaseDes->wParent);

        ptListItemDes = ptListItemDes->pxPrevious;
    }
}

void gmsi_errorlog(int wErrorNum)
{
    switch(wErrorNum)
    {
        case GMSI_SUCCESS:
        break;
        
        default:
        break;
    }
}