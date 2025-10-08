/**
**************************************************************************
* @file     mc_type.h
* @brief
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

#ifndef __MC_TYPE_H
#define __MC_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif


#include "mc_lib.h"

/**
  * @brief  An union type definition of long word and double words
  */
typedef union
{
  int32_t long_word;
  struct
  {
    int16_t low_word;
    int16_t high_word;
  };
} long_words_union;

/**
  * @brief  Drive mode type definition
  */
typedef enum
{
  BLDC_HALL_SENSOR              = 0x00,
  BLDC_SENSOR_LESS_ADC          = 0x01,
  BLDC_SENSOR_LESS_COMP         = 0x02,
  PMSM_HALL_SENSOR              = 0x03,
  PMSM_ENCODER                  = 0x04,
  PMSM_SENSOR_LESS_OPEN_LOOP    = 0x05,
  PMSM_SENSOR_LESS_ALIGN_AND_GO = 0x06,
  PMSM_SENSOR_LESS_ANGLE_INIT   = 0x07
} firmware_id_type;

/**
  * @brief  control mode type definition
  */
typedef enum
{
  OPEN_LOOP_CTRL           = 0x00,
  VF_CTRL                  = 0x01,
  ID_MANUAL_TUNE           = 0x02,
  IQ_MANUAL_TUNE           = 0x03,
  TORQUE_CTRL              = 0x04,
  SPEED_CTRL               = 0x05,
  POSITION_CTRL            = 0x06,
  COIL_PARAM_AUTO_TUNE     = 0x07,
  MECH_PARAM_AUTO_TUNE     = 0x08,
  ENC_ALIGN                = 0x09
} motor_control_mode;

/**
  * @brief  control source type definition
  */
typedef enum
{
  CTRL_SOURCE_SOFTWARE     = 0x00,
  CTRL_SOURCE_EXTERNAL     = 0x01,
} ctrl_source_type;

/**
  * @brief  encoder alignment state type definition
  */
typedef enum
{
  UNDO                     = 0x00,
  PROCESSING               = 0x01,
  FINISHED                 = 0X02,
  SUCCEED                  = 0x03,
  FAILED                   = 0x04
} process_state_type;

/**
  * @brief  controller state type definition
  */
typedef enum
{
  ESC_STATE_IDLE           = 0x00,
  ESC_STATE_SAFETY_READY   = 0x01,
  ESC_STATE_ANGLE_INIT     = 0x02,
  ESC_STATE_STARTING       = 0x03,
  ESC_STATE_RUNNING        = 0x04,
  ESC_STATE_FREE_RUN       = 0x05,
  ESC_STATE_BRAKING        = 0x06,
  ESC_STATE_ERROR          = 0x07,
  ESC_STATE_ENC_ALIGN      = 0x08,
  ESC_STATE_I_TUNE         = 0x09,
#ifdef E_BIKE_SCOOTER
  ESC_STATE_REVERSE_RUN    = 0x0A,
#endif
#ifdef MOTOR_PARAM_IDENTIFY
  ESC_STATE_WINDING_PARAM_ID  = 0x0B,
#endif
  ESC_STATE_AUTO_LEARN     = 0x0C,
  ESC_STATE_NONE           = 0x10
} esc_state_type;

/**
  * @brief  initial angle state type definition(BLDC sensorless)
  */
typedef enum
{
  START_STATE_IDLE        = 0x00,
  START_STATE_FIRST_STEP  = 0x01,
  START_STATE_SENSE_HALL  = 0x02,
  START_STATE_STABLE_RUN  = 0x03,
  START_STATE_NONE        = 0x04
} start_state_type;

/**
  * @brief  motor error code type definition
  */
typedef enum
{
  MC_NO_ERROR              = 0x00,
  MC_OVER_VOLT_ERROR       = 0x01,
  MC_UNDER_VOLT_ERROR      = 0x02,
  MC_OVER_TEMP_ERROR       = 0x04,
  MC_OVER_CURRENT_ERROR    = 0x08,
  MC_ENCODER_ERROR         = 0x10,
  MC_HALL_ERROR            = 0x20,
  MC_PARAM_IDENT_ERROR     = 0x40,
  MC_HALL_LEARN_ERROR      = 0x80,
} err_code_type;

/**
  * @brief  current shunt number type definition
  */
typedef enum
{
  ONESHUNT                = 0x01,
  TWOSHUNT                = 0x02,
  THREESHUNT              = 0x03
} shunt_nbr_type;

