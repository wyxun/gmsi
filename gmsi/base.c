#include "global_define.h"
#include "utilities/list.h"
#include "utilities/util_debug.h"
#include "base.h"

#ifdef PC_DEBUG
#include <stdio.h>
#include <assert.h>
#endif
#define GMSI_BASE_LENGTH    20
//static gmsi_base_t g_tBase[GMSI_BASE_LENGTH];
// ������ڵ�
struct xLIST        tListObject;

int gbase_Init(gmsi_base_t *ptBase, gmsi_base_cfg_t *ptCfg)
{
    int wRet = GMSI_SUCCESS;
    static uint8_t chInitCount = 0;
    // ��ʼ������������ڵ�
    if(!chInitCount)
        vListInitialise(&tListObject);
    chInitCount++;
    // ��ʼ������id
    ptBase->wId = ptCfg->wId;

    /* �¼���ʼ��Ϊ0 */
    ptBase->wEvent = 0;
    /* ����ڵ��ʼ�� */ 
    vListInitialiseItem(&ptBase->tListItem);
    ptBase->tListItem.xItemValue = ptBase->wId;
    ptBase->tListItem.pvOwner = ptBase;
    // �ڵ��������
    vListInsert(&tListObject, &ptBase->tListItem);

    // ��������ӿ�
    if(ptCfg->wParent)
        ptBase->wParent = ptCfg->wParent;
    else
        wRet = GMSI_EAGAIN;
    ptBase->pFcnInterface = &ptCfg->FcnInterface;
    #ifdef PC_DEBUG
    //printf("init base %d\n", ptBase->wId);
    //printf("ptCfg wParent %d\n", ptCfg->wParent);
    #endif
    return wRet;
}

// �¼�����
int gbase_EventPost(uint32_t wId, uint32_t wEvent)
{
    int wRet = GMSI_SUCCESS;
    uint8_t chErgodicTime = 1;
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;;
    gmsi_base_t *ptBaseDes;
    // ��������ȷ��Ŀ��id
    while(ptListItemDes != &tListObject.xListEnd){
        if(ptListItemDes->xItemValue == wId)
            break;
        // ��ƥ���������
        ptListItemDes = ptListItemDes->pxPrevious;
        chErgodicTime++;
    }
    if(chErgodicTime <= tListObject.uxNumberOfItems)
    {
        // �ҵ���Ӧ��object
        ptBaseDes = ptListItemDes->pvOwner;
        assert(NULL != ptBaseDes);
        // ����Ŀ��id�����¼�ֵ
        ptBaseDes->wEvent |= wEvent;
    }
    else
        wRet = GMSI_ENODEV;
    
    return wRet;
}

// �¼�����
uint32_t gbase_EventPend(gmsi_base_t *ptBase)
{
    uint32_t wEvent = ptBase->wEvent;
    if(0 != wEvent)
    {
        ptBase->wEvent = 0;
    }
        return wEvent;
}

int gbase_MessagePost(uint32_t wId, uint8_t *pchMessage, uint16_t hwLength)
{
    int wRet = 0;
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;
    uint8_t chErgodicTime = 1;
    gmsi_base_t *ptBaseDes;
    // ��������ȷ��Ŀ��id
    while(ptListItemDes != &tListObject.xListEnd){
        if(ptListItemDes->xItemValue == wId)
            break;
        // ��ƥ���������
        chErgodicTime++;
        ptListItemDes = ptListItemDes->pxPrevious;
    }
    if(chErgodicTime <= tListObject.uxNumberOfItems)
    {
        // �ҵ���Ӧ��object
        ptBaseDes = ptListItemDes->pvOwner;
        assert(NULL != ptBaseDes);
        // ������Ӧ��object
        ptBaseDes->tMessage.pchMessage= pchMessage;
        ptBaseDes->tMessage.hwLength = hwLength;
        ptBaseDes->wEvent |= Gmsi_Event_Transition;
    }
    else
        wRet = GMSI_ENODEV;

    return wRet;
}

void gbase_DegugListBase(void)
{
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;
    printf("List all object:\n");
    // ��������
    while(ptListItemDes != &tListObject.xListEnd){
        printf("    itme id: %d\n", ptListItemDes->xItemValue);
        ptListItemDes = ptListItemDes->pxPrevious;
    }
}

struct xLIST* gbase_GetBaseList(void)
{
    return &tListObject;
}