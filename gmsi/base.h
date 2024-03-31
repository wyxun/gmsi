#ifndef __GMSI_BASE_H__
#define __GMSI_BASE_H__
/*============================ INCLUDES ======================================*/
#include "global_define.h"
/*============================ MACROS ========================================*/


/*============================ MACROFIED FUNCTIONS ===========================*/                   
/*============================ TYPES =========================================*/
typedef struct{
    /*消息指针 具体类型根据应用确定*/
    void *vpMessage[5];
    /* 消息长度 */
    uint16_t hwLength;
}message_t;

typedef struct{
    uint32_t wId;
}gmsi_base_cfg_t;
typedef struct {
    /* id分高低部 高8位为类 低8位为实例编号*/
    uint32_t wId;
    /* 事件：有生产者和消费者 */
    uint32_t wEvent;
    /* 消息：传递类之间的通信数据 */
    message_t *ptMessage;
    /* 控制的Id */
    // uint32_t wControlId;
    /* 串行所有类的链表节点 */
    struct xLIST_ITEM   tListItem;
}gmsi_base_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/
extern int gbase_Init(gmsi_base_t *ptBase, gmsi_base_cfg_t *ptCfg);
extern gmsi_base_t *gbase_New(void);
// 事件
extern int gbase_EventSend(gmsi_base_t *ptBaseSrc, uint32_t wId, uint32_t wEvent);
extern uint32_t gbase_EventGet(gmsi_base_t *ptBase);
// 消息
extern int gbase_MessageSet(gmsi_base_t *ptBaseSrc, uint32_t wId, void *vpMessage);
extern void* gbase_MessageGet(gmsi_base_t *ptBase);
extern struct xLIST* gbase_GetBaseList(void);
#endif

