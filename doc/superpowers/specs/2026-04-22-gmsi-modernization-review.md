# MODUS Modernization and Registry System Review

This document summarizes the architectural changes introduced during the recent MODUS modularization, specifically the transition to a registry-driven initialization system and the refactoring of the `GStorage` component.

## 1. Core Architectural Shift: Registry-Driven Model
The framework transitioned from a manual, list-based initialization (controlled entirely in `main.c`) to a semi-automated **Registry-Driven** model.

### Key Macros
- **`MODUS_DECLARE_OBJECT(type, name, ...)`**:
  - **Purpose**: Declares an application-level object.
  - **Mechanism**: Creates configuration (`tNameCfg`) and instance (`tName`) while registering an entry in the `init_infos` linker section.
  - **Entry Point**: Automatically calls `type_Init` during `modus_Init`.
- **`MODUS_SERVICE_DECLARE(name, init, poll, clock, addr, cfg)`**:
  - **Purpose**: Declares internal library services (e.g., `mshell`, `mstorage`).
  - **Mechanism**: Provides separate pointers for `Init`, `Poll`, and `Clock` hooks, allowing self-sustaining execution.

### Initialization & Execution Flow
1. **`System_Init()`** (Manual): Basic Hardware setup.
2. **`modus_Init(&tModus)`**: Traverses `init_infos` and calls `pfcnInitFunc` for all registered entities.
3. **`modus_Run()`**: Traverses legacy objects, runs coroutines, and auto-polls registry services (`pfcnPollFunc`).
4. **`modus_Clock()`**: Triggers 1ms hooks for both legacy and registry-indexed modules.

---

## 2. GStorage Component Refactoring
`GStorage` is now an internal service requiring minimal main-loop logic.

### Changes:
- **Strong Symbol Requirement**: Enabling `MODUS_USE_MSTORAGE` requires defining `g_tModusStorageConfig`. Failure to do so results in a **link-time error**.
- **Auto-Initialization**: Handled via `MODUS_SERVICE_DECLARE` in `mstorage.c`.
- **Flexible Binding**: Correctly binds `ptAppFlash` at runtime via `modus_Init` dependency injection.

---

## 3. Nested Makefile System (Modularization)
Version 0.3.0.1 introduced `modus.mk` to enable project-level modularization.

- **Dynamic Inclusion**: Modules are included via `MODUS_USE_XXX` flags (e.g., `MODUS_USE_MSTORAGE`, `MODUS_USE_MSHELL`).
- **Dependency Management**: The Makefile automatically handles include paths and required core library dependencies (like `plooc` or `perf_counter`).
- **Code Size Optimization**: Only selected modules are compiled, allowing for smaller binaries in memory-constrained bootloaders.

---

## 4. Library-Wide Localization
A significant effort was made to convert the codebase for international collaboration.

- **English Comments**: All core MODUS core files, GDI (Driver Interface) layers, and utility components were translated from Chinese to English.
- **Documentation Refactor**: Technical documents like `todo.md` and `trace_report.md` were also updated to English.

---

## 5. Potential Failure Points in BLM Program
As the BLM program currently "fails to run," the following areas require immediate review:

### A. Section Mapping (Linker)
- The registry relies on `__start_init_infos` and `__stop_init_infos`. These must be correctly defined in the project's `linker.ld`.
- **Problem**: If the section is empty or misaligned, modules (including BLM core) won't initialize.

### B. Hardware Initialization Order
- `main.c` calls `System_Init` before `modus_Init`.
- **Problem**: We must verify that `SCB->VTOR` and `perfc_init` are correctly configured BEFORE any MODUS logs or flash accesses occur.

### C. GStorage CRC/Flash Failures
- The new `mstorage_Init` performs an immediate Flash Read to verify CRC.
- **Problem**: If the Flash driver (`ptAppFlash`) is not fully ready or if the address `0x0801F800` is invalid for the specific target sub-variant, it may cause a hard fault.

---
