#include "at32f421_gpio.h"
#include "at32f421_bsp.h"
#include "perf_counter.h"
#include "util_queue.h"
#include "userconfig.h"

#define USART_RXFLAG_IDLE 	0
#define USART_RXFLAG_BUSY 	1
#define USART_RXFLAG_FINISH 2
#define USART_TXFLAG_IDLE 	0
#define USART_TXFLAG_BUSY 	1
#define USART_DELAYTIME 2

extern unsigned int SystemCoreClock;

typedef struct{
    util_queue_t tRXQueue;
    uint8_t chRXFinishTime;
    uint8_t chRXFlag;

    util_queue_t tTXQueue;
    uint8_t chTXFinishTime;
    uint8_t chTXFlag;
    
    uint8_t chWaitBusIdleFlag;
    
    usart_type* ptUsart;
}usartbuffer_t;

usartbuffer_t tUsart1Buffer;
usartbuffer_t tUsart2Buffer;
usartbuffer_t tUsart3Buffer;

// RFID
#if (defined USART1_ENABLE) && (USART1_ENABLE == TRUE)
#define USART1_RX_BUFFER_MAX			100
#define USART1_TX_BUFFER_MAX			100
uint8_t chUsart1TxBuffer[USART1_TX_BUFFER_MAX];
uint8_t chUsart1RxBuffer[USART1_RX_BUFFER_MAX];
#endif

// CONNECT UPPER
#if (defined USART2_ENABLE) && (USART2_ENABLE == TRUE)
#define USART2_RX_BUFFER_MAX			100
#define USART2_TX_BUFFER_MAX			100
uint8_t chUsart2TxBuffer[USART2_TX_BUFFER_MAX];
uint8_t chUsart2RxBuffer[USART2_RX_BUFFER_MAX];
#endif

// UNUSE
const ggpio_t c_tLed = {
    .ptPort = GPIOF,
    .wPin = GPIO_PINS_0
};

const ggpio_t c_tUart2Tx = {
    .ptPort = GPIOA,
    .wPin = GPIO_PINS_2,
    .tGmux = {
        .chSource = GPIO_PINS_SOURCE2,
        .chMux = GPIO_MUX_1
    },
};

const ggpio_t c_tUart2Rx = {
    .ptPort = GPIOA,
    .wPin = GPIO_PINS_3,
    .tGmux = {
        .chSource = GPIO_PINS_SOURCE3,
        .chMux = GPIO_MUX_1
    },
};

const ggpio_t c_tUart485 = {
    .ptPort = GPIOA,
    .wPin = GPIO_PINS_1,
};

const ggpio_t c_tUart1Tx = {
    .ptPort = GPIOA,
    .wPin = GPIO_PINS_9,
    .tGmux = {
        .chSource = GPIO_PINS_SOURCE9,
        .chMux = GPIO_MUX_1
    },
};

const ggpio_t c_tUart1Rx = {
    .ptPort = GPIOA,
    .wPin = GPIO_PINS_10,
    .tGmux = {
        .chSource = GPIO_PINS_SOURCE10,
        .chMux = GPIO_MUX_1
    },
};

const rfid_io_t c_tRFIDIO = {
    .tCardIn = {
        .ptPort = GPIOA,
        .wPin = GPIO_PINS_12,
    },
    .tReset = {
        .ptPort = GPIOA,
        .wPin = GPIO_PINS_11,
    },
};

