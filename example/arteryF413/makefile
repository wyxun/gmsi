# Makefile for STM32G431RBT6 project with LLVM compiler
#COMPILER_PATH = D:\01SoftwareInstall\LLVM\LLVM_EmbeddedToolchainForArm\bin
COMPILER_PATH = ~/llvm_for_arm/bin/
BUILD_DIR = build
CHIP_DIR = at32f413
GMSI_DIR := ../../gmsi
GMSI_UTL_DIR := ../../gmsi/utilities
GMSI_THREEPART_PLOOC = ../../ThirdParty/plooc
TARGET := follow

# Compiler
CC = $(COMPILER_PATH)/clang
AS = $(COMPILER_PATH)/clang
OBJCOPY = $(COMPILER_PATH)/llvm-objcopy
SIZE = $(COMPILER_PATH)/llvm-size

LTO_FLAGS = -flto
# Compiler flags
CFLAGS = -target armv7em-none-eabi -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -std=gnu11 -Wall -Wextra -O0
CFLAGS += -fno-builtin -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fshort-enums -fomit-frame-pointer -c -fdata-sections -ffunction-sections
CFLAGS +=   \
-I$(CHIP_DIR)	\
-I$(CHIP_DIR)/inc \
-I$(CHIP_DIR)/core_support	\
-I$(GMSI_UTL_DIR)	\
-I$(GMSI_THREEPART_PLOOC) \
-I$(GMSI_DIR)

DFLAGS += -DUSE_STDPERIPH_DRIVER -DAT32F413C8T7 -DAT_START_F413_V1 -DCORTEX_M -D__NO_USE_LOG__

ASFLAGS = -target armv7em-none-eabi -mthumb

LDSCRIPT = AT32F413x8_FLASH.ld  

# Linker flags
LDFLAGS = -target armv7em-none-eabi -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
LDFLAGS += -Wl,--start-group -lc -Wl,--end-group
LDFLAGS += -Wl,--gc-sections -Wl,--print-memory-usage
LDFLAGS += -T $(LDSCRIPT)

# Source files
SRC_MAIN = main.c at32f413_board.c $(CHIP_DIR)/system_at32f413.c
SRC_CHIP = $(filter-out $(CHIP_DIR)/system_at32f413.c $(CHIP_DIR)/startup_at32f413.s, $(shell find $(CHIP_DIR) -name '*.c'))
SRC_GMSI = $(wildcard $(GMSI_DIR)/*.c)
SRC_GMSI_UTL = $(wildcard $(GMSI_UTL_DIR)/*.c)
SRC_ASM = $(CHIP_DIR)/startup_at32f413.s

# 生成 build 下的子目录结构，防止同名 .o 冲突
OBJ_MAIN = $(addprefix $(BUILD_DIR)/, $(SRC_MAIN:.c=.o))
OBJ_CHIP = $(patsubst $(CHIP_DIR)/%.c,$(BUILD_DIR)/$(CHIP_DIR)/%.o,$(SRC_CHIP))
OBJ_GMSI = $(patsubst $(GMSI_DIR)/%.c,$(BUILD_DIR)/gmsi/%.o,$(SRC_GMSI))
OBJ_GMSI_UTL = $(patsubst $(GMSI_UTL_DIR)/%.c,$(BUILD_DIR)/gmsi_utilities/%.o,$(SRC_GMSI_UTL))
OBJ_ASM = $(BUILD_DIR)/$(CHIP_DIR)/startup_at32f413.o

OBJECTS = $(OBJ_MAIN) $(OBJ_CHIP) $(OBJ_GMSI) $(OBJ_GMSI_UTL) $(OBJ_ASM)

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
# 汇编
$(BUILD_DIR)/$(CHIP_DIR)/%.o: $(CHIP_DIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $(DFLAGS) -c $< -o $@

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