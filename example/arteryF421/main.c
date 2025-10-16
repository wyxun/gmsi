#include "main.h"
#include "gmsi.h"
#include "at32f421_bsp.h"
#include "gmsi.h"
//#include "platform.h"
#include "rfid.h"
#include "step4988.h"
#include "lsm6dsl.h"
#include "correspondent.h"

// object
// 初始化rfid
rfid_cfg_t tRfidCfg = {
    .Write = usart_sendData,        // 实际串口写函数
    .Read = usart_receiveData,      // 实际串口读函数
    .wFd = 0,                       // 串口号
    //.pfcnCallback = rfid_callback
};
rfid_t tRfid;
// 初始化步进电机驱动
step4988_cfg_t tStep4988Cfg ={

};
step4988_t tStep4988;

// 初始化lsm6dsl
lsm6dsl_cfg_t tLsm6dslCfg = {
    .hwUpdateRateHz = 50,
};
lsm6dsl_t tLsm6dsl;

#define CORRESPONDENT_RING_BUFFER_SIZE 50
uint8_t gchCorrespondentBuffer[CORRESPONDENT_RING_BUFFER_SIZE] = {0};
// 初始化上位机通信模块
correspondent_cfg_t tCorrespondentCfg = {
    .wFd = 1,                       // 串口号
    .Write = usart_sendData,        // 实际串口写函数
    .Read = usart_receiveData,      // 实际串口读函数
    .hwRingSize = CORRESPONDENT_RING_BUFFER_SIZE,
    .pchRingBuffer = gchCorrespondentBuffer,
};
correspondent_t tCorrespondent;

// 初始化GMSI
gmsi_t tGmsi = {
    .ptData = NULL,
    //.pfcnLedSet = bsp_LedSet
};


int main(void)
{
    system_clock_config();
    bsp_Init();
    //platform_init();
    utildebug_LedInit(bsp_LedSet);

    // 加载模块
    rfid_Init((uintptr_t)&tRfid, (uintptr_t)&tRfidCfg);
    step4988_Init((uintptr_t)&tStep4988, (uintptr_t)&tStep4988Cfg);
    lsm6dsl_Init((uintptr_t)&tLsm6dsl, (uintptr_t)&tLsm6dslCfg);
    correspondent_Init((uintptr_t)&tCorrespondent, (uintptr_t)&tCorrespondentCfg);
    // 初始化库
    gmsi_Init(&tGmsi);

    for(;;)
    {
        // utildebug_LedBreathe(NULL);
        // gmsi_Run();
  while(1)
  {
    if (ctrl_mode_cmd != ctrl_mode_old)
    {
      param_clear();
      start_stop_btn_flag = RESET;

      ctrl_mode = ctrl_mode_cmd;
      ctrl_mode_old = ctrl_mode;
    }

    if (esc_state != esc_state_old)
    {
      switch(esc_state)
      {
      case ESC_STATE_IDLE:
        foc_rdy = RESET;
        pwm_switch_off();
        param_clear();
        led_off(ERROR_LED_PORT, ERROR_LED_GPIO_PIN);
        break;

      case ESC_STATE_SAFETY_READY:
        foc_rdy = RESET;
        pwm_switch_off();
        param_clear();
        break;

      case ESC_STATE_ANGLE_INIT:
#if defined INIT_ANGLE_STARTUP
        foc_angle_init_config();
#endif
        break;

      case ESC_STATE_STARTING:
#if (defined INIT_ANGLE_STARTUP && !defined WIND_SENSE)
        adc_ordinary_config();
        adc_preempt_config();
        tmr_pwm_init();
        mc_delay_us(100); /* wait for response of timer configuration*/
        startup.elec_angle = angle_detector.init_elec_angle;
        startup.ol_angle = angle_detector.init_elec_angle;
        state_observer.elec_angle = angle_detector.init_elec_angle;
        foc_rdy = SET;
        pwm_switch_on();
#elif defined OPENLOOP_STARTUP || defined ALIGN_AND_GO_STARTUP
        foc_rdy = SET;
        pwm_switch_on();
#endif
#ifndef WIND_SENSE
        charge_boot_cap();
#endif
        break;

      case ESC_STATE_RUNNING:
        if (ctrl_mode == OPEN_LOOP_CTRL)
        {
          foc_rdy = SET;
          pwm_switch_on();
          charge_boot_cap();
        }
        break;

      case ESC_STATE_FREE_RUN:
        foc_rdy = RESET;
        pwm_switch_off();
        param_clear();
        break;

      case ESC_STATE_BRAKING:
        foc_rdy = RESET;
        pwm_switch_off();
        param_clear();
        start_stop_btn_flag = RESET;
        break;

      case ESC_STATE_ERROR:
        foc_rdy = RESET;
        pwm_switch_off();
        param_clear();
        start_stop_btn_flag = RESET;
        led_on(ERROR_LED_PORT, ERROR_LED_GPIO_PIN);
        break;

      case ESC_STATE_I_TUNE:
        param_clear();
        foc_rdy = SET;
        pwm_switch_on();
        break;

      case ESC_STATE_AUTO_LEARN:
        break;
#ifdef MOTOR_PARAM_IDENTIFY
      case ESC_STATE_WINDING_PARAM_ID:
        motor_parameter_ID_config();
        motor_param_ident.timeout_count = 0;
        break;
#endif

#ifdef WIND_SENSE
      case ESC_STATE_SPIN_CHECK:
        charge_boot_cap();
        wind_detect_time = 0;
        break;

      case ESC_STATE_HEADWIND_BRAKE:
        tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_1, 0);
        tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_2, 0);
        tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_3, 0);
        pwm_switch_on();
        brake_time = 0;
        break;
#endif
      case ESC_STATE_NONE:
        break;
      }
      esc_state_old = esc_state;
    }
  }
    return 0;
}

void SysTick_Handler(void)
{
    gmsi_Clock();
    
    USART1_TimeOutCounter();
    USART2_TimeOutCounter();
}