/**
  * @brief  low speed control type definition
  */
typedef enum
{
  WITHOUT_CURR_CTRL         = 0x00,  /* voltage control */
  WITH_CURR_CTRL            = 0x01,
} low_spd_ctrl_type;


typedef enum
{
  PROCESS_0_LOCK          = 0x00,
  PROCESS_1_FREE_RUN      = 0x01,
  PROCESS_2_OPENLOOP      = 0x02,
  PROCESS_3_LEARNING      = 0x03,
  PROCESS_4_FINISH        = 0x04,
  PROCESS_5_ERROR         = 0x05
} hall_learn_process_type;

/**
  * @brief  current sensing of initial angle type definition(BLDC sensorless)
  */
typedef struct
{
  int16_t ab;
  int16_t ac;
  int16_t ba;
  int16_t bc;
  int16_t ca;
  int16_t cb;
} init_current_type;

/**
  * @brief  initial angle type definition(BLDC sensorless)
  */
typedef struct
{
  tmr_type* ADVTMRx;
  tmr_type* ADC_TMRx;
  uint16_t tmr_period;
  uint16_t detect_duty;
  int16_t step_count;
  init_current_type current;
  int16_t difference;
} angle_init_type;

/**
  * @brief  d/q-axis type definition
  */
typedef struct
{
  int16_t q;
  int16_t d;
} qd_type;

/**
  * @brief  a, b and c type definition
  */
typedef struct
{
  int16_t a;
  int16_t b;
  int16_t c;
} abc_type;

/**
  * @brief  alpha/beta-axis type definition
  */
typedef struct
{
  int16_t alpha;
  int16_t beta;
} alphabeta_type;

/**
  * @brief  current offset sensing type definition
  */
typedef struct
{
  uint16_t a;
  uint16_t b;
  uint16_t c;
  uint16_t bus;
} curr_offs_type;

/**
  * @brief  bus current type definition(BLDC)
  */
typedef struct
{
  int16_t Ireal_pu;
  int16_t Iref;
  int16_t Istart;
  int16_t Icalc;
  int16_t decay_const;
} i_bus_type;

/**
  * @brief  dc current type definition(FOC)
  */
typedef struct
{
  int16_t val;
  int16_t filtered;
  int16_t offset;
} i_dc_type;

/**
  * @brief  Trigonometrical functions type definition
  */
typedef struct
{
  int16_t cos_value;
  int16_t sin_value;
} trig_components_type;

/**
  * @brief  Current type definition
  */
typedef struct
{
  i_bus_type       Ibus;
  abc_type         Iabc;
  abc_type         Iabc_shunt;
  alphabeta_type   Ialphabeta;
  qd_type          Iqd;
  qd_type          Iqdref;
  qd_type          Iqd_LPF;
  curr_offs_type   offset;
  adc_type*        ADCx;
  uint16_t         span;
  uint16_t         dc_span;
  uint8_t          span_shift;
  int16_t          nominal;
  int16_t          volt_sign;
  int16_t          volt_sign_coming;
  i_dc_type        Idc;
  int16_t          ID_1shunt;
} current_type;

/**
  * @brief  Voltage type definition(FOC)
  */
typedef struct
{
  alphabeta_type   Valphabeta;
  qd_type          Vqd;
  qd_type          Vqd_filtered;
  int32_t          Vqd_squr_sum;
  int32_t          Vqd_squr_sum_max;
  int16_t          Vq_max;
  int16_t          Vd_max;
  uint8_t          sector;
  adc_type*        ADCx;
  adc_channel_select_type ADC_CH_A;
  adc_channel_select_type ADC_CH_B;
  adc_channel_select_type ADC_CH_C;
} voltage_type;

/**
  * @brief  adc trigger position type definition(FOC)
  */
typedef struct
{
  uint16_t          first_pos;
  uint16_t          second_pos[2];
} adc_trigger_type;

/**
  * @brief  pwm duty type definition(FOC)
  */
typedef struct
{
  abc_type           OF;
  abc_type           UF;
  adc_trigger_type   adc_trig;
  uint16_t           half_duty;
  uint16_t           double_duty;
  uint16_t           quarter_duty;
  uint16_t           threshold;
  int16_t            min_interval;
} pwm_duty_type;

/**
  * @brief  pid controller function type definition
  */
