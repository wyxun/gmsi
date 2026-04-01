---
description: 仅编译 AT32F421 项目（无需硬件）
---

// turbo-all

# Debug 编译

使用 `-O0` 优化级别（无优化）编译项目，方便查程序运行逻辑：

```powershell
.\make.bat clean; .\make.bat
```

检查 exit code 为 0 且 bin 大小正常即可。
