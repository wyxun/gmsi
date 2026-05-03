# SuperWaveform Macro Manager Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 新增 INI 文件驱动的动态宏按钮系统，替换 Dashboard 中的硬编码按钮，支持文件对话框加载和路径持久化。

**Architecture:** 新增独立 `MacroManager` 模块负责 INI 解析与路径持久化（`superwaveform.cfg`），`GuiLayer` 持有其实例，启动时自动恢复，UI 按 INI 顺序动态渲染按钮。

**Tech Stack:** C++17, ImGui, Windows `OPENFILENAMEA`（已有），`std::fstream`（已有），零新增依赖

---

## 文件结构

| 文件 | 操作 | 职责 |
|---|---|---|
| `tools/superwaveform/src/macro_mgr.h` | **新建** | `MacroEntry` 结构体 + `MacroManager` 类声明 |
| `tools/superwaveform/src/macro_mgr.cpp` | **新建** | INI 解析、`superwaveform.cfg` 读写实现 |
| `tools/superwaveform/src/gui_layer.h` | **修改** | 加入 `#include "macro_mgr.h"` 和 `MacroManager macro_mgr_` 成员 |
| `tools/superwaveform/src/gui_layer.cpp` | **修改** | 构造函数加载、Dashboard 区域替换、Load 按钮逻辑 |

---

## Task 1: 新建 MacroManager 模块

**Files:**
- Create: `tools/superwaveform/src/macro_mgr.h`
- Create: `tools/superwaveform/src/macro_mgr.cpp`

- [ ] **Step 1: 创建头文件 `macro_mgr.h`**

```cpp
// tools/superwaveform/src/macro_mgr.h
#ifndef MACRO_MGR_H
#define MACRO_MGR_H

#include <string>
#include <vector>

struct MacroEntry {
    std::string label;   // 按钮显示文字
    std::string command; // 发送给 Ch0 的指令（不含 \n）
};

class MacroManager {
public:
    // 从 INI 文件加载宏列表，成功返回 true
    bool LoadFromFile(const std::string& path);

    // 获取当前宏列表（只读）
    const std::vector<MacroEntry>& GetMacros() const { return macros_; }

    // 获取当前加载的文件路径（空字符串表示未加载）
    const std::string& GetCurrentPath() const { return current_path_; }

    // 从 superwaveform.cfg 恢复上次路径并加载，失败时静默跳过
    void RestoreLastSession();

    // 将当前路径保存到 superwaveform.cfg
    void PersistCurrentPath() const;

private:
    std::vector<MacroEntry> macros_;
    std::string current_path_;

    static constexpr const char* kConfigFile = "superwaveform.cfg";
    static constexpr const char* kConfigKey  = "last_macro_file";

    // 去除字符串首尾空白
    static std::string Trim(const std::string& s);
};

#endif // MACRO_MGR_H
```

- [ ] **Step 2: 创建实现文件 `macro_mgr.cpp`**

```cpp
// tools/superwaveform/src/macro_mgr.cpp
#include "macro_mgr.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

// 去除首尾空白（空格、\t、\r）
std::string MacroManager::Trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace((unsigned char)*start)) ++start;
    auto end = s.end();
    while (end != start && std::isspace((unsigned char)*(end - 1))) --end;
    return std::string(start, end);
}

bool MacroManager::LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    macros_.clear();
    std::string line;
    while (std::getline(file, line)) {
        // 去掉 \r（Windows 换行）
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::string trimmed = Trim(line);

        // 忽略空行和注释行
        if (trimmed.empty() || trimmed[0] == '#') continue;

        // 以第一个 '=' 为分隔符
        auto eq_pos = trimmed.find('=');
        if (eq_pos == std::string::npos) continue; // 格式错误行，跳过

        std::string label   = Trim(trimmed.substr(0, eq_pos));
        std::string command = Trim(trimmed.substr(eq_pos + 1));

        if (label.empty() || command.empty()) continue;

        macros_.push_back({label, command});
    }

    current_path_ = path;
    return true;
}

void MacroManager::RestoreLastSession() {
    std::ifstream cfg(kConfigFile);
    if (!cfg.is_open()) return;

    std::string line;
    while (std::getline(cfg, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        auto eq_pos = line.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string key = Trim(line.substr(0, eq_pos));
        std::string val = Trim(line.substr(eq_pos + 1));

        if (key == kConfigKey && !val.empty()) {
            LoadFromFile(val); // 失败时 current_path_ 不更新，静默跳过
            return;
        }
    }
}

void MacroManager::PersistCurrentPath() const {
    if (current_path_.empty()) return;
    std::ofstream cfg(kConfigFile);
    if (!cfg.is_open()) return;
    cfg << kConfigKey << "=" << current_path_ << "\n";
}
```

