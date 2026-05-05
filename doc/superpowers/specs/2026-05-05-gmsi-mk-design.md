# GMSI.MK — Unified Build Include Design

## Overview

Provide a single `gmsi.mk` at the repository root so external projects can include GMSI via
one line. All optional modules default to OFF (conservative). Projects opt in by setting
`?= 1` variables before or after the include.

## Design

### File location

```
gmsi/
├── gmsi.mk          ← NEW: root-level build include
├── gmsi/
│   ├── gmsi.c ...
│   ├── gdebug/ ...
│   ├── gdi/ ...
│   └── utilities/ ...
```

### Usage pattern (external project Makefile)

```makefile
GMSI_ROOT ?= lib/gmsi
include $(GMSI_ROOT)/gmsi.mk

# Opt-in to needed modules
GBLINFO_ENABLE  = 1
GSTORAGE_ENABLE = 1

C_SOURCES += $(GMSI_SRCS)
C_INCLUDES += $(GMSI_INCLUDES)
CFLAGS += $(GMSI_CFLAGS)
```

### Configuration variables

#### Enable/disable switches (all default 0)

| Variable | Default | Effect when `1` |
|----------|:------:|-----------------|
| `GSHELL_ENABLE` | 0 | Compile gshell + trace + SEGGER_RTT; requires framework guards in gmsi.c |
| `GWAVEFORM_ENABLE` | 0 | Compile gwaveform + gwaveform_protocol; requires framework guards in gmsi.c |
| `GSTORAGE_ENABLE` | 0 | Compile gstorage.c (init-section registration, no framework guards needed) |
| `GBLINFO_ENABLE` | 0 | Compile gblinfo.c (init-section registration, no framework guards needed) |
| `GMSI_USE_LOG` | 0 | Enable `GLOG`/`GLOGF` macros at compile time (`0` → `-D__NO_USE_LOG__`) |
| `GMSI_USE_ASSERT` | 0 | Enable `GMSI_ASSERT` macro (`0` → `-D__NO_USE_ASSERT`) |

#### Size-tuning overrides (have defaults, optional)

| Variable | Default | Maps to |
|----------|---------|---------|
| `GWAVEFORM_MAX_CHANNELS` | 16 | `-DGWAVEFORM_MAX_CHANNELS=N` |
| `GWAVEFORM_RTT_BUFFER_SIZE` | 1024 | `-DGWAVEFORM_RTT_BUFFER_SIZE=N` |
| `GWAVEFORM_FIFO_DEPTH` | 16 | `-DGWAVEFORM_FIFO_DEPTH=N` |
| `GWAVEFORM_DECIMATION` | 1 | `-DGWAVEFORM_DECIMATION=N` |
| `GLOG_MASK_DEFAULT` | 0x1F | `-DGLOG_MASK_DEFAULT=N` |

Lower-level overrides (`GSHELL_LINE_SIZE`, `SEGGER_RTT_MAX_NUM_UP_BUFFERS`, etc.) stay in
`userconfig.h` to keep the `.mk` lean.

### Output variables

#### Source file groups

```
GMSI_SRCS_CORE       ← gmsi.c gbase.c gcoroutine.c glog.c
                        utilities/list.c util_queue.c gringbuf.c
GMSI_SRCS_DEBUG      ← gshell.c trace.c trace_fmt.c util_debug.c SEGGER_RTT.c
GMSI_SRCS_WAVEFORM   ← gwaveform.c gwaveform_protocol.c
GMSI_SRCS_STORAGE    ← gstorage.c
GMSI_SRCS_BLINFO     ← gblinfo.c
```

#### Aggregated by switches

```makefile
GMSI_SRCS = $(GMSI_SRCS_CORE)
ifneq ($(GSHELL_ENABLE),0)
  GMSI_SRCS += $(GMSI_SRCS_DEBUG)
endif
ifneq ($(GWAVEFORM_ENABLE),0)
  GMSI_SRCS += $(GMSI_SRCS_WAVEFORM)
endif
ifneq ($(GSTORAGE_ENABLE),0)
  GMSI_SRCS += $(GMSI_SRCS_STORAGE)
endif
ifneq ($(GBLINFO_ENABLE),0)
  GMSI_SRCS += $(GMSI_SRCS_BLINFO)
endif
```

#### Include paths (fixed)

```makefile
GMSI_INCLUDES = -I$(GMSI_ROOT) -I$(GMSI_ROOT)/gmsi
```

#### C flags (generated from switches)

```makefile
# Module enable flags
ifneq ($(GSHELL_ENABLE),0)
  GMSI_CFLAGS += -DGSHELL_ENABLE=1
endif
ifneq ($(GWAVEFORM_ENABLE),0)
  GMSI_CFLAGS += -DGWAVEFORM_ENABLE=1
endif

# Feature macros
ifeq ($(GMSI_USE_LOG),0)
  GMSI_CFLAGS += -D__NO_USE_LOG__
endif
ifeq ($(GMSI_USE_ASSERT),0)
  GMSI_CFLAGS += -D__NO_USE_ASSERT
endif

# Size overrides (only inject if user changed from default)
ifneq ($(GWAVEFORM_MAX_CHANNELS),16)
  GMSI_CFLAGS += -DGWAVEFORM_MAX_CHANNELS=$(GWAVEFORM_MAX_CHANNELS)
endif
# ... same pattern for other size vars
```

### Framework prerequisites (already done)

The following guards exist in gmsi.c to support the `GSHELL_ENABLE`/`GWAVEFORM_ENABLE=0` path:

```c
// gmsi.c — conditional includes
#if GSHELL_ENABLE
#   include "gdebug/gshell.h"
#endif
#if GWAVEFORM_ENABLE
#   include "gdebug/gwaveform.h"
#endif

// gmsi_Run()
#if GSHELL_ENABLE
    gshell_Poll();
#endif

// gmsi_Clock()
#if GWAVEFORM_ENABLE
    extern void gwaveform_Default_Step_Callback(void);
    gwaveform_Default_Step_Callback();
#endif
```

`gshell.h` provides:
```c
#ifndef GSHELL_ENABLE
#define GSHELL_ENABLE 1
#endif
#if GSHELL_ENABLE == 0
#define GMSI_SHELL_CMD(name, handler, help_str)  /* empty */
#endif
```

GSTORAGE and GBLINFO use init-section registration — no framework guards needed.

### BLM example refactoring

The BLM Makefile will be changed to `include $(GMSI_ROOT)/gmsi.mk` and replace its
hard-coded C_SOURCES list with `$(GMSI_SRCS)` plus project-specific sources.

For the debug target, the BLM project enables:
```
GSHELL_ENABLE = 1
GWAVEFORM_ENABLE = 1
GMSI_USE_LOG = 1
GMSI_USE_ASSERT = 1
GBLINFO_ENABLE = 1
GSTORAGE_ENABLE = 1
```

The currently-commented `GMSI_DECLARE_OBJECT(gstorage, ...)` in `main.c` will be
uncommented to validate gstorage end-to-end.

## File list

| File | Action |
|------|--------|
| `gmsi.mk` | **CREATE** — root-level build include |
| `example/blm/makefile` | **REFACTOR** — use `include gmsi.mk`, split debug/release |
| `example/blm/main.c` | **EDIT** — uncomment `GMSI_DECLARE_OBJECT(gstorage, ...)` |
