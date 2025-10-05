# Makefile for STM32G431RBT6 project with LLVM compiler
#COMPILER_PATH = D:\01SoftwareInstall\LLVM\LLVM_EmbeddedToolchainForArm\bin
COMPILER_PATH = ~/llvm_for_arm/bin/
BUILD_DIR = build
CHIP_DIR = at32f421
GMSI_OBJECT_DIR = object
GMSI_DIR := ../../gmsi
GMSI_UTL_DIR := ../../gmsi/utilities
GMSI_THREEPART_PLOOC = ../../lib/plooc
GMSI_THREEPART_PERF_COUNTER = ../../lib/perf_counter
TARGET := 421demo

# Compiler
CC = $(COMPILER_PATH)/clang
AS = $(COMPILER_PATH)/clang
OBJCOPY = $(COMPILER_PATH)/llvm-objcopy
SIZE = $(COMPILER_PATH)/llvm-size

LTO_FLAGS = -flto
# Compiler flags
CFLAGS = -target armv7em-none-eabi -mthumb -mfloat-abi=soft
CFLAGS += -std=gnu11 -Wall -Wextra -Os
CFLAGS += -fno-builtin -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fshort-enums -fomit-frame-pointer -c -fdata-sections -ffunction-sections
CFLAGS += \
-I$(CHIP_DIR)	\
-I$(CHIP_DIR)/inc \
-I$(CHIP_DIR)/core_support \
-I$(GMSI_UTL_DIR)	\
-I$(GMSI_THREEPART_PLOOC) \
-I$(GMSI_THREEPART_PERF_COUNTER) \
-I$(GMSI_OBJECT_DIR) \
-I$(GMSI_DIR) \
-Iuser

DFLAGS += -DUSE_STDPERIPH_DRIVER -DAT32F421C8T7 -DAT_START_F421_V1 -DCORTEX_M -D__NO_USE_LOG__

ASFLAGS = -target armv7em-none-eabi -mthumb

LDSCRIPT = $(CHIP_DIR)/AT32F421x8_FLASH.ld

# Linker flags
LDFLAGS = -target armv7em-none-eabi -mthumb -mfloat-abi=soft
LDFLAGS += -Wl,--start-group -lc -Wl,--end-group
LDFLAGS += -Wl,--gc-sections -Wl,--print-memory-usage
LDFLAGS += -T $(LDSCRIPT)

# Source files
SRC_MAIN = main.c at32f421_board.c $(CHIP_DIR)/system_at32f421.c
SRC_CHIP = $(filter-out $(CHIP_DIR)/system_at32f421.c $(CHIP_DIR)/startup_at32f421.s, $(shell find $(CHIP_DIR) -name '*.c'))
SRC_GMSI = $(wildcard $(GMSI_DIR)/*.c)
SRC_GMSI_UTL = $(wildcard $(GMSI_UTL_DIR)/*.c)
SRC_PERF_COUNTER_C = $(wildcard $(GMSI_THREEPART_PERF_COUNTER)/*.c)
SRC_PERF_COUNTER_S = $(wildcard $(GMSI_THREEPART_PERF_COUNTER)/*.s)
SRC_ASM = $(CHIP_DIR)/startup_at32f421.s
SRC_USER = $(shell find user -name '*.c')
SRC_OBJECT_C = $(shell find $(GMSI_OBJECT_DIR) -name '*.c')

# 生成 build 下的子目录结构，防止同名 .o 冲突
OBJ_MAIN = $(addprefix $(BUILD_DIR)/, $(SRC_MAIN:.c=.o))
OBJ_CHIP = $(patsubst $(CHIP_DIR)/%.c,$(BUILD_DIR)/$(CHIP_DIR)/%.o,$(SRC_CHIP))
OBJ_GMSI = $(patsubst $(GMSI_DIR)/%.c,$(BUILD_DIR)/gmsi/%.o,$(SRC_GMSI))
OBJ_GMSI_UTL = $(patsubst $(GMSI_UTL_DIR)/%.c,$(BUILD_DIR)/gmsi_utilities/%.o,$(SRC_GMSI_UTL))
OBJ_PERF_COUNTER_C = $(patsubst $(GMSI_THREEPART_PERF_COUNTER)/%.c,$(BUILD_DIR)/perf_counter/%.o,$(SRC_PERF_COUNTER_C))
OBJ_PERF_COUNTER_S = $(patsubst $(GMSI_THREEPART_PERF_COUNTER)/%.s,$(BUILD_DIR)/perf_counter/%.o,$(SRC_PERF_COUNTER_S))
OBJ_ASM = $(BUILD_DIR)/$(CHIP_DIR)/startup_at32f421.o
OBJ_USER = $(patsubst user/%.c,$(BUILD_DIR)/user/%.o,$(SRC_USER))
OBJ_OBJECT_C = $(patsubst $(GMSI_OBJECT_DIR)/%.c,$(BUILD_DIR)/object/%.o,$(SRC_OBJECT_C))

OBJECTS = $(OBJ_MAIN) $(OBJ_CHIP) $(OBJ_GMSI) $(OBJ_GMSI_UTL) $(OBJ_USER) $(OBJ_PERF_COUNTER_C) $(OBJ_PERF_COUNTER_S) $(OBJ_OBJECT_C) $(OBJ_ASM)

# Output file name
OUTPUT = $(BUILD_DIR)/$(TARGET).elf 
HEX_OUTPUT = $(BUILD_DIR)/$(TARGET).hex
BIN_OUTPUT = $(BUILD_DIR)/$(TARGET).bin

# Build rule
all: create_build_dir $(OUTPUT) $(HEX_OUTPUT) $(BIN_OUTPUT)

# 通用 C 文件编译规则，自动创建 build 下的子目录
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@
# gmsi 目录下的 .c
$(BUILD_DIR)/gmsi/%.o: $(GMSI_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@
# gmsi/utilities 目录下的 .c
$(BUILD_DIR)/gmsi_utilities/%.o: $(GMSI_UTL_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@
# user 目录下的 .c
$(BUILD_DIR)/user/%.o: user/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@
# perf_counter 目录下的 .c
$(BUILD_DIR)/perf_counter/%.o: $(GMSI_THREEPART_PERF_COUNTER)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@
# perf_counter 目录下的 .s
$(BUILD_DIR)/perf_counter/%.o: $(GMSI_THREEPART_PERF_COUNTER)/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $(DFLAGS) -c $< -o $@
# 汇编
$(BUILD_DIR)/$(CHIP_DIR)/%.o: $(CHIP_DIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $(DFLAGS) -c $< -o $@
# object 目录下的 .c
$(BUILD_DIR)/object/%.o: $(GMSI_OBJECT_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@

$(OUTPUT): $(OBJECTS)
	$(CC) $(LDFLAGS) $(LTO_FLAGS) $(OBJECTS) -o $@

$(HEX_OUTPUT): $(OUTPUT)
	$(OBJCOPY) -O ihex $< $@

$(BIN_OUTPUT): $(OUTPUT)	
	$(OBJCOPY) -O binary $< $@

create_build_dir:
	@mkdir -p $(BUILD_DIR)

size: $(OUTPUT)
	$(SIZE) $(OUTPUT)

clean:
	rm -rf $(BUILD_DIR) $(BUILD_DIR)/$(CHIP_DIR)/src $(BUILD_DIR)/$(GMSI_DIR)

printf_value:
	@echo $(info show obj '$(OBJECTS)')

# 使用 openocd + DAP 下载器烧录程序
flash:
	openocd -f interface/cmsis-dap.cfg -f target/stm32f4x.cfg -c "program $(BIN_OUTPUT) verify reset exit"