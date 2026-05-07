#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "modus.h"
#include "mstorage.h"
#include "pc_clock.h"
#include "pc_uart.h"
#include "mdebug/util_debug.h"

#define BUFFER_SIZE 100

MODUS_DECLARE_OBJECT(pcuart, PcUart, 
    .pchCom = "/dev/ttyS5",
    .wOflag = O_RDWR | O_NOCTTY | O_NONBLOCK,
);

uint8_t ReceiveData[100];

MODUS_DECLARE_OBJECT(pcclock, Pcclock, 
    .chClockbase = 5,
);

/*============================ VIRTUAL FLASH FOR PC ==========================*/
static uint8_t s_chVirtualFlash[2048];

static int32_t pc_flash_erase(void *pPriv, uint32_t wAddr, uint32_t wSize)
{
    if (wAddr + wSize > sizeof(s_chVirtualFlash)) return -1;
    memset(&s_chVirtualFlash[wAddr], 0xFF, wSize);
    return 0;
}

static int32_t pc_flash_write(void *pPriv, uint32_t wAddr, const uint8_t *pchData, uint32_t wLen)
{
    if (wAddr + wLen > sizeof(s_chVirtualFlash)) return -1;
    memcpy(&s_chVirtualFlash[wAddr], pchData, wLen);
    return 0;
}

static int32_t pc_flash_read(void *pPriv, uint32_t wAddr, uint8_t *pchBuf, uint32_t wLen)
{
    if (wAddr + wLen > sizeof(s_chVirtualFlash)) return -1;
    memcpy(pchBuf, &s_chVirtualFlash[wAddr], wLen);
    return 0;
}

static int32_t pc_flash_unlock(void *pPriv) { return 0; }
static int32_t pc_flash_lock(void *pPriv)   { return 0; }

static mdi_flash_t s_tFlash = {
    .pPriv    = NULL,
    .fnErase  = pc_flash_erase,
    .fnWrite  = pc_flash_write,
    .fnRead   = pc_flash_read,
    .fnUnlock = pc_flash_unlock,
    .fnLock   = pc_flash_lock,
};

/*============================ STORAGE OBJECT ================================*/
/* Note: mstorage appends 2 bytes of CRC at the end of the buffer, 
   so we need 16 * uint16_t + 2 bytes */
uint16_t g_hwSystemDataArrary[16 + 1] = {0}; 

mstorage_data_t tSysData = {
    .ptFlash             = &s_tFlash,
    .wFlashAddr          = 0,
    .pchStorageStartAddr = (uint8_t *)g_hwSystemDataArrary,
    .hwStorageLength     = 16 * sizeof(uint16_t),
};

MODUS_DECLARE_OBJECT(mstorage, GStorage, 
    .ptStorageObject  = &tSysData,
    .hwStorageTimeOut = 100, // 100ms cycle
);

/* GLOBAL MODUS CONFIGURATION */
modus_t tModus = {
    .ptAppFlash = &s_tFlash,
};

int main()
{   
    MLOG(I, "Starting POSIX UART Example...\n");
    
    /* Using (uintptr_t) then (uint32_t) to avoid warning on 64-bit systems */
    MLOG(D, "System Data Address: 0x", (uint32_t)(uintptr_t)g_hwSystemDataArrary, "\n");
    
    modus_Init(&tModus);

    MLOG(I, "Entering main loop.\n");
    while (1)
    {
        modus_Run();
    }

    return 0;
}

void timer_handler(int signum)
{
    modus_Clock();
}

