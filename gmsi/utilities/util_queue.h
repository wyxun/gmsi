#ifndef __UTIL_QUEUE_H__
#define __UTIL_QUEUE_H__

#include <stdint.h>

#define qdata_t uint8_t

typedef enum {
    QUEUE_OK,
    QUEUE_FULL,
    QUEUE_EMPTY
}qstatus_t;

typedef struct {
    uint16_t addr_wr;        /* 写地址 */
    uint16_t addr_rd;        /* 读地址 */
    uint16_t length;         /* FIFO长度，实际存放的数据=length-1 */
    qdata_t* pfifo;
}util_queue_t;

qstatus_t queue_init(util_queue_t *q, uint8_t* pchData, uint16_t hwLength);
qstatus_t queue_read(util_queue_t *q, qdata_t *pdata);
qstatus_t queue_write(util_queue_t *q, qdata_t data);
int queue_isFull(util_queue_t *q);
int queue_isEmpty(util_queue_t *q);
int queue_print(util_queue_t *q);

#define gmsi_container_of(pointer, type, member) \
    ((type *)((char *)(pointer) - (unsigned long)(&((type *)0)->member)))
        
#endif