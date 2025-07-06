#include "bsp_lsm6dsl.h"

#define SPI_DELAY() asm("nop; nop; nop; nop")

const lsm6dslio_t c_tlsm6dslio = {
    .tLsmInt1 = {
        .wPin = GPIO_PINS_15,
        .ptPort = GPIOA,
    },
    .tLsmInt2 = {
        .wPin = GPIO_PINS_7,
        .ptPort = GPIOB,
    },
    .tLsmCs = {
        .wPin = GPIO_PINS_6,
        .ptPort = GPIOB,
    },
    .tLsmSck = {
        .wPin = GPIO_PINS_5,
        .ptPort = GPIOB,
    },
    .tLsmMosi = {
        .wPin = GPIO_PINS_4,
        .ptPort = GPIOB,
    },
    .tLsmMiso = {
        .wPin = GPIO_PINS_3,
        .ptPort = GPIOB,
    },
};
void bsp_Lsm6dslInit(void)
{
    gpio_init_type gpio_init_struct;
    // output
    gpio_init_struct.gpio_pins = c_tlsm6dslio.tLsmCs.wPin;
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(c_tlsm6dslio.tLsmCs.ptPort, &gpio_init_struct);

    gpio_init_struct.gpio_pins = c_tlsm6dslio.tLsmSck.wPin;
    gpio_init(c_tlsm6dslio.tLsmSck.ptPort, &gpio_init_struct);

    gpio_init_struct.gpio_pins = c_tlsm6dslio.tLsmMosi.wPin;
    gpio_init(c_tlsm6dslio.tLsmMosi.ptPort, &gpio_init_struct);

    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(c_tlsm6dslio.tLsmMosi.ptPort, &gpio_init_struct);

    // input
    gpio_init_struct.gpio_pins = c_tlsm6dslio.tLsmInt2.wPin;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(c_tlsm6dslio.tLsmInt2.ptPort, &gpio_init_struct);
    gpio_init_struct.gpio_pins = c_tlsm6dslio.tLsmInt1.wPin;
    gpio_init(c_tlsm6dslio.tLsmInt1.ptPort, &gpio_init_struct);

    gpio_init_struct.gpio_pins = c_tlsm6dslio.tLsmMiso.wPin;
    gpio_init(c_tlsm6dslio.tLsmMiso.ptPort, &gpio_init_struct);

}

// 软件SPI传输（模式3：CPOL=1, CPHA=1）
uint8_t SPI_Transfer(uint8_t data) {
    uint8_t rx_data = 0;
    
    // 每个bit传输
    for(uint8_t i = 0; i < 8; i++) {
        // 在下降沿前设置MOSI（模式3特性）
        gpio_bits_write(c_tlsm6dslio.tLsmMosi.ptPort, c_tlsm6dslio.tLsmMosi.wPin,
            (data & 0x80) ? 1 : 0);

        data <<= 1;
        
        // 产生下降沿（文档图3）
        gpio_bits_write(c_tlsm6dslio.tLsmSck.ptPort, c_tlsm6dslio.tLsmSck.wPin, 0);
        SPI_DELAY();
        
        // 产生上升沿并采样MISO（文档图3）
        gpio_bits_write(c_tlsm6dslio.tLsmSck.ptPort, c_tlsm6dslio.tLsmSck.wPin, 1);
        rx_data <<= 1;
        if(gpio_input_data_bit_read(c_tlsm6dslio.tLsmMiso.ptPort, c_tlsm6dslio.tLsmMiso.wPin)) {
            rx_data |= 0x01;
        }
        SPI_DELAY();
    }
    return rx_data;
}

void LSM6DSL_WriteReg(uint8_t reg, uint8_t value) {
    gpio_bits_write(c_tlsm6dslio.tLsmCs.ptPort, c_tlsm6dslio.tLsmCs.wPin, 0);
    SPI_Transfer(reg & 0x7F);  // 清除MSB表示写操作（文档6.4.2节）
    SPI_Transfer(value);
    gpio_bits_write(c_tlsm6dslio.tLsmCs.ptPort, c_tlsm6dslio.tLsmCs.wPin, 1);
}

// 读取寄存器
uint8_t LSM6DSL_ReadReg(uint8_t reg) {
    gpio_bits_write(c_tlsm6dslio.tLsmCs.ptPort, c_tlsm6dslio.tLsmCs.wPin, 0);  // 选择设备
    SPI_Transfer(reg | 0x80);  // 设置MSB表示读操作（文档6.4.1节）
    uint8_t value = SPI_Transfer(0x00);
    gpio_bits_write(c_tlsm6dslio.tLsmCs.ptPort, c_tlsm6dslio.tLsmCs.wPin, 1);
    return value;
}

// 初始化传感器
uint8_t LSM6DSL_Init(void) {
    bsp_Lsm6dslInit();
    
    // 检查设备ID（文档9.12节）
    if(LSM6DSL_ReadReg(LSM6DSL_WHO_AM_I) != 0x6A) {
        return 0;  // 设备ID不匹配
    }
    
    // 配置加速度计（文档9.13节）
    LSM6DSL_WriteReg(LSM6DSL_CTRL1_XL, ACC_ODR_416HZ | ACC_FS_8G);
    
    // 配置陀螺仪（文档9.14节）
    LSM6DSL_WriteReg(LSM6DSL_CTRL2_G, GYRO_ODR_416HZ | GYRO_FS_2000DPS);
    
    return 1;  // 初始化成功
}

void LSM6DSL_ReadRawData(int16_t* accel, int16_t* gyro) {
    uint8_t buffer[12];
    
    // 启动多字节读取（文档6.4.1节）
    gpio_bits_write(c_tlsm6dslio.tLsmCs.ptPort, c_tlsm6dslio.tLsmCs.wPin, 0);
    SPI_Transfer(LSM6DSL_OUTX_L_G | 0x80);  // 从陀螺仪X低字节开始
    
    // 连续读取12字节（6轴×2字节）
    for(uint8_t i = 0; i < 12; i++) {
        buffer[i] = SPI_Transfer(0x00);
    }
    gpio_bits_write(c_tlsm6dslio.tLsmCs.ptPort, c_tlsm6dslio.tLsmCs.wPin, 1);  // 释放设备

    // 组合加速度计数据（文档9.35-9.40节）
    accel[0] = (int16_t)((buffer[6] << 8) | buffer[7]);  // X轴
    accel[1] = (int16_t)((buffer[8] << 8) | buffer[9]);  // Y轴
    accel[2] = (int16_t)((buffer[10] << 8) | buffer[11]); // Z轴

    // 组合陀螺仪数据（文档9.29-9.34节）
    gyro[0] = (int16_t)((buffer[0] << 8) | buffer[1]);  // X轴
    gyro[1] = (int16_t)((buffer[2] << 8) | buffer[3]);  // Y轴
    gyro[2] = (int16_t)((buffer[4] << 8) | buffer[5]);  // Z轴
}
