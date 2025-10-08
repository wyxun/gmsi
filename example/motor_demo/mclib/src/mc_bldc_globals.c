/**
  **************************************************************************
  * @file     mc_bldc_globals.c
  * @brief    Global variables declaration and default values, global functions declaration
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "mc_lib.h"
#include "mc_flash_data_table.h"

const char s_fwVer[32] = FIRMWARE_VERS;
firmware_id_type firmware_id;
motor_control_mode ctrl_mode;
err_code_type error_code = MC_NO_ERROR;
err_code_type error_code_mask = MC_ERROR_MASK;

uint32_t tmr_pwm_channel_mode[7][2] =
{
  {AH_BCL_PWM_MODE_CM1, AH_BCL_PWM_MODE_CM2},
  {CH_AL_PWM_MODE_CM1, CH_AL_PWM_MODE_CM2},
  {BH_CL_PWM_MODE_CM1, BH_CL_PWM_MODE_CM2},
  {BH_AL_PWM_MODE_CM1, BH_AL_PWM_MODE_CM2},
  {AH_BL_PWM_MODE_CM1, AH_BL_PWM_MODE_CM2},
  {CH_BL_PWM_MODE_CM1, CH_BL_PWM_MODE_CM2},
  {AH_CL_PWM_MODE_CM1, AH_CL_PWM_MODE_CM2}
};
uint32_t tmr_pwm_output_mode[7] =
{AH_BCL_PWM_OUT_CCTRL, CH_AL_PWM_OUT_CCTRL, BH_CL_PWM_OUT_CCTRL, BH_AL_PWM_OUT_CCTRL, AH_BL_PWM_OUT_CCTRL, CH_BL_PWM_OUT_CCTRL, AH_CL_PWM_OUT_CCTRL};

uint16_t next_hall_state[2][7] = {
  {0, 5, 3, 1, 6, 4, 2},
  {0, 3, 6, 2, 5, 1, 4}
};

uint16_t hall_learn_state_table[6] = {0};
uint16_t hall_learn_sequence_table[6] = {0};
uint16_t next_hall_learn_state_table[7] = {0};

const uint32_t init_tmr_pwm_channel_mode[7][2] =
{
  {AH_BCL_PWM_MODE_CM1, AH_BCL_PWM_MODE_CM2},
  {CH_AL_PWM_MODE_CM1, CH_AL_PWM_MODE_CM2},
  {BH_CL_PWM_MODE_CM1, BH_CL_PWM_MODE_CM2},
  {BH_AL_PWM_MODE_CM1, BH_AL_PWM_MODE_CM2},
  {AH_BL_PWM_MODE_CM1, AH_BL_PWM_MODE_CM2},
  {CH_BL_PWM_MODE_CM1, CH_BL_PWM_MODE_CM2},
  {AH_CL_PWM_MODE_CM1, AH_CL_PWM_MODE_CM2}
};

const uint32_t init_tmr_pwm_output_mode[7] =
{AH_BCL_PWM_OUT_CCTRL, CH_AL_PWM_OUT_CCTRL, BH_CL_PWM_OUT_CCTRL, BH_AL_PWM_OUT_CCTRL, AH_BL_PWM_OUT_CCTRL, CH_BL_PWM_OUT_CCTRL, AH_CL_PWM_OUT_CCTRL};

const uint16_t init_next_hall_state[2][7] = {
  {0, 5, 3, 1, 6, 4, 2},
  {0, 3, 6, 2, 5, 1, 4}
};

const uint16_t pwm_pattern[2][7] = {
  {0, 6, 1, 2, 3, 4, 5},
  {0, 3, 2, 1, 6, 5, 4}
};

const uint16_t output_hall_state[2][7] =
{
  {0, 1, 2, 3, 4, 5, 6},
  {0, 6, 5, 4, 3, 2, 1}
};

/* type parameter */
speed_type rotor_speed =
{
  .dir = 0,
  .filtered = 0,
  .val = 0,
  .speed_count = 0,
};
current_type current =
{
  .ADCx = ADC_CONVERTER,
  .span = CURRENT_SPAN,
  .span_shift = CURRENT_SPAN_SHIFT,
  .volt_sign = 1,
  .volt_sign_coming = 1,
  .Ibus =
  {
    .Ireal_pu = 0,
    .Iref = 0,
    .Istart = 0,
    .decay_const = DECAY_CONST,
  }
};

