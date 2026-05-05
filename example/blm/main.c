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
#include "gdebug/util_debug.h"
#include "port/gdi_hw.h"
#if GWAVEFORM_ENABLE
#include "gdebug/gwaveform.h"
#include "blm_waveform_test.h"
#endif



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

bool    s_bPerfMonitorEnable = false;
static int64_t s_lMaxLoopCycles = 0;

static int64_t s_lMaxGmsiCycles = 0;
static int64_t s_lMaxBlinkCycles = 0;
static int64_t s_lMaxLogCycles = 0;

/* Detailed GMSI Sub-Section Trackers */
int64_t s_lMaxGmsiSubListCycles = 0;
int64_t s_lMaxGmsiSubShellCycles = 0;
int64_t s_lMaxGmsiSubWaveCycles = 0;
int64_t s_lMaxSysTickCycles = 0;      /* Total time in SysTick ISR */
int64_t s_lMaxGmsiClockCycles = 0;    /* Time in gmsi_Clock() */
int64_t s_lMaxWaveStepCycles = 0;     /* Time in blm_waveform_test_step() */

/* Specific Module Trackers (replaces GMSI_List general view) */

int64_t s_lMaxBlmCycles = 0;
int64_t s_lMaxGinfoCycles = 0;





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
    
    /* VERY IMPORTANT: Initialize 120MHz clock before anything else! */
    extern void blm_port_SystemClockConfig(void);
    blm_port_SystemClockConfig();
    
    /* Initialize Hardware */
    blm_port_UartInit(115200);
    blm_port_FlashInit();

    /* Initialize perf_counter:
     * MUST be false to allow perf_counter to set SysTick LOAD to 1ms
     */
    SystemCoreClock = BLM_SYSCLK;
    perfc_init(false);
    
    /* Ensure SysTick has the absolute highest priority! */
    NVIC_SetPriority(SysTick_IRQn, 0);
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
    
#if GSHELL_ENABLE
    /* Initialize TRACE */
    TRACE.Init(NULL);
#endif
    GLOG(I, "BLM Bootloader Started (Optimizing Waveform)\r\n");
    
    /* Initialize GMSI framework — ptAppFlash is bound internally */
    s_tGmsi.ptAppFlash = HW.ptAppFlash;
    gmsi_Init(&s_tGmsi);
    /* Initialize LED blink task */
    led_blink_init(&s_tLedBlink);
    
#if GWAVEFORM_ENABLE
    /* Initialize Waveform Test */
    blm_waveform_test_init();
#endif
    
    __enable_irq();



    uint32_t wCounter = 0;
    /* Main loop */
    while (1) {
        /* Capture system state to detect interruptions during measurement */
        extern int64_t get_system_ms(void);
        int64_t lTickStart, lTickEnd;
        int64_t lStart = get_system_ticks();

        /* Granular measurement (Non-invasive + Clean Pass Filtering) */
        int64_t lSectionStart;
        
        lTickStart = get_system_ms();
        lSectionStart = get_system_ticks();
        gmsi_Run();
        int64_t lGmsiUsed = get_system_ticks() - lSectionStart;
        lTickEnd = get_system_ms();
        if (s_bPerfMonitorEnable && (lTickStart == lTickEnd) && (lGmsiUsed > s_lMaxGmsiCycles)) 
            s_lMaxGmsiCycles = lGmsiUsed;

        lTickStart = get_system_ms();
        lSectionStart = get_system_ticks();
        led_blink_task(&s_tLedBlink);
        int64_t lBlinkUsed = get_system_ticks() - lSectionStart;
        lTickEnd = get_system_ms();
        if (s_bPerfMonitorEnable && (lTickStart == lTickEnd) && (lBlinkUsed > s_lMaxBlinkCycles)) 
            s_lMaxBlinkCycles = lBlinkUsed;

        lSectionStart = get_system_ticks();
        /* Maintenance tasks */
        if (perfc_is_time_out_ms(1000)) {
            wCounter++;
            // trigger save value
            // s_tAppData.chVar1 += 1;
            GLOGF(T, "[TICK] %lu s  SYSCLK=%lu Hz\r\n",
                (unsigned long)wCounter,
                (unsigned long)SystemCoreClock);
            
            float fTemp = 36.5f + (float)(wCounter % 10) * 0.1f;

            /* T-level verification: placed inside timer callback to confirm
             * periodic trigger. Use 'log -T' in gshell to toggle visibility. */
            GLOG(T, "[timer] 1s tick reached\r\n");
            GLOGF(T, "[timer] counter=%lu temp=%.1f\r\n",
                (unsigned long)wCounter, fTemp);
        }
        int64_t lLogUsed = get_system_ticks() - lSectionStart;
        /* Log maintenance (TICK) always overlaps with tick or takes long, 
           so we allow artifact here or just measure total pass */
        if (s_bPerfMonitorEnable && (lLogUsed > s_lMaxLogCycles)) s_lMaxLogCycles = lLogUsed;

        /* Update Max Loop cycls (Simple direct subtract) */
        if (s_bPerfMonitorEnable) {
            int64_t lUsed = get_system_ticks() - lStart;
            /* For the entire loop, we ONLY record if it was NOT interrupted by SysTick */
            if (get_system_ms() == lTickStart) {
                if (lUsed > s_lMaxLoopCycles) s_lMaxLoopCycles = lUsed;
            }
        }
    }








    
    return 0;
}

