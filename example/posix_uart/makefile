GMSI_DIR := ../../gmsi
GMSI_UTL_DIR := ../../gmsi/utilities
GMSI_THREEPART_PLOOC = ../../ThirdParty/plooc
BUILD_DIR := build
TARGET := demo

CC := clang
CXX := clang++
# time.h文件不齐全
DEFS := -D_XOPEN_SOURCE=700 -DLINUX_POSIX
# -lrt:timer库
LDFLAGS := -fno-autolink -Wall -lrt
CFLAGS := -std=c11 -W -Ofast -Wno-implicit-function-declaration -Wno-unused-parameter -Wno-int-to-pointer-cast -Wno-sign-compare
CFLAGS += -Wno-compare-distinct-pointer-types
CFLAGS += -I$(GMSI_DIR) -I/usr/include \
-I$(GMSI_UTL_DIR) \
-I$(GMSI_THREEPART_PLOOC) 

SOURCES += $(foreach dir, $(GMSI_DIR), $(wildcard $(dir)/*.c))
SOURCES += $(foreach dir, $(GMSI_UTL_DIR), $(wildcard $(dir)/*.c))
SOURCES += main.c pc_uart.c pc_clock.c

OBJECTS := $(SOURCES:.c=.o)
CFILENDIR := $(notdir  $(SOURCES))

# *.o文件
COBJS := $(patsubst %, $(BUILD_DIR)/%, $(CFILENDIR:.c=.o))

all: create_build_dir $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

create_build_dir:
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf obj/$(COBJS) $(TARGET) $(OBJECTS)

printf_value:
	@echo $(info source files is '$(SOURCES)')
	@echo $(info CFILENDIR files is '$(CFILENDIR)')
