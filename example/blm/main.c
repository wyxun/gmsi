/**
 * @file main.c
 * @brief BLM Bootloader Entry Point
 * 
 * Uses PERFC_PT state machine following perf_counter conventions
 */

/*============================ INCLUDES ======================================*/
#include "core/blm.h"
#include "port/blm_port.h"
#include "gstorage.h"
#include "gblinfo.h"
#include <perf_counter.h>
#include "utilities/util_debug.h"
#include "port/gdi_hw.h"
#include "blm_waveform_test.h"
/* blm_shell_test.h 已不再需要，通过 GMSI_SHELL_CMD 自动发现 */

#if defined(AT32F407xx)
#   include "cmsis/at32f407xx.h"
#elif defined(STM32G431xx)
#   include "cmsis/stm32g431xx.h"
#endif

#if defined(__IS_COMPILER_ARM_COMPILER_5__)
#   pragma diag_suppress 550, 546, 111
#endif

/*============================ MACROS ========================================*/
#undef this
#define this    (*ptThis)

/*============================ TYPES =========================================*/

/**
 * @brief Test application data structure for GStorage
 */
typedef struct {
    uint8_t  chVar1;
    uint16_t hwVar2;
    uint32_t wVar3;
    int8_t   nVar4;
    uint32_t wVar5;
    uint16_t hwCrc; // Reserved for GStorage
} test_app_data_t;

/**
 * @brief LED blink control block (following pt_example convention)
 */
typedef struct {
    uint8_t chState;
} led_blink_cb_t;

/*============================ GLOBAL VARIABLES ==============================*/

/* Receive Buffer - reduced for size optimization */
#define BLM_RX_BUFFER_SIZE  512
static uint8_t s_achBlmRxBuffer[BLM_RX_BUFFER_SIZE];

/* LED blink control block */
static led_blink_cb_t s_tLedBlink;

/* Test application data */
static test_app_data_t s_tAppData = {
    .chVar1 = 0xAA,
    .hwVar2 = 0xBBBB,
    .wVar3 = 0xCCCCCCCC,
    .nVar4 = -10,
    .wVar5 = 0x12345678
};

/*============================ OBJECT DECLARATION ============================*/

GMSI_DECLARE_OBJECT(blm, Blm,
    .wAppAddr = BLM_APP_ADDR,
    .wAppMaxSize = BLM_APP_MAX_SIZE,
    .wSharedInfoAddr = BLM_SHARED_INFO_ADDR,
    .wTimeoutMs = BLM_WAIT_TIMEOUT_MS,
    .pchRxBuffer = s_achBlmRxBuffer,
    .hwRxBufferSize = BLM_RX_BUFFER_SIZE,
);

GMSI_DECLARE_OBJECT(gblinfo, Gblinfo,
    .wSharedInfoAddr = GBLINFO_SHARED_ADDR,
);

/* Flash Storage Address (Page for storing app config) */
#define GMSI_STORAGE_FLASH_ADDR     0x0800F800

/* Storage data descriptor — ptFlash filled at runtime before gmsi_Init() */
static gstorage_data_t s_tStorageData = {
    .ptFlash             = NULL,
    .wFlashAddr          = GMSI_STORAGE_FLASH_ADDR,
    .pchStorageStartAddr = (uint8_t *)&s_tAppData,
    .hwStorageLength     = sizeof(test_app_data_t) - 2,
};

GMSI_DECLARE_OBJECT(gstorage, GStorage,
    .ptStorageObject = &s_tStorageData,
    .hwStorageTimeOut = 65000,
);

/*============================ IMPLEMENTATION ================================*/

/**
 * @brief Initialize LED blink control block
 */
led_blink_cb_t *led_blink_init(led_blink_cb_t *ptThis)
{
    if (NULL == ptThis) {
        return NULL;
    }
    
    this.chState = 0;
    
    return ptThis;
}

/**
 * @brief LED blink task using PERFC_PT (following pt_example convention)
 */
fsm_rt_t led_blink_task(led_blink_cb_t *ptThis)
{
PERFC_PT_BEGIN(this.chState)

    do {
        /* LED ON */
        GDI_Write(HW.ptLedStatus, GDI_GPIO_HIGH);
        
    PERFC_PT_DELAY_MS(500);
        
        /* LED OFF */
        GDI_Write(HW.ptLedStatus, GDI_GPIO_LOW);
        
    PERFC_PT_DELAY_MS(500);

    } while (1);

PERFC_PT_END()

    return fsm_rt_cpl;
}

/*============================ SYSTEM INIT ===================================*/

/**
 * @brief System initialization
 */
static void System_Init(void)
{
    /* Ensure VTOR points to Flash */
    SCB->VTOR = FLASH_BASE;
    
    /* Initialize Hardware */
    blm_port_UartInit(115200);
    blm_port_FlashInit();

    /* Initialize perf_counter */
    SystemCoreClock = BLM_SYSCLK;
    perfc_init(false);
}

/*============================ GMSI CONFIG ===================================*/

static gmsi_t s_tGmsi;  /* ptAppFlash assigned at runtime before gmsi_Init */

/*============================ MAIN ==========================================*/

/**
 * @brief Bootloader main entry point
 */
int main(void)
{
    /* System initialization */
    System_Init();
    
    /* Initialize TRACE */
    TRACE.Init(NULL);
    GLOG(I, "BLM Bootloader Started\r\n");
    
    /* Initialize GMSI framework — ptAppFlash is bound internally */
    s_tGmsi.ptAppFlash = HW.ptAppFlash;
    gmsi_Init(&s_tGmsi);
    /* Initialize LED blink task */
    led_blink_init(&s_tLedBlink);
    
    /* Initialize Waveform Test */
    blm_waveform_test_init();
    
    __enable_irq();
    uint32_t wCounter = 0;
    /* Main loop */
    while (1) {
        /* Run GMSI (includes BLM and GStorage) */
        gmsi_Run();
        
        /* Run LED blink task */
        led_blink_task(&s_tLedBlink);

        /* Simulate variable changes to trigger GStorage auto-save */
        if (perfc_is_time_out_ms(10000)) { 
            s_tAppData.chVar1++;
            s_tAppData.wVar3 += 100;
            GLOGF(I, "chVar1: %d\n", s_tAppData.chVar1);
            GLOGF(I, "wVar3: 0x%08x\n", s_tAppData.wVar3);
        }
        if (perfc_is_time_out_ms(1000)) {
            wCounter++;
            GLOGF(I, "[TICK] %lu s  SYSCLK=%lu Hz\r\n",
                (unsigned long)wCounter,
                (unsigned long)SystemCoreClock);
            /* 验证 %f 浮点支持（v0.2.1.0 新增）*/
            float fTemp = 36.5f + (float)(wCounter % 10) * 0.1f;
            GLOGF(I, "Temp: %f C\r\n", fTemp);
        }
    }
    
    return 0;
}

/*============================ INTERRUPTS ====================================*/

/**
 * @brief SysTick interrupt handler (1ms)
 */
void SysTick_Handler(void)
{
    /* GMSI clock tick (1ms) */
    gmsi_Clock();
    
    /* Waveform test step (1ms) */
    blm_waveform_test_step();
}
