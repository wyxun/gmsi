#include "main.h"
#include "gmsi.h"
#include "mc_lib.h"
//#include "at32f413_board.h"

/** @addtogroup AT32F413_pmsm_foc_demo
  * @{
  */
crm_clocks_freq_type crm_clocks_freq_struct = {0};

void disable_swo_debug_config(void);
void check_configured_core_clock(void);

/* Provide a weak stub in case board-specific implementation is not present */
void disable_swo_debug_config(void)
{
  /* no-op: SWO debug not configured for this build */
}

gmsi_t tGmsi = {NULL};

int main(void)
{
#if 1
  system_clock_config();

  disable_swo_debug_config();

  systick_init();

  mc_delay_init();

  button_exint_init();

  led_config();

  nvic_config();

  adc_ordinary_config();

  adc_preempt_config();

  tmr_pwm_init();

  speed_timer_init();

  mode_switch_init();

  crm_clocks_freq_get(&crm_clocks_freq_struct);

  uart_init(&ui_usart);
  dma_uart_configuration();
  uart_rx_init();

  enable_pwm_timer();

  mc_delay_ms(500);

#if VOLT_SENSE
  voltage_offset_init(&(motor_emf.emf_abc_offset), &motor_voltage);
#endif

  I_offset_init();

  charge_boot_cap();

  firmware_id = get_fw_id();
  param_initial_rdy = mc_param_init(firmware_id);

  param_init();

  led_blink();
#endif
    gmsi_Init(&tGmsi);

    for(;;)
    {
        gmsi_Run();
    }
    return 0;
}