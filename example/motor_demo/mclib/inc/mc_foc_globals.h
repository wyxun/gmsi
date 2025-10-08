/**
  **************************************************************************
  * @file     mc_foc_globals.h
  * @brief    Global variables declaration and default values, global functions declaration header file
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

#ifndef __MC_FOC_GLOBALS_H
#define __MC_FOC_GLOBALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

/* Motor-related parameters calculation */
#define RS                                 ((float)RS_LL/2.0f)
#define LS                                 ((float)LS_LL/2.0f)


/* Control-related parameters calculation */
/* Deadtime */
#define DEADTIME_CLK_DIV                   (1 << DEADTIME_CLK_SFT_BITS)
#define DEADTIME_CLK                       (TMR_CLK/DEADTIME_CLK_DIV)
#define DEADTIME                           ((uint8_t)((uint64_t)DEADTIME_CLK*DEADTIME_NS/1000000000uL))
/* Others */
#define TUNE_TARGET_CURRENT_PU             ((int16_t) (0x7FFF*TUNE_TARGET_CURRENT/CURRENT_BASE))
#define STARTUP_CURRENT_PU                 ((int16_t) (0x7FFF*STARTUP_CURRENT/CURRENT_BASE))

/* Angle init parameter */
#define ANGLE_INIT_DIV_FREQ                10000000
#define ANGLE_INIT_DIV                     ((uint16_t)(TMR_CLK/ANGLE_INIT_DIV_FREQ))
#define ANGLE_INIT_PERIOD                  ((uint16_t)((uint64_t)DETECT_PULSE_WIDTH*2*ANGLE_INIT_DIV_FREQ/(ANGLE_INIT_DIV_FREQ/10)))
#define ANGLE_INIT_HALF_PERIOD             (ANGLE_INIT_PERIOD/2)
#if defined THREE_SHUNT
#define DETECT_ADC_TRIG_TIME_1             (DETECT_PULSE_WIDTH*2-2.2f)   /* Ib, Ic */
#define DETECT_ADC_TRIG_TIME_2             (DETECT_PULSE_WIDTH*2-1.1f)	  /* Ia */
#define DETECT_ADC_TRIG_TIME_3             (DETECT_PULSE_WIDTH*2-1.1f)   /* Ia, Ic */
#define DETECT_ADC_TRIG_TIME_4             (DETECT_PULSE_WIDTH*2-2.2f)	  /* Ib */
#define DETECT_ADC_TRIG_TIME_5             (DETECT_PULSE_WIDTH*2-1.1f)	  /* Ia, Ib */
#define DETECT_ADC_TRIG_TIME_6             (DETECT_PULSE_WIDTH*2-3.3f)   /* Ic */
#elif defined TWO_SHUNT || defined ONE_SHUNT
#define DETECT_ADC_TRIG_TIME_1             (DETECT_PULSE_WIDTH*2-1)      /* Ib, Ic */
#define DETECT_ADC_TRIG_TIME_2             (DETECT_PULSE_WIDTH*2-1)      /* Ia */
#define DETECT_ADC_TRIG_TIME_3             (DETECT_PULSE_WIDTH*2-1)      /* Ia, Ic */
#define DETECT_ADC_TRIG_TIME_4             (DETECT_PULSE_WIDTH*2-1)      /* Ib */
#define DETECT_ADC_TRIG_TIME_5             (DETECT_PULSE_WIDTH*2-1)      /* Ia, Ib */
#define DETECT_ADC_TRIG_TIME_6             (DETECT_PULSE_WIDTH*2-1)      /* Ic */
#endif
#define DETECT_ADC_TRIG_POINT_1            ((uint16_t)(DETECT_ADC_TRIG_TIME_1*(ANGLE_INIT_DIV_FREQ/1000000.0f)))
#define DETECT_ADC_TRIG_POINT_2            ((uint16_t)(DETECT_ADC_TRIG_TIME_2*(ANGLE_INIT_DIV_FREQ/1000000.0f)))
#define DETECT_ADC_TRIG_POINT_3            ((uint16_t)(DETECT_ADC_TRIG_TIME_3*(ANGLE_INIT_DIV_FREQ/1000000.0f)))
#define DETECT_ADC_TRIG_POINT_4            ((uint16_t)(DETECT_ADC_TRIG_TIME_4*(ANGLE_INIT_DIV_FREQ/1000000.0f)))
#define DETECT_ADC_TRIG_POINT_5            ((uint16_t)(DETECT_ADC_TRIG_TIME_5*(ANGLE_INIT_DIV_FREQ/1000000.0f)))
#define DETECT_ADC_TRIG_POINT_6            ((uint16_t)(DETECT_ADC_TRIG_TIME_6*(ANGLE_INIT_DIV_FREQ/1000000.0f)))