adc_sample_type adc_sample =
{
  .adc_tmr_x = ADC_TIMER,
  .change_phase_tmr_x = CHANGE_PHASE_TIMER,
  .adc_x = ADC_CONVERTER,
  .i_sample_min_cnt = I_SAMP_MIN_CNT,
  .i_sample_delay_count = I_SAMP_DLY_CNT,
  .emf =
  {
#if defined HALL_SENSORS
    .read_emf_tmr_x = HALL_CAPTURE_TIMER,
#else
    .read_emf_tmr_x = READ_EMF_TIMER,
#endif
    .emf_change_duty_low = EMF_CHANGE_DUTY_L,
    .emf_change_duty_high = EMF_CHANGE_DUTY_H,
    .emf_low_spd_sample_point = EMF_LOW_SPD_SAMPLE_POINT,
    .emf_high_spd_sample_cnt = EMF_HIGH_SPD_SAMPLE_CNT,
    .emf_avoid_noise_times = EMF_AVOID_NOISE_INIT_TIMES,
    .emf_pull_up = RESET,
    .emf_comp_continous_mode = RESET,
  }
};

angle_init_type angle_init =
{
  .ADVTMRx = PWM_ADVANCE_TIMER,
  .ADC_TMRx = ADC_TIMER,
  .tmr_period = ANGLE_INIT_PERIOD,
  .detect_duty = ANGLE_INIT_DETECT_DUTY,
  .step_count = -1,
  .current =
  {
    .ab = 0,
    .ac = 0,
    .ba = 0,
    .bc = 0,
    .ca = 0,
    .cb = 0,
  },
  .difference = ANGLE_INIT_I_DIFF
};

pid_ctrl_type pid_is = {0};
pid_ctrl_type pid_spd = {0};
pid_ctrl_type pid_spd_volt =
{
  .kp_gain = PID_SPD_VOLT_KP_DEFUALT,
  .ki_gain = PID_SPD_VOLT_KI_DEFUALT,
  .upper_limit_output = INT16_MAX,    /* Lower Limit for Output limitation */
  .lower_limit_output = 0,         /* Lower Limit for Output limitation */
  .kp_shift = PID_SPD_VOLT_KP_GAIN_DIV_LOG,
  .ki_shift = PID_SPD_VOLT_KI_GAIN_DIV_LOG,
  .upper_limit_integral = (INT16_MAX << PID_SPD_VOLT_KI_GAIN_DIV_LOG),
  .lower_limit_integral = 0,
  .integral = 0x0000U,
  .error = 0x0000U,
  .pre_error = 0x0000U,
};
low_spd_ctrl_type low_spd_ctrl_mode = WITHOUT_CURR_CTRL;

ramp_cmd_type speed_ramp =
{
  .cmd_final = 0x0000U,
  .command = 0x0000U,
};

hall_sensor_type hall =
{
  .state = 6,
  .pre_state = 0x0000U,
  .next_state = 0x0000U,
  .double_interval = MAX_CAP_COUNT,
  .H1_port = HALL_A_PORT,
  .H1_pin = HALL_A_GPIO_PIN,
  .H2_port = HALL_B_PORT,
  .H2_pin = HALL_B_GPIO_PIN,
  .H3_port = HALL_C_PORT,
  .H3_pin = HALL_C_GPIO_PIN,
};

hall_learn_type hall_learn =
{
  .process_state = PROCESS_0_LOCK,
  .hall_state =
  {
    .state = 6,
    .pre_state = 0x0000U,
    .next_state = 0x0000U,
    .double_interval = MAX_CAP_COUNT,
    .H1_port = HALL_A_PORT,
    .H1_pin = HALL_A_GPIO_PIN,
    .H2_port = HALL_B_PORT,
    .H2_pin = HALL_B_GPIO_PIN,
    .H3_port = HALL_C_PORT,
    .H3_pin = HALL_C_GPIO_PIN,
  },
  .start_flag = RESET,
  .dir = 0,
  .learn_period = LEARN_OLC_PERIOD,
  .learn_angle_inc = 0,
  .learn_volt = LEARN_OLC_VOLT_CMD,
  .align_time = LEARN_ALIGN_TIME,
  .count = 0,
  .step = 0,
  .check_flag = RESET,
};

usart_config_type ui_usart =
{
  .baudrate = UI_UART_BAUDRATE,
  .usart_clock = COMM_UART_CRM_CLK,
  .usart_tx_gpio_clock = COMM_UART_TX_GPIO_CRM_CLK,
  .usart_rx_gpio_clock = COMM_UART_RX_GPIO_CRM_CLK,
  .usart_tx_gpio_port = COMM_UART_TX_PORT,
  .usart_rx_gpio_port = COMM_UART_RX_PORT,
  .usart_tx_gpio_pin = COMM_UART_TX_PIN,
  .usart_rx_gpio_pin = COMM_UART_RX_PIN,
#ifdef AT32F421xx
  .gpio_tx_pin_source = COMM_UART_TX_GPIO_PIN_SOURCE,
  .gpio_rx_pin_source = COMM_UART_RX_GPIO_PIN_SOURCE,
  .gpio_tx_pin_mux = COMM_UART_TX_IOMUX,
  .gpio_rx_pin_mux = COMM_UART_RX_IOMUX,
#else
  .usart_gpio_remap = COMM_UART_IOMUX,
#endif
  .usart_x = COMM_UART,
};
ui_wave_param_type ui_wave_param =
{
  .speed_meas_filter_pu = 0,
  .speed_reference_pu = 0,
  .iBusVoltage_meas = 0,
  .iMosTemperature_meas = 0,
  .user_define_a = 0,
  .user_define_b = 0,
};

