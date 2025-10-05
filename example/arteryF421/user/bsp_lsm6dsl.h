#ifndef __BSP_LSM6DSL_H
#define __BSP_LSM6DSL_H

#include "at32f421_gpio.h"
#include "at32f421_bsp.h"

typedef struct{
    ggpio_t tLsmInt1;
    ggpio_t tLsmInt2;
    ggpio_t tLsmCs;
    ggpio_t tLsmSck;
    ggpio_t tLsmMiso;
    ggpio_t tLsmMosi;
}lsm6dslio_t;

// LSM6DSL寄存器地址（来自文档表19）
#define LSM6DSL_WHO_AM_I      0x0F
#define LSM6DSL_CTRL1_XL     0x10
#define LSM6DSL_CTRL2_G      0x11
#define LSM6DSL_STATUS_REG    0x1E
#define LSM6DSL_OUTX_L_G     0x22
#define LSM6DSL_OUTX_L_XL    0x28

// 加速度计和陀螺仪配置（根据需求调整）
#define ACC_ODR_416HZ        0x60  // 416Hz输出数据率
#define ACC_FS_8G           0x0C  // ±8g量程
#define GYRO_ODR_416HZ      0x60  // 416Hz输出数据率
#define GYRO_FS_2000DPS     0x0C  // ±2000dps量程

uint8_t LSM6DSL_Init(void);
void LSM6DSL_ReadRawData(int16_t* accel, int16_t* gyro);


#endif // __BSP_LSM6DSL_H