/* brake pwm */
#define BRAKE_PULSE_WIDTH                  (10.0f)  /* ms */
#define BRAKE_PWM_DIV_FREQ                 10000
#define BRAKE_PWM_DIV                      ((uint16_t)(TMR_CLK/BRAKE_PWM_DIV_FREQ))
#define BRAKE_PWM_PERIOD                   ((uint16_t)(BRAKE_PULSE_WIDTH*2*BRAKE_PWM_DIV_FREQ/(BRAKE_PWM_DIV_FREQ/10)))

/* SP */
#define SP_OFFSET                          ((int16_t)(4095*SP_THRESHOLD/3.3f))
#define SP_RUN_POINT                       ((int16_t)(4095*(SP_RUN_VALUE-SP_THRESHOLD)/3.3f))
#define SP_STOP_POINT                      ((int16_t)(4095*(SP_STOP_VALUE-SP_THRESHOLD)/3.3f))
#define SP_TO_I_CMD                        ((int32_t)(32767.0f*32767.0f*(NOMINAL_CURRENT/CURRENT_BASE)/(4095*(1-(SP_THRESHOLD/SP_MAX_VOLT)))))
#define SP_TO_SPD_CMD                      ((int32_t)(32767.0f*(MAX_SPEED_RPM-MIN_SPEED_RPM)/(4095*(1-(SP_THRESHOLD/SP_MAX_VOLT)))/2))

/* unit transformation */
#define RPM_TO_SPEED_PU                    ((int32_t)(0x8000*26213.0f/(MAX_SPEED_RPM)))
#define DEGREE_TO_POS_PU                   ((float)(0x8000*26213.0f/(MAX_POSITION_ANGLE*100.0f)))

/* Drive-related parameters calculation */
#define I_SENSE_GAIN                       (R_SHUNT*OP_GAIN)
#define IDC_SENSE_GAIN                     (RDC_SHUNT*DC_OP_GAIN)
/* Basic */
#define VOLTAGE_BASE                       ((float)(ADC_REFERENCE_VOLT/V_SENSE_GAIN))  /* 63.987V */
#define CURRENT_BASE                       ((float)((ADC_REFERENCE_VOLT-CURR_OFFSET_VOLT)/I_SENSE_GAIN))  		 /* ONE_SHUNT / THREE_SHUNT/TWO_SHUNT: 50A(peak) */
#define DC_CURRENT_BASE                    ((float)((ADC_REFERENCE_VOLT-IDC_OFFSET_VOLT)/IDC_SENSE_GAIN))
/* Current */
#define NOMINAL_CURRENT_PU                 ((int16_t) (0x7FFF*NOMINAL_CURRENT/CURRENT_BASE))
#define MAX_CURRENT_PU                     ((int16_t) (0x7FFF*MAX_CURRENT/CURRENT_BASE))
#define MIN_CURRENT_PU                     ((int16_t) (0x7FFF*MIN_CURRENT/CURRENT_BASE))
#define DC_MAX_CURRENT_PU                  ((int16_t) (0x7FFF*DC_MAX_CURRENT/CURRENT_BASE))
#define BRAKING_CURRENT_PU                 ((int16_t) (0x7FFF*BRAKING_CURRENT/CURRENT_BASE))
#define REVERSE_CURRENT_PU                 ((int16_t) (0x7FFF*REVERSE_CURRENT/CURRENT_BASE))
#define CURRENT_SPAN                       ((uint16_t)((uint32_t)(0x7FFF*(3.3f/(3.3f-CURR_OFFSET_VOLT)))) >> CURRENT_SPAN_SHIFT)
#define DC_CURRENT_SPAN                    ((uint16_t)((uint32_t)(0x7FFF*(3.3f/(3.3f-IDC_OFFSET_VOLT))*(float)(DC_CURRENT_BASE/CURRENT_BASE))) >> CURRENT_SPAN_SHIFT)
#define CURR_OFFSET_VALUE                  ((uint16_t)(ADC_DIGITAL_SCALE_12BITS*CURR_OFFSET_VOLT/ADC_REFERENCE_VOLT))
#define DC_CURR_MA_NBR                     (20)      /* dc current moving average filter number */
#define REDUCE_IQ                          (2)
#define RECOVER_IQ                         (1)
/* Voltage */
#define OLC_VOLT_CMD                       (((int16_t)32767.0f*OLC_VOLT/VDC_RATED*SQRT_3))
#define ALIGN_VOLT_CMD                     ((int16_t)((int16_t)32767.0f*ALIGN_VOLT/VDC_RATED*SQRT_3))
/* EMF */
#define EMF_BASE                           (ADC_REFERENCE_VOLT/EMF_SENSE_GAIN)  /* 34.964V */
#define EMF_VBUS_SENSE_FACTOR              ((int16_t) (0x7FFF*V_SENSE_GAIN/EMF_SENSE_GAIN))  /* 17895 */
/* Field weakening */
#define FW_MAX_ID_CURR_PU                  ((int16_t) (0x7FFF*FW_MAX_ID_CURR/CURRENT_BASE))
#define FW_MAX_CURR_SQUR                   ((int32_t) (NOMINAL_CURRENT_PU*NOMINAL_CURRENT_PU))
/* Protection */
#define OVER_CURRENT                       ((OVER_CURRENT_VREF - CURR_OFFSET_VOLT)/R_SHUNT/OP_GAIN)   /* ONE_SHUNT:46.77 A(peak), THREE_SHUNT/TWO_SHUNT:42.36 A(peak) */
#define OVERVOLTAGE_THRESHOLD_d            (uint16_t)(OVER_VOLT_THRESHOLD*ADC_DIGITAL_SCALE_12BITS/VOLTAGE_BASE)
#define UNDERVOLTAGE_THRESHOLD_d           (uint16_t)(UNDER_VOLT_THRESHOLD*ADC_DIGITAL_SCALE_12BITS/VOLTAGE_BASE)
#define TEMPERATURE_THRESHOLD_d            (uint16_t)((V0_V+dV_dT*(OVER_TEMP_THRESHOLD-T0_C))*ADC_DIGITAL_SCALE_12BITS/ADC_REFERENCE_VOLT)

