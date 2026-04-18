# Spec: SuperWaveform Documentation System

## Goal
Create a comprehensive documentation suite for the SuperWaveform utility to ensure ease of use, smooth firmware integration, and maintainable host code.

## Document Structure

### 1. `tools/superwaveform/README.md`
- **Overview**: High-level purpose of the tool.
- **Visuals**: Placeholder for screenshots.
- **Quick Build**: Steps for MSYS2/MinGW environments.
- **License**: GMSI standard.

### 2. `tools/superwaveform/docs/user_manual.md` (Target: A)
- **UI Components**: Detail the Oscilloscope, Terminal, and Dashboard.
- **Interaction**: Mouse/keyboard controls for zooming and panning.
- **Macros**: How to use and customize command buttons.
- **Troubleshooting**: Connection status meanings.

### 3. `tools/superwaveform/docs/mcu_integration.md` (Target: B)
- **Pre-requisites**: RTT setup and GMSI utilities.
- **API Reference**: 
    - `gwaveform_Init()`
    - `gwaveform_AddChannel()`
    - `gwaveform_PushRaw()`
    - `gwaveform_Commit()`
- **Code Snippet**: Full boilerplate for initialization and SysTick data pushing.
- **Protocol Spec**: Brief frame format for non-RTT transport porting.

### 4. `tools/superwaveform/docs/architecture_design.md` (Target: C)
- **Host Stack**: SDL2 + OpenGL + ImGui + ImPlot.
- **Module Responsibilities**:
    - `NetworkMgr`: Background threading and Winsock non-blocking IO.
    - `ProtocolParser`: Binary frame state machine.
    - `GuiLayer`: UI state management and ring buffer data storage.
- **Design Patterns**: Singleton (NetworkMgr), Observer-like data fetching.

## Verification Plan
- Verify all links in README.md are correct.
- Ensure C code snippets in `mcu_integration.md` are syntactically correct and match current GMSI APIs.
