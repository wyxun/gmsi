# ==============================================================================
# BLM Bootloader Debugging Script (GDB)
# ==============================================================================
# usage: gdb-multiarch -x debug.gdb build/at32f4/blm.elf
# 
# Pre-requisite:
#   OpenOCD server must be running on localhost:3333
#   Run `make debug-server` in another terminal.
# ==============================================================================

# 1. Connect to OpenOCD GDB Server
target remote :3333

# 2. Reset the chip and halt the CPU
monitor reset halt

# 3. Load the program (optional if already flashed, but useful to ensure sync)
#    Note: 'load' flashes the code to the target. If you just want to debug
#    code already in flash, you can skip this, but ensure symbols match.
load

# 4. Set Breakpoints
# ------------------------------------------------------------------------------
# Break at main() entry
break main

# Example: Break at specific functions
# break led_blink_task
# break blm_port_LedSet
# break gmsi_Run

# Example: Conditional Breakpoint
# Break when chState equals 1
# break led_blink_task if ptThis->chState == 1

# 5. Start Execution
# ------------------------------------------------------------------------------
# Continue until the first breakpoint (main)
continue

# ==============================================================================
# Debugging Commands Cheat Sheet
# ==============================================================================

# --- Control Flow ---
# c (continue)      : Resume program execution
# n (next)          : Execute next line (step over functions)
# s (step)          : Step into function
# fin (finish)      : Run until current function returns
# bt (backtrace)    : Show call stack

# --- Inspection ---
# p <var>           : Print variable value
# p/x <var>         : Print hex value
# p *ptThis         : Print structure content pointed by ptThis
# info locals       : Show local variables
# info args         : Show function arguments
# info registers    : Show CPU registers (r0-r15, xpsr, etc.)

# --- Memory Inspection ---
# x/4w 0x08000000   : Examine 4 words at Flash beginning (Vector Table)
# x/10w $sp         : Examine 10 words at Stack Pointer
# x/s <addr>        : Examine string at address

# --- Watchpoints ---
# watch <var>       : Break when variable is written
# rwatch <addr>     : Break when address is read

# ==============================================================================
# Automated Scripting Example (Uncomment to use)
# ==============================================================================
# define hook-stop
#   # This hook runs every time the target stops (e.g. at breakpoint)
#   echo \n[Stopped] State:\n
#   print/x $pc
#   bt
# end

# ==============================================================================
# Advanced: Peripheral View (requires SVD, viewing raw memory here)
# ==============================================================================
# View RCC CR register (AT32F407: 0x40021000)
# x/1w 0x40021000
