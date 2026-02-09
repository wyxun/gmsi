/****************************************************************************
*  Copyright 2024 Gorgon Meducer (Email:embedded_zhuoran@hotmail.com)       *
*                                                                           *
*  Licensed under the Apache License, Version 2.0 (the "License");          *
*  you may not use this file except in compliance with the License.         *
*  You may obtain a copy of the License at                                  *
*                                                                           *
*     http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                           *
*  Unless required by applicable law or agreed to in writing, software      *
*  distributed under the License is distributed on an "AS IS" BASIS,        *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
*  See the License for the specific language governing permissions and      *
*  limitations under the License.                                           *
*                                                                           *
****************************************************************************/

/**
 * @file perfc_port_user.h
 * @brief perf_counter porting for Cortex-M (BLM Bootloader)
 */
#ifndef __PERFC_PORT_USER_H__
#define __PERFC_PORT_USER_H__

/*============================ INCLUDES ======================================*/
#include <stdint.h>

#if __PERFC_USE_USER_CUSTOM_PORTING__

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
typedef uint32_t perfc_global_interrupt_status_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

/**
 * @brief Disable global interrupts (Cortex-M PRIMASK)
 */
static inline
perfc_global_interrupt_status_t perfc_port_disable_global_interrupt(void)
{
    perfc_global_interrupt_status_t tStatus;
    
    __asm volatile (
        "mrs %0, primask\n"
        "cpsid i\n"
        : "=r" (tStatus)
        :
        : "memory"
    );
    
    return tStatus;
}

/**
 * @brief Resume global interrupts (Cortex-M PRIMASK)
 */
static inline
void perfc_port_resume_global_interrupt(perfc_global_interrupt_status_t tStatus)
{
    __asm volatile (
        "msr primask, %0\n"
        :
        : "r" (tStatus)
        : "memory"
    );
}

#endif  /* __PERFC_USE_USER_CUSTOM_PORTING__ */

#endif  /* __PERFC_PORT_USER_H__ */
