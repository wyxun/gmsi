GMSI_DIR := ../../gmsi
GMSI_UTL_DIR := ../../gmsi/utilities
GMSI_THREEPART_PLOOC = ../../lib/plooc
GMSI_THREEPART_PERF = ../../lib/perf_counter
BUILD_DIR := build
TARGET := $(BUILD_DIR)/example

CC := clang
CXX := clang++

DEFS := -D_XOPEN_SOURCE=700 -DLINUX_POSIX
# -lrt:timer
# Link with rt and pthread (pthread provides pthread_sigmask)
LDFLAGS := -fno-autolink -Wall -lrt -lpthread
CFLAGS := -std=gnu11 -W -Ofast -Wno-implicit-function-declaration -Wno-unused-parameter -Wno-int-to-pointer-cast -Wno-sign-compare
CFLAGS += -Wno-compare-distinct-pointer-types
CFLAGS += -pthread
# 支持 debug 模式
DEBUG ?= 0

ifeq ($(DEBUG),1)
CFLAGS := -std=gnu11 -W -g -O0 -DDEBUG -Wno-implicit-function-declaration -Wno-unused-parameter -Wno-int-to-pointer-cast -Wno-sign-compare
CFLAGS += -Wno-compare-distinct-pointer-types
CFLAGS += -pthread
LDFLAGS := -fno-autolink -Wall -lrt -lpthread
TARGET := $(BUILD_DIR)/example_debug
endif

CFLAGS += -I$(GMSI_DIR) -I/usr/include \
-I$(GMSI_DIR)/gdi \
-I$(GMSI_UTL_DIR) \
-I$(GMSI_THREEPART_PERF) \
-I$(GMSI_THREEPART_PLOOC)\
-I.

CFLAGS += -D__PERFC_USE_PORTING__=1  
CFLAGS += -D__PERFC_USE_USER_CUSTOM_PORTING__=1  
CFLAGS += -D__PERFC_CFG_DISABLE_DEFAULT_SYSTICK_PORTING__=1  
CFLAGS += -D__PERFC_CFG_PORTING_INCLUDE__=\"perfc_port_user.h\"
CFLAGS += -D__C_LANGUAGE_EXTENSIONS_PERFC_PT__=1
CFLAGS += -D__C_LANGUAGE_EXTENSIONS_PERFC_COROUTINE__=0
CFLAGS += -D__COMPILER_HAS_GNU_EXTENSIONS__=1

SOURCES += $(foreach dir, $(GMSI_DIR), $(wildcard $(dir)/*.c))
SOURCES += $(foreach dir, $(GMSI_UTL_DIR), $(wildcard $(dir)/*.c))
SOURCES += $(GMSI_THREEPART_PERF)/perf_counter.c
SOURCES += main.c example.c template.c perfc_port_user.c

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
$(BUILD_DIR)/%.o: $(GMSI_THREEPART_PERF)/%.c
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@
clean:
	rm -rf $(BUILD_DIR)

printf_value:
	@echo $(info HFILE files is '$(HFILE)')