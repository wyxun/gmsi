# SuperWaveform Documentation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Create a comprehensive documentation suite for SuperWaveform.

**Architecture:** Modular markdown files in `docs/` and a root `README.md`.

**Tech Stack:** Markdown

---

### Task 1: Initialize Directory and README
**Files:**
- Create: `tools/superwaveform/README.md`

- [ ] **Step 1: Create README.md**
```markdown
# SuperWaveform

High-performance MODUS Oscilloscope and Terminal host utility. Built with SDL2 + ImGui + ImPlot.

## Features
- **Dual Channel Integration**: Monitor RTT Channel 0 (Shell) and Channel 1 (Waveform) simultaneously.
- **Deep Buffering**: Million-point local storage prevents data loss during power-on transients.
- **Professional Interaction**: Time-axis zoom/pan and history review.
- **Custom Macros**: Integrated buttons for common debug commands.

## Quick Start (MSYS2)
```bash
cd tools/superwaveform
make
./superwaveform.exe
```

## Documentation
- [User Manual](./docs/user_manual.md)
- [MCU Integration](./docs/mcu_integration.md)
- [Architecture Design](./docs/architecture_design.md)
```

- [ ] **Step 2: Commit**
```bash
git add tools/superwaveform/README.md
git commit -m "docs: init superwaveform readme"
```

### Task 2: Create User Manual
**Files:**
- Create: `tools/superwaveform/docs/user_manual.md`

- [ ] **Step 1: Write user_manual.md**
```markdown
# SuperWaveform User Manual

## Interface
- **Oscilloscope**: Main plot area for real-time waves.
- **GShell Terminal**: Dedicated sidebar for MCU logs and commands.
- **Dashboard**: Connection status, pause/resume, and macros.

## Interaction
- **Zoom**: Scroll wheel on the plot.
- **Pan**: Right-click and drag in pause mode.
```

- [ ] **Step 2: Commit**
```bash
git add tools/superwaveform/docs/user_manual.md
git commit -m "docs: add user manual"
```

### Task 3: Create MCU Integration Guide
**Files:**
- Create: `tools/superwaveform/docs/mcu_integration.md`

- [ ] **Step 1: Write mcu_integration.md**
```markdown
# MCU Integration Guide

## API Example
```c
#include "mwaveform.h"

void init() {
    mwaveform_Init();
    mwaveform_AddChannel("Sine", 100.0f);
}

void loop() {
    mwaveform_PushRaw(0, val);
    mwaveform_Commit();
}
```
```

- [ ] **Step 2: Commit**
```bash
git add tools/superwaveform/docs/mcu_integration.md
git commit -m "docs: add integration guide"
```

### Task 4: Create Architecture Design
**Files:**
- Create: `tools/superwaveform/docs/architecture_design.md`

- [ ] **Step 1: Write architecture_design.md**
```markdown
# Architecture Design

## Modules
- **NetworkMgr**: Winsock threads.
- **ProtocolParser**: Frame parsing.
- **GuiLayer**: ImPlot rendering.
```

- [ ] **Step 2: Commit**
```bash
git add tools/superwaveform/docs/architecture_design.md
git commit -m "docs: add architecture doc"
```
