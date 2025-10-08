/**
  **************************************************************************
  * @file     motor_control_drive_param.h
  * @brief    Motor-related, drive-related and control-related parmeters, such as number of motor poles, maximum sensing voltage/current and pid speed parameters
  *           User defined motor drive modes (current sampling mode, sensor mode, etc.)
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

#ifndef __MOTOR_CONTROL_DRIVE_PARAM_H
#define __MOTOR_CONTROL_DRIVE_PARAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

#define AT_MOTOR_EVB_V2
//#define AT_MOTOR_EVB_V1

/* gate driver low side inverting logic input or non-inverting logic input*/
#define GATE_DRIVER_LOW_SIDE_INVERT


#define FOC_CONTROL

#define THREE_SHUNT
//#define TWO_SHUNT
//#define ONE_SHUNT


#ifdef TWO_SHUNT
#define U_V_SHUNT
//#define V_W_SHUNT
//#define U_W_SHUNT
#endif


#define SENSORLESS

#if defined THREE_SHUNT || defined TWO_SHUNT
#define CURRENT_LP_FILTER   0
#elif defined ONE_SHUNT
#define CURRENT_LP_FILTER   1
#endif

#ifdef SENSORLESS
#define OPENLOOP_STARTUP
//#define ALIGN_AND_GO_STARTUP
//#define INIT_ANGLE_STARTUP
#endif

#ifdef SENSORLESS
#define VOLT_SENSE       0         /* C67,C71,C74,C66,C70,C72 need to be soldered on the AT-MOTOR-EVB by using voltage sensing */
#endif

//#define FIELD_WEAKENING


//#define INTERNAL_CLOCK_SOURCE     /* use hick as clock source */


#define MOTOR_PARAM_IDENTIFY

/********************************* Motor-related parameter *********************************/
#define POLE_PAIRS                      (8/2)
#define RS_LL                           (1.89f)      /* Stator resistance(line-to-line), ohm */
#define LS_LL                           (0.002387f)  /* Stator inductance(line-to-line), H */

#define NOMINAL_CURRENT                 (1.7f)

/*** Quadrature encoder ***/
#define ENCODER_PPR                     1000  						/* Number of pulses per revolution */
#define ENC_IDX_COUNT                   11          /* Number of counts per index for JK42SBL01; default: 1 or 2 or 4 */
#define ENC_STALL_TIME                  1000								/* ms */


/* hall learn table */
#define HALL_LEARN_DIR                  (0)          /* Polarity, 0 or 1 */
#define HALL_LEARN_0_STATE              (1)
#define HALL_LEARN_1_STATE              (5)
#define HALL_LEARN_2_STATE              (4)
#define HALL_LEARN_3_STATE              (6)
#define HALL_LEARN_4_STATE              (2)
#define HALL_LEARN_5_STATE              (3)

/********************************* Drive-related parameter *********************************/
/* basic */
#define VDC_RATED                       (24.0f)
#define V_SENSE_GAIN                    (10/(3.9f+180+10))  // 0.05157
#define ADC_REFERENCE_VOLT              (3.3f)
#define ADC_DIGITAL_SCALE_12BITS        (4095)
/* Clock */
#define SYSTEM_CORE_CLOCK               200000000
#define TMR_CLK                         SYSTEM_CORE_CLOCK      //system_core_clock	
#define DEADTIME_CLK_SFT_BITS           ((tmr_clock_division_type) 2)
#define DEADTIME_NS                     ((uint16_t)300)      /* in nsec; check the MOSFET/IGBT/IPM specification
                                                                SYSTEM_CORE_CLOCK = 240MHz, range is [0...2000];
                                                                SYSTEM_CORE_CLOCK = 200MHz, range is [0...2500];
                                                                SYSTEM_CORE_CLOCK = 150MHz, range is [0...3000];
                                                                SYSTEM_CORE_CLOCK = 120MHz, range is [0...4000]; */
#define MIN_INTERVAL_TIME               (2000)    /* ns; pwm shift time for one shunt */
/* Current */
#define MAX_CURRENT                     (5.0f)
#define MIN_CURRENT                     (-MAX_CURRENT)
#define DC_MAX_CURRENT                  (10.0f)
#define CURRENT_SPAN_SHIFT              ((uint8_t) 1)
#ifdef ONE_SHUNT
#define R_SHUNT                         (0.005f)
#define OP_GAIN                         (39.0f/(1+39)*(1+9.1/1))                        // 9.8475 (9.8488)
#define CURR_OFFSET_VOLT                (ADC_REFERENCE_VOLT*(1.0/(1+39)*(1+9.1/1)))    // 0.83325 V  0.824
#else
#define R_SHUNT                         (0.002f)
#define OP_GAIN                         (33.0f/(1+33)*(1+16/1))                         // 16.5
#define CURR_OFFSET_VOLT                (ADC_REFERENCE_VOLT*(1.0f/(1+33)*(1+16/1)))     // 1.65 V
#endif

