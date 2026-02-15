# BLM Bootloader Makefile
# Supported chips: stm32g4, at32f4
# Usage: make CHIP=stm32g4   (default)
#        make CHIP=at32f4
# Toolchain: LLVM Embedded Toolchain for Arm (clang)

# Chip selection (default: stm32g4)
CHIP ?= at32f4

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

# Common source files (platform-independent)
C_SOURCES = \
    main.c \
    perfc_port_user.c \
    core/blm.c \
    core/blm_protocol.c \
    core/trace_stub.c \
    $(GMSI_DIR)/gmsi.c \
    $(GMSI_DIR)/gbase.c \
    $(GMSI_DIR)/gblinfo.c \
    $(GMSI_DIR)/gcoroutine.c \
    $(GMSI_DIR)/glog.c \
    $(GMSI_DIR)/gstorage.c \
    $(GMSI_DIR)/utilities/list.c \
    $(LIB_DIR)/perf_counter/perf_counter.c

# Assembly sources (perf_counter SysTick wrapper)
ASM_SOURCES = \
    $(LIB_DIR)/perf_counter/systick_wrapper_gcc.S

# Target flags (shared for all Cortex-M4F)
TARGET_TRIPLE = --target=armv7em-none-eabi
CPU_FLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# ============================================================
# Chip-specific configuration
# ============================================================
ifeq ($(CHIP),stm32g4)
    # --- STM32G4 ---
    CHIP_DEF      = -DSTM32G431xx
    CHIP_SOURCES  = port/stm32g4/blm_port_stm32g4.c \
                    port/stm32g4/startup_stm32g4.c
    LDSCRIPT      = port/stm32g4/linker.ld
    OPENOCD_TARGET = target/stm32g4x.cfg
    OPENOCD_INTERFACE = interface/cmsis-dap.cfg
else ifeq ($(CHIP),at32f4)
    # --- AT32F4 ---
    CHIP_DEF      = -DAT32F407xx
    CHIP_SOURCES  = port/at32f4/blm_port_at32f4.c \
                    port/at32f4/startup_at32f4.c
    LDSCRIPT      = port/at32f4/linker.ld
    OPENOCD_TARGET = target/at32f407xx.cfg
    OPENOCD_INTERFACE = interface/cmsis-dap.cfg
else
    $(error Unsupported CHIP=$(CHIP). Use: stm32g4, at32f4)
endif

# Append chip-specific sources
C_SOURCES += $(CHIP_SOURCES)

# Build directory (per chip)
BUILD_DIR = build/$(CHIP)

# Optimization
# Optimization
# Default to -Oz (Optimize for size)
OPT = -Oz

# Use -O0 (No optimization) for debug targets
ifneq ($(filter debug debug-server,$(MAKECMDGOALS)),)
    OPT = -O0
endif

# C definitions
C_DEFS = \
    $(CHIP_DEF) \
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
CFLAGS = $(TARGET_TRIPLE) $(CPU_FLAGS) $(C_DEFS) $(C_INCLUDES) $(OPT) -g
CFLAGS += -Wall -Wextra
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -fno-exceptions -fno-rtti
CFLAGS += -std=gnu11
CFLAGS += -Wno-compare-distinct-pointer-types -Wno-unused-variable -Wno-sign-compare -Wno-unused-command-line-argument

# Assembler flags
ASFLAGS = $(TARGET_TRIPLE) $(CPU_FLAGS) -g

# Linker flags
LDFLAGS = $(TARGET_TRIPLE) $(CPU_FLAGS)
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref
LDFLAGS += -Wl,--wrap=SysTick_Handler
LDFLAGS += -nostartfiles
LDFLAGS += -lcrt0 -lc -lm -lsemihost

# Build targets
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin size

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.S | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(CP) -O ihex $< $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(CP) -O binary $< $@

$(BUILD_DIR):
	mkdir -p $@

size: $(BUILD_DIR)/$(TARGET).elf
	$(SZ) $<

clean:
	rm -rf build

# Check size target
check-size: $(BUILD_DIR)/$(TARGET).elf
	@echo "Checking bootloader size (must be < 16KB)..."
	@$(SZ) $< | awk 'NR==2 {if ($$1 > 16384) {print "ERROR: Code size exceeds 16KB!"; exit 1} else {print "OK: Code size is " $$1 " bytes"}}'

# OpenOCD
# Use "cmsis_dap_backend hid" for WSL2 compatibility (bulk interface blocked by cdc_acm)
OPENOCD_CMD = openocd -f $(OPENOCD_INTERFACE) -c "cmsis_dap_backend hid" -f $(OPENOCD_TARGET)

flash: $(BUILD_DIR)/$(TARGET).hex
	$(OPENOCD_CMD) -c "program $< verify reset exit"

debug-server:
	$(OPENOCD_CMD)

debug: $(BUILD_DIR)/$(TARGET).elf
	gdb-multiarch -x debug.gdb $<

# Print current config
info:
	@echo "CHIP     = $(CHIP)"
	@echo "BUILD    = $(BUILD_DIR)"
	@echo "LDSCRIPT = $(LDSCRIPT)"
	@echo "SOURCES  = $(CHIP_SOURCES)"

.PHONY: all clean flash debug-server size check-size info