/* others */
#define PWM_PERIOD                         ((uint16_t) (TMR_CLK/PWM_FREQ/2))
#define HALF_PWM_PERIOD                    ((uint16_t) (TMR_CLK/PWM_FREQ/2)/2)
#define DOUBLE_PWM_PERIOD                  ((uint16_t) (PWM_PERIOD*2-1))
#define PWM_PERIOD_90PCT                   ((uint16_t) (PWM_PERIOD*0.866))
#define ADC_TRIG_POS                       ((uint16_t) (PWM_PERIOD-1))
#define MIN_INTERVAL                       ((int16_t) ((float)0x7FFF*MIN_INTERVAL_TIME*2.0f*PWM_FREQ/1000000000))
#define PWM_SHIFT_DUTY_CHECK               (MIN_INTERVAL_TIME*2*PWM_FREQ)-66987298
#define QUARTER_DUTY_CYCLE                 ((int16_t) (0x7FFF*0.5f))

/* encoder sensor */
#define ENC_CPR_NBR                        ((int32_t) (4*ENCODER_PPR)) 								/* Number of counts per revolution */
#define ENC_HALF_CPR_NBR                   ((int32_t) (ENC_CPR_NBR/2))
#define ENC_ALIGN_TIME                     ((uint32_t) (2.0*PWM_FREQ)) 																/* Wait 2 sec	*/
#define ENC_MECH_TO_ELECT_ANGLE            ((float) (32767.0f*POLE_PAIRS/(ENC_CPR_NBR-1))) 				/* Convert Mechanical Angle to Electric Angle */
#define ENC_CPR_TO_RPM                     ((float) (SPEED_LOOP_FREQ*60.0f/ENC_CPR_NBR))  				 /* in RPM => pulse convert to RPM	(speed control loop)*/
#define ENC_RPM_TO_CPR                     ((float) (ENC_CPR_NBR/60.0f/POSITION_LOOP_FREQ))	   /* in pulse => RPM convert to pulse (position control loop)	*/

