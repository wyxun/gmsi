# Makefile for STM32G431RBT6 project with LLVM compiler
BUILD_DIR = build
COMPILER_PATH = /home/xun/LLVMEmbeddedToolchainForArm-16.0.0-Linux-AArch64/bin
GMSI_DIR := ../../gmsi
GMSI_THREEPART_PLOOC = ../../ThirdParty/plooc

# Compiler
CC = $(COMPILER_PATH)/clang
AS = $(COMPILER_PATH)/clang 
OBJCOPY = $(COMPILER_PATH)/llvm-objcopy
SIZE = $(COMPILER_PATH)/llvm-size

LTO_FLAGS = -flto
# Compiler flags
CFLAGS = -target armv7em-none-eabi -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -std=gnu11 -Wall -Wextra -Ofast
CFLAGS += -fno-builtin -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fshort-enums -fomit-frame-pointer -c -fdata-sections -ffunction-sections
CFLAGS += -I$(GMSI_DIR)
CFLAGS += -DSTM32G431xx
CFLAGS +=   \
-IInc \
-IDrivers/STM32G4xx_HAL_Driver/Inc \
-IDrivers/STM32G4xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32G4xx/Include \
-IDrivers/CMSIS/Include \
-I$(GMSI_THREEPART_PLOOC) \
-I$(GMSI_DIR)
# Assembler flags
ASFLAGS = -target armv7em-none-eabi -mthumb

# Linker script
LDSCRIPT = STM32G431.lds

# Linker flags
LDFLAGS = -target armv7em-none-eabi -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
LDFLAGS += -Wl,--start-group -lc -Wl,--end-group
LDFLAGS += -Wl,--gc-sections -Wl,--print-memory-usage
LDFLAGS += -T $(LDSCRIPT)

# Source files
SOURCES =  \
Src/main.c \
Src/gpio.c \
Src/fdcan.c \
Src/spi.c \
Src/usart.c \
Src/stm32g4xx_it.c \
Src/stm32g4xx_hal_msp.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_fdcan.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart_ex.c \
Src/system_stm32g4xx.c  

SOURCES += $(foreach dir, $(GMSI_DIR), $(wildcard $(dir)/*.c))

ASSEMBLY = startup_stm32g431xx.s

OBJECTS := $(addprefix $(BUILD_DIR)/, $(SOURCES:.c=.o) $(ASSEMBLY:.s=.o))
OUTPUT = $(BUILD_DIR)/5dcontrol.elf 
BIN_OUTPUT = $(BUILD_DIR)/5dcontrol.bin
HEX_OUTPUT = $(BUILD_DIR)/5dcontrol.hex

# Build rule
all: create_build_dir $(OUTPUT) $(HEX_OUTPUT) $(BIN_OUTPUT)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@
    
$(OUTPUT): $(OBJECTS)
	$(CC) $(LDFLAGS) $(LTO_FLAGS) $(OBJECTS) -o $@

$(HEX_OUTPUT): $(OUTPUT)
	$(OBJCOPY) -O ihex $< $@

$(BIN_OUTPUT): $(OUTPUT)
	$(OBJCOPY) -O binary $< $@

size: $(OUTPUT)
	$(SIZE) $(OUTPUT)

create_build_dir:
	@mkdir -p $(BUILD_DIR) $(BUILD_DIR)/Drivers/STM32G4xx_HAL_Driver/Src $(BUILD_DIR)/Src

clean:
	rm -rf $(BUILD_DIR)

printf_value:
	@echo $(info show obj '$(BUILD_DIR)')