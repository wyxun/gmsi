# BLM Bootloader Makefile
# Target: STM32G431 (Cortex-M4)
# Toolchain: LLVM Embedded Toolchain for Arm (clang)

# Toolchain path
LLVM_PATH = ~/llvm_for_arm/bin/

# Toolchain commands
CC = $(LLVM_PATH)clang
AS = $(LLVM_PATH)clang
CP = $(LLVM_PATH)llvm-objcopy
SZ = $(LLVM_PATH)llvm-size
AR = $(LLVM_PATH)llvm-ar
LD = $(LLVM_PATH)clang

# Project name
TARGET = blm

# Directories
GMSI_DIR = ../../gmsi
LIB_DIR = ../../lib

# Source files
C_SOURCES = \
    main.c \
    startup_stm32g431xx.c \
    perfc_port_user.c \
    core/blm.c \
    core/blm_protocol.c \
    core/trace_stub.c \
    port/blm_port_stm32g431.c \
    $(GMSI_DIR)/gmsi.c \
    $(GMSI_DIR)/gbase.c \
    $(GMSI_DIR)/gblinfo.c \
    $(GMSI_DIR)/gcoroutine.c \
    $(GMSI_DIR)/glog.c \
    $(GMSI_DIR)/gstorage.c \
    $(GMSI_DIR)/utilities/list.c \
    $(LIB_DIR)/perf_counter/perf_counter.c

# ASM sources (startup file - user must provide)
ASM_SOURCES = \
    # startup_stm32g431xx.s

# Build directory
BUILD_DIR = build

# Target: armv7m-none-eabi for Cortex-M4 (LLVM Embedded Toolchain for Arm)
TARGET_TRIPLE = --target=armv7em-none-eabi

# CPU flags for Cortex-M4 with FPU
CPU_FLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# Optimization
OPT = -Oz

# C definitions
C_DEFS = \
    -DSTM32G431xx \
    -D__PERFC_USE_USER_CUSTOM_PORTING__=1 \
    -D__C_LANGUAGE_EXTENSIONS_PERFC_PT__=1 \
    -D__PERFC_CFG_PORTING_INCLUDE__=\"perfc_port_user.h\" \
    -D__COMPILER_HAS_GNU_EXTENSIONS__=1

# Include paths
C_INCLUDES = \
    -I. \
    -I./core \
    -I./port \
    -I./cmsis \
    -I$(GMSI_DIR) \
    -I$(LIB_DIR)/perf_counter \
    -I$(LIB_DIR)/plooc

# Compiler flags
CFLAGS = $(TARGET_TRIPLE) $(CPU_FLAGS) $(C_DEFS) $(C_INCLUDES) $(OPT)
CFLAGS += -Wall -Wextra
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -fno-exceptions -fno-rtti
CFLAGS += -std=gnu11
CFLAGS += -Wno-compare-distinct-pointer-types -Wno-unused-variable -Wno-sign-compare -Wno-unused-command-line-argument

# Linker script
LDSCRIPT = linker.ld

# Linker flags (using LLVM Embedded Toolchain libraries)
LDFLAGS = $(TARGET_TRIPLE) $(CPU_FLAGS)
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref
LDFLAGS += -nostartfiles
LDFLAGS += -lcrt0 -lc -lm -lsemihost

# Build targets
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin size

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(CP) -O binary $< $@

$(BUILD_DIR):
	mkdir -p $@

size: $(BUILD_DIR)/$(TARGET).elf
	$(SZ) $<

clean:
	rm -rf $(BUILD_DIR)

# Check size target
check-size: $(BUILD_DIR)/$(TARGET).elf
	@echo "Checking bootloader size (must be < 8KB)..."
	@$(SZ) $< | awk 'NR==2 {if ($$1 > 8192) {print "ERROR: Code size exceeds 8KB!"; exit 1} else {print "OK: Code size is " $$1 " bytes"}}'

# OpenOCD Configuration
# PWLink usually emulates CMSIS-DAP. If not, change INTERFACE to specific config.
OPENOCD_INTERFACE ?= interface/cmsis-dap.cfg
OPENOCD_TARGET    ?= target/stm32g4x.cfg
OPENOCD_CMD        = openocd -f $(OPENOCD_INTERFACE) -f $(OPENOCD_TARGET)

# Flash the device
flash: $(BUILD_DIR)/$(TARGET).elf
	$(OPENOCD_CMD) -c "program $< verify reset exit"

# Start OpenOCD debug server
debug-server:
	$(OPENOCD_CMD)

# Phony targets
.PHONY: all clean flash debug-server size check-size
