# Makefile for STM32G431RBT6 project with LLVM compiler
BUILD_DIR = build
CHIP_DIR = ../../arch/cortex_m/at32f413
GMSI_DIR := ../../gmsi
GMSI_THREEPART_PLOOC = ../../ThirdParty/plooc
TARGET := follow
# Compiler
CC = D:\GNUEmbeddedToolchain\LLVMEmbeddedToolchainForArm\bin\clang.exe
AS = D:\GNUEmbeddedToolchain\LLVMEmbeddedToolchainForArm\bin\clang.exe  # 汇编器
OBJCOPY = D:\GNUEmbeddedToolchain\LLVMEmbeddedToolchainForArm\bin\llvm-objcopy
SIZE = D:\GNUEmbeddedToolchain\LLVMEmbeddedToolchainForArm\bin\llvm-size

# 启用LTO选项
LTO_FLAGS = -flto
# Compiler flags
CFLAGS = -target armv7em-none-eabi -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -std=gnu11 -Wall -Wextra -Ofast
CFLAGS += -fno-builtin -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fshort-enums -fomit-frame-pointer -c -fdata-sections -ffunction-sections
CFLAGS +=   \
-I$(CHIP_DIR)	\
-I$(CHIP_DIR)/inc \
-I$(CHIP_DIR)/core_support	\
-I$(GMSI_DIR)/utilities	\
-I$(GMSI_THREEPART_PLOOC) \
-I$(GMSI_DIR)

DFLAGS += -DUSE_STDPERIPH_DRIVER -DAT32F413C8T7 -DAT_START_F413_V1 -DCORTEX_M -D__NO_USE_LOG__

# Assembler flags
ASFLAGS = -target armv7em-none-eabi -mthumb

# Linker script 添加芯片的链接脚本路径
LDSCRIPT = AT32F413x8_FLASH.ld  

# Linker flags
LDFLAGS = -target armv7em-none-eabi -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
LDFLAGS += -Wl,--start-group -lc -Wl,--end-group
LDFLAGS += -Wl,--gc-sections -Wl,--print-memory-usage
LDFLAGS += -T $(LDSCRIPT)

# Source files
SOURCES =  \
main.c	\
at32f413_board.c	\
$(CHIP_DIR)/system_at32f413.c

SOURCES += $(foreach dir, $(CHIP_DIR)/src, $(wildcard $(dir)/*.c))
SOURCES += $(foreach dir, $(GMSI_DIR), $(wildcard $(dir)/*.c))
# 添加项目的源文件
ASSEMBLY =   \
$(CHIP_DIR)/startup_at32f413.s
# 添加汇编文件路径

# Object files
#OBJECTS = $(SOURCES:.c=.o) $(ASSEMBLY:.s=.o)  # 将汇编文件的.o文件添加到OBJECTS变量中
#OBJECTS := $(addprefix $(BUILD_DIR)/, $(SOURCES:.c=.o) $(SOURCES:.s=.o))
OBJECTS := $(SOURCES:.c=.o) $(ASSEMBLY:.s=.o) 
# Output file name
OUTPUT = $(TARGET).elf  # 定义输出文件名
HEX_OUTPUT = $(TARGET).hex
# Build rule
all: $(OUTPUT) $(HEX_OUTPUT) $(BUILD_DIR)

#$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
%.o: %.c
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@

#$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
%.o: %.s
	$(AS) $(ASFLAGS) $(DFLAGS) -c $< -o $@
	
$(OUTPUT): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) $(LTO_FLAGS) $(OBJECTS) -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(HEX_OUTPUT): $(OUTPUT)
	$(OBJCOPY) -O ihex $< $@

size: $(OUTPUT)
	$(SIZE) $(OUTPUT)

clean:
	rm -f $(OBJECTS) $(OUTPUT) $(BIN_OUTPUT) $(HEX_OUTPUT)

printf_value:
	@echo $(info show obj '$(OBJECTS)')