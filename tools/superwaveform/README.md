# SuperWaveform

高性能 GMSI 示波器与终端上位机工具。基于 SDL2 + ImGui + ImPlot 开发，旨在为嵌入式开发提供极致流畅的波形观察与调试体验。

## 核心特性
- **双通道融合**：同时监控 RTT Channel 0 (Shell) 和 Channel 1 (Waveform)。
- **深度缓存**：本地支持百万级点位存储，上电瞬间数据不再丢失。
- **专业交互**：支持时间轴滚动缩放、鼠标平移查看历史数据。
- **自定义宏**：集成常用调试指令按钮，提升调试效率。

## 快速开始 (MSYS2 环境)
```bash
cd tools/superwaveform
make
./superwaveform.exe
```

## 文档索引
- [环境部署指南](./docs/environment_setup.md) - **编译与环境搭建必读**。
- [用户使用手册](./docs/user_manual.md) - 界面功能与操作说明。
- [下位机集成指南](./docs/mcu_integration.md) - 如何在 MCU 侧推送波形数据。
- [架构设计说明](./docs/architecture_design.md) - 软件设计原理与二次开发。
