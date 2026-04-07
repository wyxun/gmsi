#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include "example.h"
#include "template.h"
#include "gstorage.h"
#include "utilities/util_debug.h"

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

static gdi_flash_t s_tFlash = {
    .pPriv    = NULL,
    .fnErase  = pc_flash_erase,
    .fnWrite  = pc_flash_write,
    .fnRead   = pc_flash_read,
    .fnUnlock = pc_flash_unlock,
    .fnLock   = pc_flash_lock,
};

/*============================ STORAGE OBJECT ================================*/
/* Note: gstorage appends 2 bytes of CRC at the end of the buffer */
uint16_t g_hwSystemDataArrary[16 + 1] = {0};

gstorage_data_t tSysData = {
    .ptFlash             = &s_tFlash,
    .wFlashAddr          = 0,
    .pchStorageStartAddr = (uint8_t *)g_hwSystemDataArrary,
    .hwStorageLength     = 16 * sizeof(uint16_t),
};

GMSI_DECLARE_OBJECT(gstorage, GStorage, 
    .ptStorageObject  = &tSysData,
    .hwStorageTimeOut = 100, // 100ms cycle
);

/* GLOBAL GMSI CONFIGURATION */
gmsi_t tGmsi = {
    .ptAppFlash = &s_tFlash,
};

int main()
{   
    // example hardware init
    GLOG(I, "Starting Template Example...\n");
    
    gmsi_Init(&tGmsi);
    GLOG(I, "Entering main loop.\n");
    while (1)
    {
        gmsi_Run();
    }

    return GMSI_SUCCESS;
}

void timer_handler(int signum)
{
    gmsi_Clock();
}