- [ ] **Step 3: 验证编译（仅编译，不链接）**

在 `tools/superwaveform` 目录下执行：
```bash
D:/0_software/msys64/mingw64/bin/clang++ -std=c++17 -Wall -Isrc -c src/macro_mgr.cpp -o build/macro_mgr.cpp.o
```
预期输出：无错误，生成 `build/macro_mgr.cpp.o`

- [ ] **Step 4: Commit**

```bash
git add tools/superwaveform/src/macro_mgr.h tools/superwaveform/src/macro_mgr.cpp
git commit -m "feat(superwaveform): add MacroManager module for INI-driven macro buttons"
```

---

## Task 2: 修改 gui_layer.h — 集成 MacroManager

**Files:**
- Modify: `tools/superwaveform/src/gui_layer.h:1-13`（include 区域）
- Modify: `tools/superwaveform/src/gui_layer.h:62-80`（成员变量区域）

- [ ] **Step 1: 在 `gui_layer.h` 顶部 include 区域加入 `macro_mgr.h`**

在现有 `#include <fstream>` 行之后添加：
```cpp
#include "macro_mgr.h"
```

完整 include 区域变为：
```cpp
#include "protocol_parser.h"
#include <vector>
#include <string>
#include <map>
#include "imgui.h"

#include <fstream>
#include <ctime>
#include <iomanip>
#include "macro_mgr.h"   // ← 新增
```

- [ ] **Step 2: 在 `gui_layer.h` 的 `private:` 成员区域加入 `MacroManager` 成员**

在 `// Adaptive Smoothing State` 注释块之前，加入：
```cpp
    // Macro Manager
    MacroManager macro_mgr_;
```

完整结构示意（仅展示新增行位置）：
```cpp
    // Terminal data
    std::string term_log_;
    char term_input_buf_[256];
    bool auto_scroll_ = true;
    ImGuiTextFilter term_filter_;

    // Macro Manager
    MacroManager macro_mgr_;   // ← 新增

    // UI Helpers
    void FetchNetworkData();
    ...
```

- [ ] **Step 3: 验证编译**

```bash
D:/0_software/msys64/mingw64/bin/clang++ -std=c++17 -Wall -Isrc -Ithirdparty/imgui -c src/gui_layer.cpp -o build/gui_layer.cpp.o
```
预期输出：无错误

- [ ] **Step 4: Commit**

```bash
git add tools/superwaveform/src/gui_layer.h
git commit -m "feat(superwaveform): add MacroManager member to GuiLayer"
```

---

## Task 3: 修改 gui_layer.cpp — 构造函数加载 + Dashboard 动态按钮

**Files:**
- Modify: `tools/superwaveform/src/gui_layer.cpp:15-17`（构造函数）
- Modify: `tools/superwaveform/src/gui_layer.cpp:258-268`（Dashboard 宏区域）

- [ ] **Step 1: 在构造函数中调用 `RestoreLastSession()`**

找到构造函数（L15-17）：
```cpp
GuiLayer::GuiLayer() : parser_(8) {
    std::memset(term_input_buf_, 0, sizeof(term_input_buf_));
}
```

修改为：
```cpp
GuiLayer::GuiLayer() : parser_(8) {
    std::memset(term_input_buf_, 0, sizeof(term_input_buf_));
    macro_mgr_.RestoreLastSession();
}
```

- [ ] **Step 2: 替换 Dashboard 中的硬编码宏区域（L260-265）**

