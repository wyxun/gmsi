# SuperWaveform Macro Manager — Design Spec

**Date:** 2026-05-03  
**Status:** Approved

---

## 背景与目标

SuperWaveform 的 Dashboard 当前硬编码了 4 个固定宏按钮（`wave start`、`wave stop`、`log -I`、`log -A`），无法适配不同项目的扩展指令集。

目标是引入 **INI 文件驱动的动态宏按钮系统**，让用户可以：
1. 手写 INI 文件定义任意数量的宏按钮
2. 通过 UI 按钮加载指定 INI 文件（文件对话框）
3. 下次启动时自动恢复上次加载的文件

---

## 架构

### 新增模块：`MacroManager`

| 文件 | 职责 |
|---|---|
| `src/macro_mgr.h` | 接口声明：`LoadFromFile()`, `RestoreLastSession()`, `GetMacros()` |
| `src/macro_mgr.cpp` | INI 解析、`superwaveform.cfg` 读写 |

### 持久化策略

- **宏路径持久化**：`superwaveform.cfg`（exe 同目录），存储上次加载的 INI 路径
- **格式**：极简 Key-Value，无需第三方库

```
last_macro_file=E:\Project\gmsi\blm_macros.ini
```

- 启动时：`MacroManager` 读取 `superwaveform.cfg` → 若路径有效则自动加载对应 INI
- 加载新文件后：自动更新 `superwaveform.cfg`

---

## INI 文件格式

```ini
# 注释行（# 开头，忽略）
# 格式：按钮标签 = 发送给 Ch0 的指令

wave start = wave start
wave stop = wave stop
log hide info = log -I
log show all = log -A
set freq 1M = freq set 1000000
calibrate = cal run -v
```

**解析规则：**
- 空行忽略
- `#` 开头的行忽略（注释）
- `=` 左侧 trim 后为按钮 Label，右侧 trim 后为指令字符串
- 发送时自动追加 `\n`
- 行内含多个 `=` 时，以第一个 `=` 为分隔符（指令可以包含 `=`）

---

## UI 变化

### Dashboard — "Quick User Macros" 区域

**改造前（硬编码）：**
```
Quick User Macros (Ch0):
[wave start] [wave stop] [log -I (Hide Info)] [log -A (Show All)]
```

**改造后（动态）：**
```
[Load Macros...]  已加载: blm_macros.ini
─────────────────────────────────────────────────
[wave start] [wave stop] [log hide info] [set freq 1M] [calibrate] ...
```

- `[Load Macros...]`：打开文件对话框，过滤 `*.ini`，加载后刷新按钮列表并保存路径
- 已加载路径显示文件名（不显示完整路径，节省空间）
- 未加载时显示灰色提示文字 `(No macros loaded)`
- 按钮按 INI 顺序水平排列，使用 `ImGui::SameLine()` 自动换行

---

## 数据结构

```cpp
struct MacroEntry {
    std::string label;   // 按钮显示文字
    std::string command; // 发送给 Ch0 的指令（不含 \n）
};
```

---

## 接口设计

```cpp
class MacroManager {
public:
    bool LoadFromFile(const std::string& path);
    const std::vector<MacroEntry>& GetMacros() const;
    const std::string& GetCurrentPath() const;
    void RestoreLastSession();
    void PersistCurrentPath() const;

private:
    std::vector<MacroEntry> macros_;
    std::string current_path_;
    static constexpr const char* kConfigFile = "superwaveform.cfg";
    static constexpr const char* kConfigKey  = "last_macro_file";
};
```

---

## 涉及改动文件清单

| 文件 | 改动类型 | 说明 |
|---|---|---|
| `src/macro_mgr.h` | **新增** | MacroManager 声明 |
| `src/macro_mgr.cpp` | **新增** | INI 解析 + cfg 持久化逻辑 |
| `src/gui_layer.h` | **修改** | 成员加入 `MacroManager macro_mgr_` |
| `src/gui_layer.cpp` | **修改** | 构造函数调用 `RestoreLastSession()`；Dashboard 区域替换硬编码按钮 |
| `Makefile` | **无需改动** | 已使用 `wildcard *.cpp`，自动包含新文件 |

---

## 边界情况处理

| 情况 | 处理方式 |
|---|---|
| `superwaveform.cfg` 不存在 | 静默跳过，空宏列表启动 |
| 上次路径文件已删除 | `LoadFromFile` 返回 false，保持空宏列表，不崩溃 |
| INI 格式错误行 | 跳过该行，继续解析后续行 |
| INI 为空 | 空宏列表，UI 显示提示 |
| 同名标签 | 允许（按顺序生成多个同名按钮） |
