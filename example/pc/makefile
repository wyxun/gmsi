GMSI_DIR := ../../gmsi
GMSI_UTL_DIR := ../../gmsi/utilities
GMSI_THREEPART_PLOOC = ../../ThirdParty/plooc
BUILD_DIR := build
TARGET := GMSI_DEMO

CC := clang
CXX := clang++
LDFLAGS := -fno-autolink -luser32 -W
CFLAGS := -std=c11 -W -Ofast 
CFLAGS += -I$(GMSI_DIR) \
-I$(GMSI_UTL_DIR) \
-I$(GMSI_THREEPART_PLOOC)

#头文件
#INCLUDE += $(foreach dir, $(GMSI_DIR), $(wildcard $(dir)/*.h))
#INCLUDE += $(foreach dir, $(GMSI_UTL_DIR), $(wildcard $(dir)/*.h))

SOURCES += $(foreach dir, $(GMSI_DIR), $(wildcard $(dir)/*.c))
SOURCES += $(foreach dir, $(GMSI_UTL_DIR), $(wildcard $(dir)/*.c))
SOURCES += main.c pc_uart.c

OBJECTS := $(SOURCES:.c=.o)
CFILENDIR := $(notdir  $(SOURCES))

# *.o文件
COBJS := $(patsubst %, $(BUILD_DIR)/%, $(CFILENDIR:.c=.o))


all: create_build_dir $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

#$(COBJS) : $(SOURCES)
#	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

create_build_dir:
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf obj/$(COBJS) $(TARGET) $(OBJECTS)

printf_value:
	@echo $(info source files is '$(SOURCES)')
	@echo $(info CFILENDIR files is '$(CFILENDIR)')
