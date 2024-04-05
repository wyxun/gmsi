#include "example.h"
#include "userconfig.h"

int example_Clock(uint32_t wObjectAddr);
int example_Run(uint32_t wObjectAddr);

static gmsi_base_t s_tExampleBase;
gmsi_base_cfg_t s_tExampleBaseCfg = {
    .wId = EXAMPLE,
    /* ��ȡ��ָ�� */
    .wParent = 0,
    .FcnInterface = {
        .Clock = example_Clock,
        .Run = example_Run,
    },
};

static void example_EventHandle(example_t *ptThis, uint32_t wEvent)
{
    if(wEvent & Gmsi_Event_Transition)
    {
        printf("get event Transition\n");
    }
    if(wEvent & Event_SyncButtonPushed)
    {

    }
    if(wEvent & Event_PacketReceived)
    {

    }
}
int example_Run(uint32_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;
    // ָ����
    example_t *ptThis = (example_t *)wObjectAddr;
    GMSI_ASSERT(NULL != ptThis);

    // �¼�����
    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent)
        example_EventHandle(ptThis, wEvent);
    
    // �߼������״̬������

    return wRet;
}

int example_Clock(uint32_t wObjectAddr)
{
    return 0;
}

//int example_Init(example_t *ptThis, example_cfg_t *ptCfg)
int example_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr)
{
    int wRet = GMSI_SUCCESS;
    example_t *ptThis = (example_t *)wObjectAddr;
    example_cfg_t *ptCfg = (example_cfg_t *)wObjectCfgAddr;
    GMSI_ASSERT(NULL != ptThis);
    GMSI_ASSERT(NULL != ptCfg);
    
    ptThis->ptBase = &s_tExampleBase;
    ptThis->chExampleData = ptCfg->chExampleData;

    // ��Դ������
    if(NULL == ptThis->ptBase)
        wRet = GMSI_EAGAIN;
    else
    {
        // �趨��ָ��
        s_tExampleBaseCfg.wParent = wObjectAddr;
        wRet = gbase_Init(ptThis->ptBase, &s_tExampleBaseCfg);
    }
    return wRet;
}