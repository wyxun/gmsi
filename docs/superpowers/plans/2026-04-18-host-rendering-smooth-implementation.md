# Host Rendering Smoothness Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement adaptive sample rate estimation and smooth plotting to resolve waveform jitter on the host.

**Architecture:** Update `GuiLayer` to track data density and use a virtual clock for X-axis coordinates.

**Tech Stack:** C++, ImGui, ImPlot

---

### Task 1: Update GuiLayer State Variables
**Files:**
- Modify: `e:/Project/modus/tools/superwaveform/src/gui_layer.h`

- [ ] **Step 1: Add private members to GuiLayer class**
```cpp
    double m_virtualClock = 0.0;
    double m_smoothedPeriod = 0.001; // Initial guess 1kHz
    double m_lastWindowTime = 0.0;
    int m_pointsInWindow = 0;
```

### Task 2: Implement Adaptive Smoothing Logic
**Files:**
- Modify: `e:/Project/modus/tools/superwaveform/src/gui_layer.cpp`

- [ ] **Step 1: Update UpdateWaveformData with adaptive estimation**
```cpp
    double now = ImGui::GetTime();
    
    // 1. Adaptive period estimation (every 500ms)
    m_pointsInWindow += frames.size();
    if (now - m_lastWindowTime > 0.5) {
        if (m_pointsInWindow > 0) {
            double estimated = (now - m_lastWindowTime) / m_pointsInWindow;
            m_smoothedPeriod = m_smoothedPeriod * 0.8 + estimated * 0.2;
        }
        m_lastWindowTime = now;
        m_pointsInWindow = 0;
    }

    // 2. Data distribution
    for (const auto& frame : frames) {
        // Sync check: if virtual clock drifts too much from wall clock
        if (std::abs(now - m_virtualClock) > m_smoothedPeriod * 50.0) {
            m_virtualClock = now;
        } else {
            m_virtualClock += m_smoothedPeriod;
        }
        
        // Add to buffer using m_virtualClock
        // ... (existing logic)
    }
```

### Task 3: Update Dashboard UI
**Files:**
- Modify: `e:/Project/modus/tools/superwaveform/src/gui_layer.cpp`

- [ ] **Step 1: Show Sample Rate in the Dashboard**
```cpp
    ImGui::Text("Detected Rate: %.1f Hz", 1.0 / m_smoothedPeriod);
```
