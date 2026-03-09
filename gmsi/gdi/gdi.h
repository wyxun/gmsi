/**
 * @file gdi.h
 * @brief GDI (GMSI Driver Interface) - 统一调度接口
 *
 * 利用 C11 _Generic 实现类"函数重载"的多态宏，使用户无需关心具体设备类型。
 * 通过参数计数宏区分不同参数形式（2参 vs 3参）。
 *
 * @usage
 *   GDI_Write(HW.ptLedStatus, GDI_GPIO_HIGH);       // GPIO Set
 *   GDI_Write(HW.ptSerialDebug, buf, len);           // Stream Write
 *   GDI_Read(HW.ptBatSensor);                        // ADC Read
 *   GDI_Toggle(HW.ptLedStatus);                      // GPIO Toggle
 *   GDI_Enable(HW.ptMotorMain, true);                // PWM Enable
 *
 * @note 需要 -std=c11 或 -std=gnu11 编译器支持。
 * @note 不包含任何 OS 相关头文件，纯裸机设计。
 */
#ifndef __GDI_H__
#define __GDI_H__

#include "gdi_types.h"
#include <stddef.h>     /* NULL */

/*============================================================================
 * Inline 包装函数
 *
 * 所有包装函数负责：①空指针检查 ②解引用函数指针并调用
 * 返回 int32_t，失败返回 -1（GMSI_EPERM 级别）
 *===========================================================================*/

/* ---- GPIO ---- */

static inline int32_t gdi_gpio_Set(gdi_gpio_t *ptDev, gdi_gpio_level_t eLevel)
{
    if (NULL == ptDev || NULL == ptDev->fnSet) { return -1; }
    return ptDev->fnSet(ptDev->pPriv, eLevel);
}

static inline int32_t gdi_gpio_Get(gdi_gpio_t *ptDev)
{
    if (NULL == ptDev || NULL == ptDev->fnGet) { return -1; }
    return ptDev->fnGet(ptDev->pPriv);
}

static inline int32_t gdi_gpio_Toggle(gdi_gpio_t *ptDev)
{
    if (NULL == ptDev || NULL == ptDev->fnToggle) { return -1; }
    return ptDev->fnToggle(ptDev->pPriv);
}

/* ---- PWM ---- */

static inline int32_t gdi_pwm_SetDuty(gdi_pwm_t *ptDev, uint32_t wDuty)
{
    if (NULL == ptDev || NULL == ptDev->fnSetDuty) { return -1; }
    return ptDev->fnSetDuty(ptDev->pPriv, wDuty);
}

static inline int32_t gdi_pwm_Enable(gdi_pwm_t *ptDev, bool bEnable)
{
    if (NULL == ptDev || NULL == ptDev->fnEnable) { return -1; }
    return ptDev->fnEnable(ptDev->pPriv, bEnable);
}

/* ---- Stream ---- */

static inline int32_t gdi_stream_Write(gdi_stream_t *ptDev,
                                       const uint8_t *pchData,
                                       uint32_t wLen)
{
    if (NULL == ptDev || NULL == ptDev->fnWrite) { return -1; }
    return ptDev->fnWrite(ptDev->pPriv, pchData, wLen);
}

static inline int32_t gdi_stream_Read(gdi_stream_t *ptDev,
                                      uint8_t *pchBuf,
                                      uint32_t wLen)
{
    if (NULL == ptDev || NULL == ptDev->fnRead) { return -1; }
    return ptDev->fnRead(ptDev->pPriv, pchBuf, wLen);
}

static inline int32_t gdi_stream_IsBusy(gdi_stream_t *ptDev)
{
    if (NULL == ptDev || NULL == ptDev->fnIsBusy) { return -1; }
    return ptDev->fnIsBusy(ptDev->pPriv);
}

/* ---- ADC ---- */

static inline int32_t gdi_adc_Read(gdi_adc_t *ptDev)
{
    if (NULL == ptDev || NULL == ptDev->fnRead) { return -1; }
    return ptDev->fnRead(ptDev->pPriv);
}

/* ---- IIC ---- */

static inline int32_t gdi_iic_Write(gdi_iic_t *ptDev,
                                    const uint8_t *pchData,
                                    uint32_t wLen)
{
    if (NULL == ptDev || NULL == ptDev->fnWrite) { return -1; }
    return ptDev->fnWrite(ptDev->pPriv, pchData, wLen);
}

