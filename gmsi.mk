# GMSI — Bare-metal Embedded Framework Build Include
#
# Usage in external project Makefile:
#   GMSI_ROOT ?= lib/gmsi
#   include $(GMSI_ROOT)/gmsi.mk
#
#   # Opt-in to needed modules (all default 0)
#   GBLINFO_ENABLE   = 1
#   GSTORAGE_ENABLE  = 1
#
#   C_SOURCES += $(GMSI_SRCS)
#   C_INCLUDES += $(GMSI_INCLUDES)
#   CFLAGS += $(GMSI_CFLAGS)

# ---------------------------------------------------------------------------
# Enable/disable switches — conservative defaults (all OFF)
# ---------------------------------------------------------------------------

# gshell + trace + SEGGER_RTT
GSHELL_ENABLE    ?= 0

# gwaveform + gwaveform_protocol
GWAVEFORM_ENABLE ?= 0

# gstorage
GSTORAGE_ENABLE  ?= 0

# gblinfo
GBLINFO_ENABLE   ?= 0

# GLOG / GLOGF macros
GMSI_USE_LOG     ?= 0

# GMSI_ASSERT macro
GMSI_USE_ASSERT  ?= 0

# ---------------------------------------------------------------------------
# Size-tuning overrides (defaults; override with ?= or = in your Makefile)
# ---------------------------------------------------------------------------
GWAVEFORM_MAX_CHANNELS      ?= 16
GWAVEFORM_RTT_BUFFER_SIZE   ?= 1024
GWAVEFORM_FIFO_DEPTH        ?= 16
GWAVEFORM_DECIMATION        ?= 1
GLOG_MASK_DEFAULT            ?= 0x1F

# ---------------------------------------------------------------------------
# GMSI_ROOT — path to this file's directory (auto-detected if not set)
# ---------------------------------------------------------------------------
GMSI_ROOT ?= $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

# ---------------------------------------------------------------------------
# Source files — core (always compiled)
# ---------------------------------------------------------------------------
GMSI_SRCS_CORE = \
    $(GMSI_ROOT)/gmsi/gmsi.c \
    $(GMSI_ROOT)/gmsi/gbase.c \
    $(GMSI_ROOT)/gmsi/gcoroutine.c \
    $(GMSI_ROOT)/gmsi/glog.c \
    $(GMSI_ROOT)/gmsi/utilities/list.c \
    $(GMSI_ROOT)/gmsi/utilities/util_queue.c \
    $(GMSI_ROOT)/gmsi/utilities/gringbuf.c

# ---------------------------------------------------------------------------
# Source files — debug (gshell + trace + SEGGER_RTT)
# ---------------------------------------------------------------------------
GMSI_SRCS_DEBUG = \
    $(GMSI_ROOT)/gmsi/gdebug/gshell.c \
    $(GMSI_ROOT)/gmsi/gdebug/trace.c \
    $(GMSI_ROOT)/gmsi/gdebug/trace_fmt.c \
    $(GMSI_ROOT)/gmsi/gdebug/util_debug.c \
    $(GMSI_ROOT)/gmsi/gdebug/segger_rtt/SEGGER_RTT.c

# ---------------------------------------------------------------------------
# Source files — waveform
# ---------------------------------------------------------------------------
GMSI_SRCS_WAVEFORM = \
    $(GMSI_ROOT)/gmsi/gdebug/gwaveform.c \
    $(GMSI_ROOT)/gmsi/gdebug/gwaveform_protocol.c

# ---------------------------------------------------------------------------
# Source files — storage / blinfo
# ---------------------------------------------------------------------------
GMSI_SRCS_STORAGE = \
    $(GMSI_ROOT)/gmsi/gstorage.c

GMSI_SRCS_BLINFO = \
    $(GMSI_ROOT)/gmsi/gblinfo.c

# ---------------------------------------------------------------------------
# Aggregate source list by switches
# ---------------------------------------------------------------------------
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

# ---------------------------------------------------------------------------
# Include paths (fixed)
# ---------------------------------------------------------------------------
GMSI_INCLUDES = \
    -I$(GMSI_ROOT) \
    -I$(GMSI_ROOT)/gmsi \
    -I$(GMSI_ROOT)/gmsi/gdi

# ---------------------------------------------------------------------------
# Compile flags generated from switches
# ---------------------------------------------------------------------------

# Module enable flags (always emit as 0 or 1)
ifneq ($(GSHELL_ENABLE),0)
GMSI_CFLAGS += -DGSHELL_ENABLE=1
else
GMSI_CFLAGS += -DGSHELL_ENABLE=0
endif

ifneq ($(GWAVEFORM_ENABLE),0)
GMSI_CFLAGS += -DGWAVEFORM_ENABLE=1
else
GMSI_CFLAGS += -DGWAVEFORM_ENABLE=0
endif

# Log / assert (0 = disabled via negative define)
ifeq ($(GMSI_USE_LOG),0)
GMSI_CFLAGS += -D__NO_USE_LOG__
endif
ifeq ($(GMSI_USE_ASSERT),0)
GMSI_CFLAGS += -D__NO_USE_ASSERT
endif

# Size overrides (only emit if changed from default)
ifneq ($(GWAVEFORM_MAX_CHANNELS),16)
GMSI_CFLAGS += -DGWAVEFORM_MAX_CHANNELS=$(GWAVEFORM_MAX_CHANNELS)
endif
ifneq ($(GWAVEFORM_RTT_BUFFER_SIZE),1024)
GMSI_CFLAGS += -DGWAVEFORM_RTT_BUFFER_SIZE=$(GWAVEFORM_RTT_BUFFER_SIZE)
endif
ifneq ($(GWAVEFORM_FIFO_DEPTH),16)
GMSI_CFLAGS += -DGWAVEFORM_FIFO_DEPTH=$(GWAVEFORM_FIFO_DEPTH)
endif
ifneq ($(GWAVEFORM_DECIMATION),1)
GMSI_CFLAGS += -DGWAVEFORM_DECIMATION=$(GWAVEFORM_DECIMATION)
endif
ifneq ($(GLOG_MASK_DEFAULT),0x1F)
GMSI_CFLAGS += -DGLOG_MASK_DEFAULT=$(GLOG_MASK_DEFAULT)
endif
