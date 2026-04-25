# SuperWaveform Host Tool Makefile
# Compiler: mingw-w64-x86_64-clang++

MSYS64_PATH = D:/0_software/msys64
CXX = $(MSYS64_PATH)/mingw64/bin/clang++
CFLAGS = -Wall -Wextra -O2 -g
CXXFLAGS = -std=c++17 $(CFLAGS)

# Directories
SRC_DIR = src
THIRD_PARTY_DIR = thirdparty
BUILD_DIR = build

# Include Paths
INCLUDES = -I$(SRC_DIR) \
           -I$(SRC_DIR)/utils \
           -I$(THIRD_PARTY_DIR)/imgui \
           -I$(THIRD_PARTY_DIR)/imgui/backends \
           -I$(THIRD_PARTY_DIR)/implot \
           -I$(MSYS64_PATH)/mingw64/include/SDL2

# Source files (Core)
SRCS = $(wildcard $(SRC_DIR)/*.cpp) \
       $(SRC_DIR)/utils/gringbuf.c

# ImGui & ImPlot Sources (Only compile if they exist)
IMGUI_SRCS = $(wildcard $(THIRD_PARTY_DIR)/imgui/*.cpp) \
             $(THIRD_PARTY_DIR)/imgui/backends/imgui_impl_sdl2.cpp \
             $(THIRD_PARTY_DIR)/imgui/backends/imgui_impl_opengl3.cpp
IMPLOT_SRCS = $(wildcard $(THIRD_PARTY_DIR)/implot/*.cpp)

ALL_SRCS = $(SRCS) $(IMGUI_SRCS) $(IMPLOT_SRCS)

# Object files (Flattened in build directory)
OBJS = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(notdir $(ALL_SRCS))))

# Libraries
LIBS = -lmingw32 -lSDL2main -lSDL2 -lws2_32 -lopengl32 -lcomdlg32 -mwindows

TARGET = superwaveform.exe

all: check_deps $(TARGET)

check_deps:
	@if [ ! -f $(THIRD_PARTY_DIR)/imgui/imgui.cpp ]; then echo "[ERROR] ImGui submodule missing. Please run 'git submodule update --init' or manually download to thirdparty/imgui"; exit 1; fi
	@if [ ! -f $(THIRD_PARTY_DIR)/implot/implot.cpp ]; then echo "[ERROR] ImPlot submodule missing. Please run 'git submodule update --init' or manually download to thirdparty/implot"; exit 1; fi

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LIBS)

# VPATH tells make where to look for source files
vpath %.cpp $(SRC_DIR) $(THIRD_PARTY_DIR)/imgui $(THIRD_PARTY_DIR)/imgui/backends $(THIRD_PARTY_DIR)/implot
vpath %.c $(SRC_DIR)/utils

# Build rule for C++ files
$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Build rule for C files
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(MSYS64_PATH)/mingw64/bin/clang $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET)

.PHONY: all clean check_deps