static inline int32_t gdi_iic_Read(gdi_iic_t *ptDev,
                                   uint8_t *pchBuf,
                                   uint32_t wLen)
{
    if (NULL == ptDev || NULL == ptDev->fnRead) { return -1; }
    return ptDev->fnRead(ptDev->pPriv, pchBuf, wLen);
}

static inline int32_t gdi_iic_IsBusy(gdi_iic_t *ptDev)
{
    if (NULL == ptDev || NULL == ptDev->fnIsBusy) { return -1; }
    return ptDev->fnIsBusy(ptDev->pPriv);
}

/* ---- SPI ---- */

static inline int32_t gdi_spi_Write(gdi_spi_t *ptDev,
                                    const uint8_t *pchData,
                                    uint32_t wLen)
{
    if (NULL == ptDev || NULL == ptDev->fnWrite) { return -1; }
    return ptDev->fnWrite(ptDev->pPriv, pchData, wLen);
}

static inline int32_t gdi_spi_Read(gdi_spi_t *ptDev,
                                   uint8_t *pchBuf,
                                   uint32_t wLen)
{
    if (NULL == ptDev || NULL == ptDev->fnRead) { return -1; }
    return ptDev->fnRead(ptDev->pPriv, pchBuf, wLen);
}

static inline int32_t gdi_spi_IsBusy(gdi_spi_t *ptDev)
{
    if (NULL == ptDev || NULL == ptDev->fnIsBusy) { return -1; }
    return ptDev->fnIsBusy(ptDev->pPriv);
}

/* ---- Flash (专用接口，不走 _Generic) ---- */

static inline int32_t gdi_flash_Erase(gdi_flash_t *ptDev,
                                      uint32_t wAddr, uint32_t wSize)
{
    if (NULL == ptDev || NULL == ptDev->fnErase) { return -1; }
    return ptDev->fnErase(ptDev->pPriv, wAddr, wSize);
}

static inline int32_t gdi_flash_Write(gdi_flash_t *ptDev,
                                      uint32_t wAddr,
                                      const uint8_t *pchData,
                                      uint32_t wLen)
{
    if (NULL == ptDev || NULL == ptDev->fnWrite) { return -1; }
    return ptDev->fnWrite(ptDev->pPriv, wAddr, pchData, wLen);
}

static inline int32_t gdi_flash_Read(gdi_flash_t *ptDev,
                                     uint32_t wAddr,
                                     uint8_t *pchBuf,
                                     uint32_t wLen)
{
    if (NULL == ptDev || NULL == ptDev->fnRead) { return -1; }
    return ptDev->fnRead(ptDev->pPriv, wAddr, pchBuf, wLen);
}

static inline int32_t gdi_flash_Unlock(gdi_flash_t *ptDev)
{
    if (NULL == ptDev || NULL == ptDev->fnUnlock) { return -1; }
    return ptDev->fnUnlock(ptDev->pPriv);
}

static inline int32_t gdi_flash_Lock(gdi_flash_t *ptDev)
{
    if (NULL == ptDev || NULL == ptDev->fnLock) { return -1; }
    return ptDev->fnLock(ptDev->pPriv);
}

/*============================================================================
 * 参数计数辅助宏
 *===========================================================================*/

#define GDI_CONCAT_(a, b)   a##b
#define GDI_CONCAT(a, b)    GDI_CONCAT_(a, b)

#define GDI_NARGS_(...)             GDI_NARGS_IMPL_(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define GDI_NARGS_IMPL_(_1, _2, _3, _4, _5, N, ...)  N
#define GDI_NARGS(...)              GDI_NARGS_(__VA_ARGS__)

/*============================================================================
 * GDI_Write — 统一写入宏
 *
 *   2参: GDI_Write(gpio_dev, level)      → gdi_gpio_Set
 *        GDI_Write(pwm_dev, duty)        → gdi_pwm_SetDuty
 *   3参: GDI_Write(stream_dev, data, len) → gdi_stream_Write
 *        GDI_Write(iic_dev, data, len)    → gdi_iic_Write
 *        GDI_Write(spi_dev, data, len)    → gdi_spi_Write
 *===========================================================================*/

