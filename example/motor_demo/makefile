#COMPILER_PATH = D:\01SoftwareInstall\LLVM\LLVM_EmbeddedToolchainForArm\bin
COMPILER_PATH = ~/llvm_for_arm/bin/
BUILD_DIR = build
CHIP_DIR = at32f413
GMSI_DIR := ../../gmsi
GMSI_UTL_DIR := ../../gmsi/utilities
GMSI_THREEPART_PLOOC = ../../lib/plooc
GMSI_THREEPART_PERF = ../../lib/perf_counter
USER_DIR = ./user
HW_DIR = ./mchw
MC_DIR = ./mclib
# MC library - prefer .a (Unix archive) for cross-linking; fall back to .lib if only that exists
# look for libmc_foc_kernal.a first, then mc_foc_kernal.lib
MC_LIB_A := $(wildcard $(MC_DIR)/src/libmc_foc_kernal.a)
MC_LIB_LIB := $(wildcard $(MC_DIR)/src/mc_foc_kernal.lib)
ifneq ($(MC_LIB_A),)
MC_LIB := $(MC_LIB_A)
else ifneq ($(MC_LIB_LIB),)
MC_LIB := $(MC_LIB_LIB)
else
MC_LIB :=
endif

MC_LIB_PATH := $(MC_DIR)/src
# Target executable name
TARGET := motor

# Compiler
CC = $(COMPILER_PATH)/clang
AS = $(COMPILER_PATH)/clang
OBJCOPY = $(COMPILER_PATH)/llvm-objcopy
SIZE = $(COMPILER_PATH)/llvm-size

# Path to OpenOCD executable (can be overridden on make command line)
# Example: make OPENOCD=/home/xun/usr/home/tools/openocd/bin/openocd flash
OPENOCD ?= $(HOME)/usr/home/tools/OpenOCD/bin/openocd

LTO_FLAGS = -flto
# Compiler flags
CFLAGS = -target armv7em-none-eabi -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -std=gnu11 -Wall -Wextra -O0
CFLAGS += -fno-builtin -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fshort-enums -fomit-frame-pointer -c -fdata-sections -ffunction-sections
CFLAGS +=   \
-I$(CHIP_DIR)	\
-I$(CHIP_DIR)/inc \
-I$(HW_DIR)	\
-I$(USER_DIR)	\
-I$(MC_DIR)/inc	\
-I$(CHIP_DIR)/core_support	\
-I$(GMSI_UTL_DIR)	\
-I$(GMSI_THREEPART_PLOOC) \
-I$(GMSI_THREEPART_PERF) \
-I$(GMSI_DIR)

DFLAGS += -DUSE_STDPERIPH_DRIVER -DAT32F413C8T7 -DAT_START_F413_V1 -DCORTEX_M -D__NO_USE_LOG__
ASFLAGS = -target armv7em-none-eabi -mthumb

#LDSCRIPT = AT32F413x8_FLASH.ld  
LDSCRIPT = AT32F413xC_FLASH.ld

# Linker flags
LDFLAGS = -target armv7em-none-eabi -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
LDFLAGS += -Wl,--start-group -lc -Wl,--end-group
LDFLAGS += -Wl,--gc-sections -Wl,--print-memory-usage
LDFLAGS += -T $(LDSCRIPT)