typedef struct
{
  int16_t kp_gain;
  int16_t ki_gain;
  int16_t kd_gain;
  int32_t upper_limit_integral;
  int32_t lower_limit_integral;
  int16_t upper_limit_output;
  int16_t lower_limit_output;
  int32_t integral;
  int32_t error;
  int32_t pre_error;
  uint16_t kp_shift;
  uint16_t ki_shift;
  uint16_t kd_shift;
  int16_t ki_gain_1st;
  int16_t ki_gain_2nd;
  int32_t out_start;
} pid_ctrl_type;

/**
  * @brief  pid controller function type definition
  */
typedef struct
{
  int16_t kp_gain;
  int16_t ki_gain;
  int16_t kd_gain;
  int16_t upper_limit_output;
  int16_t lower_limit_output;
  int32_t error;
  int32_t pre_error;
  uint16_t shift;
  long_words_union integral;
} pid_ctrl_dc_type;

/**
  * @brief  rotor angle type definition(FOC)
  */
typedef struct
{
  int16_t elec_angle_val;
  int16_t elec_angle_pre_val;
  int16_t mech_anlge_val;
} rotor_angle_type;

/**
  * @brief  encoder function type definition(FOC)
  */
typedef struct
{
  int32_t count;
  int32_t pre_count;
  int32_t offset;
  int32_t val;
  int32_t delta;
  int16_t stop_counter;
  process_state_type align;
  confirm_state idx_reset_flag;
  tmr_type* TMRx;
  tmr_type* TMRx_Cap;
  exint_type* EXINTx;
  uint32_t ext_z_line;
  int32_t cpr_nbr;
  int32_t half_cpr_nbr;
  uint32_t align_time;
  uint32_t mech_to_elect_angle;
  uint8_t mech_to_elect_angle_shift;
  float cpr_to_rpm;
  uint16_t  stall_time;
  uint8_t pole_pairs;
  qd_type volt;
  uint8_t spd_period_count;
  uint32_t const_timer_clks;
  uint32_t est_speed_const;
  int16_t index_count;
} encoder_type;

/**
  * @brief  open loop control type definition(FOC)
  */
typedef struct
{
  int16_t theta;
  int8_t  inc;
  qd_type volt;
} open_loop_type;
/**
  * @brief  open loop control type definition(BLDC)
  */
typedef struct
{
  int16_t olc_count;
  int16_t volt_ref;
  int16_t period_ref;
  int16_t olc_init_volt;
  int16_t olc_init_spd;
  int16_t olc_init_period;
  int16_t olc_volt_inc;
  int16_t olc_period_dec;
  int16_t olc_times;
  int16_t olc_final_spd;
  int16_t olc_final_period;
} olc_type;
/**
  * @brief  speed function type definition
  */
typedef struct
{
  int32_t val;
  int32_t filtered;
  int8_t dir;
  uint16_t shift;
  int32_t val_temp;
  uint32_t speed_count;
  long_words_union interval_filter;
} speed_type;

/**
  * @brief  value type definition
  */
typedef struct
{
  int16_t val;
  int16_t filtered;
  int16_t old;
} value_type;

/**
  * @brief  hall function type definition
  */
typedef struct
{
  uint8_t state;
  uint8_t pre_state;
  uint8_t next_state;
  int32_t offset;
  uint32_t hall_interval;
  uint32_t hall_interval_filt;
  uint32_t double_interval;
  int16_t  theta_inc;
  int32_t  theta_cmpsat;
  int16_t  slick_speed;
  gpio_type* H1_port;
  gpio_type* H2_port;
  gpio_type* H3_port;
  uint32_t  H1_pin;
  uint32_t  H2_pin;
  uint32_t  H3_pin;
} hall_sensor_type;

/**
  * @brief  emf sampling function type definition(BLDC sensorless)
  */
typedef struct
{
  tmr_type *read_emf_tmr_x;
  int16_t emf_low_spd_sample_point;
  int16_t emf_change_duty_low;
  int16_t emf_change_duty_high;
  int16_t emf_high_spd_offset_rising;
  int16_t emf_high_spd_offset_falling;
  int16_t emf_half_vdc_val;
  uint16_t emf_high_spd_sample_cnt;
  int16_t emf_avoid_noise_times;
  flag_status emf_pull_up;
  flag_status emf_comp_continous_mode;
} emf_sample_type;

/**
  * @brief  adc sampling function type definition(BLDC sensorless)
  */
