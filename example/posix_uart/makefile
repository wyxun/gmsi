GMSI_DIR := ../../gmsi
GMSI_UTL_DIR := ../../gmsi/utilities
GMSI_THREEPART_PLOOC = ../../ThirdParty/plooc
BUILD_DIR := build
TARGET := $(BUILD_DIR)/demo

CC := clang
CXX := clang++

DEFS := -D_XOPEN_SOURCE=700 -DLINUX_POSIX
# -lrt:timer
LDFLAGS := -fno-autolink -Wall -lrt
CFLAGS := -std=c11 -W -Ofast -Wno-implicit-function-declaration -Wno-unused-parameter -Wno-int-to-pointer-cast -Wno-sign-compare
CFLAGS += -Wno-compare-distinct-pointer-types
CFLAGS += -I$(GMSI_DIR) -I/usr/include \
-I$(GMSI_UTL_DIR) \
-I$(GMSI_THREEPART_PLOOC)

SOURCES += $(foreach dir, $(GMSI_DIR), $(wildcard $(dir)/*.c))
SOURCES += $(foreach dir, $(GMSI_UTL_DIR), $(wildcard $(dir)/*.c))
SOURCES += main.c pc_uart.c pc_clock.c

OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SOURCES)))

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@
$(BUILD_DIR)/%.o: $(GMSI_DIR)/%.c
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@
$(BUILD_DIR)/%.o: $(GMSI_UTL_DIR)/%.c
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

printf_value:
	@echo $(info HFILE files is '$(HFILE)')