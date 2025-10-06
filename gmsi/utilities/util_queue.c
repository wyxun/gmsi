#include "util_queue.h"

qstatus_t queue_init(util_queue_t *q, uint8_t* pchData, uint16_t hwLength)
{
    int i = 0;

    q->addr_wr = 0;
    q->addr_rd = 0;
    q->pfifo = pchData;
    q->length = hwLength;
    for(i = 0; i < q->length; i++)
        q->pfifo[i] = 0;

    return QUEUE_OK;
}

qstatus_t queue_write(util_queue_t *q, qdata_t data)
{
    if(queue_isFull(q))
    {
        return QUEUE_FULL;
    }

    q->pfifo[q->addr_wr] = data;
    q->addr_wr = (q->addr_wr + 1) % q->length;
    
    return QUEUE_OK;
}

qstatus_t queue_read(util_queue_t *q, qdata_t *pdata)
{
    if(queue_isEmpty(q))
    {
        return QUEUE_EMPTY;
    }

    *pdata = q->pfifo[q->addr_rd];
    q->addr_rd = (q->addr_rd + 1) % q->length;
    
    return QUEUE_OK;
}

int queue_isEmpty(util_queue_t *q)
{
    return (q->addr_wr == q->addr_rd);
}

int queue_isFull(util_queue_t *q)
{
    return ((q->addr_wr + 1) % q->length == q->addr_rd);
}

int queue_count(util_queue_t *q)
{
    if(q->addr_rd <= q->addr_wr)
        return (q->addr_wr - q->addr_rd);

    return (q->length + q->addr_wr - q->addr_rd);
}