/* encoder sensor using capture */
#define ENC_CAPTURE_CLK                    3000000
#define ENC_TIM_CAP_CLK_DIV                ((uint16_t) ((double)TMR_CLK/ENC_CAPTURE_CLK))
#define ENC_CONST_TIMER_CLKS               ((uint32_t) (ENC_CAPTURE_CLK/SPEED_LOOP_FREQ))
#define ENC_SPEED_SHIFT                    3
#define ENC_EST_SPEED_CONST                ((int32_t) ((uint32_t)(60.0f*ENC_CAPTURE_CLK/(ENC_CPR_NBR))<<ENC_SPEED_SHIFT))

/* speed */
#define SPD_LOOP_TMR_PR                    (1000)
#define SPD_LOOP_TMR_DIV                   ((TMR_CLK/SPD_LOOP_TMR_PR/SPEED_LOOP_FREQ)-1)

/* position */
#ifdef HALL_SENSORS
#define HALL_ELEC_ANGLE                    ((int32_t)32768*POLE_PAIRS)
#define HALL_RPM_TO_CPR                    ((float)(HALL_ELEC_ANGLE/60.0f/POSITION_LOOP_FREQ))	   /* in pulse => RPM convert to pulse (position control loop)	*/
#define ANGLE_TO_PULSE                     ((float)(HALL_ELEC_ANGLE/360.0f/100))
#define PULSE_TO_ANGLE                     ((float)(100*360.0f/HALL_ELEC_ANGLE))
#define SPEED_SLOPE_RATION                 ((int16_t)(0.4f*1000/POSITION_LOOP_FREQ))  /* systick = 1000 Hz */
#define ROTOR_LOCK_GAP                     ((int16_t)0x7FFF/(360.0f/ROTOR_LOCK_ANGLE_GAP))
#else /* ENCODER */
#define ANGLE_TO_PULSE                     ((float)(ENC_CPR_NBR/360.0f/100))
#define PULSE_TO_ANGLE                     ((float)(100*360.0f/ENC_CPR_NBR))
#define SPEED_SLOPE_RATION                 ((int16_t)(1000/POSITION_LOOP_FREQ))  /* systick = 1000 Hz */
#endif
#define POSITION_LOOP_COUNT                ((uint16_t) (SPEED_LOOP_FREQ/POSITION_LOOP_FREQ-1))

/* hall sensor */
#define MAX_CAP_COUNT                      0xFFFF																							/* Maximum number of Hall signals capture counts */
#define CAPTURE_CLK                        (MAX_CAP_COUNT*MIN_SPEED_RPM/60*POLE_PAIRS*(6/2))   /* 2 times small Hall cycle time */
#define TIM_CAP_CLK_DIV                    ((uint16_t) ((double)TMR_CLK/CAPTURE_CLK))
#define HALL_SPEED_SHIFT                   4
#define MIN_SPEED_RPM_SHIFT                (MIN_SPEED_RPM << HALL_SPEED_SHIFT)
#define MIN_CONTROL_SPEED_SHIFT            (MIN_CONTROL_SPEED << HALL_SPEED_SHIFT)
#define STABLE_SPEED_RPM_SHIFT             (STABLE_SPEED_RPM << HALL_SPEED_SHIFT)
#define SLICK_SPEED_RPM_SHIFT              (SLICK_SPEED_RPM << HALL_SPEED_SHIFT)
#define MIN_SPD_HALL_INR                   ((int32_t)((MAX_CAP_COUNT*MIN_SPEED_RPM/2)<<HALL_SPEED_SHIFT))
#define HALF_MAX_SPD_HALL_INR              ((int32_t)((MIN_SPD_HALL_INR/MAX_SPEED_RPM/2)>>HALL_SPEED_SHIFT))
#define THETA_CMPSAT                       ((int32_t)((32767.0f*65536.0f*POLE_PAIRS/PWM_FREQ/60)+1)>>HALL_SPEED_SHIFT)
#define HALL_INTERVAL_CONST                ((int16_t)(360/60))

#define THIRTY_DEGREE                      ((int16_t) (0x7FFF*30/360))
#define SIXTY_DEGREE                       ((int16_t) (0x7FFF*60/360))
#define SEVENTY_DEGREE                     ((int16_t) (0x7FFF*70/360))
#define NINETY_DEGREE                      ((int16_t) (0x7FFF*90/360))
#define HALF_TURN_DEGREE                   ((int16_t) (0x7FFF*180/360))

