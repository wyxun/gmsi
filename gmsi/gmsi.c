#include <stdint.h>
#include "gmsi.h"
#include "utilities/util_debug.h"

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


const char *stringErrorMessage[21];
void gmsi_Run(void)
{
    // read point only
    struct xLIST *const ptListObject = gbase_GetBaseList();
    // read value only
    const struct xLIST_ITEM *ptListItemDes = ptListObject->xListEnd.pxPrevious;
    gmsi_base_t *ptBaseDes;

    // 遍历链表
    while(ptListItemDes != &ptListObject->xListEnd){
        ptBaseDes = ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptBaseDes);
        ptBaseDes->pFcnInterface->Run(ptBaseDes->wParent);

        ptListItemDes = ptListItemDes->pxPrevious;
    }

    //gcoroutine_Run();
}

void gmsi_Init(void)
{
    //printf("gmsi version chPurpose:%d, chInterface:%d, chMajor:%d, chMinor:%d\n",\
                GMSIVersion.chPurpose, GMSIVersion.chInterface, GMSIVersion.chMajor, GMSIVersion.chMinor);
    LOG_OUT("GMSI VERSION :");
    LOG_OUT((uint8_t *)&GMSIVersion, 4);
    // 初始化协程
    //gcoroutine_Init();
}
void gmsi_Clock(void)
{
    // read point only
    struct xLIST *const ptListObject = gbase_GetBaseList();
    // read value only
    const struct xLIST_ITEM *ptListItemDes = ptListObject->xListEnd.pxPrevious;
    gmsi_base_t *ptBaseDes;

    // 遍历链表
    while(ptListItemDes != &ptListObject->xListEnd){
        ptBaseDes = ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptBaseDes);
        ptBaseDes->pFcnInterface->Clock(ptBaseDes->wParent);

        ptListItemDes = ptListItemDes->pxPrevious;
    }
}

void gmsi_errorlog(int wErrorNum)
{
#ifdef LINUX_POSIX
    printf("Err_Message:%s \n", stringErrorMessage[abs(wErrorNum)]);
#endif
}


void gmsi_LogPrintf(const char *pString)
{
    char *file = __FILE__;
    int line = __LINE__;
#ifdef LINUX_POSIX
    printf("gmsi_LogPrintf: %s, in file: %s, line :%d\n", pString, file, line);
#endif
}

const char *stringErrorMessage[21] = {
    "Success num:0",
    "Operation not permitted num:1",
    "No such file or directory num:2",
    "No such process num:3",
    "Interrupted system call num:4",
    "I/O error num:5",
    "No such device or address num:6",
    "Arg list too long num:7",
    "Exec format error num:8",
    "Bad file number num:9",
    "No child processes num:10",
    "Try again num:11",
    "Out of memory num:12",
    "Permission denied num:13",
    "Bad address num:14",
    "Block device required num:15",
    "Device or resource busy num:16",
    "File exists num:17",
    "Cross-device link num:18",
    "No such device num:19",
    "Not a directory num:20"
};
void assert_failed(char *file, uint32_t line)
{
    LOG_OUT("assert failed-->");
    LOG_OUT(file);
    LOG_OUT(" on line:");
    LOG_OUT((uint16_t)line);
    LOG_OUT("\n");
    while(1);
}