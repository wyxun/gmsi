/**
  **************************************************************************
  * @file     mc_foc_globals.c
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

motor_control_mode ctrl_mode = {MOTOR_CONTROL_MODE}, ctrl_mode_old = {MOTOR_CONTROL_MODE}, ctrl_mode_cmd = {MOTOR_CONTROL_MODE};
firmware_id_type firmware_id;

voltage_type volt_cmd =
{
  .Vqd_squr_sum_max = VQD_SQUR_SUM_MAX,
  .Vq_max = VQ_MAX,
  .Vd_max = VD_MAX,
  .ADCx = ADC1,
  .ADC_CH_A = CURR_PHASE_A_ADC_CH,
  .ADC_CH_B = CURR_PHASE_B_ADC_CH,
  .ADC_CH_C = CURR_PHASE_C_ADC_CH,
};

rotor_angle_type rotor_angle_encoder, rotor_angle_hall, rotor_angle_hall_old;

current_type current =
{
  .ADCx = ADC_CONVERTER,
  .span = CURRENT_SPAN,
  .dc_span = DC_CURRENT_SPAN,
  .span_shift = CURRENT_SPAN_SHIFT,
  .nominal = NOMINAL_CURRENT_PU,
};

encoder_type encoder =
{
  .TMRx = ENCODER_MODE_TIMER,
#if defined MT_METHOD
  .TMRx_Cap = ENCODER_CAPTURE_TIMER,
#endif
  .EXINTx = EXINT,
  .count = 0x0000U,
  .pre_count = 0x0000U,
  .offset = 0x0000U,
  .val = 0x0000U,
  .delta = 0x0000U,
  .align = UNDO,
  .idx_reset_flag = FALSE,
  .half_cpr_nbr = ENC_HALF_CPR_NBR,
  .align_time = ENC_ALIGN_TIME,
  .cpr_to_rpm = ENC_CPR_TO_RPM,
  .stall_time = ENC_STALL_TIME,
  .pole_pairs = POLE_PAIRS,
  .const_timer_clks = ENC_CONST_TIMER_CLKS,
  .est_speed_const = ENC_EST_SPEED_CONST,
#ifdef ABZ
  .ext_z_line = ENCODER_Z_EXINT_LINE,
  .index_count = ENC_IDX_COUNT,
#endif
};

open_loop_type openloop = {0};

speed_type rotor_speed_encoder =
{
  .shift = ENC_SPEED_SHIFT,
};

speed_type rotor_speed_hall =
{
  .dir = 1,
  .shift = HALL_SPEED_SHIFT,
};

hall_sensor_type hall =
{
  .state = 0x0000U,
  .pre_state = 0x0000U,
  .offset = MAX_CAP_COUNT,
  .double_interval = MAX_CAP_COUNT,
  .theta_inc = 0x0000U,
  .theta_cmpsat = THETA_CMPSAT,
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
  .learn_period = 0,
  .learn_angle_inc = LEARN_OLC_ANGLE_INC,
  .learn_volt = LEARN_OLC_VOLT_CMD,
  .align_time = LEARN_ALIGN_TIME,
  .count = 0,
  .step = 0,
  .check_flag = RESET,
};

pid_ctrl_type pid_iq =
{
#if defined INCREM_ENCODER || defined MAGNET_ENCODER_W_ABZ || defined MAGNET_ENCODER_WO_ABZ || defined SENSORLESS
  .upper_limit_output = VQ_MAX,    /* Lower Limit for Output limitation */
  .lower_limit_output = -VQ_MAX,   /* Lower Limit for Output limitation */
#elif defined HALL_SENSORS
  .upper_limit_output = VQ_MAX,    /* Lower Limit for Output limitation */
  .lower_limit_output = 0,   			 /* Lower Limit for Output limitation */
#endif
  .integral = 0x0000U,
  .error = 0x0000U,
  .pre_error = 0x0000U,
};

pid_ctrl_type pid_id =
{
  .upper_limit_output = VD_MAX,    /* Lower Limit for Output limitation */
  .lower_limit_output = -VD_MAX,   /* Lower Limit for Output limitation */
  .integral = 0x0000U,
  .error = 0x0000U,
  .pre_error = 0x0000U,
};


pid_ctrl_type pid_spd = {0};

pid_ctrl_type pid_pos = {0};

ramp_cmd_type speed_ramp = {0};

position_type pos =
{
  .brake_flag = RESET,
  .stable = TRUE,
  .spd_slope_ratio = SPEED_SLOPE_RATION,
#ifdef HALL_SENSORS
  .rpm_to_cpr = HALL_RPM_TO_CPR,
#else
  .rpm_to_cpr = ENC_RPM_TO_CPR,
#endif
  .cmd_to_val_gap = CMD_TO_VAL_GAP,
  .small_pos_cmd_gap = SMALL_POS_CMD_GAP,
  .min_pos_ctrl_spd = MIN_POSCTL_SPD,
};