#define HALL_LEARN_CHECK_FLAG              (RESET)
#define LEARN_OLC_VOLT_CMD                 ((int16_t)((int16_t)32767.0f*LEARN_OLC_VOLT/VDC_RATED*SQRT_3))
#define HALL_LEARN_TABLE_LENGTH            (6)
#define LEARN_CHECK_TIMES                  (12)      /* check 12 times of hall state */

/* math */
#define SQRT3_2                            ((int16_t) (0x7FFF*0.866025404f))
#define INV_SQRT_3                         ((int16_t) (0x7FFF*0.57735027f))
#define SQRT_3                             (1.73205081f)
#define INV_3                              ((int16_t) (0x7FFF*(1.0f/3.0f)))

/* Vd Vq */
#define DEAD_ZONE_SIZE                     ((int16_t)(0x7FFF*DEADTIME_NS*2.0f*PWM_FREQ/1000000000))
#if defined ONE_SHUNT
#define MAX_VOLT_ZONE                      (MIN_INTERVAL_TIME*2.0f*PWM_FREQ/1000000000)
#define ZONE_SIZE                          ((int16_t)(75673*MAX_VOLT_ZONE-5069))
#if PWM_SHIFT_DUTY_CHECK > 0
#define MAX_VS                             (int16_t)(INT16_MAX-ZONE_SIZE)
#else
#define MAX_VS                             (INT16_MAX)
#endif
#elif defined THREE_SHUNT
#define MAX_VS                             (INT16_MAX-DEAD_ZONE_SIZE)
#elif defined TWO_SHUNT
#define MAX_VS                             ((INT16_MAX-DEAD_ZONE_SIZE)*0.95f)
#endif

#define VQ_MAX                             (int16_t)(MAX_VS*0.999f)
#define VD_MAX                             (int16_t)(MAX_VS*0.999f)
#define VQD_SQUR_SUM_MAX                   (int32_t)(VQ_MAX*VQ_MAX)

/* field weakening */
#define FW_MA_NBR                          (512)      /* moving average filter number */
#define FW_MA_NBR_LOG                      (LOG2(FW_MA_NBR))

/* motor direction */
#define CCW                                (-1)
#define	CW                                 (1)

/* sensorless */
#define V_PHASE_BASE                       (VOLTAGE_BASE/SQRT_3)        /* 36.943 */
#define ZS_BASE                            (float)(V_PHASE_BASE/CURRENT_BASE)  /* 0.7388 */

#define C2                                 (OBS_GAIN1)
#define C4                                 (OBS_GAIN2)
#define C5                                 (2*PI)

#define ELEC_SPD_TO_RPM                    ((PWM_FREQ*60.0f)/(0x7FFF*POLE_PAIRS))
#define STARTUP_OL_DELAY_CNT               ((uint16_t)((uint32_t)ELEC_SPD_TO_RPM*PWM_FREQ/STARTUP_OL_SLOPE))
#define STARTUP_ALIGN_COUNT                ((uint32_t)(STARTUP_ALIGN_TIME*PWM_FREQ/1000.0f))
#define STARTUP_START_COUNT                ((uint32_t)(STARTUP_START_TIME*PWM_FREQ/1000.0f))

#define Q30_MAX                            ((int32_t) (1073741823))

#define STEP_0_SETTING                     0x00
#define STEP_1_AH_PWM_BCL                  0x01
#define STEP_2_BCH_PWM_AL                  0x02
#define STEP_3_BH_PWM_ACL                  0x03
#define STEP_4_ACH_PWM_BL                  0x04
#define STEP_5_CH_PWM_ABL                  0x05
#define STEP_6_ABH_PWM_CL                  0x06
#define STEP_7_ANGLE_EST                   0x07
#define STEP_8_MAX                         0x08

/* motor parameter identify init parameter */
#define PWM_FREQ_ID                        30000	/* Hz */
#define PWM_PERIOD_ID                      ((uint16_t) (TMR_CLK/PWM_FREQ_ID/2))
#define HALF_PWM_PERIOD_ID                 ((uint16_t) (TMR_CLK/PWM_FREQ_ID/2)/2)
#define DOUBLE_PWM_PERIOD_ID               ((uint16_t) (PWM_PERIOD_ID*2-1))
#define ADC_TRIG_POS_ID                    ((uint16_t) (PWM_PERIOD_ID-1))
#define ADC_TRIG_DELAY_ID                  (0.0)   /* us */
#define ADC_TRIG_DELAY_COUNT               ((uint16_t) (ADC_TRIG_DELAY_ID*TMR_CLK/1000000))   /* ns */
#define SYSTEM_TICK_FREQ                   1000
#define ALIGN_TIME                         0.5f   /* second */
#define ALIGN_TIME_COUNT                   ((uint16_t)(ALIGN_TIME*SYSTEM_TICK_FREQ))
#define Z_BASE                             ((double)(VOLTAGE_BASE/CURRENT_BASE))  /* 1.27974 */
#define THRESHOLD_CURRENT                  ((double)(CURRENT_BASE*0.003f))
#define THRESHOLD_CURRENT_PU               ((int16_t)(0x7FFF*THRESHOLD_CURRENT/CURRENT_BASE))

