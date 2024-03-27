#include "util_queue.h"
#include <stdio.h>

/* pfifo复位 */
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

/* pfifo写入数据 */
qstatus_t queue_write(util_queue_t *q, qdata_t data)
{
    if(queue_isFull(q))
    {
        //printf("Write failed(%d), queue is full\n", data);
        return QUEUE_FULL;
    }

    q->pfifo[q->addr_wr] = data;
    q->addr_wr = (q->addr_wr + 1) % q->length;
    //printf("write success: %02d\n", data);
    //queue_print(q);
    
    return QUEUE_OK;
}

/* pfifo读出数据 */
qstatus_t queue_read(util_queue_t *q, qdata_t *pdata)
{
    if(queue_isEmpty(q))
    {
        //printf("Read failed, queue is empty\n");
        return QUEUE_EMPTY;
    }

    *pdata = q->pfifo[q->addr_rd];
    q->addr_rd = (q->addr_rd + 1) % q->length;

    //printf("read success: %02d\n", *pdata);
    //queue_print(q);
    
    return QUEUE_OK;
}


/* pfifo是否为空 */
int queue_isEmpty(util_queue_t *q)
{
    return (q->addr_wr == q->addr_rd);
}

/* pfifo是否为满 */
int queue_isFull(util_queue_t *q)
{
    return ((q->addr_wr + 1) % q->length == q->addr_rd);
}

/* pfifo内数据的个数 */
int queue_count(util_queue_t *q)
{
    if(q->addr_rd <= q->addr_wr)
        return (q->addr_wr - q->addr_rd);
    //addr_rd > addr_wr;
    return (q->length + q->addr_wr - q->addr_rd);
}

/* 打印当前pfifo内的数据和读写指针的位置 */
int queue_print(util_queue_t *q)
{
    int i = 0;
    int j = 0;

    for(i = 0; i < q->addr_rd; i++)
        printf("     ");

    printf("rd=%d", q->addr_rd);
    printf("\n");

    for(i = 0; i < q->length; i++)
    {
        if(q->addr_wr > q->addr_rd)
        {
            if(i >= q->addr_rd && i < q->addr_wr)
                printf("[%02d] ", q->pfifo[i]);
            else
                printf("[  ] ");
        }
        else//addr_rd > addr_wr
        {
            if(i < q->addr_wr || i >= q->addr_rd)
                printf("[%02d] ", q->pfifo[i]);
            else
                printf("[  ] ");
        }
    }
    printf("------count = %d\n", queue_count(q));

    for(i = 0; i < q->addr_wr; i++)
        printf("     ");

    printf("wr=%d", q->addr_wr);
    printf("\n");

    return QUEUE_OK;
}


