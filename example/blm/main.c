/**
 * @file main.c
 * @brief BLM Bootloader Entry Point
 * 
 * Uses PERFC_PT state machine following perf_counter conventions
 */

/*============================ INCLUDES ======================================*/
#include "core/blm.h"
#include "gblinfo.h"
#include <perf_counter.h>

#if defined(__IS_COMPILER_ARM_COMPILER_5__)
#   pragma diag_suppress 550, 546, 111
#endif

/*============================ MACROS ========================================*/
#undef this
#define this    (*ptThis)

/*============================ TYPES =========================================*/

/**
 * @brief LED blink control block (following pt_example convention)
 */
typedef struct {
    uint8_t chState;
} led_blink_cb_t;

/*============================ GLOBAL VARIABLES ==============================*/

/* Receive Buffer */
#define BLM_RX_BUFFER_SIZE  256
static uint8_t s_achBlmRxBuffer[BLM_RX_BUFFER_SIZE];

/* LED blink control block */
static led_blink_cb_t s_tLedBlink;

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
        /* LED ON - Toggle LED or visual indicator */
        /* blm_port_LedSet(1); */
        
    PERFC_PT_DELAY_MS(500);
        
        /* LED OFF */
        /* blm_port_LedSet(0); */
        
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
    /* SystemInit() is typically called by startup code */
    
    /* Initialize perf_counter with SysTick */
    perfc_init(false);  /* We manage SysTick ourselves */
}

/*============================ MAIN ==========================================*/

/**
 * @brief Bootloader main entry point
 */
int main(void)
{
    /* System initialization */
    System_Init();
    
    /* Initialize GMSI framework */
    gmsi_Init(NULL);
    
    /* Initialize LED blink task */
    led_blink_init(&s_tLedBlink);
    
    /* Main loop */
    while (1) {
        /* Run GMSI (includes BLM state machine) */
        gmsi_Run();
        
        /* Run LED blink task */
        led_blink_task(&s_tLedBlink);
    }
    
    return 0;
}

/*============================ INTERRUPTS ====================================*/

/**
 * @brief SysTick interrupt handler
 * 
 * Called by startup code, updates perf_counter and GMSI clock
 */
void SysTick_Handler(void)
{
    /* perf_counter overflow handler */
    perfc_port_insert_to_system_timer_insert_ovf_handler();
    
    /* GMSI clock tick */
    gmsi_Clock();
}