/* auto-tune current PI parameter */
#define CURRENT_BW_LIMIT                   ((uint16_t)(2.0f*PI*PWM_FREQ/15))

/**************** define drive output enable pattern ******************/
#define TMR_PWM_OUT_MODE_MASK              0x0555

#define TMR_PWM_OUT_1TOP_2BOTTOM_3BOTTOM   0x1441  /* Ia */
#define TMR_PWM_OUT_1BOTTOM_2TOP_3TOP      0x1114  /* -Ia */
#define TMR_PWM_OUT_1BOTTOM_2TOP_3BOTTOM   0x1414  /* Ib */
#define TMR_PWM_OUT_1TOP_2BOTTOM_3TOP      0x1141  /* -Ib */
#define TMR_PWM_OUT_1BOTTOM_2BOTTOM_3TOP   0x1144  /* Ic */
#define TMR_PWM_OUT_1TOP_2TOP_3BOTTOM      0x1411  /* -Ic */

/**************** define pwm mode pattern for angle init ******************/
#define TMR_PWM_MODE_CM1_MASK              0x7070
#define TMR_PWM_MODE_CM2_MASK              0x0070

#define TMR_PWM_1PWMA_2PWMA                0x6060
#define TMR_PWM_3PWMA                      0x0060
#define TMR_PWM_1LOW_2LOW                  0x4040
#define TMR_PWM_3LOW                       0x0040

#define TMR_PWM_1PWMB_2PWMB                0x7070
#define TMR_PWM_1PWMB_2OFF                 0x0070
#define TMR_PWM_1OFF_2PWMB                 0x7000
#define TMR_PWM_3PWMB                      0x0070

#define TMR_PWM_1OFF_2PWMA                 0x6000
#define TMR_PWM_3PWMA_4PWMB                0x7060

/* select pid controller function*/
#define pid_controller                     pid_controller_static_clamp

/**
  * @brief  Macro to compute logarithm of two
  */
#define LOG2(x) \
  ((x) >= 65536 ? 16 : \
   ((x) >= 2*2*2*2*2*2*2*2*2*2*2*2*2*2*2 ? 15 : \
    ((x) >= 2*2*2*2*2*2*2*2*2*2*2*2*2*2 ? 14 : \
     ((x) >= 2*2*2*2*2*2*2*2*2*2*2*2*2 ? 13 : \
      ((x) >= 2*2*2*2*2*2*2*2*2*2*2*2 ? 12 : \
       ((x) >= 2*2*2*2*2*2*2*2*2*2*2 ? 11 : \
        ((x) >= 2*2*2*2*2*2*2*2*2*2 ? 10 : \
         ((x) >= 2*2*2*2*2*2*2*2*2 ? 9 : \
          ((x) >= 2*2*2*2*2*2*2*2 ? 8 : \
           ((x) >= 2*2*2*2*2*2*2 ? 7 : \
            ((x) >= 2*2*2*2*2*2 ? 6 : \
             ((x) >= 2*2*2*2*2 ? 5 : \
              ((x) >= 2*2*2*2 ? 4 : \
               ((x) >= 2*2*2 ? 3 : \
                ((x) >= 2*2 ? 2 : \
                 ((x) >= 2 ? 1 : \
                  ((x) >= 1 ? 0 : -1)))))))))))))))))


/**
  * @brief  Macro to compute exponential by taylor expansion
  */
#define EXP_TAYLOR(x) (1+x*(1.0f+x/2.0f*(1+x/3.0f*(1+x/4.0f*(1+x/5.0f*(1+x/6.0f*(1+x/7.0f*(1+x/8.0f*(1+x/9.0f*(1+x/10.0f))))))))))



