#include "global_define.h"
#include "glog.h"
#include <stdio.h>
#include <stdint.h>
#include "gstorage.h"
#include "utilities/util_debug.h"

int gstorage_Run(uintptr_t wObjectAddr);
int gstorage_Clock(uintptr_t wObjectAddr);

static gmsi_base_t s_tStorageBase;
gmsi_base_cfg_t s_tStorageBaseCfg = {
    .wId = GMSI_STORAGE,
    /* ��ȡ��ָ�� */
    .wParent = 0,
    .FcnInterface = {
        .Clock = gstorage_Clock,
        .Run = gstorage_Run,
    },
};

void gstorage_EventHandle(gstorage_t *ptThis, uint32_t wEvent)
{
    if(wEvent & Event_Storage)
    {
        GLOG_PRINTF("get Event_Storage");
        ptThis->ptData->fcnWrite(ptThis->ptData->phwStorageStartAddr, ptThis->ptData->hwStorageLength);
    }
    if(wEvent & Event_GetData)
    {
        GLOG_PRINTF("get Event_GetData");
        ptThis->ptData->fcnRead(ptThis->ptData->phwStorageStartAddr, ptThis->ptData->hwStorageLength);
    }
}

int gstorage_Run(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;
    // ָ����
    gstorage_t *ptThis = (gstorage_t *)wObjectAddr;
    GMSI_ASSERT(NULL != ptThis);

    // �¼�����
    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent)
        gstorage_EventHandle(ptThis, wEvent);
    
    // �߼������״̬������

    return wRet;
}

int gstorage_Clock(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;

    // ָ����
    gstorage_t *ptThis = (gstorage_t *)wObjectAddr;
    GMSI_ASSERT(NULL != ptThis);

    return wRet;
}

int gstorage_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    int wRet = GMSI_SUCCESS;
    // ��ȡָ��
    gstorage_t *ptThis = (gstorage_t *)wObjectAddr;
    gstorage_cfg_t *ptCfg = (gstorage_cfg_t *)wObjectCfgAddr;
    GMSI_ASSERT(NULL != ptThis);
    GMSI_ASSERT(NULL != ptCfg);

    ptThis->ptData = ptCfg->ptData;

    ptThis->ptBase = &s_tStorageBase;

    // ��Դ������
    if(NULL == ptThis->ptBase)
        wRet = GMSI_EAGAIN;
    else
    {
        // �趨��ָ��
        s_tStorageBaseCfg.wParent = wObjectAddr;
        wRet = gbase_Init(ptThis->ptBase, &s_tStorageBaseCfg);
    }

    return wRet;
}