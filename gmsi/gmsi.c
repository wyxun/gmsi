#include <stdint.h>
#include "gmsi.h"
#include "gstorage.h"

#ifdef LINUX_POSIX
#include <stdio.h>
#endif

#define GENERAL_PURPOSE                 0               //!< General purpose 
#define GMSI_PURPOSE                    GENERAL_PURPOSE //!< GMSI purpose   
#define GMSI_INTERFACE_VERSION          1               //!< GMSI interface version
#define GMSI_MAJOR_VERSION              1               //!< GMSI major version
#define GMSI_MINOR_VERSION              0               

// GMSI version
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

// // GMSI configuration
// gstorage_cfg_t tGstorageCfg = {
//     .ptData = NULL,
//     .hwStorageTimeOut = 60000,
// };
// gstorage_t tGstorage;
uint8_t g_chSystemData;
GMSI_DECLARE_OBJECT(gstorage, Gstorage, 
    .ptData = &g_chSystemData,
    .hwStorageTimeOut = 60000,
);

static bool s_bGmsiInit = false;

/* GNU ld style (weak) */
extern const gmsi_init_info_t __start_init_infos[] __attribute__((weak));
extern const gmsi_init_info_t __stop_init_infos[]  __attribute__((weak));

#ifdef __ARMCC_VERSION
/* Keil/armlink style symbols (objects) */
extern const uint8_t Image$$init_infos$$Base;
extern const uint8_t Image$$init_infos$$Limit;
#endif

/**
 * Function: gmsi_Init
 * ----------------------------
 * This function initializes the GMSI framework. It initializes the GMSI storage and coroutine, 
 * and prints the GMSI version.
 *
 * Parameters: 
 * ptGmsi: A pointer to the GMSI structure.
 *
 * Returns: 
 * None
 */
void gmsi_Init(gmsi_t *ptGmsi)
{
    const gmsi_init_info_t *start = NULL;
    const gmsi_init_info_t *stop  = NULL;

    // Check if the input parameter is NULL
    if (ptGmsi == NULL) {
        LOG_OUT("Error: ptGmsi is NULL.\n");
        return;
    }

    LOG_OUT("GMSI VERSION :");
    LOG_OUT((uint8_t *)&GMSIVersion, 4);

    tGstorageCfg.ptData = ptGmsi->ptData;

if ((const void *)__start_init_infos != NULL && (const void *)__stop_init_infos != NULL &&
    __start_init_infos < __stop_init_infos) {
    start = __start_init_infos;
    stop  = __stop_init_infos;
}
#ifdef __ARMCC_VERSION
else {
    /* armlink provides Image$$<sec>$$Base/Limit as addresses — take address & cast */
    const gmsi_init_info_t *arm_start = (const gmsi_init_info_t *)&Image$$init_infos$$Base;
    const gmsi_init_info_t *arm_stop  = (const gmsi_init_info_t *)&Image$$init_infos$$Limit;
    if ((const void *)arm_start != NULL && (const void *)arm_stop != NULL && arm_start < arm_stop) {
        start = arm_start;
        stop  = arm_stop;
    }
}
#endif

    if (start != NULL && stop != NULL && start < stop) {
        for (const gmsi_init_info_t *info = start; info < stop; ++info) {
            if (info->pfcnInitFunc) {
                info->pfcnInitFunc(info->wObjectAddr, info->wConfigAddr);
            }
        }
    }

    // Initialize coroutine
    gcoroutine_Init();
    // Print list information
    gbase_DebugListBase();
    /* Mark framework as initialized so gmsi_Clock will execute callbacks */
    s_bGmsiInit = true;
}

/**
 * Function: gmsi_Run
 * ----------------------------
 * This function runs the GMSI framework. It traverses the list of objects, and
 * for each object, it calls the object's Run function.
 *
 * Parameters: 
 * None
 *
 * Returns: 
 * None
 */
void gmsi_Run(void)
{
    // read point only
    struct xLIST *const ptListObject = gbase_GetBaseList();

    // Check if the list object is NULL
    if (ptListObject == NULL) {
        LOG_OUT("Error: ptListObject is NULL.\n");
        return;
    }

    // read value only
    const struct xLIST_ITEM *ptListItemDes;
    gmsi_base_t *ptBaseDes;

    for (ptListItemDes = ptListObject->xListEnd.pxPrevious;                 \
                ptListItemDes != &ptListObject->xListEnd;                   \
            ptListItemDes = ptListItemDes->pxPrevious) {
        ptBaseDes = ptListItemDes->pvOwner;

        // Check if the base descriptor is NULL
        if (NULL == ptBaseDes) {
            LOG_OUT("Error: ptBaseDes is NULL.\n");
            return;
        }

        // Check if the function interface is NULL
        if (NULL == ptBaseDes->pFcnInterface) {
            LOG_OUT("Error: ptBaseDes->pFcnInterface is NULL.\n");
            return;
        }

        ptBaseDes->pFcnInterface->Run(ptBaseDes->wParent);
    }

    gcoroutine_Run();
}

/**
 * Function: gmsi_Clock
 * ----------------------------
 * This function calls the Clock function of each object in the GMSI framework. 
 * It traverses the list of objects, and for each object, it calls the object's 
 * Clock function.
 * 
 * Parameters: 
 * None
 *
 * Returns: 
 * None
 */
void gmsi_Clock(void)
{
    // read point only
    struct xLIST *const ptListObject = gbase_GetBaseList();
    
    if(false == s_bGmsiInit)
        return;
    
    // Check if the list object is NULL
    if (NULL == ptListObject) {
        LOG_OUT("Error: ptListObject is NULL.\n");
        return;
    }

    // read value only
    const struct xLIST_ITEM *ptListItemDes;
    gmsi_base_t *ptBaseDes;

    for (ptListItemDes = ptListObject->xListEnd.pxPrevious;                 \
            ptListItemDes != &ptListObject->xListEnd;                       \
            ptListItemDes = ptListItemDes->pxPrevious) {
        ptBaseDes = ptListItemDes->pvOwner;

        // Check if the base descriptor is NULL
        if (NULL == ptBaseDes) {
            LOG_OUT("Error: ptBaseDes is NULL.\n");
            return;
        }

        // Check if the function interface is NULL
        if (ptBaseDes->pFcnInterface == NULL) {
            LOG_OUT("Error: ptBaseDes->pFcnInterface is NULL.\n");
            return;
        }

        ptBaseDes->pFcnInterface->Clock(ptBaseDes->wParent);
    }
}

/**
 * Function: assert_failed
 * ----------------------------
 * This function is called when an assertion fails. It prints an error message, 
 * including the file where the assertion failed and the line number of the failure.
 *
 * Parameters: 
 * file: The file where the assertion failed.
 * line: The line number of the assertion failure.
 *
 * Returns: 
 * None. This function enters an infinite loop after printing the error message.
 */
void assert_failed(char *file, uint32_t line)
{
    LOG_OUT("assert failed-->");
    LOG_OUT(file);
    LOG_OUT(" on line:");
    LOG_OUT((uint16_t)line);
    LOG_OUT("\n");
    while(1);
}