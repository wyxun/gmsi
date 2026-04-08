# BLM Bootloader Makefile
# Supported chips: stm32g4, at32f4
# Usage: make CHIP=stm32g4   (default)
#        make CHIP=at32f4
# Toolchain: LLVM Embedded Toolchain for Arm (clang)

# Chip selection (default: stm32g4)
CHIP ?= at32f4

# OS sensing
ifeq ($(OS),Windows_NT)
    SHELL = cmd.exe
    SW_ROOT ?= D:/software
    LLVM_PATH = $(SW_ROOT)/llvm_for_arm/bin/
    MKDIR = if not exist $(subst /,\,$(BUILD_DIR)) mkdir $(subst /,\,$(BUILD_DIR))
    RMDIR = if exist build rd /s /q build
    # Tools
    OPENOCD_BIN     = $(SW_ROOT)/msys64/mingw64/bin/openocd.exe
    OPENOCD_SCRIPTS = $(SW_ROOT)/msys64/mingw64/share/openocd/scripts
    OPENOCD_CMD = $(OPENOCD_BIN) -s $(OPENOCD_SCRIPTS) \
                  -f $(OPENOCD_INTERFACE) -c "cmsis_dap_backend hid" \
                  -f $(OPENOCD_TARGET)
    NM = $(LLVM_PATH)llvm-nm.exe
else
    # Linux/WSL
    LLVM_PATH = ~/llvm_for_arm/bin/
    MKDIR = mkdir -p $(BUILD_DIR)
    RMDIR = rm -rf build
    OPENOCD_CMD = openocd -f $(OPENOCD_INTERFACE) \
                  -c "cmsis_dap_backend hid" -f $(OPENOCD_TARGET)
    NM = $(LLVM_PATH)llvm-nm
endif

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
    $(GMSI_DIR)/utilities/trace.c \
    $(GMSI_DIR)/utilities/trace_fmt.c \
    $(GMSI_DIR)/gmsi.c \
    $(GMSI_DIR)/gbase.c \
    $(GMSI_DIR)/gblinfo.c \
    $(GMSI_DIR)/gcoroutine.c \
    $(GMSI_DIR)/glog.c \
    $(GMSI_DIR)/gstorage.c \
    $(GMSI_DIR)/utilities/list.c \
    $(GMSI_DIR)/utilities/util_queue.c \
    $(GMSI_DIR)/utilities/util_debug.c \
    $(GMSI_DIR)/utilities/segger_rtt/SEGGER_RTT.c \
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
    -D__COMPILER_HAS_GNU_EXTENSIONS__=1 \
    -DTRACE_USE_LIBC_PRINTF=0 \
    '-DTRACE_MCU_WRITE_STRING(s)=SEGGER_RTT_WriteString(0,(s))' \
    '-DTRACE_MCU_WRITE_BUFFER(p,l)=SEGGER_RTT_Write(0,(p),(l))'

# Include paths
C_INCLUDES = \
    -I. \
    -I./core \
    -I./port \
    -I./cmsis \
    -I$(GMSI_DIR) \
    -I$(GMSI_DIR)/gdi \
    -I$(LIB_DIR)/perf_counter \
    -I$(LIB_DIR)/plooc

# Compiler flags
CFLAGS = $(TARGET_TRIPLE) $(CPU_FLAGS) $(C_DEFS) $(C_INCLUDES) $(OPT) -g
CFLAGS += -Wall -Wextra
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -fno-exceptions -fno-rtti
CFLAGS += -std=gnu11
CFLAGS += -Wno-compare-distinct-pointer-types -Wno-unused-variable -Wno-sign-compare -Wno-unused-command-line-argument -Wno-implicit-function-declaration

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
	$(MKDIR)

size: $(BUILD_DIR)/$(TARGET).elf
	$(SZ) $<

clean:
	$(RMDIR)

# Check size target
check-size: $(BUILD_DIR)/$(TARGET).elf
	@echo "Checking bootloader size (must be < 16KB)..."
	@$(SZ) $< | awk 'NR==2 {if ($$1 > 16384) {print "ERROR: Code size exceeds 16KB!"; exit 1} else {print "OK: Code size is " $$1 " bytes"}}'

flash: $(BUILD_DIR)/$(TARGET).hex
	$(OPENOCD_CMD) -c "program $< verify reset exit"

# Flash via already-running OpenOCD (e.g. RTT session), then restart RTT
flash-rtt: $(BUILD_DIR)/$(TARGET).hex
ifeq ($(OS),Windows_NT)
	@powershell -NoProfile -Command \
	    "$$hex = (Resolve-Path '$<').Path; \
	     $$c = New-Object System.Net.Sockets.TcpClient('localhost', 4444); \
	     $$s = $$c.GetStream(); \
	     $$w = New-Object System.IO.StreamWriter($$s); \
	     $$w.WriteLine(\"program $$hex verify reset\"); $$w.WriteLine('exit'); \
	     $$w.Flush(); Start-Sleep 2; $$c.Close()"
else
	@ABS=$$(realpath $<); \
	printf "program $$ABS verify reset\nexit\n" | nc -w 10 localhost 4444; \
	sleep 1; \
	printf "rtt stop\nrtt start\n" | nc -w 3 localhost 4444
endif

debug-server:
	$(OPENOCD_CMD)

debug: $(BUILD_DIR)/$(TARGET).elf
	gdb-multiarch -x debug.gdb $<

# RTT address from nm
ifeq ($(OS),Windows_NT)
    RTT_ADDR = $(shell powershell -NoProfile -Command "$$out = & '$(NM)' $(BUILD_DIR)/$(TARGET).elf 2>$$null | Select-String '_SEGGER_RTT$$' | Select-Object -First 1; if ($$out.Line -match '^([0-9a-fA-F]+)') { '0x' + $$matches[1] }")
else
    RTT_ADDR = $(shell $(NM) $(BUILD_DIR)/$(TARGET).elf 2>/dev/null | awk '/_SEGGER_RTT$$/ {print "0x"$$1}')
endif

# Print RTT address
rtt-addr: $(BUILD_DIR)/$(TARGET).elf
	@echo "RTT CB address: $(RTT_ADDR)"

# Start OpenOCD with RTT server on port 9090
rtt: $(BUILD_DIR)/$(TARGET).elf
	@echo "RTT CB address: $(RTT_ADDR)"
	$(OPENOCD_CMD) -c "init" -c 'rtt setup $(RTT_ADDR) 0xa8 "SEGGER RTT"' -c "rtt start" -c "rtt server start 9090 0"

# Print current config
info:
	@echo "CHIP     = $(CHIP)"
	@echo "BUILD    = $(BUILD_DIR)"
	@echo "LDSCRIPT = $(LDSCRIPT)"
	@echo "SOURCES  = $(CHIP_SOURCES)"

.PHONY: all clean flash flash-rtt debug-server size check-size info rtt rtt-addr