void system_clock_config(void)
{
    crm_clocks_freq_type crm_clocks_freq_struct = {0};
    /* config flash psr register */
    flash_psr_set(FLASH_WAIT_CYCLE_3);

    /* reset crm */
    crm_reset();

    crm_clock_source_enable(CRM_CLOCK_SOURCE_HICK, TRUE);

    while(crm_flag_get(CRM_HICK_STABLE_FLAG) != SET)
    {
        
    }

    /* config pll clock resource */
    crm_pll_config(CRM_PLL_SOURCE_HICK, CRM_PLL_MULT_30);

    /* enable pll */
    crm_clock_source_enable(CRM_CLOCK_SOURCE_PLL, TRUE);

    /* wait till pll is ready */
    while(crm_flag_get(CRM_PLL_STABLE_FLAG) != SET)
    {
    }

    /* config ahbclk */
    crm_ahb_div_set(CRM_AHB_DIV_1);

    /* config apb2clk, the maximum frequency of APB1/APB2 clock is 120 MHz  */
    crm_apb2_div_set(CRM_APB2_DIV_1);

    /* config apb1clk, the maximum frequency of APB1/APB2 clock is 120 MHz  */
    crm_apb1_div_set(CRM_APB1_DIV_1);

    /* enable auto step mode */
    crm_auto_step_mode_enable(TRUE);

    /* select pll as system clock source */
    crm_sysclk_switch(CRM_SCLK_PLL);

    /* wait till pll is used as system clock source */
    while(crm_sysclk_switch_status_get() != CRM_SCLK_PLL)
    {
    }

    /* disable auto step mode */
    crm_auto_step_mode_enable(FALSE);

    /* update SystemCoreClock global variable */
    system_core_clock_update();
}

void bsp_UartInit(void)
{
    gpio_init_type gpio_init_struct;
    /* enable the usart1 and gpio clock */
    gpio_default_para_init(&gpio_init_struct);
    
#if (defined USART1_ENABLE) && (USART1_ENABLE == TRUE)
    crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);
    // TX
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = c_tUart1Tx.wPin;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(c_tUart1Tx.ptPort, &gpio_init_struct);
    // RX
    gpio_init_struct.gpio_pins = c_tUart1Rx.wPin;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(c_tUart1Rx.ptPort, &gpio_init_struct);

    gpio_pin_mux_config(c_tUart1Rx.ptPort, c_tUart1Rx.tGmux.chSource, c_tUart1Rx.tGmux.chMux);
    gpio_pin_mux_config(c_tUart1Tx.ptPort, c_tUart1Tx.tGmux.chSource, c_tUart1Tx.tGmux.chMux);
  
    nvic_irq_enable(USART1_IRQn, 1, 2);
    /* configure usart1 param */
    usart_init(USART1, 9600, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART1, TRUE);
    usart_receiver_enable(USART1, TRUE);
    
    usart_interrupt_enable(USART1, USART_RDBF_INT, TRUE);
    usart_enable(USART1, TRUE);

	usart_flag_clear(USART1, USART_TDBE_FLAG);
	usart_flag_clear(USART1, USART_TDC_FLAG);
	usart_flag_clear(USART1, USART_RDBF_FLAG);
    
    usart_interrupt_enable(USART1, USART_TDC_INT, TRUE);
    queue_init((util_queue_t *)&tUsart1Buffer.tRXQueue, chUsart1RxBuffer, USART1_RX_BUFFER_MAX);
    queue_init((util_queue_t *)&tUsart1Buffer.tTXQueue, chUsart1TxBuffer, USART1_TX_BUFFER_MAX);
    tUsart1Buffer.ptUsart = USART1;
    
#if (defined USART1_RS485_ENABLE) && (USART1_RS485_ENABLE == TRUE)
    gpio_init_struct.gpio_pins = c_tUart485.wPin;
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(c_tUart485.ptPort, &gpio_init_struct);
#endif
#endif

