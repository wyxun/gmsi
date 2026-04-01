---
description: 完整自动化流程：编译 (Debug) + 烧录 + RTT 调试验证
---

# 自动化测试流程

依次执行编译、烧录，并自动启动 RTT Server 验证输出。

## 1. 编译 (Debug 模式)

// turbo
```powershell
.\make.bat clean; .\make.bat
```

确认输出末尾看到 `text`/`bss` 大小信息，无编译错误。

## 2. 烧录

// turbo
```powershell
.\make.bat flash
```

确认输出包含 `verified` 字样，表示烧录成功。

## 3. 启动 RTT Server（新窗口，保持运行）

```powershell
Start-Process powershell -ArgumentList "-NoExit", "-Command", ".\make.bat rtt"
```

等待新窗口出现 `Listening on port 9090 for rtt connections`。

## 4. 验证 RTT 输出（新开监控日志窗口）

运行以下命令，会自动弹出一个名为 RTT Log Viewer 的绿色环保界面，用来专职打印 RTT 上的日志：

// turbo
```powershell
Start-Sleep -Seconds 4
Start-Process powershell -ArgumentList "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", ".\.agent\workflows\rtt_viewer.ps1"
```

看到弹出新屏幕，并显示 `=== RTT Log Viewer Connected ===` 即为正常。不再需要查看时只要关闭该窗口即可。

## 5. 关闭 RTT Server（调试结束后）

```powershell
Get-Process -Name openocd -ErrorAction SilentlyContinue | Stop-Process -Force
```
