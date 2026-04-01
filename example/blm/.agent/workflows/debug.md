---
description: 烧录固件并启动 RTT 调试（需要硬件连接）
---

# 1. 烧录

```powershell
.\make.bat flash
```

确认输出包含 `verified` 和 `reset` 字样，表示烧录成功。

# 2. RTT 日志查看

```powershell
.\make.bat rtt
```

该命令会启动 OpenOCD 并开启 RTT server（端口 9090），保持运行。
可通过 `telnet localhost 9090` 查看 RTT 输出。
