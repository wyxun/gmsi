/**
 * @file template_port.h
 * @brief GDI 适配层模板 — 头文件
 *
 * 定义项目专属的 gdi_hardware_t 结构体和 HW 全局声明。
 * 开发者复制此文件到具体项目的 port 目录中，改名为 gdi_hw.h，
 * 按实际外设增删成员即可。
 *
 * @usage
 *   1. 复制 template_port.h → 项目/port/gdi_hw.h
 *   2. 复制 template_port.c → 项目/port/gdi_hw.c
 *   3. 修改 gdi_hardware_t 成员以匹配实际外设
 *   4. 在需要使用 GDI 的 .c 文件中 #include "gdi_hw.h"
 */
#ifndef __GDI_HW_H__
#define __GDI_HW_H__

#include "gdi/gdi.h"

/*============================================================================
 * 项目硬件资源池定义
 *
 * 按需增删成员。每个指针对应一个外设实例。
 * 命名建议：pt + 功能名（如 ptLedStatus, ptSerialDebug）
 *===========================================================================*/

typedef struct {
    /* ---------- GPIO ---------- */
    gdi_gpio_t   *ptLedStatus;      /**< 状态 LED            */
    gdi_gpio_t   *ptLedError;       /**< 错误 LED (可选)     */
    gdi_gpio_t   *ptKeyInput;       /**< 按键输入            */
    /* 按需添加更多 GPIO:
     * gdi_gpio_t   *ptKeyUp;
     * gdi_gpio_t   *ptKeyDown;
     * gdi_gpio_t   *ptRelay1;
     */

    /* ---------- Stream (UART / USB CDC) ---------- */
    gdi_stream_t *ptSerialDebug;    /**< 调试串口            */
    gdi_stream_t *ptSerialComm;     /**< 通信串口 (可选)     */
    /* 按需添加更多串口:
     * gdi_stream_t *ptSerialGps;
     */

    /* ---------- PWM ---------- */
    gdi_pwm_t    *ptMotorMain;      /**< 主电机 PWM (可选)   */
    /* gdi_pwm_t    *ptBuzzer; */

    /* ---------- ADC ---------- */
    gdi_adc_t    *ptBatSensor;      /**< 电池电压 ADC (可选) */
    /* gdi_adc_t    *ptTempSensor; */

    /* ---------- IIC ---------- */
    /* gdi_iic_t    *ptIicSensor; */

    /* ---------- SPI ---------- */
    /* gdi_spi_t    *ptSpiFlash; */

    /* ---------- Flash ---------- */
    /* gdi_flash_t  *ptAppFlash; */

} gdi_hardware_t;

/**
 * @brief 全局硬件资源池声明（在 gdi_hw.c / port_gdi.c 中实例化）
 */
extern const gdi_hardware_t HW;

#endif  /* __GDI_HW_H__ */