typedef struct
{
  tmr_type *adc_tmr_x;
  tmr_type *change_phase_tmr_x;
  adc_type *adc_x;
  int16_t i_sample_min_cnt;
  int16_t i_sample_delay_count;
  int16_t current_sampling_point;
  int16_t emf_sampling_point;
  int16_t adc_sample_point[2][3];
  int16_t adc_sample_page;
  emf_sample_type emf;
} adc_sample_type;

/**
  * @brief  ramp command function type definition
  */
typedef struct
{
  int32_t cmd_final;
  int32_t command;
  int16_t acc_slope;
  int16_t dec_slope;
} ramp_cmd_type;

/**
  * @brief  position controller function type definition
  */
typedef struct
{
  int32_t cmd_new;
  int32_t cmd_final;
  int32_t command;
  int32_t val;
  int32_t error;
  uint8_t count;
  int32_t spd_cmd;
  int16_t spd_slope;
  int16_t spd_slope_ratio;
  int16_t cmd_to_val_gap;
  int16_t small_pos_cmd_gap;
  int16_t min_pos_ctrl_spd;
  flag_status brake_flag;
  confirm_state stable;
  float rpm_to_cpr;
} position_type;

/**
  * @brief  mechanical angle type definition(FOC)
  */
typedef struct
{
  int32_t cmd_final;
  int32_t command;
  int32_t val;
} angle_type;

/**
  * @brief  usart queue function type definition
  */
typedef struct
{
  uint16_t  data_len;
  uint16_t  frame_len;
  uint16_t  no_error;
} usart_data_index;

/**
  * @brief  moving average function type definition
  */
typedef struct
{
  uint16_t order;
  uint16_t index;
  uint16_t full_flag;
  int32_t  sum;
  int32_t* buffer;
} moving_average_type;

/**
  * @brief  field weakening function type definition
  */
typedef struct
{
  pid_ctrl_type pid_fw;
  pid_ctrl_type *pid_speed;
  int16_t Idref_offset;
  qd_type *Vqd_filt;
  int16_t volt_filt;
  //int32_t volt_limit;
  //int16_t max_module;
  int16_t Id_max;
  int32_t curr_squr_max;
  int32_t fw_err_filt;
} field_weakening_type;

/**
  * @brief  low-pass filter function type definition
  */
typedef struct
{
  int32_t sample_freq;
  int16_t bandwidth;
  int16_t coef1;
  int16_t coef2;
  int64_t output_temp;
} lowpass_filter_type;

/**
  * @brief  motor voltage type definition(FOC sensorless)
  */
typedef struct
{
  alphabeta_type* Valphabeta;
  alphabeta_type* Ialphabeta;
  uint16_t* Vbus;
  uint16_t* Va;
  uint16_t* Vb;
  uint16_t* Vc;
  int16_t emf_factor;
  alphabeta_type Vpu;
} motor_volt_type;

/**
  * @brief  emf voltage type definition(FOC sensorless)
  */
typedef struct
{
  abc_type emf_abc_voltage;
  abc_type emf_abc_offset;
  alphabeta_type emf_alphabeta_voltage;
} motor_emf_type;

/**
  * @brief  state observer function type definition(FOC sensorless)
  */
typedef struct
{
  float *Rs;
  float *Ls;
  int16_t c1;
  int32_t c2;
  int32_t c3;
  int16_t c4;
  float c5;
  int32_t wIalpha_est;
  int32_t wIbeta_est;
  int16_t hIalpha_est;
  int16_t hIbeta_est;
  int32_t wBemf_alpha_est;
  int32_t wBemf_beta_est;
  int16_t hBemf_alpha_est;
  int16_t hBemf_beta_est;
  int16_t elec_angle;
  int8_t* dir;
  speed_type motor_speed;
  float elec_spd_to_rpm;
  pid_ctrl_type pid_pll;
} state_observer_type;

/**
  * @brief  sensorless startup function type definition(FOC sensorless)
  */
typedef struct
{
  int16_t max_speed;
  //int16_t ol_volt;
  int16_t ol_angle;
  uint16_t ol_delay_count;
  uint16_t elec_spd_to_rpm;
  //int16_t align_volt;
  uint32_t align_count;
  uint32_t start_count;
  int16_t start_current;
  int16_t elec_angle;
  int8_t dir;
  flag_status closeloop_rdy;
  flag_status closeloop_rdy_old;
} sensorless_startup_type;

/**
  * @brief  initial angle function type definition(FOC sensorless)
  */
