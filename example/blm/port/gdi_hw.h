/**
 * @file gdi_hw.h
 * @brief BLM 项目的 GDI 硬件资源池定义
 *
 * 定义 blm 项目使用的外设清单。
 * 其他项目复制此文件后按需修改。
 */
#ifndef __GDI_HW_H__
#define __GDI_HW_H__

#include "gdi/gdi.h"

typedef struct {
    gdi_gpio_t   *ptLedStatus;      /**< 状态 LED    */
    gdi_stream_t *ptSerialDebug;    /**< 调试/通信串口 */
    gdi_flash_t  *ptAppFlash;       /**< 应用程序 Flash */
} gdi_hardware_t;

extern const gdi_hardware_t HW;

#endif  /* __GDI_HW_H__ */
