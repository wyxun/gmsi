---
description: 编译并生成 Release 版本固件（-Oz 优化）
---

// turbo-all

# Release 编译

使用 `-Oz` 优化级别编译项目：

```powershell
.\make.bat clean; .\make.bat BUILD=release
```

检查要点：
- 确认 `OPT` 为 `-Oz`
- 检查 `build/template.bin` 大小是否正常（~20KB）
- 编译完成后生成的固件将位于 `build/` 目录下。