# Source files
#SRC_MAIN = main.c at32f413_board.c $(CHIP_DIR)/system_at32f413.c
SRC_MAIN = $(wildcard $(USER_DIR)/*.c)
SRC_MAIN += $(wildcard $(HW_DIR)/*.c)
# Collect chip C sources reliably (top-level and src/ subdir)
# Avoid using shell find on Windows; use wildcard to list files
SRC_CHIP = $(wildcard $(CHIP_DIR)/*.c) $(wildcard $(CHIP_DIR)/src/*.c)
SRC_GMSI = $(wildcard $(GMSI_DIR)/*.c)
SRC_GMSI_UTL = $(wildcard $(GMSI_UTL_DIR)/*.c)
SRC_ASM = $(CHIP_DIR)/startup_at32f413.s

SRC_MC = $(MC_DIR)/src/mc_delay.c $(MC_DIR)/src/mc_foc.c \
	$(MC_DIR)/src/mc_curr_fdbk.c $(MC_DIR)/src/mc_encoder.c \
	$(MC_DIR)/src/mc_pid_controller.c $(MC_DIR)/src/mc_foc_globals.c \
	$(MC_DIR)/src/mc_math.c $(MC_DIR)/src/mc_field_weakening.c \
	$(MC_DIR)/src/mc_hall.c $(MC_DIR)/src/mc_foc_sensorless.c \
	$(MC_DIR)/src/mc_comm_uart.c $(MC_DIR)/src/user_interface_foc.c \
	$(MC_DIR)/src/motor_control_foc.c \
	$(MC_DIR)/src/mc_flash_data_table_foc.c

# 生成 build 下的子目录结构，防止同名 .o 冲突
OBJ_MAIN = $(addprefix $(BUILD_DIR)/, $(SRC_MAIN:.c=.o))
# Map any chip source path (may include subdirs like src/) to build/<path>.o
OBJ_CHIP = $(patsubst %,$(BUILD_DIR)/%,$(SRC_CHIP:.c=.o))
OBJ_GMSI = $(patsubst $(GMSI_DIR)/%.c,$(BUILD_DIR)/gmsi/%.o,$(SRC_GMSI))
OBJ_GMSI_UTL = $(patsubst $(GMSI_UTL_DIR)/%.c,$(BUILD_DIR)/gmsi_utilities/%.o,$(SRC_GMSI_UTL))
OBJ_MC = $(patsubst $(MC_DIR)/src/%.c,$(BUILD_DIR)/mclib/%.o,$(SRC_MC))
OBJ_ASM = $(BUILD_DIR)/$(CHIP_DIR)/startup_at32f413.o

OBJECTS = $(OBJ_MAIN) $(OBJ_CHIP) $(OBJ_GMSI) $(OBJ_GMSI_UTL) $(OBJ_MC) $(OBJ_ASM)

# Output file name
OUTPUT = $(BUILD_DIR)/$(TARGET).elf 
HEX_OUTPUT = $(BUILD_DIR)/$(TARGET).hex
BIN_OUTPUT = $(BUILD_DIR)/$(TARGET).bin

# Build rule
all: create_build_dir $(OUTPUT) $(HEX_OUTPUT) $(BIN_OUTPUT)

# gmsi 目录下的 .c
$(BUILD_DIR)/gmsi/%.o: $(GMSI_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@
# gmsi/utilities 目录下的 .c
$(BUILD_DIR)/gmsi_utilities/%.o: $(GMSI_UTL_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@
# mclib/src 目录下的 .c
$(BUILD_DIR)/mclib/%.o: $(MC_DIR)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@
$(BUILD_DIR)/perf_counter/%.o: $(GMSI_THREEPART_PERF)/%.c
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

# 通用 C 文件编译规则（最后）：自动创建 build 下的子目录
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@

# 汇编
$(BUILD_DIR)/$(CHIP_DIR)/%.o: $(CHIP_DIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $(DFLAGS) -c $< -o $@

$(OUTPUT): $(OBJECTS)
	# link objects first, then libraries (order matters)
	$(CC) $(LDFLAGS) $(LTO_FLAGS) $(OBJECTS) $(MC_LIB) -o $@

$(HEX_OUTPUT): $(OUTPUT)
	$(OBJCOPY) -O ihex $< $@

$(BIN_OUTPUT): $(OUTPUT)	
	$(OBJCOPY) -O binary $< $@

create_build_dir:
	@mkdir -p $(BUILD_DIR)

size: $(OUTPUT)
	$(SIZE) $(OUTPUT)

clean:
	@echo Cleaning build artifacts...
	@rm -rf $(BUILD_DIR)

.PHONY: all create_build_dir clean size printf_value

printf_value:
	@echo $(info show obj '$(OBJECTS)')

# 使用 openocd + DAP 下载器烧录程序
flash:
	@echo Flashing via OpenOCD...
	@if [ ! -x "$(OPENOCD)" ]; then \
		echo "OpenOCD not found or not executable at '$(OPENOCD)'."; \
		echo "Set OPENOCD=path/to/openocd or install OpenOCD there."; \
		exit 1; \
	fi
	@"$(OPENOCD)" -f interface/cmsis-dap.cfg -f target/at32f413xx.cfg -c "program $(OUTPUT) verify reset exit"