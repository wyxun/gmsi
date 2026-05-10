# 环境部署指南

本指南详细介绍如何搭建 MStudio 的开发与编译环境。

## 1. 上位机开发环境 (Windows + MSYS2)

MStudio 使用 MinGW-w64 架构的 Clang 编译器进行构建。

### 1.1 安装 MSYS2
1. 从 [msys2.org](https://www.msys2.org/) 下载并安装。
2. 建议安装路径为 `C:\msys64` 或自定义路径（如 `D:\software\msys64`）。

### 1.2 安装编译器与依赖库
打开 **MSYS2 MINGW64** 终端，执行以下命令安装 Clang 和 SDL2：
```bash
# 更新同步数据库
pacman -Sy

# 安装 Clang 编译器 (x86_64)
pacman -S mingw-w64-x86_64-clang

# 安装 SDL2 开发库
pacman -S mingw-w64-x86_64-SDL2

# 安装 Make 工具
pacman -S make
```

### 1.3 编译器路径配置
由于 MSYS2 的 `bin` 目录通常不建议加入系统环境变量以免冲突，我们在 `Makefile` 中使用了绝对路径：
- 请确保 `Makefile` 中的 `CXX` 和 `CC` 指向您实际的 MSYS2 安装位置。
- 默认配置参考：`CXX = D:/software/msys64/mingw64/bin/clang++`

---

## 2. 第三方库管理 (Git Submodule)

MStudio 依赖 `imgui` 和 `implot`。

### 2.1 拉取代码
在项目根目录下执行：
```bash
git submodule update --init --recursive
```

### 2.2 切换 ImGui 分支 (重要)
MStudio 的布局依赖 ImGui 的 **docking** 分支：
```bash
cd thirdparty/imgui
git checkout docking
```

---

## 3. 下位机 (MCU) 依赖环境

要使上位机正常显示波形，下位机必须满足以下配置：

### 3.1 固件依赖
- **MODUS Utilities**: 必须包含 `mwaveform.c/h` 和 `mringbuf.c/h`。
- **SEGGER RTT**: 下位机必须支持多通道 RTT 输出。

### 3.2 调试器端口映射 (OpenOCD/JLink)
上位机通过 TCP 连接 RTT 服务。必须确保调试器开启了以下两个端口的监听：
- **TCP 9090**: 对应 RTT Channel 0 (用于 mshell 终端交互)。
- **TCP 9091**: 对应 RTT Channel 1 (用于二进制波形流)。

如果在 Makefile 中使用 OpenOCD，请确保包含以下命令：
`-c "rtt server start 9090 0" -c "rtt server start 9091 1"`