angle_type angle;
int16_t sys_counter = 0;
int16_t elec_angle_val;
int32_t rotor_speed_val, rotor_speed_val_filt;
moving_average_type *speed_ma_fliter;
flag_status curr_offset_rdy = RESET;
flag_status foc_rdy = RESET;
flag_status param_initial_rdy = RESET;
float fMosTemperature;
uint16_t I_tune_count;
int16_t current_tune_target_current;
uint16_t current_tune_total_period;
uint16_t current_tune_step_period;
ctrl_source_type ctrl_source;
RCP_Frame_t rx_data_command;
TCP_Frame_t tx_data_response;

flag_status start_stop_btn_flag = RESET;

esc_state_type esc_state_old = ESC_STATE_NONE;
esc_state_type esc_state = ESC_STATE_IDLE;

pwm_duty_type pwm_duty =
{
  .half_duty = HALF_PWM_PERIOD,
  .double_duty = DOUBLE_PWM_PERIOD,
  .quarter_duty = QUARTER_DUTY_CYCLE,
  .threshold = PWM_PERIOD_90PCT,
  .min_interval = MIN_INTERVAL,
  .OF =
  {
    .a = HALF_PWM_PERIOD,
    .b = HALF_PWM_PERIOD,
    .c = HALF_PWM_PERIOD,
  },
  .UF =
  {
    .a = HALF_PWM_PERIOD,
    .b = HALF_PWM_PERIOD,
    .c = HALF_PWM_PERIOD,
  },
  .adc_trig =
  {
    .first_pos = ADC_TRIG_POS,
    .second_pos[0] = ADC_TRIG_POS,
    .second_pos[1] = PWM_PERIOD,   /* third trigger point to prevent false triggering */
  },
};

moving_average_type *hall_interval_moving_average;

err_code_type error_code = MC_NO_ERROR;
err_code_type error_code_mask = MC_ERROR_MASK;

field_weakening_type field_weakening =
{
  .pid_fw = {0},
  .Idref_offset =  0x0000U,
  .Vqd_filt = &volt_cmd.Vqd_filtered,
  .pid_speed = &pid_spd,
  .curr_squr_max = FW_MAX_CURR_SQUR,
};

lowpass_filter_type q_current_LPF =
{
  .sample_freq = PWM_FREQ,
  .bandwidth = CURR_LP_BANDWIDTH,
  .coef1 = 0x0000U,
  .coef2 = 0x0000U,
};

lowpass_filter_type d_current_LPF =
{
  .sample_freq = PWM_FREQ,
  .bandwidth = CURR_LP_BANDWIDTH,
  .coef1 = 0x0000U,
  .coef2 = 0x0000U,
};

lowpass_filter_type obs_speed_LPF =
{
  .sample_freq = PWM_FREQ,
  .bandwidth = OBS_SPD_LP_BANDWIDTH,
  .coef1 = 0x0000U,
  .coef2 = 0x0000U,
};

motor_volt_type motor_voltage =
{
  .Valphabeta = &volt_cmd.Valphabeta,
  .Ialphabeta = &current.Ialphabeta,
  .Vbus	= &adc_in_tab[ADC_BUS_VOLT_IDX],
  .Va = &adc_in_tab[ADC_BEMF_A_IDX],
  .Vb = &adc_in_tab[ADC_BEMF_B_IDX],
  .Vc = &adc_in_tab[ADC_BEMF_C_IDX],
  .emf_factor = EMF_VBUS_SENSE_FACTOR,
};

state_observer_type	state_observer =
{
  .Rs = &motor_param_ident.Rs.f,
  .Ls = &motor_param_ident.Ls.f,
  .c5 = C5,
  .elec_angle = 0x0000U,
  .dir = &startup.dir,
  .elec_spd_to_rpm = ELEC_SPD_TO_RPM,
  .pid_pll =
  {
    .upper_limit_output = INT16_MAX,
    .lower_limit_output = -INT16_MAX,
  },
};

sensorless_startup_type startup =
{
  .elec_spd_to_rpm = (uint16_t)ELEC_SPD_TO_RPM,
  .elec_angle = 0x0000U,
  .closeloop_rdy = RESET,
  .closeloop_rdy_old = RESET,
};

foc_angle_init_type angle_detector =
{
  .ADVTMRx = PWM_ADVANCE_TIMER,
  .ADC_TMRx = ADC_TIMER,
  .ADC_TMRx_CHx = ADC_TIMER_SELECT_CHANNEL,
  .detect_duty = ANGLE_INIT_HALF_PERIOD,
  .period = ANGLE_INIT_PERIOD,
  .adc_trig = angle_init_adc_trig_table,
  .step_count = -1,
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
  .usart_gpio_remap = COMM_UART_IOMUX,
#if defined AT32F421xx || defined AT32F423xx || defined AT32F425xx
  .gpio_tx_pin_source = COMM_UART_TX_GPIO_PIN_SOURCE,
  .gpio_rx_pin_source = COMM_UART_RX_GPIO_PIN_SOURCE,
  .gpio_tx_pin_mux = COMM_UART_TX_IOMUX,
  .gpio_rx_pin_mux = COMM_UART_RX_IOMUX,
#endif
  .usart_x = COMM_UART,
};