#define RDC_SHUNT                       (0.005f)
#define DC_OP_GAIN                      (39.0f/(1+39)*(1+9.1/1))                        // 9.8475 (9.8488)
#define IDC_OFFSET_VOLT                 (ADC_REFERENCE_VOLT*(1.0f/(1+39)*(1+9.1/1)))    // 0.83325 V  0.824

/* EMF */
#define EMF_SENSE_GAIN                  (3.9f/(3.9f+37.4f))   //0.09443

/* Protection */
/* Power */
#define OVER_POWER_THRESHOLD            (300)
/* Current */
#ifdef ONE_SHUNT
#define OVER_CURRENT_VREF               (ADC_REFERENCE_VOLT*(19.1f/(1.0+19.1f))							 // 3.136 V
#else
#define OVER_CURRENT_VREF               (ADC_REFERENCE_VOLT*(12.1f/(1.0+12.1f))							 // 3.048 V
#endif
/* Bus voltage */
#define OVER_VOLT_THRESHOLD             (55)
#define UNDER_VOLT_THRESHOLD            (10)
/* Temperature sensing section */
/* V[V]=V0+dV/dT[V/Celsius]*(T-T0)[Celsius] */
#define V0_V                            (0.480f) /*!< in Volts */
#define T0_C                            (0) /*!< in Celsius degrees */
#define dV_dT                           (0.024f) /*!< V/Celsius degrees */
#define OVER_TEMP_THRESHOLD             (70) /*!< Celsius degrees */
/* error code mask */
#define MC_ERROR_MASK                   (err_code_type) (MC_OVER_VOLT_ERROR | MC_UNDER_VOLT_ERROR | MC_OVER_TEMP_ERROR | MC_OVER_CURRENT_ERROR | MC_ENCODER_ERROR | MC_HALL_ERROR | MC_PARAM_IDENT_ERROR | MC_HALL_LEARN_ERROR)






/********************************* Control-related parameter *********************************/
#define PWM_FREQ                        16000	/* Hz */
#define MOTOR_CONTROL_MODE              OPEN_LOOP_CTRL
#define CTRL_SOURCE                     CTRL_SOURCE_SOFTWARE
#define UI_UART_BAUDRATE                (1500000UL)

/* current tuning parameter */
#define TUNE_TARGET_CURRENT             (1.0f)
#define TUNE_CURRENT_TOTAL_PERIOD       100
#define TUNE_CURRENT_STEP_PERIOD        2

/* SPEED */
#define SPEED_LOOP_FREQ                 1000 /* Hz */
#define MIN_SPEED_RPM                   10
#define MAX_SPEED_RPM                   6000
#define STABLE_SPEED_RPM                50
#define SLICK_SPEED_RPM                 50
#define MIN_POSCTL_SPD                  100
#define MIN_CONTROL_SPEED               200
#define ACC_SPD_SLOPE                   5    /* (rpm/ms) */
#define DEC_SPD_SLOPE                   5

/* POSITION */
#define POSITION_LOOP_FREQ              200     /* Hz */
#define MAX_POSITION_ANGLE              360000   /* Degree */
#define MIN_POSITION_ANGLE              (-MAX_POSITION_ANGLE)
#ifdef HALL_SENSORS
#define CMD_TO_VAL_GAP                  20000   /* hall eletric angle(32767 = 360 E-degree);  position is close target command*/
#else
#define CMD_TO_VAL_GAP                  1000    /* counts (ex: encoder CPR = 4000; 1000/4000*360 = 90 M-degree);  position is close target command*/
#endif
#define SMALL_POS_CMD_GAP               100
#define ROTOR_LOCK_ANGLE_GAP            120.0f

/* open loop control */
#define OLC_ANGLE_INC                  (0)
#define OLC_VOLT                       (0.0f)        /* (V) */

/* hall learn */
#define LEARN_OLC_VOLT                 (0.8f)     /* (V) */
#define LEARN_OLC_ANGLE_INC            (5)
#define LEARN_TIME                     (10000)   /* (ms) */
#define LEARN_ALIGN_TIME               (500) 				/* (ms) */

/* encoder align voltage */
#define ALIGN_VOLT                     (0.8f)    /* (V) */

/* current pid auto-tune */
#define CURRENT_BANDWIDTH               3500   /* 2*pi*freq */

/* Id/Iq pid parameter */
#define PID_ID_KP_DEFUALT               25000
#define PID_ID_KI_DEFUALT               3000
#define PID_ID_KP_GAIN_DIV              2048
#define PID_ID_KP_GAIN_DIV_LOG          LOG2(PID_ID_KP_GAIN_DIV)
#define PID_ID_KI_GAIN_DIV              4096
#define PID_ID_KI_GAIN_DIV_LOG          LOG2(PID_ID_KI_GAIN_DIV)

#define PID_IQ_KP_DEFUALT               25000
#define PID_IQ_KI_DEFUALT               3000
#define PID_IQ_KP_GAIN_DIV              2048
#define PID_IQ_KP_GAIN_DIV_LOG          LOG2(PID_IQ_KP_GAIN_DIV)
#define PID_IQ_KI_GAIN_DIV              4096
#define PID_IQ_KI_GAIN_DIV_LOG          LOG2(PID_IQ_KI_GAIN_DIV)

