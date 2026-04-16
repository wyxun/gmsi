#include "gringbuf.h"
#include "global_define.h"
#include <string.h>

/*============================ MACROS ========================================*/
#ifndef MIN
#   define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/*============================ IMPLEMENTATION ================================*/

static bool is_power_of_two(uint16_t n)
{
    return (n != 0) && ((n & (n - 1)) == 0);
}

int gringbuf_Init(gringbuf_t *ptRB, uint8_t *pchBuf, uint16_t hwLen)
{
    if (NULL == ptRB || NULL == pchBuf || !is_power_of_two(hwLen)) {
        return GMSI_EINVAL;
    }

    ptRB->pchBuffer = pchBuf;
    ptRB->hwHead = 0;
    ptRB->hwTail = 0;
    ptRB->hwSizeMask = hwLen - 1;

    return GMSI_SUCCESS;
}

uint16_t gringbuf_GetUsed(gringbuf_t *ptRB)
{
    /* Use local copy to ensure consistency during calculation */
    uint16_t hwHead = ptRB->hwHead;
    uint16_t hwTail = ptRB->hwTail;
    return (hwHead - hwTail) & ptRB->hwSizeMask;
}

uint16_t gringbuf_GetFree(gringbuf_t *ptRB)
{
    /* Total capacity is size - 1 to distinguish FULL from EMPTY */
    return ptRB->hwSizeMask - gringbuf_GetUsed(ptRB);
}

uint16_t gringbuf_Write(gringbuf_t *ptRB, uint8_t chByte)
{
    uint16_t hwHead = ptRB->hwHead;
    uint16_t hwNextHead = (hwHead + 1) & ptRB->hwSizeMask;

    if (hwNextHead == ptRB->hwTail) {
        return 0; /* Full */
    }

    ptRB->pchBuffer[hwHead] = chByte;
    ptRB->hwHead = hwNextHead;

    return 1;
}

uint16_t gringbuf_Read(gringbuf_t *ptRB, uint8_t *pchByte)
{
    uint16_t hwTail = ptRB->hwTail;

    if (hwTail == ptRB->hwHead) {
        return 0; /* Empty */
    }

    if (pchByte) {
        *pchByte = ptRB->pchBuffer[hwTail];
    }
    ptRB->hwTail = (hwTail + 1) & ptRB->hwSizeMask;

    return 1;
}

uint16_t gringbuf_WriteBulk(gringbuf_t *ptRB, const uint8_t *pchData, uint16_t hwLen)
{
    if (NULL == pchData || 0 == hwLen) return 0;

    uint16_t hwFree = gringbuf_GetFree(ptRB);
    uint16_t hwToWrite = MIN(hwLen, hwFree);
    if (0 == hwToWrite) return 0;

    uint16_t hwHead = ptRB->hwHead;
    uint16_t hwSize = ptRB->hwSizeMask + 1;
    
    /* First part: to the end of buffer */
    uint16_t hwFirstPart = MIN(hwToWrite, hwSize - hwHead);
    memcpy(&ptRB->pchBuffer[hwHead], pchData, hwFirstPart);

    /* Second part: wrap around */
    if (hwToWrite > hwFirstPart) {
        memcpy(ptRB->pchBuffer, &pchData[hwFirstPart], hwToWrite - hwFirstPart);
    }

    ptRB->hwHead = (hwHead + hwToWrite) & ptRB->hwSizeMask;

    return hwToWrite;
}

uint16_t gringbuf_ReadBulk(gringbuf_t *ptRB, uint8_t *pchData, uint16_t hwLen)
{
    if (0 == hwLen) return 0;

    uint16_t hwUsed = gringbuf_GetUsed(ptRB);
    uint16_t hwToRead = MIN(hwLen, hwUsed);
    if (0 == hwToRead) return 0;

    uint16_t hwTail = ptRB->hwTail;
    uint16_t hwSize = ptRB->hwSizeMask + 1;

    /* First part: to the end of buffer */
    uint16_t hwFirstPart = MIN(hwToRead, hwSize - hwTail);
    if (pchData) {
        memcpy(pchData, &ptRB->pchBuffer[hwTail], hwFirstPart);
    }

    /* Second part: wrap around */
    if (hwToRead > hwFirstPart) {
        if (pchData) {
            memcpy(&pchData[hwFirstPart], ptRB->pchBuffer, hwToRead - hwFirstPart);
        }
    }

    ptRB->hwTail = (hwTail + hwToRead) & ptRB->hwSizeMask;

    return hwToRead;
}
