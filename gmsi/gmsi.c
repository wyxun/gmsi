#include <stdint.h>
#include "gmsi.h"
#include "gdebug/gshell.h"
#include "gdebug/gwaveform.h"

/* gstorage 可选模块附着点：
 * 当 gstorage.c 未加入编译时，此 weak 定义生效 —— no-op。
 * 当 gstorage.c 已加入编译时，其内部的强定义自动覆盖此 stub。 */
__attribute__((weak)) void gstorage_SetDefaultFlash(void *ptFlash)
{
    (void)ptFlash; /* gstorage not included in this build */
}

#ifdef LINUX_POSIX
#include <stdio.h>
#endif

// GMSI version static structure is defined using macros from global_define.h
const struct {
    uint8_t chPurpose;          //!< software framework purpose
    uint8_t chInterface;        //!< interface version
    uint8_t chMajor;            //!< major version
    uint8_t chMinor;            //!< minor version
} GMSIVersion = GMSI_VERSION;

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
        GLOG(E, "Error: ptGmsi is NULL.\n");
        return;
    }

    GLOG(I, "GMSI VERSION :");
    GLOG(I, (uint8_t *)&GMSIVersion, 4);
    GLOG(I, "\n");

    /* 将调用方提供的默认 Flash 注入 gstorage 层，
     * 使 ptFlash == NULL 的 gstorage 实例在 Init 时自动绑定 */
    gstorage_SetDefaultFlash(ptGmsi->ptAppFlash);


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
        GLOG(E, "Error: ptListObject is NULL.\n");
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
            GLOG(E, "Error: ptBaseDes is NULL.\n");
            return;
        }

        // Check if the function interface is NULL
        if (NULL == ptBaseDes->pFcnInterface) {
            GLOG(E, "Error: ptBaseDes->pFcnInterface is NULL.\n");
            return;
        }

        ptBaseDes->pFcnInterface->Run(ptBaseDes->wParent);
    }

    gcoroutine_Run();
    gshell_Poll();          /* 调试 shell 轮询 */
#if GWAVEFORM_ENABLE
    gwaveform.Poll();       /* 波形采集轮询 */
#endif
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
        GLOG(E, "Error: ptListObject is NULL.\n");
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
            GLOG(E, "Error: ptBaseDes is NULL.\n");
            return;
        }

        // Check if the function interface is NULL
        if (ptBaseDes->pFcnInterface == NULL) {
            GLOG(E, "Error: ptBaseDes->pFcnInterface is NULL.\n");
            return;
        }

        ptBaseDes->pFcnInterface->Clock(ptBaseDes->wParent);
    }
    
    extern void gwaveform_Default_Step_Callback(void);
    gwaveform_Default_Step_Callback();
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
    GLOGF(E, "assert failed--> %s on line: %u\n", file, (unsigned)line);
    while(1);
}