/* speed pid parameter */
#define PID_SPD_KP_DEFUALT              2000
#define PID_SPD_KI_DEFUALT              50
#define PID_SPD_KD_DEFUALT              0
#define PID_SPD_KP_GAIN_DIV             4096
#define PID_SPD_KP_GAIN_DIV_LOG         LOG2(PID_SPD_KP_GAIN_DIV)
#define PID_SPD_KI_GAIN_DIV             32768
#define PID_SPD_KI_GAIN_DIV_LOG         LOG2(PID_SPD_KI_GAIN_DIV)

/* position pid parameter */
#define PID_POS_KP_DEFUALT              800
#define PID_POS_KI_DEFUALT              0
#define PID_POS_KI_DEFUALT_STABLE       200     /* position is close to target, enhance the ki_gain to reach the target quickly */
#define PID_POS_KD_DEFUALT              0
#define PID_POS_KP_GAIN_DIV             4096
#define PID_POS_KP_GAIN_DIV_LOG         LOG2(PID_POS_KP_GAIN_DIV)
#define PID_POS_KI_GAIN_DIV             65536
#define PID_POS_KI_GAIN_DIV_LOG         LOG2(PID_POS_KI_GAIN_DIV)
#define PID_POS_KD_GAIN_DIV             65536
#define PID_POS_KD_GAIN_DIV_LOG         LOG2(PID_POS_KD_GAIN_DIV)

/* field weakening parameter */
#define FW_MAX_ID_CURR                  (0.3f)   /* unit: A ; 30% of nominal current */
#define FW_KP_GAIN                      10 /*!< Default Kp gain */
#define FW_KI_GAIN                      2000 /*!< Default Ki gain */
#define FW_KP_GAIN_DIV                  2048
#define FW_KP_GAIN_DIV_LOG              LOG2(FW_KP_GAIN_DIV)
#define FW_KI_GAIN_DIV                  32768
#define FW_KI_GAIN_DIV_LOG              LOG2(FW_KI_GAIN_DIV)

/* low pass filter parameter */
#define CURR_LP_BANDWIDTH               (6000.0f)  /* 2*pi*freq */
#define OBS_SPD_LP_BANDWIDTH            (300.0f)   /* 2*pi*freq */

/* State observer parameter */
#define OBS_GAIN1                       20000
#define OBS_GAIN2                       -20000
/* PLL gains */
#define PLL_KP_GAIN                     3000
#define PLL_KI_GAIN                     5
#define PLL_KP_GAIN_DIV                 32768
#define PLL_KP_GAIN_DIV_LOG             LOG2(PLL_KP_GAIN_DIV)
#define PLL_KI_GAIN_DIV                 32768
#define PLL_KI_GAIN_DIV_LOG             LOG2(PLL_KI_GAIN_DIV)

/* start-up parameter */
#define STARTUP_MAX_SPD                 400			   /* rpm */
#define STARTUP_CURRENT                 (0.5f)   /* A */
/* Open loop startup */
#define STARTUP_OL_SLOPE                800      /* rpm/s */
/* Fixed alpha-axis startup */
#define STARTUP_ALIGN_TIME              1000	    /* ms */
#define STARTUP_START_TIME              10			    /* ms */
/* Angle init parameter */
#define DETECT_PULSE_WIDTH              20.0f   	/* us;  range [1...10000] */

/* SP */
#define SP_MAX_VOLT                     (3.3f)   /* V */
#define SP_THRESHOLD                    (0.1f)   /* V */
#define SP_RUN_VALUE                    (0.3f)   /* V */
#define SP_STOP_VALUE                   (0.11f)  /* V */

/* E_BIKE_SCOOTER mode */
#define REVERSE_MAX_SPEED_RPM           -500      /* For E_BIKE_SCOOTER use only */
#define REVERSE_CURRENT                 (-0.3f)   /* For E_BIKE_SCOOTER use only */
#define BRAKING_CURRENT                 (-0.3f)   /* For E_BIKE_SCOOTER use only */

/* low speed voltage control parameter */
#define HYSTERESIS_LOW_SPEED            300
#define HYSTERESIS_HIGH_SPEED           400
/* low speed pid parameter for voltage control */
#define PID_SPD_VOLT_KP_DEFUALT         10000
#define PID_SPD_VOLT_KI_DEFUALT         300
#define PID_SPD_VOLT_KD_DEFUALT         0
#define PID_SPD_VOLT_KP_GAIN_DIV        1024
#define PID_SPD_VOLT_KP_GAIN_DIV_LOG    LOG2(PID_SPD_VOLT_KP_GAIN_DIV)
#define PID_SPD_VOLT_KI_GAIN_DIV        2048
#define PID_SPD_VOLT_KI_GAIN_DIV_LOG    LOG2(PID_SPD_VOLT_KI_GAIN_DIV)

#ifdef __cplusplus
}
#endif

#endif

