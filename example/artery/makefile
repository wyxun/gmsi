# Makefile for STM32G431RBT6 project with LLVM compiler
COMPILER_PATH = D:\01SoftwareInstall\LLVM\LLVM_EmbeddedToolchainForArm\bin
BUILD_DIR = build
CHIP_DIR = at32f413
GMSI_DIR := ../../gmsi
GMSI_THREEPART_PLOOC = ../../thirdParty/plooc
TARGET := follow

# Compiler
CC = $(COMPILER_PATH)\clang.exe
AS = $(COMPILER_PATH)\clang.exe 
OBJCOPY = $(COMPILER_PATH)\llvm-objcopy
SIZE = $(COMPILER_PATH)\llvm-size

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

ASFLAGS = -target armv7em-none-eabi -mthumb

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

ASSEMBLY = $(CHIP_DIR)/startup_at32f413.s

OBJECTS := $(addprefix $(BUILD_DIR)/, $(SOURCES:.c=.o) $(ASSEMBLY:.s=.o))
# Output file name
OUTPUT = $(BUILD_DIR)/$(TARGET).elf 
HEX_OUTPUT = $(BUILD_DIR)/$(TARGET).hex
BIN_OUTPUT = $(BUILD_DIR)/$(TARGET).bin

# Build rule
all: create_build_dir $(OUTPUT) $(HEX_OUTPUT) $(BIN_OUTPUT)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $(DFLAGS) -c $< -o $@

$(OUTPUT): $(OBJECTS)
	$(CC) $(LDFLAGS) $(LTO_FLAGS) $(OBJECTS) -o $@

$(HEX_OUTPUT): $(OUTPUT)
	$(OBJCOPY) -O ihex $< $@

$(BIN_OUTPUT): $(OUTPUT)	
	$(OBJCOPY) -O binary $< $@

create_build_dir:
	@mkdir -p $(BUILD_DIR) $(BUILD_DIR)/$(CHIP_DIR)/src $(BUILD_DIR)/$(GMSI_DIR)

size: $(OUTPUT)
	$(SIZE) $(OUTPUT)

clean:
	rm -rf $(BUILD_DIR) $(BUILD_DIR)/$(CHIP_DIR)/src $(BUILD_DIR)/$(GMSI_DIR)

printf_value:
	@echo $(info show obj '$(OBJECTS)')