typedef struct
{
  tmr_type* ADVTMRx;
  tmr_type* ADC_TMRx;
  tmr_channel_select_type ADC_TMRx_CHx;
  uint16_t period;
  uint16_t detect_duty;
  uint16_t *adc_trig;
  int16_t step_count;
  int16_t Iap;
  int16_t Ian;
  int16_t Ibp;
  int16_t Ibn;
  int16_t Icp;
  int16_t Icn;
  int16_t init_elec_angle;
} foc_angle_init_type;

/**
  * @brief  MOS's Rds(on) calibration fucntion type definition(FOC, E_BIKE_SCOOTER)
  */
typedef struct
{
  int32_t P1;
  int32_t P2;
  int32_t P1_filt;
  int32_t P2_filt;
  float P2_Const;
  int16_t Perr;
  uint16_t P1_ma_nbr_log;
  uint16_t P2_ma_nbr_log;
  qd_type *I_val;
  qd_type I_filt_val;
  lowpass_filter_type Iq_LPF;
  lowpass_filter_type Id_LPF;
  int16_t span_upper_limit;
  int16_t span_lower_limit;
} rds_cali_type;

/**
  * @brief  usart configuration type definition
  */
typedef struct
{
  uint32_t baudrate;
  crm_periph_clock_type usart_clock;
  crm_periph_clock_type usart_tx_gpio_clock;
  crm_periph_clock_type usart_rx_gpio_clock;
  gpio_type* usart_tx_gpio_port;
  uint32_t usart_tx_gpio_pin;
  gpio_type* usart_rx_gpio_port;
  uint32_t usart_rx_gpio_pin;
  uint32_t usart_gpio_remap;
  gpio_pins_source_type gpio_tx_pin_source;
  gpio_pins_source_type gpio_rx_pin_source;
#if defined AT32F421xx || defined AT32F423xx || defined AT32F425xx
  gpio_mux_sel_type gpio_tx_pin_mux;
  gpio_mux_sel_type gpio_rx_pin_mux;
#endif
  usart_type* usart_x;
} usart_config_type;

/**
  * @brief  user interface wavefrom type definition
  */
typedef struct
{
  int16_t speed_meas_filter_pu;
  int16_t speed_reference_pu;
  int16_t iMosTemperature_meas;
  int16_t iBusVoltage_meas;
  int16_t position_meas_pu;
  int16_t position_reference_pu;
  int16_t user_define_a;
  int16_t user_define_b;
  uint8_t sample_cycle;
} ui_wave_param_type;

/**
  * @brief  current pid auto-tune type definition
  */
typedef struct
{
  float *Rs;
  float *Ls;
  double current_base;
  uint16_t Vdc_rated;
  uint16_t bandwidth;
  uint16_t BW_limit;
  int32_t sample_freq;
  double kp_coff;
  double ki_coff;
  int16_t kp;
  int16_t ki;
  uint16_t kp_shift;
  uint16_t ki_shift;
  process_state_type state_flag;
} i_auto_tune_type;

/**
  * @brief  motor parameter identification type definition
  */
typedef struct
{
  tmr_type* ADVTMRx;
  int32_t sample_freq;
  uint16_t pwm_period;
  int16_t I_nominal;
  int16_t I_threshold;
  int16_t* I_val;
  union
  {
    float f;
    uint32_t u32;
  } Rs;
  union
  {
    float f;
    uint32_t u32;
  } Ls;
  union
  {
    float f;
    uint32_t u32;
  } Rs_Old;
  union
  {
    float f;
    uint32_t u32;
  } Ls_Old;
  double z_base;
  uint16_t align_time_count;
  uint8_t count;
  uint16_t duty;
  uint16_t* Vbus;
  flag_status id_flag;
  process_state_type state_flag;
  uint8_t step_flag;
#ifdef MOTOR_PARAM_IDENTIFY
  int16_t I_Ls_tab[200];
#else
  int16_t *I_Ls_tab;
#endif
} motor_param_id_type;

/**
  * @brief  hall state auto-learn type definition
  */
typedef struct
{
  hall_learn_process_type process_state;
  hall_sensor_type hall_state;
  flag_status start_flag;
  int8_t dir;
  int16_t learn_period;
  int16_t learn_angle_inc;
  int16_t learn_volt;
  int16_t align_time;
  int16_t count;
  int16_t step;
  flag_status check_flag;
} hall_learn_type;


#ifdef __cplusplus
}
#endif

#endif

