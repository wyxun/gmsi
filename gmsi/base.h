#ifndef __GMSI_BASE_H__
#define __GMSI_BASE_H__
/*============================ INCLUDES ======================================*/
#include "global_define.h"
/*============================ MACROS ========================================*/


/*============================ MACROFIED FUNCTIONS ===========================*/                   
/*============================ TYPES =========================================*/
typedef struct{
    /*��Ϣָ�� �������͸���Ӧ��ȷ��*/
    uint8_t *pchMessage;
    /* ��Ϣ���� */
    uint16_t hwLength;
}message_t;

typedef struct{
    uint32_t wId;
}gmsi_base_cfg_t;
typedef struct {
    /* id�ָߵͲ� ��8λΪ�� ��8λΪʵ�����*/
    uint32_t wId;
    /* �¼����������ߺ������� */
    uint32_t wEvent;
    /* ��Ϣ��������֮���ͨ������ */
    message_t tMessage;

    /* ���������������ڵ� */
    struct xLIST_ITEM   tListItem;
}gmsi_base_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/
extern int gbase_Init(gmsi_base_t *ptBase, gmsi_base_cfg_t *ptCfg);
extern gmsi_base_t *gbase_New(void);
// �¼�
extern int gbase_EventSend(uint32_t wId, uint32_t wEvent);
extern uint32_t gbase_EventGet(gmsi_base_t *ptBase);
// ��Ϣ
extern int gbase_MessageSet(uint32_t wId, uint8_t *pchMessage, uint16_t hwLength);
extern struct xLIST* gbase_GetBaseList(void);
#endif