olc_type openloop =
{
  .olc_count = 0,
  .volt_ref = 0,
  .period_ref = 0,
  .olc_init_volt = 0,
  .olc_volt_inc = 0,
  .olc_init_spd = 0,
  .olc_init_period = 0,
  .olc_period_dec = 0,
  .olc_final_spd = 0,
  .olc_final_period = 0,
};

/* basic */
int16_t volt_cmd;
uint16_t pwm_pr = PWM_PERIOD;
int16_t pwm_comp_value;
ctrl_source_type ctrl_source;
int16_t max_current_pu;
int16_t min_current_pu;
int16_t sys_counter = 0;
/* start-up */
int16_t start_current_cmd;
int16_t start_volt_cmd;
uint16_t start_period;
/* usart/user interface*/
RCP_Frame_t rx_data_command;
TCP_Frame_t tx_data_response;
int16_t *monitor_data[2];
uint8_t monitor_data_buffer_num = 0;
int32_t save_param_buffer[INT32_SIZE_ARRAY];
usart_data_index usart_data_idx;
flag_status cmd_response_rdy = RESET;
uint8_t monitor_data_buffer[2][DATA_BUFFER_FRAME_SIZE];
uint8_t extra_data_buffer[DATA_BUFFER_EXTRA_SIZE] = {0};
uint16_t buffer_index = 10;   								/*sync frame size = 10*/
uint8_t ui_receive_cmd_index = 1;
/* measure */
float fMosTemperature;
uint32_t motor_power_meas;
/* flag */
flag_status param_initial_rdy = RESET;
flag_status start_stop_btn_flag = RESET;
flag_status curr_offset_rdy = RESET;
flag_status change_phase_flag = RESET;
flag_status bldc_rdy = RESET;
flag_status low_spd_switch_flag = RESET;
flag_status closeloop_rdy = RESET;
flag_status current_loop_ctrl = RESET;
flag_status const_current_ctrl = RESET;
flag_status calc_spd_rdy = RESET;
/* esc state */
esc_state_type esc_state_old = ESC_STATE_NONE;
esc_state_type esc_state = ESC_STATE_IDLE;
start_state_type start_state;
int16_t sense_hall_steps = 0;

/* emf */
int16_t emf_adc_value;
int16_t zero_cross_point;
int16_t zcp_lowspd_fall, zcp_lowspd_rise, zcp_highspd_fall, zcp_highspd_rise;
int16_t lowspd_sample_end, highspd_sample_end;
volatile uint16_t emf_avoid_noise_counter = 0;
volatile uint8_t emf_comp_state;
volatile int16_t emf_comp_hall[3], read_gpio_num;
int16_t ke_compen = 0;
uint8_t EMF_switch_sample_position = 0;
/* speed */
moving_average_type *interval_moving_average_fliter;
uint32_t spd_cval = 0;
uint32_t spd_last_cval = PWM_PERIOD;
int32_t  spd_total_cval;
/* for tunning */
uint16_t I_tune_count;
int16_t current_tune_target_current;
uint16_t current_tune_total_period;
uint16_t current_tune_step_period;

i_auto_tune_type I_auto_tune =
{
  .Rs = &motor_param_ident.Rs.f,
  .Ls = &motor_param_ident.Ls.f,
  .current_base = CURRENT_BASE,
  .Vdc_rated = VDC_RATED,
  .bandwidth = CURRENT_BANDWIDTH,
  .BW_limit = CURRENT_BW_LIMIT,
  .sample_freq = PWM_FREQ,
  .state_flag = UNDO,
};

motor_param_id_type motor_param_ident =
{
  .ADVTMRx = PWM_ADVANCE_TIMER,
  .sample_freq = PWM_FREQ_ID,
  .pwm_period = (PWM_PERIOD_ID-1),
  .I_nominal = NOMINAL_CURRENT_PU,
  .I_threshold = THRESHOLD_CURRENT_PU,
  .I_val = &current.ID_1shunt,
  .z_base = Z_BASE,
  .align_time_count = ALIGN_TIME_COUNT,
  .Vbus	= &adc_in_tab[ADC_BUS_VOLT_IDX],
  .id_flag = RESET,
};