#if (defined USART2_ENABLE)
    crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);
    // TX
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = c_tUart2Tx.wPin;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(c_tUart2Tx.ptPort, &gpio_init_struct);
    // RX
    gpio_init_struct.gpio_pins = c_tUart2Rx.wPin;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(c_tUart2Rx.ptPort, &gpio_init_struct);

    gpio_pin_mux_config(c_tUart2Rx.ptPort, c_tUart2Rx.tGmux.chSource, c_tUart2Rx.tGmux.chMux);
    gpio_pin_mux_config(c_tUart2Tx.ptPort, c_tUart2Tx.tGmux.chSource, c_tUart2Tx.tGmux.chMux);
  
    nvic_irq_enable(USART2_IRQn, 1, 3);
    /* configure usart2 param */
    usart_init(USART2, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART2, TRUE);
    usart_receiver_enable(USART2, TRUE);
    
    usart_interrupt_enable(USART2, USART_RDBF_INT, TRUE);
    usart_enable(USART2, TRUE);

	usart_flag_clear(USART2, USART_TDBE_FLAG);
	usart_flag_clear(USART2, USART_TDC_FLAG);
	usart_flag_clear(USART2, USART_RDBF_FLAG);
    
    usart_interrupt_enable(USART2, USART_TDC_INT, TRUE);
    queue_init((util_queue_t *)&tUsart2Buffer.tRXQueue, chUsart2RxBuffer, USART2_RX_BUFFER_MAX);
    queue_init((util_queue_t *)&tUsart2Buffer.tTXQueue, chUsart2TxBuffer, USART2_TX_BUFFER_MAX);
    tUsart2Buffer.ptUsart = USART2;
#if (defined USART2_RS485_ENABLE) && (USART2_RS485_ENABLE == TRUE)
    gpio_init_struct.gpio_pins = c_tUart485.wPin;
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(c_tUart485.ptPort, &gpio_init_struct);
#endif
#endif
    
}

void bsp_WatchDogInit(void)
{
    /* disable register write protection */
    wdt_register_write_enable(TRUE);
    /* set the wdt divider value */
    wdt_divider_set(WDT_CLK_DIV_4);
    /* set reload value
    timeout = reload_value * (divider / lick_freq )    (s)
    lick_freq    = 40000 Hz
    divider      = 4
    reload_value = 3000
    timeout = 3000 * (4 / 40000 ) = 0.3s = 300ms
    */
    wdt_reload_value_set(3000 - 1);

    /* reload wdt counter */
    wdt_counter_reload();

    /* enable wdt */
    wdt_enable();
}

void bsp_SystemClockInit(void)
{
    crm_clocks_freq_type crm_clocks_freq_struct = {0};
    #if 1
    crm_clocks_freq_get(&crm_clocks_freq_struct);
    SystemCoreClock = crm_clocks_freq_struct.sclk_freq;
    if(SysTick_Config((crm_clocks_freq_struct.sclk_freq) / 1000))
        {while(1);}
    nvic_irq_enable(SysTick_IRQn, 2, 2);
    init_cycle_counter(true);
    #else
    SystemCoreClock = crm_clocks_freq_struct.sclk_freq;
    #endif
}

void bsp_ClockInit(void)
{
    //crm_periph_clock_enable(CRM_BPR_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
    pwc_battery_powered_domain_access(TRUE);
    //bpr_tamper_pin_enable(FALSE);
    crm_lext_bypass(FALSE);
    crm_clock_source_enable(CRM_CLOCK_SOURCE_LEXT, FALSE);
    pwc_battery_powered_domain_access(FALSE);

    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOF_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_TMR14_PERIPH_CLOCK, TRUE);
    
    //crm_periph_clock_enable(CRM_BPR_PERIPH_CLOCK, TRUE);
//    crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
//    pwc_battery_powered_domain_access(TRUE);
//    crm_lext_bypass(FALSE);
//    crm_clock_source_enable(CRM_CLOCK_SOURCE_LEXT, FALSE);
//    pwc_battery_powered_domain_access(TRUE);
}

void bsp_GpioInit(void)
{
    gpio_init_type gpio_init_struct;
    uint8_t chOffset = 0;
    // output
    gpio_init_struct.gpio_pins = c_tLed.wPin;
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(c_tLed.ptPort, &gpio_init_struct);

    gpio_init_struct.gpio_pins = c_tRFIDIO.tReset.wPin;
    gpio_init(c_tRFIDIO.tReset.ptPort, &gpio_init_struct);

    // input
    gpio_init_struct.gpio_pins = c_tRFIDIO.tCardIn.wPin;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(c_tRFIDIO.tCardIn.ptPort, &gpio_init_struct);
}

void bsp_LedSet(bool bStatus)
{
    if(true == bStatus)
        gpio_bits_reset(c_tLed.ptPort, c_tLed.wPin);
    else
        gpio_bits_set(c_tLed.ptPort, c_tLed.wPin);
}