/* RDS Calibrated parameter */
#define P2_CONST                        (1.0f)
#define P1_MA_NBR                       (8)
#define P1_MA_NBR_LOG                   LOG2(P1_MA_NBR)
#define P2_MA_NBR                       (8)
#define P2_MA_NBR_LOG                   LOG2(P2_MA_NBR)
#define IQD_LP_BANDWIDTH                (1000.0f)  /* 2*pi*freq */
#define SPAN_UPPER_LIMIT                (UINT16_MAX>>CURRENT_SPAN_SHIFT)
#define SPAN_LOWER_LIMIT                (SPAN_UPPER_LIMIT>>4)

extern motor_control_mode ctrl_mode, ctrl_mode_old, ctrl_mode_cmd;
extern firmware_id_type firmware_id;
extern voltage_type volt_cmd;
extern rotor_angle_type rotor_angle_encoder, rotor_angle_hall, rotor_angle_hall_old;
extern current_type current;
extern encoder_type encoder, encoder1;
extern open_loop_type openloop;
extern speed_type rotor_speed_encoder, rotor_speed_hall;
extern hall_sensor_type hall;
extern pid_ctrl_type pid_iq, pid_id, pid_spd, pid_pos;
extern ramp_cmd_type speed_ramp;
extern position_type pos;
extern angle_type angle;
extern flag_status param_initial_rdy;
extern flag_status curr_offset_rdy;
extern flag_status foc_rdy;
extern int16_t elec_angle_val;
extern int32_t rotor_speed_val, rotor_speed_val_filt;
extern moving_average_type *speed_ma_fliter;
extern float fMosTemperature;
extern uint16_t I_tune_count;
extern int16_t current_tune_target_current;
extern uint16_t current_tune_total_period;
extern uint16_t current_tune_step_period;
extern ctrl_source_type ctrl_source;
extern RCP_Frame_t rx_data_command;
extern TCP_Frame_t tx_data_response;
extern flag_status start_stop_btn_flag;
extern esc_state_type esc_state_old;
extern esc_state_type esc_state;
extern pwm_duty_type pwm_duty;
extern moving_average_type *hall_interval_moving_average;
extern err_code_type error_code, error_code_mask;
extern field_weakening_type field_weakening;
extern lowpass_filter_type q_current_LPF, d_current_LPF, obs_speed_LPF;
extern motor_volt_type motor_voltage;
extern state_observer_type state_observer;
extern sensorless_startup_type startup;
extern foc_angle_init_type angle_detector;
extern usart_config_type ui_usart;
extern ui_wave_param_type ui_wave_param;
extern flag_status cmd_response_rdy;
extern int16_t *monitor_data[2];
extern uint8_t monitor_data_buffer[2][DATA_BUFFER_FRAME_SIZE];
extern uint8_t extra_data_buffer[DATA_BUFFER_EXTRA_SIZE];
extern uint8_t monitor_data_buffer_num;
extern uint16_t buffer_index;
extern uint8_t ui_receive_cmd_index;
extern int32_t save_param_buffer[INT32_SIZE_ARRAY];
extern usart_data_index usart_data_idx;
extern uint16_t angle_init_adc_trig_table[6];
extern int16_t *hall_next_state_table, *hall_theta_table;
extern int16_t hall_cw_next_state_table[8];
extern int16_t hall_ccw_next_state_table[8];
extern uint16_t hall_startup_theta_table[8];
extern int16_t hall_cw_theta_table[8];
extern int16_t hall_ccw_theta_table[8];
extern hall_learn_type hall_learn;
extern int16_t sys_counter;
extern uint16_t hall_learn_state_table[6];
extern uint16_t hall_learn_sequence_table[6];
extern uint16_t next_hall_learn_state_table[7];
extern uint8_t charge_boot_count;
extern qd_type I_ref, Iref, Ival;;
extern int16_t Iq_ref_cmd;
extern moving_average_type *Idc_ma_fliter;
extern flag_status write_flash_flag;
extern rds_cali_type Rds_Cali;
extern motor_emf_type motor_emf;
extern const char s_fwVer[32];
extern int32_t *motor_speed_val;
extern int32_t motor_speed_filter;
extern flag_status reverse_flag;
extern i_auto_tune_type I_auto_tune;
extern motor_param_id_type motor_param_ident;
#ifdef LOW_SPEED_VOLT_CTRL
extern pid_ctrl_type pid_spd_volt;
extern low_spd_ctrl_type low_spd_ctrl_mode;
#endif
#ifdef __cplusplus
}
#endif

#endif