找到现有代码：
```cpp
    ImGui::Separator();
    ImGui::Text("Quick User Macros (Ch0):");
    if (ImGui::Button("wave start")) { net.SendToCh0("wave start\n"); } ImGui::SameLine();
    if (ImGui::Button("wave stop")) { net.SendToCh0("wave stop\n"); } ImGui::SameLine();
    if (ImGui::Button("log -I (Hide Info)")) { net.SendToCh0("log -I\n"); } ImGui::SameLine();
    if (ImGui::Button("log -A (Show All)")) { net.SendToCh0("log -A\n"); }
```

替换为：
```cpp
    ImGui::Separator();

    // --- Macro Manager UI ---
    ImGui::Text("Quick User Macros (Ch0):");
    ImGui::SameLine();
    if (ImGui::Button("Load Macros...")) {
        // 复用已有文件对话框，过滤 *.ini
        std::string path = OpenIniFileDialog();
        if (!path.empty()) {
            if (macro_mgr_.LoadFromFile(path)) {
                macro_mgr_.PersistCurrentPath();
            }
        }
    }
    ImGui::SameLine();
    const std::string& cur_path = macro_mgr_.GetCurrentPath();
    if (cur_path.empty()) {
        ImGui::TextDisabled("(No macros loaded)");
    } else {
        // 只显示文件名，不显示完整路径
        size_t slash = cur_path.find_last_of("/\\");
        std::string fname = (slash != std::string::npos) ? cur_path.substr(slash + 1) : cur_path;
        ImGui::TextDisabled("%s", fname.c_str());
    }

    // 动态渲染宏按钮
    const auto& macros = macro_mgr_.GetMacros();
    if (!macros.empty()) {
        ImGui::Separator();
        for (size_t i = 0; i < macros.size(); ++i) {
            // 用唯一 ID 避免 ImGui 同名按钮冲突
            char btn_id[128];
            snprintf(btn_id, sizeof(btn_id), "%s##macro%zu", macros[i].label.c_str(), i);
            if (ImGui::Button(btn_id)) {
                net.SendToCh0(macros[i].command + "\n");
            }
            if (i + 1 < macros.size()) ImGui::SameLine();
        }
    }
    // --- End Macro Manager UI ---
```

- [ ] **Step 3: 新增 `OpenIniFileDialog()` 辅助函数**

在 `gui_layer.h` 的 `private:` 声明中加入：
```cpp
    std::string OpenIniFileDialog();
```

在 `gui_layer.cpp` 中，在 `OpenFileDialog()` 函数定义（L419）之后，添加：
```cpp
std::string GuiLayer::OpenIniFileDialog() {
#ifdef _WIN32
    OPENFILENAMEA ofn;
    char szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "INI Files\0*.ini\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(ofn.lpstrFile);
    }
#endif
    return "";
}
```

- [ ] **Step 4: 完整构建验证**

在 `tools/superwaveform` 目录下执行完整构建：
```bash
make clean && make
```
预期输出：
```
clang++ ... -o superwaveform.exe
```
无编译错误，生成 `superwaveform.exe`

- [ ] **Step 5: 手动验证功能**

1. 在 `tools/superwaveform` 目录创建测试 INI 文件 `test_macros.ini`：
   ```ini
   # Test Macros
   wave start = wave start
   wave stop = wave stop
   log hide = log -I
   ```
2. 运行 `superwaveform.exe`
3. Dashboard 中点击 `[Load Macros...]`，选择 `test_macros.ini`
4. 验证：出现 3 个按钮 `[wave start]` `[wave stop]` `[log hide]`
5. 关闭并重新打开程序
6. 验证：按钮自动恢复（路径已持久化到 `superwaveform.cfg`）

- [ ] **Step 6: Commit**

```bash
git add tools/superwaveform/src/gui_layer.h tools/superwaveform/src/gui_layer.cpp
git commit -m "feat(superwaveform): replace hardcoded macro buttons with dynamic INI-driven system"
```

---

## 示例 INI 文件（供参考）

在 `tools/superwaveform` 目录创建 `blm_macros.ini`，并提交为示例：

```ini
# BLM Bootloader Macros
# 格式：按钮标签 = 发送给 Ch0 的指令

wave start = wave start
wave stop = wave stop
log hide = log -I
log show = log -A
bl enter = bl enter
bl status = bl status
fw info = fw info
```

```bash
git add tools/superwaveform/blm_macros.ini
git commit -m "chore(superwaveform): add example blm_macros.ini for BLM project"
```