void bsp_writeIO(io_t *ptIO, bool bStatus)
{
    if(true == bStatus)
        gpio_bits_set(ptIO->ptPort, ptIO->hwPin);
    else
        gpio_bits_reset(ptIO->ptPort, ptIO->hwPin);
}

bool bsp_readIO(io_t *ptIO)
{
    if(SET == gpio_input_data_bit_read(ptIO->ptPort, ptIO->hwPin))
        return true;
    else
        return false;
}

uint16_t usart_sendData(uint8_t chUsartNum, uint8_t *pchSendData, uint16_t hwLength)
{
    usartbuffer_t *ptUsartBuffer;
    uint16_t ret = 0;
    uint16_t hwCounter = 0;
    uint8_t chData;

    if(chUsartNum == 1)ptUsartBuffer = (usartbuffer_t *)&tUsart2Buffer;
    else if(!chUsartNum)ptUsartBuffer = (usartbuffer_t *)&tUsart1Buffer;
    else ptUsartBuffer = (usartbuffer_t *)&tUsart3Buffer;
    
    for(hwCounter=0; hwCounter<hwLength; hwCounter++)
    {
        if(queue_write((util_queue_t *)&ptUsartBuffer->tTXQueue, *(pchSendData+hwCounter)) != QUEUE_OK)
        {
            return hwCounter;
        }
    }
    
    if(ptUsartBuffer->chTXFlag == USART_TXFLAG_BUSY)return hwCounter;

#if (defined USART1_RS485_ENABLE) && (USART1_RS485_ENABLE == TRUE)
    // 总线忙，不可发数据
    if(ptUsartBuffer->chRXFlag == USART_RXFLAG_BUSY)
        return hwCounter;
    
    if(chUsartNum == 0)
    {
        gpio_bits_set(c_tUart485.ptPort, c_tUart485.wPin);
    }
#endif
#if (defined USART2_RS485_ENABLE) && (USART2_RS485_ENABLE == TRUE)
    // 总线忙，不可发数据
    if(ptUsartBuffer->chRXFlag == USART_RXFLAG_BUSY)
    {
        ptUsartBuffer->chWaitBusIdleFlag = 1;
        return hwCounter;
    }
        
    
    if(chUsartNum == 1)
    {
        gpio_bits_set(c_tUart485.ptPort, c_tUart485.wPin);
    }
#endif

    queue_read((util_queue_t *)&ptUsartBuffer->tTXQueue, (uint8_t *)&chData);
    usart_data_transmit(ptUsartBuffer->ptUsart, chData);
    ptUsartBuffer->chTXFlag = USART_TXFLAG_BUSY;
    return hwCounter;
}


uint16_t usart_receiveData(uint8_t chUsartNum, uint8_t *pchReceiveData)
{
    uint16_t hwCounter=0;
    usartbuffer_t *ptUsartBuffer;
    qstatus_t tQueueStatus;
    uint8_t chData;
    
    if(chUsartNum == 1)ptUsartBuffer = (usartbuffer_t *)&tUsart2Buffer;
    else if(!chUsartNum)ptUsartBuffer = (usartbuffer_t *)&tUsart1Buffer;
    else ptUsartBuffer = (usartbuffer_t *)&tUsart3Buffer;
    
    if(ptUsartBuffer->chRXFlag != USART_RXFLAG_FINISH)return hwCounter;
    
    do{
        tQueueStatus = queue_read((util_queue_t *)&ptUsartBuffer->tRXQueue, pchReceiveData);
        if(QUEUE_OK == tQueueStatus)
        {
            hwCounter++;
            pchReceiveData++;
        }
    }while(tQueueStatus != QUEUE_EMPTY);
    
    ptUsartBuffer->chRXFlag = USART_RXFLAG_IDLE;
    
    // 有数据压栈未发送
    if(ptUsartBuffer->chWaitBusIdleFlag == 1)
    {
        if(queue_read((util_queue_t *)&ptUsartBuffer->tTXQueue, (uint8_t *)&chData) == QUEUE_OK)
        {
            // 启动发送
            usart_data_transmit(ptUsartBuffer->ptUsart, chData);
            ptUsartBuffer->chTXFlag = USART_TXFLAG_BUSY;
            ptUsartBuffer->chWaitBusIdleFlag = 0;
        }
    }
    return hwCounter;
}