ui_wave_param_type ui_wave_param =
{
  .speed_meas_filter_pu = 0,
  .speed_reference_pu = 0,
  .iBusVoltage_meas = 0,
  .iMosTemperature_meas = 0,
};

int16_t *monitor_data[2];
uint8_t monitor_data_buffer_num = 0;
int32_t save_param_buffer[INT32_SIZE_ARRAY];
usart_data_index usart_data_idx;
flag_status cmd_response_rdy = RESET;
uint8_t monitor_data_buffer[2][DATA_BUFFER_FRAME_SIZE];
uint8_t extra_data_buffer[DATA_BUFFER_EXTRA_SIZE] = {0};
uint16_t buffer_index = 10;   								/*sync frame size = 10*/
uint8_t ui_receive_cmd_index = 1;
int16_t *hall_next_state_table, *hall_theta_table;
uint16_t hall_learn_state_table[6] = {0};
uint16_t hall_learn_sequence_table[6] = {0};
uint16_t next_hall_learn_state_table[7] = {0};
motor_emf_type motor_emf;

uint16_t angle_init_adc_trig_table[6] =
{
  DETECT_ADC_TRIG_POINT_1,
  DETECT_ADC_TRIG_POINT_2,
  DETECT_ADC_TRIG_POINT_3,
  DETECT_ADC_TRIG_POINT_4,
  DETECT_ADC_TRIG_POINT_5,
  DETECT_ADC_TRIG_POINT_6,
};

int16_t hall_cw_next_state_table[8] = {-1,0,0,0,0,0,0,-1};

int16_t hall_ccw_next_state_table[8] = {-1,0,0,0,0,0,0,-1};

uint16_t hall_startup_theta_table[8] = {0};

int16_t hall_cw_theta_table[8] = {0};

int16_t hall_ccw_theta_table[8] = {0};

const char s_fwVer[32] = FIRMWARE_VERS;
uint8_t charge_boot_count;
qd_type I_ref, Iref, Ival;
int16_t Iq_ref_cmd;
moving_average_type *Idc_ma_fliter;
flag_status write_flash_flag = RESET;

rds_cali_type Rds_Cali =
{
  .P2_Const = P2_CONST,
  .P1_ma_nbr_log = P1_MA_NBR_LOG,
  .P2_ma_nbr_log = P2_MA_NBR_LOG,
  .I_val = &Ival,
  .Iq_LPF =
  {
    .sample_freq = PWM_FREQ,
    .bandwidth = IQD_LP_BANDWIDTH,
    .coef1 = 0x0000U,
    .coef2 = 0x0000U,
  },
  .Id_LPF =
  {
    .sample_freq = PWM_FREQ,
    .bandwidth = IQD_LP_BANDWIDTH,
    .coef1 = 0x0000U,
    .coef2 = 0x0000U,
  },
  .span_upper_limit = SPAN_UPPER_LIMIT,
  .span_lower_limit = SPAN_LOWER_LIMIT,
};

int32_t *motor_speed_val = &state_observer.motor_speed.val;
int32_t motor_speed_filter;
flag_status reverse_flag = RESET;

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
#if defined ONE_SHUNT
  .I_val = &current.ID_1shunt,
#elif defined THREE_SHUNT || defined TWO_SHUNT
  .I_val = &current.Iabc.a,
#endif
  .z_base = Z_BASE,
  .align_time_count = ALIGN_TIME_COUNT,
  .Vbus	= &adc_in_tab[ADC_BUS_VOLT_IDX],
  .id_flag = RESET,
};

#ifdef LOW_SPEED_VOLT_CTRL
pid_ctrl_type pid_spd_volt =
{
  .kp_gain = PID_SPD_VOLT_KP_DEFUALT,
  .ki_gain = PID_SPD_VOLT_KI_DEFUALT,
  .upper_limit_output = VQ_MAX,    /* Lower Limit for Output limitation */
  .lower_limit_output = 0,         /* Lower Limit for Output limitation */
  .kp_shift = PID_SPD_VOLT_KP_GAIN_DIV_LOG,
  .ki_shift = PID_SPD_VOLT_KI_GAIN_DIV_LOG,
  .upper_limit_integral = (VQ_MAX << PID_SPD_VOLT_KI_GAIN_DIV_LOG),
  .lower_limit_integral = 0,
  .integral = 0x0000U,
  .error = 0x0000U,
  .pre_error = 0x0000U,
};

low_spd_ctrl_type low_spd_ctrl_mode = WITHOUT_CURR_CTRL;
#endif
