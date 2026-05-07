# MODUS.MK — Unified Build Include Design

## Overview

Provide a single `modus.mk` at the repository root so external projects can include MODUS via
one line. All optional modules default to OFF (conservative). Projects opt in by setting
`?= 1` variables before or after the include.

## Design

### File location

```
modus/
├── modus.mk          ← NEW: root-level build include
├── modus/
│   ├── modus.c ...
│   ├── mdebug/ ...
│   ├── mdi/ ...
│   └── utilities/ ...
```

### Usage pattern (external project Makefile)

```makefile
MODUS_ROOT ?= lib/modus
include $(MODUS_ROOT)/modus.mk

# Opt-in to needed modules
MBLINFO_ENABLE  = 1
MSTORAGE_ENABLE = 1

C_SOURCES += $(MODUS_SRCS)
C_INCLUDES += $(MODUS_INCLUDES)
CFLAGS += $(MODUS_CFLAGS)
```

### Configuration variables

#### Enable/disable switches (all default 0)

| Variable | Default | Effect when `1` |
|----------|:------:|-----------------|
| `MSHELL_ENABLE` | 0 | Compile mshell + trace + SEGGER_RTT; requires framework guards in modus.c |
| `MWAVEFORM_ENABLE` | 0 | Compile mwaveform + mwaveform_protocol; requires framework guards in modus.c |
| `MSTORAGE_ENABLE` | 0 | Compile mstorage.c (init-section registration, no framework guards needed) |
| `MBLINFO_ENABLE` | 0 | Compile mblinfo.c (init-section registration, no framework guards needed) |
| `MODUS_USE_LOG` | 0 | Enable `MLOG`/`MLOGF` macros at compile time (`0` → `-D__NO_USE_LOG__`) |
| `MODUS_USE_ASSERT` | 0 | Enable `MODUS_ASSERT` macro (`0` → `-D__NO_USE_ASSERT`) |

#### Size-tuning overrides (have defaults, optional)

| Variable | Default | Maps to |
|----------|---------|---------|
| `MWAVEFORM_MAX_CHANNELS` | 16 | `-DMWAVEFORM_MAX_CHANNELS=N` |
| `MWAVEFORM_RTT_BUFFER_SIZE` | 1024 | `-DMWAVEFORM_RTT_BUFFER_SIZE=N` |
| `MWAVEFORM_FIFO_DEPTH` | 16 | `-DMWAVEFORM_FIFO_DEPTH=N` |
| `MWAVEFORM_DECIMATION` | 1 | `-DMWAVEFORM_DECIMATION=N` |
| `MLOG_MASK_DEFAULT` | 0x1F | `-DMLOG_MASK_DEFAULT=N` |

Lower-level overrides (`MSHELL_LINE_SIZE`, `SEGGER_RTT_MAX_NUM_UP_BUFFERS`, etc.) stay in
`userconfig.h` to keep the `.mk` lean.

### Output variables

#### Source file groups

```
MODUS_SRCS_CORE       ← modus.c mbase.c mcoroutine.c mlog.c
                        utilities/list.c util_queue.c mringbuf.c
MODUS_SRCS_DEBUG      ← mshell.c trace.c trace_fmt.c util_debug.c SEGGER_RTT.c
MODUS_SRCS_WAVEFORM   ← mwaveform.c mwaveform_protocol.c
MODUS_SRCS_STORAGE    ← mstorage.c
MODUS_SRCS_BLINFO     ← mblinfo.c
```

#### Aggregated by switches

```makefile
MODUS_SRCS = $(MODUS_SRCS_CORE)
ifneq ($(MSHELL_ENABLE),0)
  MODUS_SRCS += $(MODUS_SRCS_DEBUG)
endif
ifneq ($(MWAVEFORM_ENABLE),0)
  MODUS_SRCS += $(MODUS_SRCS_WAVEFORM)
endif
ifneq ($(MSTORAGE_ENABLE),0)
  MODUS_SRCS += $(MODUS_SRCS_STORAGE)
endif
ifneq ($(MBLINFO_ENABLE),0)
  MODUS_SRCS += $(MODUS_SRCS_BLINFO)
endif
```

#### Include paths (fixed)

```makefile
MODUS_INCLUDES = -I$(MODUS_ROOT) -I$(MODUS_ROOT)/modus
```

#### C flags (generated from switches)

```makefile
# Module enable flags
ifneq ($(MSHELL_ENABLE),0)
  MODUS_CFLAGS += -DMSHELL_ENABLE=1
endif
ifneq ($(MWAVEFORM_ENABLE),0)
  MODUS_CFLAGS += -DMWAVEFORM_ENABLE=1
endif

# Feature macros
ifeq ($(MODUS_USE_LOG),0)
  MODUS_CFLAGS += -D__NO_USE_LOG__
endif
ifeq ($(MODUS_USE_ASSERT),0)
  MODUS_CFLAGS += -D__NO_USE_ASSERT
endif

# Size overrides (only inject if user changed from default)
ifneq ($(MWAVEFORM_MAX_CHANNELS),16)
  MODUS_CFLAGS += -DMWAVEFORM_MAX_CHANNELS=$(MWAVEFORM_MAX_CHANNELS)
endif
# ... same pattern for other size vars
```

### Framework prerequisites (already done)

The following guards exist in modus.c to support the `MSHELL_ENABLE`/`MWAVEFORM_ENABLE=0` path:

```c
// modus.c — conditional includes
#if MSHELL_ENABLE
#   include "mdebug/mshell.h"
#endif
#if MWAVEFORM_ENABLE
#   include "mdebug/mwaveform.h"
#endif

// modus_Run()
#if MSHELL_ENABLE
    mshell_Poll();
#endif

// modus_Clock()
#if MWAVEFORM_ENABLE
    extern void mwaveform_Default_Step_Callback(void);
    mwaveform_Default_Step_Callback();
#endif
```

`mshell.h` provides:
```c
#ifndef MSHELL_ENABLE
#define MSHELL_ENABLE 1
#endif
#if MSHELL_ENABLE == 0
#define MODUS_SHELL_CMD(name, handler, help_str)  /* empty */
#endif
```

MSTORAGE and MBLINFO use init-section registration — no framework guards needed.

### BLM example refactoring

The BLM Makefile will be changed to `include $(MODUS_ROOT)/modus.mk` and replace its
hard-coded C_SOURCES list with `$(MODUS_SRCS)` plus project-specific sources.

For the debug target, the BLM project enables:
```
MSHELL_ENABLE = 1
MWAVEFORM_ENABLE = 1
MODUS_USE_LOG = 1
MODUS_USE_ASSERT = 1
MBLINFO_ENABLE = 1
MSTORAGE_ENABLE = 1
```

The currently-commented `MODUS_DECLARE_OBJECT(mstorage, ...)` in `main.c` will be
uncommented to validate mstorage end-to-end.

## File list

| File | Action |
|------|--------|
| `modus.mk` | **CREATE** — root-level build include |
| `example/blm/makefile` | **REFACTOR** — use `include modus.mk`, split debug/release |
| `example/blm/main.c` | **EDIT** — uncomment `MODUS_DECLARE_OBJECT(mstorage, ...)` |