void USART1_IRQHandler(void)
{
    uint8_t chData;

    if(USART1->ctrl1_bit.rdbfien != RESET)
	{
		if(usart_flag_get(USART1, USART_RDBF_FLAG) != RESET)
		{
			queue_write((util_queue_t *)&tUsart1Buffer.tRXQueue, usart_data_receive(USART1));
			tUsart1Buffer.chRXFinishTime = USART_DELAYTIME;	
			tUsart1Buffer.chRXFlag= USART_RXFLAG_BUSY;
		}
	}
	if(USART1->ctrl1_bit.tdcien != RESET)
	{
		if(usart_flag_get(USART1, USART_TDC_FLAG) != RESET)
		{
			usart_flag_clear(USART1, USART_TDC_FLAG);
            if(queue_read((util_queue_t *)&tUsart1Buffer.tTXQueue, (uint8_t *)&chData) == QUEUE_OK)
            {
                usart_data_transmit(USART1, chData);
                tUsart1Buffer.chTXFlag = USART_TXFLAG_BUSY;
            }
            else
            {
                tUsart1Buffer.chTXFlag = USART_TXFLAG_IDLE;
#if (defined USART1_RS485_ENABLE) && (USART1_RS485_ENABLE == TRUE)
                gpio_bits_set(c_tUart485.ptPort, c_tUart485.wPin);
#endif
            }
		}
	}
}

void USART2_IRQHandler(void)
{
    uint8_t chData;
    if(USART2->ctrl1_bit.rdbfien != RESET)
	{
		if(usart_flag_get(USART2, USART_RDBF_FLAG) != RESET)
		{
			queue_write((util_queue_t *)&tUsart2Buffer.tRXQueue, usart_data_receive(USART2));
			tUsart2Buffer.chRXFinishTime = USART_DELAYTIME;	
			tUsart2Buffer.chRXFlag= USART_RXFLAG_BUSY;
		}
	}

	if(USART2->ctrl1_bit.tdcien != RESET)
	{
		if(usart_flag_get(USART2, USART_TDC_FLAG) != RESET)
		{

			usart_flag_clear(USART2, USART_TDC_FLAG);
            if(queue_read((util_queue_t *)&tUsart2Buffer.tTXQueue, (uint8_t *)&chData) == QUEUE_OK)
            {
                usart_data_transmit(USART2, chData);
                tUsart2Buffer.chTXFlag = USART_TXFLAG_BUSY;
            }
            else
            {
                tUsart2Buffer.chTXFlag = USART_TXFLAG_IDLE;
#if (defined USART2_RS485_ENABLE) && (USART2_RS485_ENABLE == TRUE)
                gpio_bits_reset(c_tUart485.ptPort, c_tUart485.wPin);
                
#endif
            }
		}
	}
}


void USART1_TimeOutCounter(void)
{
    if(tUsart1Buffer.chRXFlag==USART_RXFLAG_BUSY)
    {
        if(tUsart1Buffer.chRXFinishTime) {
            tUsart1Buffer.chRXFinishTime--;
        }
        else {
            tUsart1Buffer.chRXFlag=USART_RXFLAG_FINISH;
        }
    }
}

void USART2_TimeOutCounter(void)
{
    if(tUsart2Buffer.chRXFlag==USART_RXFLAG_BUSY)
    {
        if(tUsart2Buffer.chRXFinishTime) {
            tUsart2Buffer.chRXFinishTime--;
        }
        else {
            tUsart2Buffer.chRXFlag=USART_RXFLAG_FINISH;
        }
    }
}

void bsp_Init(void)
{
    bsp_ClockInit();
    bsp_GpioInit();
    bsp_SystemClockInit();
    bsp_UartInit();
    //bsp_WatchDogInit();
}