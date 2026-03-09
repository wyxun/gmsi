GMSI 项目需求文档：硬件抽象层 (GDI) 实现
1. 项目背景与目标
项目名称：wyxun/gmsi (Bare-metal Embedded Framework)
核心目标：实现应用层代码与底层芯片代码的彻底解耦。
技术路径：

构建一个统一的硬件抽象层 GDI (GMSI Driver Interface)。

利用 C11 _Generic 关键字实现类似于“函数重载”的统一 API (GDI_Write, GDI_Read)。

底层驱动通过 git submodules 管理，并通过 Make 构建系统动态链接。

不使用 OS，保持纯裸机（Bare-metal）的高性能与低开销。

2. 目录结构规划
请在项目根目录下建立/更新以下文件结构：

Plaintext
gmsi/
├── gdi/                    # [NEW] 硬件抽象接口层
│   ├── gdi_types.h         # 定义外设行为结构体 (GPIO/PWM/Stream等)
│   ├── gdi.h               # 定义 _Generic 宏与全局资源池
│   └── template_port.c     # [NEW] 供开发者参考的适配层模版
/--------------------------------------------
project(我们这里没有protect，是外部的项目调用gmsi,所以不用管)
├── drivers/                # [EXISTING] 存放不同芯片的 submodule
│   └── bsp_xxx/            # 具体芯片实现 (如 bsp_stm32)
│       ├── module.mk       # 模块编译定义
│       └── port_gdi.c      # 实现 gdi 接口的具体代码
|── gmsi
|   ├── gdi/
└── Makefile                # [UPDATE] 支持基于 CHIP 变量的模块编译
3. 详细代码规范 (Specifications)
3.1 核心类型定义 (gmsi/gdi/gdi_types.h)
必须包含以下四类外设的标准化结构体。所有结构体必须包含 void *priv 指针用于底层传参。

GPIO (控制类):

方法：Set(level), Get(), Toggle()

状态枚举：GDI_GPIO_LOW, GDI_GPIO_HIGH

PWM (动力类):

方法：SetDuty(val), Enable(bool)

Stream (通信类 - UART/SPI/I2C 统一定义):

方法：Write(data, len), Read(data, len), IsBusy()

注：暂时统一为阻塞式接口，优先跑通 UART。(串口的配置为收发中断，会先在blm程序上做测试，外设配置不动)

ADC (传感类):

方法：Read()

IIC/SPI (传感类):

方法：Write(data, len), Read(data, len), IsBusy()

3.2 统一调度接口 (gmsi/gdi/gdi.h)
实现基于 C11 _Generic 的多态宏，使用户无需关心具体设备类型。

必须实现的宏接口：

GDI_Write(dev, ...): 自动路由到 GPIO Set, PWM SetDuty 或 Stream Write。

GDI_Read(dev, ...): 自动路由到 GPIO Get, Stream Read 或 ADC Read。

GDI_Toggle(dev): 路由到 GPIO Toggle。

GDI_Enable(dev, en): 路由到 PWM Enable。

全局资源池定义：

C
typedef struct {
    gdi_gpio_t   *LedStatus;
    gdi_gpio_t   *KeyInput;
    gdi_stream_t *SerialDebug;
    gdi_stream_t *SerialComm;
    gdi_pwm_t    *MotorMain;
    gdi_adc_t    *BatSensor;
} gdi_hardware_t;

extern const gdi_hardware_t HW; // 全局单例
3.3 适配层模版 (gmsi/gdi/template_port.c)
创建一个标准模版文件，包含上述所有接口的空实现（桩代码）。

代码中应包含详细注释，指导用户如何将 HAL_GPIO_WritePin 等厂商函数填入。

最后必须实例化 const gdi_hardware_t HW 结构体。

4. 构建系统需求 (Makefile)
修改根目录 Makefile 以支持基于环境变量的驱动切换。

逻辑要求：

引入变量 CHIP (默认为空或指定某芯片)。

根据 CHIP 的值，自动 include drivers/bsp_$(CHIP)/module.mk。

如果 CHIP 未定义或路径不存在，应报错提示。

Compile Flag: 确保支持 -std=c11 (为了 _Generic 支持)。

示例逻辑：

Makefile
CHIP ?= stm32
DRIVER_PATH := drivers/bsp_$(CHIP)
include $(DRIVER_PATH)/module.mk
CFLAGS += -std=c11 -Igmsi/gdi
5. 交付清单 (Definition of Done)
[x] Source Code: gmsi/gdi/ 下的 3 个关键文件 (gdi_types.h, gdi.h, template_port.c) 已创建。

[x] Verification: gdi.h 中的 _Generic 宏通过语法检查（无编译错误）。

[x] Build: Makefile 已更新，能正确识别 CHIP 变量并链接子模块。

[x] Usage: 在 app/main.c 中可以使用 GDI_Write(HW.ptLedStatus, GDI_GPIO_HIGH) 和 GDI_Write(HW.ptSerialDebug, buf, len) 进行混合调用。

给 Antigravity 的提示：

请优先保证 Header-only 逻辑的正确性（Type definitions）。

不要引入任何 OS 相关的头文件（如 pthread 或 FreeRTOS）。

保持代码风格简洁，符合嵌入式 C 标准。