#define GDI_Write(dev, ...)  \
    GDI_CONCAT(GDI_Write_, GDI_NARGS(__VA_ARGS__))(dev, __VA_ARGS__)

/* 2参版 — GPIO Set / PWM SetDuty */
#define GDI_Write_1(dev, val)  _Generic((dev),          \
    gdi_gpio_t*:    gdi_gpio_Set,                       \
    gdi_pwm_t*:     gdi_pwm_SetDuty                     \
)(dev, val)

/* 3参版 — Stream / IIC / SPI Write */
#define GDI_Write_2(dev, data, len)  _Generic((dev),    \
    gdi_stream_t*:  gdi_stream_Write,                   \
    gdi_iic_t*:     gdi_iic_Write,                      \
    gdi_spi_t*:     gdi_spi_Write                       \
)(dev, data, len)

/*============================================================================
 * GDI_Read — 统一读取宏
 *
 *   1参: GDI_Read(gpio_dev)            → gdi_gpio_Get
 *        GDI_Read(adc_dev)             → gdi_adc_Read
 *   3参: GDI_Read(stream_dev, buf, len) → gdi_stream_Read
 *        GDI_Read(iic_dev, buf, len)    → gdi_iic_Read
 *        GDI_Read(spi_dev, buf, len)    → gdi_spi_Read
 *===========================================================================*/

#define GDI_Read(dev, ...)  \
    GDI_CONCAT(GDI_Read_, GDI_NARGS(__VA_ARGS__))(dev, __VA_ARGS__)

/* 仅 dev — GPIO Get / ADC Read（0 个额外参数，但宏计数从 dev 后开始） */
/* 注意：无额外参数时 __VA_ARGS__ 为空，需要特殊处理 */

/*
 * 对于 GDI_Read(dev) 无额外参数的情况：
 * 由于 __VA_ARGS__ 为空时参数计数不可靠，采用独立的无参宏路径。
 * 使用 _Generic 直接分发。
 */
#undef GDI_Read
#define GDI_Read(...)  \
    GDI_CONCAT(GDI_Read_N_, GDI_NARGS(__VA_ARGS__))(__VA_ARGS__)

/* 1参: GDI_Read(dev) — GPIO Get / ADC Read */
#define GDI_Read_N_1(dev)  _Generic((dev),              \
    gdi_gpio_t*:    gdi_gpio_Get,                       \
    gdi_adc_t*:     gdi_adc_Read                        \
)(dev)

/* 3参: GDI_Read(dev, buf, len) — Stream / IIC / SPI Read */
#define GDI_Read_N_3(dev, buf, len)  _Generic((dev),    \
    gdi_stream_t*:  gdi_stream_Read,                    \
    gdi_iic_t*:     gdi_iic_Read,                       \
    gdi_spi_t*:     gdi_spi_Read                        \
)(dev, buf, len)

/*============================================================================
 * GDI_Toggle — GPIO 翻转 (固定参数)
 *===========================================================================*/

#define GDI_Toggle(dev)  _Generic((dev),                \
    gdi_gpio_t*:    gdi_gpio_Toggle                     \
)(dev)

/*============================================================================
 * GDI_Enable — PWM 使能 (固定参数)
 *===========================================================================*/

#define GDI_Enable(dev, en)  _Generic((dev),            \
    gdi_pwm_t*:     gdi_pwm_Enable                      \
)(dev, en)

/*============================================================================
 * GDI_IsBusy — 总线忙检测 (固定参数)
 *===========================================================================*/

#define GDI_IsBusy(dev)  _Generic((dev),                \
    gdi_stream_t*:  gdi_stream_IsBusy,                  \
    gdi_iic_t*:     gdi_iic_IsBusy,                     \
    gdi_spi_t*:     gdi_spi_IsBusy                      \
)(dev)

/*============================================================================
 * 全局硬件资源池
 *
 * gdi_hardware_t 和 HW 由各项目自行定义 (见 gdi_types.h 末尾注释)。
 * 使用 GDI 宏的源文件需要 #include 项目的 gdi_hw.h。
 *
 * 参考模板: gmsi/gdi/template_port.h
 *===========================================================================*/

#endif  /* __GDI_H__ */
