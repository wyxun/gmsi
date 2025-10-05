GMSI_DIR := ../../gmsi
GMSI_UTL_DIR := ../../gmsi/utilities
GMSI_THREEPART_PLOOC = ../../lib/plooc
BUILD_DIR := build
TARGET := $(BUILD_DIR)/example

CC := clang
CXX := clang++

DEFS := -D_XOPEN_SOURCE=700 -DLINUX_POSIX
# -lrt:timer
LDFLAGS := -fno-autolink -Wall -lrt
CFLAGS := -std=c11 -W -Ofast -Wno-implicit-function-declaration -Wno-unused-parameter -Wno-int-to-pointer-cast -Wno-sign-compare
CFLAGS += -Wno-compare-distinct-pointer-types

# 支持 debug 模式
DEBUG ?= 0

ifeq ($(DEBUG),1)
CFLAGS := -std=c11 -W -g -O0 -DDEBUG -Wno-implicit-function-declaration -Wno-unused-parameter -Wno-int-to-pointer-cast -Wno-sign-compare
CFLAGS += -Wno-compare-distinct-pointer-types
LDFLAGS := -fno-autolink -Wall -lrt
TARGET := $(BUILD_DIR)/example_debug
endif

CFLAGS += -I$(GMSI_DIR) -I/usr/include \
-I$(GMSI_UTL_DIR) \
-I$(GMSI_THREEPART_PLOOC)

SOURCES += $(foreach dir, $(GMSI_DIR), $(wildcard $(dir)/*.c))
SOURCES += $(foreach dir, $(GMSI_UTL_DIR), $(wildcard $(dir)/*.c))
SOURCES += main.c example.c template.c

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