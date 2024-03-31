#include "global_define.h"
#include "utilities/list.h"
#include "utilities/util_debug.h"
#include "base.h"
#define GMSI_BASE_LENGTH    20
static gmsi_base_t g_tBase[GMSI_BASE_LENGTH];
// ������ڵ�
struct xLIST        tListClass;

int gbase_Init(gmsi_base_t *ptBase, gmsi_base_cfg_t *ptCfg)
{
    static uint8_t chInitCount = 0;
    // ��ʼ����������ڵ�
    if(!chInitCount)
        vListInitialise(&tListClass);
    
    // ��ʼ����id
    ptBase->wId = ptCfg->wId;

    /* �¼���ʼ��Ϊ0 */
    ptBase->wEvent = 0;
    /* ����ڵ��ʼ�� */ 
    vListInitialiseItem(&ptBase->tListItem);
    ptBase->tListItem.xItemValue = ptBase->wId;
    ptBase->tListItem.pvOwner = ptBase;
    // �ڵ��������
    vListInsert(&tListClass, &ptBase->tListItem);
    
    return 0;
}

// �¼�����
int gbase_EventSend(gmsi_base_t *ptBaseSrc, uint32_t wId, uint32_t wEvent)
{
    gmsi_base_t *ptBaseDes;
    struct xLIST_ITEM *ptListDes;
    
    // ��������ȷ��Ŀ��id
    for(ptListDes = listGET_HEAD_ENTRY(&tListClass);        \
        ptListDes != NULL;                                  \
        ptListDes = listGET_NEXT(ptListDes)
        )
    {
        if(ptListDes->xItemValue == wId)
            break;
    }
    
    // �ҵ���Ӧ����ʵ��
    if(NULL != ptListDes)
    {
        ptBaseDes = ptListDes->pvOwner;
        // ����Ŀ��id�����¼�ֵ
        ptBaseDes->wEvent |= wEvent;
    }

    return 0;
}

// �¼�����
uint32_t gbase_EventGet(gmsi_base_t *ptBase)
{
    uint32_t wEvent = ptBase->wEvent;
    if(0 != wEvent)
    {
        ptBase->wEvent = 0;
    }
        return wEvent;
}

int gbase_MessageSet(gmsi_base_t *ptBaseSrc, uint32_t wId, void *vpMessage)
{
    gmsi_base_t *ptBaseDes = NULL;
    struct xLIST_ITEM *ptListDes;
    
    // ��������ȷ��Ŀ��id
    for(ptListDes = listGET_HEAD_ENTRY(&tListClass);        \
        ptListDes != NULL;                                  \
        ptListDes = listGET_NEXT(ptListDes)
        )
    {
        if(ptListDes->xItemValue == wId)
            break;
    }
    
    // �ҵ���Ӧ����ʵ��
    if(NULL != ptListDes)
    {
        ptBaseDes->ptMessage->vpMessage[ptBaseDes->ptMessage->hwLength++] = vpMessage;
    }
    return 0;
}

void* gbase_MessageGet(gmsi_base_t *ptBase)
{
    uint16_t hwLength = ptBase->ptMessage->hwLength;
    if(hwLength)
        return ptBase->ptMessage->vpMessage[hwLength--]; 
    else return NULL;
    //return ptBase->ptMessage;
}

gmsi_base_t *gbase_New(void)
{
    static uint8_t s_chBaseOffset = 0;
    if(s_chBaseOffset < GMSI_BASE_LENGTH)
        return &g_tBase[s_chBaseOffset];
    else
        return NULL;
}

void gbase_DegugListBase(void)
{
    //gmsi_base_t *ptBase;
    struct xLIST_ITEM *ptList;
    // ��������
    for(ptList = listGET_HEAD_ENTRY(&tListClass);        \
        ptList != NULL;                                  \
        ptList = listGET_NEXT(ptList)
        )
    {
        LOG_OUT("\r\n");
        LOG_OUT(ptList->xItemValue);
    }
}

struct xLIST* gbase_GetBaseList(void)
{
    return &tListClass;
}