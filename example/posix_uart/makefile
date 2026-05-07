MODUS_DIR := ../../modus
MODUS_UTL_DIR := ../../modus/utilities
MODUS_THREEPART_PLOOC = ../../lib/plooc
MODUS_THREEPART_PERF = ../../lib/perf_counter
BUILD_DIR := build
TARGET := $(BUILD_DIR)/demo

CC := clang
CXX := clang++

DEFS := -D_XOPEN_SOURCE=700 -DLINUX_POSIX -DTRACE_USE_LIBC_PRINTF=1

# -lrt:timer
# Link with rt and pthread (pthread provides pthread_sigmask)
LDFLAGS := -fno-autolink -Wall -lrt -lpthread
CFLAGS := -std=gnu11 -W -O0 -Wno-implicit-function-declaration -Wno-unused-parameter -Wno-int-to-pointer-cast -Wno-sign-compare
CFLAGS += -Wno-compare-distinct-pointer-types
CFLAGS += -pthread
CFLAGS += -I$(MODUS_DIR) -I/usr/include \
-I$(MODUS_UTL_DIR) \
-I$(MODUS_THREEPART_PERF) \
-I$(MODUS_THREEPART_PLOOC) \
-I.

CFLAGS += -D__PERFC_USE_PORTING__=1  
CFLAGS += -D__PERFC_USE_USER_CUSTOM_PORTING__=1  
CFLAGS += -D__PERFC_CFG_DISABLE_DEFAULT_SYSTICK_PORTING__=1  
CFLAGS += -D__PERFC_CFG_PORTING_INCLUDE__=\"perfc_port_user.h\"
CFLAGS += -D__C_LANGUAGE_EXTENSIONS_PERFC_PT__=1
CFLAGS += -D__C_LANGUAGE_EXTENSIONS_PERFC_COROUTINE__=0
CFLAGS += -D__COMPILER_HAS_GNU_EXTENSIONS__=1

SOURCES += $(foreach dir, $(MODUS_DIR), $(wildcard $(dir)/*.c))
SOURCES += $(foreach dir, $(MODUS_UTL_DIR), $(wildcard $(dir)/*.c))
# Only compile necessary perf_counter files, exclude coroutine support
SOURCES += $(MODUS_THREEPART_PERF)/perf_counter.c
SOURCES += main.c pc_uart.c pc_clock.c perfc_port_user.c

OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SOURCES)))

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@
$(BUILD_DIR)/%.o: $(MODUS_DIR)/%.c
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@
$(BUILD_DIR)/%.o: $(MODUS_UTL_DIR)/%.c
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@
$(BUILD_DIR)/%.o: $(MODUS_THREEPART_PERF)/%.c
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

printf_value:
	@echo $(info HFILE files is '$(HFILE)')