static void cmd_perf(const char *args)
{
    if (strncmp(args, "on", 2) == 0) {
        s_bPerfMonitorEnable = true;
        s_lMaxLoopCycles = 0;
        s_lMaxGmsiCycles = 0;
        s_lMaxBlinkCycles = 0;
        s_lMaxLogCycles = 0;
        s_lMaxGmsiSubListCycles = 0;
        s_lMaxGmsiSubShellCycles = 0;
        s_lMaxGmsiSubWaveCycles = 0;
        s_lMaxBlmCycles = 0;
        s_lMaxGinfoCycles = 0;
        GLOG(I, "Perf Monitor: ON (Max cleared)\r\n");
    } else if (strncmp(args, "off", 3) == 0) {
        s_bPerfMonitorEnable = false;
        GLOG(I, "Perf Monitor: OFF\r\n");
    } else if (strncmp(args, "status", 6) == 0) {
        GLOGF(I, "Perf: %s\r\n", s_bPerfMonitorEnable ? "ON" : "OFF");
        GLOGF(I, " - Max Loop : %d us\r\n", (int)perfc_convert_ticks_to_us(s_lMaxLoopCycles));
        GLOGF(I, " - Max GMSI : %d us\r\n", (int)perfc_convert_ticks_to_us(s_lMaxGmsiCycles));
        GLOGF(I, " - Max Blink: %d us\r\n", (int)perfc_convert_ticks_to_us(s_lMaxBlinkCycles));
        GLOGF(I, " - Max Log  : %d us\r\n", (int)perfc_convert_ticks_to_us(s_lMaxLogCycles));
        GLOGF(I, " - Max ISR  : %d us (Clock %d, Wave %d)\r\n", 
            (int)perfc_convert_ticks_to_us(s_lMaxSysTickCycles),
            (int)perfc_convert_ticks_to_us(s_lMaxGmsiClockCycles),
            (int)perfc_convert_ticks_to_us(s_lMaxWaveStepCycles));
        
#if GWAVEFORM_ENABLE
        GLOGF(I, " - Wave Drop: %lu (Cumulative), %lu (Last 1s)\r\n",
            (unsigned long)gwaveform.GetDropCount(),
            (unsigned long)gwaveform.GetLastIntervalDrops());
        extern uint32_t gwaveform_GetRTTFullCount(void);
        GLOGF(I, " - RTT Congest: %lu times\r\n", (unsigned long)gwaveform_GetRTTFullCount());
#endif



    } else if (strncmp(args, "clear", 5) == 0) {


        s_lMaxLoopCycles = 0;
        s_lMaxGmsiCycles = 0;
        s_lMaxBlinkCycles = 0;
        s_lMaxLogCycles = 0;
        s_lMaxGmsiSubListCycles = 0;
        s_lMaxGmsiSubShellCycles = 0;
        s_lMaxGmsiSubWaveCycles = 0;
        s_lMaxBlmCycles = 0;
        s_lMaxGinfoCycles = 0;
        s_lMaxSysTickCycles = 0;
        s_lMaxGmsiClockCycles = 0;
        s_lMaxWaveStepCycles = 0;
#if GWAVEFORM_ENABLE
        gwaveform.ClearDropCount();
        GLOG(I, "Perf: Max cycles and Wave drops cleared.\r\n");
#endif
    } else {
        GLOG(I, "Usage: perf <on|off|status|clear>\r\n");
    }
}


GMSI_SHELL_CMD(perf, cmd_perf, "Performance monitor (on/off/status/clear)");


/*============================ INTERRUPTS ====================================*/

/**
 * @brief SysTick interrupt handler (1ms)
 */
void SysTick_Handler(void)
{
    int64_t lStart = get_system_ticks();

    /* GMSI clock tick (1ms) */
    gmsi_Clock();
    
#if GWAVEFORM_ENABLE
    /* Waveform test step (1ms) */
    blm_waveform_test_step();
#endif

    if (s_bPerfMonitorEnable) {
        int64_t lUsed = get_system_ticks() - lStart;
        if (lUsed > s_lMaxSysTickCycles) s_lMaxSysTickCycles = lUsed;
    }
}



